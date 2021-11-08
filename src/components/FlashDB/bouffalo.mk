#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_ADD_INCLUDEDIRS += ./inc ./port/fal/inc

## not be exported to project level
COMPONENT_PRIV_INCLUDEDIRS :=

## This component's src


#COMPONENT_OBJS := $(patsubst %.c,%.o, $(COMPONENT_SRCS))

COMPONENT_SRCDIRS := ./src ./port/fal/src