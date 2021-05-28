PERIPHERALS = adc dac
DRIVERS = rf pacer usb_serial mpu ledtape
TARGET = main.bin

SRC = $(wildcard *.c)
OPT = -O3

INCLUDES += -I../common
SRC += ../common/common.c

ifneq ($(BOARD), hat)
$(error BOARD should be set to hat!)
endif

include ../../boards/board.mk
