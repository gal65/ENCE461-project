/** @file   target.h
    @author M. P. Hayes, UCECE
    @date   12 February 2018
    @brief 
*/
#ifndef TARGET_H
#define TARGET_H

#include "mat91lib.h"

/* This is for the carhat (chart) board configured as a racer!  */

/* System clocks  */
#define F_XTAL 12.00e6
#define MCU_PLL_MUL 16
#define MCU_PLL_DIV 1

#define MCU_USB_DIV 2
/* 192 MHz  */
#define F_PLL (F_XTAL / MCU_PLL_DIV * MCU_PLL_MUL)
/* 96 MHz  */
#define F_CPU (F_PLL / 2)

/* TWI  */
#define TWI_TIMEOUT_US_DEFAULT 10000

/* USB  */
//#define USB_VBUS_PIO PA5_PIO
#define USB_CURRENT_MA 500

/* LEDs  */
#define LED1_PIO PA1_PIO
#define LED2_PIO PA0_PIO

/* Radio  */
#define RADIO_CS_PIO PA11_PIO
#define RADIO_CE_PIO PA23_PIO
#define RADIO_IRQ_PIO PA15_PIO

#define LEDTAPE_PIO PB0_PIO

//AIN1
#define LEFT_MOTOR_PWM_PIO PA20_PIO
//AIN2
#define LEFT_MOTOR_DIRECTION_PIO PA19_PIO
//BIN1
#define RIGHT_MOTOR_PWM_PIO PA16_PIO
//BIN2
#define RIGHT_MOTOR_DIRECTION_PIO PA17_PIO
//nSLEEP
#define nSLP_PIO PA29_PIO

#define SERVO_PIO PA24_PIO

//define bumper pin
#define BUMPER_DETECT PA30_PIO

//radio select
#define TOP_SW PB14_PIO
#define BOT_SW PB13_PIO

#endif /* TARGET_H  */
