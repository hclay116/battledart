/* 
 * Date: Tuesday 3/08
 * Goal: Use multiple Hall Effect magnet sensors with interrupts
 *  
 * What it looks like on the breadboard when looking at the flat side of the sensor:
 * - left side goes to GPIO pin 16 (sensor 1) or pin 20 (sensor 2)
 * - middle goes to ground
 * - right goes to 5V
 */

#include "gpio.h"
#include "gpio_extra.h"
#include "uart.h"
#include "printf.h"
#include "gpio_interrupts.h"
#include "interrupts.h"


static int all_pins[2];
static size_t num_pins; 
static unsigned int state = 0b11;
static unsigned int prev_state = 0b00;

// if the value is 1 it's out of range, not in range
// based on the prev and cur states, print which sensor states have changed
void print_magnets(void) {
    // print only the updated magnets
    unsigned int state_copy = state;
    unsigned int prev_copy = prev_state;
    unsigned int i = 0;
    while (state_copy > 0  && prev_copy > 0) {
        if ((prev_copy & 1) == 0 && (state_copy & 1) == 1)
            printf("magnet out of range on sensor at pin %d\n", all_pins[i]);
        else if ((prev_copy & 1) == 1 && (state_copy & 1) == 0)
            printf("magnet detected on sensor at pin %d\n", all_pins[i]);
        else
            printf("no change at pin %d\n", all_pins[i]);
        state_copy >>= 1;
        prev_copy >>= 1;
        i++;
    }
    printf("\n");
}

// aux_data is the index in all_pins
void handle_detection(unsigned int pc, void *aux_data) {
    prev_state = state;  // keep track of the old state
    // gpio_read(all_pins[0]) is the LSB in state
    unsigned int i = *(unsigned int *) aux_data;
    unsigned int sensor_pin = all_pins[i];
    unsigned int value = gpio_read(sensor_pin);
    printf("i: %d, p: %d, v: %x\n", i, sensor_pin, value);
    state &= !(1 < i);  // clear just that bit
    state |= value < i; // update new state    
    gpio_clear_event(sensor_pin);
}


void main(void) {

    gpio_init();
    uart_init();

    interrupts_init();

    // all_pins = {GPIO_PIN2, GPIO_PIN3};
    all_pins[0] = GPIO_PIN2;
    all_pins[1] = GPIO_PIN3;
    num_pins = sizeof(all_pins) / sizeof(all_pins[0]);

    printf("%x\n", state);
    printf("%x\n", prev_state);

    print_magnets();
    prev_state = state;
    //print_magnets();
    state = 0b10;
    //print_magnets();


/*
    for (int i = 0; i < num_pins; i++) {
        unsigned vout = all_pins[i];
        gpio_set_function(vout, GPIO_FUNC_INPUT);
        gpio_set_pullup(vout);
        gpio_enable_event_detection(vout, GPIO_DETECT_FALLING_EDGE);

        // printf("%d\n", i);

    }

    gpio_interrupts_init();


    for (int i = 0; i < num_pins; i++) {
        unsigned vout = all_pins[i];
        void *aux = &i;
        gpio_interrupts_register_handler(vout, handle_detection, aux);  // *auxdata = i
    }

    gpio_interrupts_enable(); 
    interrupts_global_enable();

    while (1) {
        if (prev_state != state) {
            //printf("state: %x, prev: %x\n", state, prev_state);
            print_magnets();
        }
    }
*/

}


