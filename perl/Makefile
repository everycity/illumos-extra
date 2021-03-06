#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#
# Copyright (c) 2012, Joyent, Inc.  All rights reserved.
#

VER =	perl-5.12.3

include ../Makefile.defs

UNPACK_SENTINEL =	Configure

LF_CPPFLAGS = \
	-D_LARGEFILE_SOURCE \
	-D_FILE_OFFSET_BITS=64

BASIC_CPPFLAGS = \
	-fno-strict-aliasing \
	-pipe \
	-fstack-protector

CPPFLAGS += \
	$(BASIC_CPPFLAGS) \
	$(LF_CPPFLAGS) \
	-DPERL_USE_SAFE_PUTENV

LDFLAGS += \
	-fstack-protector

SHARED_LDFLAGS = \
	-G \
	$(LDFLAGS)

PATCHES += \
	dtrace.patch

#
# Perl doesn't know how to build native tools for use during the build.
# It instead assumes that it's safe to run target programs.  This is broken
# in several ways.  We partially work around it here by forcing it to pick
# up the new platform libraries as well.  This will break if there has been
# a user/kernel change relative to the build system.  The correct solution
# is for perl to build native copies of anything needed at build time -- as
# for any cross-compilation environment.
#
OS_1828_WAR = LDLIBPTH="LD_LIBRARY_PATH=$(DESTDIR)/usr/lib:$(DESTDIR)/lib"

#
# This is where things get really gross.  There is stuff in ON that is going
# to use the bootstrap perl to build various artifacts (binary and otherwise).
# When it does so, that perl needs to know to look in the *proto* area, not
# the bootstrap proto area, for everything except its own installed files.  So
# all the header and library flags need to point there.  Similarly, when we
# build the perl that will end up in the platform, it needs to go in the proto
# area but its configuration must reflect the installed (platform) paths.
#
# To accommodate both of these, we split the familiar config.sh into the
# perl-build-time config.over and the perl-run-time (consumer build-time)
# Config_heavy.pl.  We further introduce the notion of the "next" proto area;
# that is, where the perl we're building should tell its consumers to look
# for the rest of the system.  Similarly, "this" proto simply means where
# *this* perl will itself be installed.  This system implies knowledge of
# the mechanism used to build bootstrap and ordinary proto areas in
# smartos-live and it will not otherwise work.
#
# Further complicating matters is the need for differences in runtime
# configuration between the bootstrap and delivered perl, giving rise to two
# different Config_heavy.pl templates.
#
# Finally, because the bootstrap perl actually knows that it's going to end
# up in the bootstrap proto area (it has to, since it will be run from there
# and needs to know to look there for pieces of itself), we do NOT use
# DESTDIR when installing it.  We *do* when installing the delivered perl,
# which is never run on the build system.  Thus, INSTALL_DESTDIR is set only
# for the regular build; for the bootstrap build it is empty.
#
# The alternative to this is to run the perl on the build system during the
# build.  However, the platform image on the build machine may be arbitrarily
# old, and doing this would create a platform image (and this also fake subset)
# flag day if and when the perl in this repo is changed.  This is because it is
# necessary for the binary modules in ON to be built against the same perl that
# will actually be installed on the running system.  In this regard they are
# quite similar to other ON consumers of -extra bits.  The only complication is
# that consuming perl as a binary module conventionally requires not only that
# the headers and libraries be present but also that a matching perl be
# available and executable at build time.
#
# I can't believe we've made it this far without any profanity.  Holy fuck!
#
INSTALL_DESTDIR :=	$(shell [[ -z "$(STRAP)" ]] && echo "$(DESTDIR)")
NEXTPROTO :=	$(shell [[ -n "$(STRAP)" ]] && echo "$(DESTDIR:.strap=)")

CONFIG_PL =	Config_heavy.pl$(STRAP)
ROOTBASE =	$(DESTDIR)/usr/perl5/5.12
ROOTCONFIG_PL =	$(ROOTBASE)/lib/i86pc-solaris-64int/Config_heavy.pl

CLEANFILES +=	$(CONFIG_PL)

XFORM.sh = \
	sed -e 's;@@CC@@;$(GCC);g' \
	    -e 's;@@CPPFLAGS@@;$(CPPFLAGS);g' \
	    -e 's;@@CFLAGS@@;$(CFLAGS);g' \
	    -e 's;@@BASIC_CPPFLAGS@@;$(BASIC_CPPFLAGS);g' \
	    -e 's;@@LF_CPPFLAGS@@;$(LF_CPPFLAGS);g' \
	    -e 's;@@LDFLAGS@@;$(LDFLAGS);g' \
	    -e 's;@@LIBS@@;$(LIBS);g' \
	    -e 's;@@GMAKE@@;$(MAKE);g' \
	    -e 's;@@THISPROTO@@;$(STRAP:strap=$(DESTDIR));g' \
	    -e 's;@@NEXTPROTO@@;$(NEXTPROTO);g' \
	    -e 's;@@SHARED_LDFLAGS@@;$(SHARED_LDFLAGS);g' < $< > $@

include ../Makefile.targ

all: $(VER.32)/perldtrace.h
	(cd $(VER.32); LC_ALL=C $(MAKE) $(OS_1828_WAR))

test: all
	(cd $(VER.32); $(MAKE) $(OS_1828_WAR) test)

install: all $(CONFIG_PL)
	(cd $(VER.32);\
	    rm -f $(DESTDIR)/usr/perl5/5.12/lib/i86pc-solaris-64int/.packlist;\
	    gmake DESTDIR=$(INSTALL_DESTDIR) $(OS_1828_WAR) install)
	rm -f $(DESTDIR)/usr/perl5/5.12/lib/i86pc-solaris-64int/Config_heavy.pl
	cp $(CONFIG_PL) $(ROOTCONFIG_PL)

$(VER.32)/config.over: config.over.in $(VER.32)/$(UNPACK_SENTINEL) Makefile
	$(XFORM.sh)

$(CONFIG_PL): $(CONFIG_PL).in Makefile
	$(XFORM.sh)

$(VER.32)/cflags: $(VER.32)/config.over $(VER.32)/Configure
	(cd $(VER.32) && \
	    ./Configure -des -Dcc=$(GCC) -Duse64bitint)

$(VER.32)/perldtrace.h: $(VER.32)/cflags
	(cd $(VER.32) && \
	    /usr/sbin/dtrace -h -s perldtrace.d -o perldtrace.h)
