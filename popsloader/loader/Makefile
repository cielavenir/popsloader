TARGET = popsloader
OBJS = main.o ui.o ../common/conf.o

#PRO_HOME = d:/PSP/635PRO

CFLAGS = -O2 -G0 -Wall -I../common -I$(PRO_HOME)/include -I$(PRO_HOME)/Common
DEBUG=0
ifeq ($(DEBUG), 1)
CFLAGS += -DDEBUG=1
OBJS += $(PRO_HOME)/Common/utils.o $(PRO_HOME)/Common/printk.o
else
OBJS += ../common/utils.o ../common/vsnprintf.o
endif

LDFLAGS = -mno-crt0 -nostartfiles -L. -L $(PRO_HOME)/libs
LIBS = -lpspsystemctrl_kernel -lpspkubridge -lpsppower_driver
PSP_FW_VERSION=620
PRX_EXPORTS = exports.exp

BUILD_PRX=1
USE_KERNEL_LIBS=1
USE_KERNEL_LIBC=1

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
