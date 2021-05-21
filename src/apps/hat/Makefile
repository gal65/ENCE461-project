PERIPHERALS = adc
DRIVERS = rf pacer
TARGET = hat.bin

SRC = hat.c
OPT = -O2

ifneq ($(BOARD), hat)
$(error BOARD should be set to hat!)
endif

include ../../boards/board.mk
