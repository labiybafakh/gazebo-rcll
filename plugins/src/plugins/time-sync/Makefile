#*****************************************************************************
#               Makefile Build System for Fawkes: Gazebo plugin for timesync
#
#   Created on Mon Mar 02 15:52:44 2015
#   Copyright (C) 2015 by Frederik Zwilling
#
##*****************************************************************************
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#*****************************************************************************

BASEDIR = ../../..

include $(BASEDIR)/etc/buildsys/config.mk
include $(BUILDSYSDIR)/gazebo.mk
include $(BUILDSYSDIR)/protobuf.mk

LIBS_gazebo_libtimesync =	gazsim_msgs configurable
OBJS_gazebo_libtimesync =	time_sync.o

OBJS_all    = $(OBJS_gazebo_libtimesync)

ifeq ($(HAVE_GAZEBO)$(HAVE_PROTOBUF),11)
  CFLAGS  += $(CFLAGS_GAZEBO) $(CFLAGS_PROTOBUF)
  LDFLAGS += $(LDFLAGS_GAZEBO) $(LDFLAGS_PROTOBUF) -lm $(call boost-libs-ldflags,system) -lboost_system

  LIBS_all = $(LIBDIR)/gazebo/libtimesync.so
else
  ifneq ($(HAVE_GAZEBO),1)
    WARN_TARGETS += warning_gazebo
  endif
  ifneq ($(HAVE_PROTOBUF),1)
    WARN_TARGETS += warning_protobuf
  endif
endif

ifeq ($(OBJSSUBMAKE),1)
all:	$(WARN_TARGETS)
.PHONY:	warning_gazebo warning_protobuf
warning_gazebo:
	$(SILENT)echo -e "$(INDENT_PRINT)--> $(TRED)Omitting gazsim-robotino Plugin$(TNORMAL) " \
		"(Gazebo Simulator not found)"
warning_protobuf:
	$(SILENT)echo -e "$(INDENT_PRINT)--> $(TRED)Omitting gazsim-robotino Plugin$(TNORMAL) " \
		"(protobuf[-devel] not installed)"
endif

include $(BUILDSYSDIR)/base.mk
