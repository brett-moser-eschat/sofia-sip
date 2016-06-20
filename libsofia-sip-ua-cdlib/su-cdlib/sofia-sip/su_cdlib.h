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

#ifndef SU_CDLIB_SOURCE_H
#define SU_CDLIB_SOURCE_H

/**
 * @file su_cdlib.h
 *
 * @author Pekka Pessi <Pekka.Pessi@nokia.com>
 * @author Kai Vehmanen <first.surname@nokia.com>
 */

#ifndef SU_WAIT_H
#include <sofia-sip/su_wait.h>
#endif

SOFIA_BEGIN_DECLS

SOFIAPUBFUN su_root_t *su_cdlib_root_create(su_root_magic_t *) __attribute__((__malloc__));
SOFIAPUBFUN void su_cdlib_prefer(void);

SOFIA_END_DECLS

#endif /* !defined SU_CDLIB_SOURCE_H */
