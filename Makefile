TARGET = ctrl

OBJS = pspctrl.o

PSP_FW_VERSION = 620
BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR =
CFLAGS = -O2 -G0 -Wall #-g -DDEBUG
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR =
LDFLAGS = -mno-crt0 -nostartfiles
LIBS = -nodefaultlibs -nostdlib

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak