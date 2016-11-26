#
# phoxygen -- PHP documentation tool. (C) 2015--2016 Baubadil GmbH.
# 
# phoxygen is free software; you can redistribute it and/or modify it under the terms of the GNU
# General Public License as published by the Free Software Foundation, in version 2 as it comes
# in the "LICENSE" file of the phoxygen main distribution. This program is distributed in the hope
# that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the LICENSE file for more details.
#
# This makefile is for "kmk", the make utility of kBuild (http://trac.netlabs.org/kbuild). GNU make
# probably won't like it.
#
# See README.md for instructions.

SUB_DEPTH = .
include $(KBUILD_PATH)/subheader.kmk

#
# Target lists.
#
include $(PATH_CURRENT)/src/xwp/Makefile.kmk

PROGRAMS += phoxygen
phoxygen_TEMPLATE = EXE
phoxygen_SOURCES = 
phoxygen_LIBS = $(PATH_STAGE_LIB)/xwp.a libpcre

include $(PATH_CURRENT)/src/phoxygen/Makefile.kmk

include $(FILE_KBUILD_SUB_FOOTER)
