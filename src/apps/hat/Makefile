PERIPHERALS = adc dac pwm
DRIVERS = rf pacer usb_serial mpu ledtape
TARGET = main.bin

SRC = $(wildcard *.c)
OPT = -O3

INCLUDES += -I../../mmculib
# INCLUDES += -I../../mat91lib
INCLUDES += -I../common
SRC += ../common/common.c
SRC += ../../mmculib/mmelody.c
SRC += ../../mmculib/ticker.c
SRC += ../../mmculib/tweeter.c

ifneq ($(BOARD), hat)
$(error BOARD should be set to hat!)
endif

include ../../boards/board.mk
