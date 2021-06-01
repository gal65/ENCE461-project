PERIPHERALS = pit pwm adc
DRIVERS = led pacer usb_serial rf ledtape
TARGET = racer.bin

SRC = racer.c
INCLUDES += -I../common
SRC += ../common/common.c
OPT = -O2

ifneq ($(BOARD), racer)
$(error BOARD should be set to racer!)
endif

include ../../boards/board.mk
