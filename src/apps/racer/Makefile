PERIPHERALS = pwm pit adc
DRIVERS = led pacer usb_serial rf ledtape

export BOARD = racer

TARGET = main.bin
SRC = $(wildcard *.c)
OPT = -O3

INCLUDES += -I../common
SRC += ../common/common.c

include ../../boards/board.mk
