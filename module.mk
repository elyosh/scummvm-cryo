MODULE := engines/cryo
 
MODULE_OBJS := \
	console.o \
	detection.o \
	cryo.o \
	font.o \
	music.o \
	resource.o \
	sentences.o \
	sprite.o
	
MODULE_DIRS += \
	engines/cryo
 
# This module can be built as a plugin
ifeq ($(ENABLE_CRYO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk