PERIPHERALS = adc dac pwm
DRIVERS = rf pacer usb_serial mpu ledtape
TARGET = main.bin

export BOARD = hat

SRC = $(wildcard *.c)
OPT = -O3

INCLUDES += -I../../mmculib
# INCLUDES += -I../../mat91lib
INCLUDES += -I../common
SRC += ../common/common.c
SRC += ../../mmculib/mmelody.c
SRC += ../../mmculib/ticker.c
SRC += ../../mmculib/tweeter.c

include ../../boards/board.mk
