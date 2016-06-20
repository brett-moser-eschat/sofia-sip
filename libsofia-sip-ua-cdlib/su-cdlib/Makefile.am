#
# Makefile.am for su-cdlib module
#
# Copyright (C) 2015 SLA Corporation
# Contact: Tarik Miller <tmiller@slacorp.com>
# Licensed under LGPL. See file COPYING.
#

# ----------------------------------------------------------------------
# Headers

S_BASE = $(top_srcdir)/libsofia-sip-ua
B_BASE = $(top_builddir)/libsofia-sip-ua

INCLUDES =		-I$(S_BASE)/su -I$(B_BASE)/su $(CDLIB_CFLAGS) \
			$(SOFIA_CDLIB_CFLAGS)

# ----------------------------------------------------------------------
# Build targets

noinst_LTLIBRARIES = 	libsu-cdlib.la

check_PROGRAMS = 	su_cdlib_test torture_su_cdlib_timer

# ----------------------------------------------------------------------
# Tests

TESTS = 		su_cdlib_test torture_su_cdlib_timer

# ----------------------------------------------------------------------
# Rules for building the targets

nobase_include_sofia_HEADERS =
if HAVE_CDLIB
nobase_include_sofia_HEADERS += \
			sofia-sip/su_cdlib.h
endif

libsu_cdlib_la_SOURCES = su_cdlib_port.c

libsu_cdlib_la_DEPENDENCIES = \
			../../libsofia-sip-ua/libsofia-sip-ua.la

#LDADD = 		libsu-cdlib.la \
#			../../libsofia-sip-ua/libsofia-sip-ua.la \
#			$(CDLIB_LIBS)
LDADD = 		libsu-cdlib.la \
			../../libsofia-sip-ua/libsofia-sip-ua.la 

# ----------------------------------------------------------------------
# Install and distribution rules

EXTRA_DIST =		Doxyfile su_cdlib.docs

# ----------------------------------------------------------------------
# Automake options

AUTOMAKE_OPTIONS = 	foreign

# ----------------------------------------------------------------------
# Sofia specific rules

include $(top_srcdir)/rules/sofia.am
