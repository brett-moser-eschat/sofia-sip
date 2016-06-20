#
# Makefile.am for sofia-sip/libsofia-sip-ua-cdlib
#
# Copyright (C) 2015 SLA Corporation
# Contact: Tarik Miller <tmiller@slacorp.com>
# Licensed under LGPL. See file COPYING.
#

AUTOMAKE_OPTIONS = foreign 1.7

SUBDIRS=su-cdlib

CDLIB_TARGETS=su-cdlib/libsu-cdlib.la

lib_LTLIBRARIES =
if HAVE_CDLIB
lib_LTLIBRARIES += libsofia-sip-ua-cdlib.la
endif

libsofia_sip_ua_cdlib_la_SOURCES =
#libsofia_sip_ua_cdlib_la_LIBADD = \
#	$(top_builddir)/libsofia-sip-ua/libsofia-sip-ua.la \
#	$(CDLIB_TARGETS) $(CDLIB_LIBS)
libsofia_sip_ua_cdlib_la_LIBADD = \
	$(top_builddir)/libsofia-sip-ua/libsofia-sip-ua.la \
	$(CDLIB_TARGETS)

# set the libtool version info version:revision:age for libsofia-sip-ua-cdlib
# - soname to 'libsofia-sip-ua-cdlib.so.(CUR-AGE)'
libsofia_sip_ua_cdlib_la_LDFLAGS = \
	-version-info $(LIBVER_SOFIA_SIP_UA_CDLIB_CUR):$(LIBVER_SOFIA_SIP_UA_CDLIB_REV):$(LIBVER_SOFIA_SIP_UA_CDLIB_AGE)

DOXYGEN = doxygen

EXTRA_DIST = docs/Doxyfile.aliases \
	     docs/Doxyfile.conf \
	     docs/Doxyfile.version

doxygen: built-sources
	@mkdir -p docs docs/html &&\
	for d in $(DIST_SUBDIRS) $(DIST_SUBDIRS); do \
	  test -r $$d/Doxyfile \
	  && pushd $$d > /dev/null \
	  && echo running ${DOXYGEN} in $$d \
          && ${DOXYGEN} \
          && popd > /dev/null ; \
	done
	${top_srcdir}/scripts/hide_emails.sh docs/html

PHONY = doxygen

include $(top_srcdir)/rules/recursive.am

if HAVE_LCOV
include $(top_srcdir)/rules/lcov.am
endif

include $(top_srcdir)/rules/silent.am

.PHONY = $(PHONY)

CLEANFILES = docs/*.doxytags

distclean-local:
	-rm -rf docs/html
