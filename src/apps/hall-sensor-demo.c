/* 
 * Date: Wednesday 3/02
 * Goal: Recreate Hall Effect magnet sensor demo code from Pat's lecture on 2/25
 * 
 * Demo Code:
 * https://github.com/cs107e/cs107e.github.io/blob/master/lectures/Sensors/code/hall-effect/hall.c
 * 
 * What it looks like on the breadboard when looking at the flat side of the sensor:
 * - left side goes to GPIO pin 2
 * - middle goes to ground
 * - right goes to 5V
 */

#include "gpio.h"
#include "gpio_extra.h"
#include "uart.h"
#include "printf.h"

void print_magnet(unsigned int vout)
{
    printf(vout ?  "magnet out of range\n" : "magnet detected\n" );
}

void main(void) {
	const unsigned vout= GPIO_PIN2;

    gpio_init();
    uart_init();

  	gpio_set_function(vout, GPIO_FUNC_INPUT);
  	gpio_set_pullup(vout);

    // vout is 1 when the magnet is out of range of the sensor
    print_magnet(1);
	while(1) {
  		while(gpio_read(vout) == 1) {} // wait for low
		print_magnet(0);
  		while(gpio_read(vout) == 0) {} // wait for high
		print_magnet(1);
	}
}

