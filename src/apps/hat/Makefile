PERIPHERALS = adc
DRIVERS = rf pacer usb_serial mpu led
TARGET = hat.bin

SRC = hat.c
OPT = -O2

INCLUDES += -I../common
SRC += ../common/common.c

ifneq ($(BOARD), hat)
$(error BOARD should be set to hat!)
endif

include ../../boards/board.mk
