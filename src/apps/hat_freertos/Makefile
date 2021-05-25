PERIPHERALS = adc
DRIVERS = rf pacer
TARGET = main.bin

SRC = $(wildcard *.c)
SRC += $(wildcard freertos/*.c)
SRC += freertos/port/port.c

CFLAGS += -mfloat-abi=soft
OPT = -O2

INCLUDES += -I./freertos/include
INCLUDES += -I./freertos/port
INCLUDES += -I.

ifneq ($(BOARD), hat)
$(error BOARD should be set to hat!)
endif

include ../../boards/board.mk
