
/* 
 * Testing out 74HC595 shift registers! For now just with 5 LEDs
 * 
 * helpful website:
 * https://lastminuteengineers.com/74hc595-shift-register-arduino-tutorial/
 * 
 * notes on pinout:
 * 14 - SER (Serial Input) - feed bit data to register 
 * 11 - SRCLK (Shift Register Clock) - rising edge, HIGH means shift bits
 * 12 - RCLK (Register Clock / Latch) - final step, HIGH means copy shift 
                                        register to latch register for output
 * 10 - SRCLR (Shift Register Clear) - 5V, LOW means reset all bits
 * 13 - OE (Output Enable) - GND, HIGH means disable output pins
 * 15, 1-7 - QA–QH (Output Enable) - data pins, connected to LEDS
 * 9 - QH’ - outputs bit 7 of register
 */

#include "gpio.h"
#include "uart.h"
#include "timer.h"


static int latch_pin = 20;  // RCLK is at GPIO pin 20
static int clock_pin = 16;  // SRCLK is at GPIO pin 16
static int data_pin = 21;  // SER is at GPIO pin 21

static int NUM_LEDS = 5;  // technically 5
static char led_pattern = 0; // will hold the bit pattern

void shift_out(int datap, int clockp, unsigned int bits) {
    // like the arduino function but for us in RPi 
    // https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/wiring_shift.c
	for (int i = 0; i < NUM_LEDS; i++)  {
		gpio_write(datap, bits & 1);
		bits >>= 1;
		
		gpio_write(clockp, 1);
		gpio_write(clockp, 0);		
	}
}

void update_shift_register(void) {
    // set latch to low, shift in bit pattern, set latch to high
    gpio_write(latch_pin, 0);
    shift_out(data_pin, clock_pin, led_pattern);
    gpio_write(latch_pin, 1);
}

void main(void) {
    gpio_init();
    uart_init();

    gpio_set_output(latch_pin);
    gpio_set_output(clock_pin);
    gpio_set_output(data_pin);

    while (1) {
        led_pattern = 0;
        update_shift_register();
        timer_delay(1);
        for (int i = 0; i < NUM_LEDS; i++)	// Turn all the LEDs ON one by one.
        {
            led_pattern |= (1 << i);
            update_shift_register();
            timer_delay(1);
        }

    }
}
