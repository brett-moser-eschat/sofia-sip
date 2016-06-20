/*
 * This file is part of the Sofia-SIP package
 *
 * Copyright (C) 2015 SLA Corporation.
 *
 * Contact: Tarik Miller <tmiller@slacorp.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

/**
 * @file su_cdlib_port.c
 * @brief Wrapper for CDLib IOReactor.
 *
 * Refs:
 *
 * @author Tarik Miller <tmiller@slacorp.com>.
 *
 * @date Created: Wed Apr  8 15:15:15 2015 tmiller
 *
 */

#include "config.h"

/* CDLib includes */
#include <IOReactorCWrap.h>

/* Sofia includes */
#define su_port_s su_cdlib_port_s

#include "su_port.h"
#include "sofia-sip/su.h"
#include "sofia-sip/su_alloc.h"

/* system includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct su_cdlib_port_s
{
	su_socket_port_t sup_base[1];

	unsigned sup_multishot;
	unsigned sup_registers;
	unsigned sup_index_count;
	unsigned sup_depth;

	struct su_cdlib_register
	{
		void *scr_ioev;
		su_wakeup_f scr_cb;
		su_wakeup_arg_t *scr_arg;
		su_root_t *scr_root;
		int scr_id;
		su_wait_t scr_wait[1];
	} *sup_indices;
};

static su_port_t *su_cdlib_port_create(void) __attribute__((__malloc__));

static void su_cdlib_port_decref(su_port_t *self,
                                 int blocking,
                                 char const *who);
static int su_cdlib_port_register(su_port_t *self,
                                  su_root_t *root,
                                  su_wait_t *wait,
                                  su_wakeup_f callback,
                                  su_wakeup_arg_t *arg,
                                  int priority);
static int su_cdlib_port_unregister(su_port_t *port,
                                    su_root_t *root,
                                    su_wait_t *wait,
                                    su_wakeup_f callback,
                                    su_wakeup_arg_t *arg);
static int su_cdlib_port_deregister(su_port_t *self, int i);
static int su_cdlib_port_unregister_all(su_port_t *self, su_root_t *root);
static int su_cdlib_port_eventmask(su_port_t *self,
                                   int index,
                                   int socket,
                                   int events);
static int su_cdlib_port_multishot(su_port_t *self, int multishot);
static int su_cdlib_port_wait_events(su_port_t *self, su_duration_t tout);
static char const *su_cdlib_port_name(su_port_t const *self);

static void su_cdlib_event(void *ioev, unsigned int revents, void *udata);

su_port_vtable_t const su_cdlib_port_vtable[1] =
{{
	/* su_vtable_size: */ sizeof su_cdlib_port_vtable,
	su_pthread_port_lock,
	su_pthread_port_unlock,
	su_base_port_incref,
	su_cdlib_port_decref,
	su_base_port_gsource,
	su_base_port_send,
	su_cdlib_port_register,
	su_cdlib_port_unregister,
	su_cdlib_port_deregister,
	su_cdlib_port_unregister_all,
	su_cdlib_port_eventmask,
	su_base_port_run,
	su_base_port_break,
	su_base_port_step,
	su_pthread_port_thread,
	su_base_port_add_prepoll,
	su_base_port_remove_prepoll,
	su_base_port_timers,
	su_cdlib_port_multishot,
	su_cdlib_port_wait_events,
	su_base_port_getmsgs,
	su_base_port_getmsgs_from,
	su_cdlib_port_name,
	su_base_port_start_shared,
	su_pthread_port_wait,
	su_pthread_port_execute,
	su_base_port_deferrable,
	su_base_port_max_defer,
	su_socket_port_wakeup,
	su_base_port_is_running,
}};

/*=============== Public function definitions ===============*/

su_root_t *su_cdlib_root_create(su_root_magic_t *magic)
{
	return su_root_create_with_port(magic, su_cdlib_port_create());
}

/*=============== Private function definitions ===============*/

static char const *su_cdlib_port_name(su_port_t const *self)
{
  return "CDLib";
}

static void su_cdlib_port_decref(su_port_t *self, int blocking, char const *who)
{
	(void)su_base_port_decref(self, blocking, who);
}

static void su_cdlib_port_deinit(void *arg)
{
	su_port_t *self = arg;

	SU_DEBUG_9(("%s(%p) called\n", __func__, (void *)self));

	su_socket_port_deinit(self->sup_base);
}

int su_cdlib_port_register(su_port_t *self,
                           su_root_t *root,
                           su_wait_t *wait,
                           su_wakeup_f callback,
                           su_wakeup_arg_t *arg,
                           int priority)
{
	struct su_cdlib_register *indices = self->sup_indices;
	int i;

	assert(su_port_own_thread(self));

	SU_DEBUG_9(("register for event: fd %d mask 0x%04x\n", wait->fd, wait->events));

	if (self->sup_registers >= SU_WAIT_MAX)
	{
		return su_seterrno(ENOMEM);
	}

	// allocate more event registration slots if needed
	// (allocates in blocks of 16)
	if (self->sup_registers >= self->sup_index_count)
	{
		indices = su_realloc(su_port_home(self), indices, (self->sup_index_count + 16) * (sizeof(struct su_cdlib_register)));
		if (!indices)
		{
			return su_seterrno(ENOMEM);
		}

		// make sure the newly created slots are marked as unsued
		for (i = self->sup_index_count; i<(self->sup_index_count + 16); ++i)
		{
			indices[i].scr_ioev = NULL;
		}

		self->sup_indices = indices;
		self->sup_index_count += 16;
	}

	// find free registration slot
	for (i = 0; i<self->sup_index_count; ++i)
	{
		if (!self->sup_indices[i].scr_ioev) break;
	}

	if (i >= self->sup_index_count)
	{
		// no free slots available
		return su_seterrno(ENOMEM);
	}

	self->sup_indices[i].scr_ioev = ioReactor_createEvent(wait->fd);
	if (!self->sup_indices[i].scr_ioev)
	{
		return su_seterrno(ENOMEM);
	}

	ioReactor_setEventCallback(self->sup_indices[i].scr_ioev, su_cdlib_event, &self->sup_indices[i]);

	self->sup_indices[i].scr_cb = callback;
	self->sup_indices[i].scr_arg = arg;
	self->sup_indices[i].scr_root = root;
	self->sup_indices[i].scr_wait[0] = *wait;
	// note: index must be > 0 because su framework treats value 0 as an error, so we
	// add one here and will remember to subtract one when the value is used later
	self->sup_indices[i].scr_id = i + 1;

	ioReactor_registerEvent(self->sup_indices[i].scr_ioev, wait->events);
	++self->sup_registers;

	return self->sup_indices[i].scr_id;
}

int su_cdlib_port_unregister(su_port_t *self,
                             su_root_t *root,
                             su_wait_t *wait,
                             su_wakeup_f callback,
                             su_wakeup_arg_t *arg)
{
	int rc = -1;
	int i;

	assert(self);
	assert(su_port_own_thread(self));

	// find the event slot
	for (i=0; i<self->sup_index_count; ++i)
	{
		if (self->sup_indices[i].scr_ioev &&
		    SU_WAIT_CMP(wait[0], self->sup_indices[i].scr_wait[0]) == 0)
		{
			ioReactor_unregisterEvent(self->sup_indices[i].scr_ioev);
			ioReactor_destroyEvent(self->sup_indices[i].scr_ioev);
			self->sup_indices[i].scr_ioev = NULL;
			if (self->sup_registers > 0)
			{
				// todo: should we realloc to a smaller size if reg count drops below
				// some threshold?
				--self->sup_registers;
			}
			rc = i+1; // return slot index +1 (su framework thinks 0 is an error)
			break;
		}
	}

	if (rc < 0)
	{
		su_seterrno(ENOENT);
	}
	return rc;
}

int su_cdlib_port_deregister(su_port_t *self, int i)
{
	int rc = -1, I;
	
	if (i <= 0 || i > (self->sup_index_count+1))
	{
		su_seterrno(EBADF);
		return rc;
	}

	I = i - 1; // one based to zero based conversion
	if (!self->sup_indices[I].scr_ioev)
	{
		su_seterrno(EBADF);
		return rc;
	}

	ioReactor_unregisterEvent(self->sup_indices[I].scr_ioev);
	ioReactor_destroyEvent(self->sup_indices[I].scr_ioev);
	self->sup_indices[I].scr_ioev = NULL;
	su_wait_destroy(self->sup_indices[I].scr_wait);
	if (self->sup_registers > 0)
	{
		--self->sup_registers;
	}
	return i;
}

int su_cdlib_port_unregister_all(su_port_t *self, su_root_t *root)
{
	assert(self);
	assert(root);
	assert(su_port_own_thread(self));

	int unreg_count = 0;
	int i;

	for (i = 0; i <= self->sup_index_count; ++i)
	{
		if (self->sup_indices[i].scr_root != root) continue;

		ioReactor_unregisterEvent(self->sup_indices[i].scr_ioev);
		ioReactor_destroyEvent(self->sup_indices[i].scr_ioev);
		self->sup_indices[i].scr_ioev = NULL;
		if (self->sup_registers > 0)
		{
			--self->sup_registers;
		}
		++unreg_count;
	}
	return unreg_count;
}

int su_cdlib_port_eventmask(su_port_t *self, int index, int socket, int events)
{
	// TODO
	return -1;
}

int su_cdlib_port_multishot(su_port_t *self, int multishot)
{
	assert(self);
	self->sup_multishot = multishot;
	return multishot;
}

int su_cdlib_port_wait_events(su_port_t *self, su_duration_t tout)
{
	assert(self);
	
	int maxEvents = 16;
	int eventsProcessed = 0;

	if (self->sup_multishot = 0)
	{
		maxEvents = 1;
	}

	if (self->sup_depth == 0)
	{
		++self->sup_depth;
		ioReactor_setTimeout((unsigned int) tout);
		eventsProcessed = ioReactor_handleEvents(maxEvents);
		--self->sup_depth;
	}
	else
	{
		usleep(tout * 1000);
	} 
	return eventsProcessed;
}

static su_port_t *su_cdlib_port_create(void)
{
	su_port_t *self;

	self = su_home_new(sizeof *self);
	if (!self)
	{
		return NULL;
	}

	SU_DEBUG_9(("%s(%p): cdlib_create() => : %s\n",
	   __func__, self, "OK"));
	if (su_home_destructor(su_port_home(self), su_cdlib_port_deinit) < 0)
	{
		su_home_unref(su_port_home(self));
		return NULL;
	}

	self->sup_multishot = 1;
	self->sup_registers = 0;
	self->sup_index_count = 0;
	self->sup_depth = 0;

	if (su_socket_port_init(self->sup_base, su_cdlib_port_vtable) < 0)
	{
		su_home_unref(su_port_home(self));
		return NULL;
	}

	self->sup_indices = su_zalloc(su_port_home(self), 16 * sizeof(struct su_cdlib_register));
	if (!self->sup_indices)
	{
		su_home_unref(su_port_home(self));
		return NULL;
	}
	self->sup_index_count = 16;

	return self;
}

int su_cdlib_clone_start(su_root_t *parent,
                         su_clone_r return_clone,
                         su_root_magic_t *magic,
                         su_root_init_f init,
                         su_root_deinit_f deinit)
{
	return su_pthreaded_port_start(su_cdlib_port_create,
	                               parent,
 	                               return_clone,
	                               magic,
	                               init,
	                               deinit);
}

void su_cdlib_event(void *ioev, unsigned int revents, void *udata)
{
	assert(ioev);
	assert(udata);

	struct su_cdlib_register *scr = (struct su_cdlib_register *)udata;
	su_root_magic_t *magic = scr->scr_root ? su_root_magic(scr->scr_root) : NULL;
	scr->scr_wait->revents = revents;

	scr->scr_cb(magic, scr->scr_wait, scr->scr_arg);
}

void su_cdlib_prefer(void)
{
	su_port_prefer(su_cdlib_port_create, NULL);
}

