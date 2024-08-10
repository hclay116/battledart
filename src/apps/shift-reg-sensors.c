/* 
 * Testing out 1 74HC165 shift register! For now just with 1 hall effect sensor
 * wired and the rest of the data pins connected to 3.3 V
 * don't forget the pullup resistors!
 * 
 * helpful websites:
 * http://domoticx.com/ic-74hc165-shift-register-input-8-bit/
 * http://domoticx.com/wp-content/uploads/2020/01/SN74HC165-datasheet-Texas-Instruments.pdf
 * helpful code references:
 * https://dronebotworkshop.com/shift-registers/#Arduino_74HC165_Sketch
 * https://github.com/ve3wwg/custom_interfaces_pi/blob/master/hc165/hc165.cpp
 * 
 * notes on pinout:
 * 1 - SH/LD (parallel load) - low reads the data bits, high stops loading
 * 2 - CLK (clock) - rising edge (low to high) sends the next bit
 * 11-14, 3-6 - Data (Qa - Qh)
 * 15 - CLK INH (clock enable aka inhibit) - low makes it behave like clock
 * 10 - SER (Serial Input) - use later for daisy chaining! 
 */

#include "gpio.h"
#include "gpio_extra.h"
#include "uart.h"
#include "timer.h"
#include "printf.h"

static int pload_pin = 21;  // yellow SH/LD is at GPIO pin 21
static int clockinhibit_pin = 20;  // orange CLK INH is at GPIO pin 20
static int clock_pin = 16;  // brown (or green) wired CLK is at GPIO pin 16
static int data_pin = 23;  // blue wired Qh at GPIO pin 23

#define num_regs 2
static int NUM_SENSORS = 8*num_regs;  // 8 * number of shift registers you have
static unsigned int state = 0x00;  // will hold the bit pattern
static unsigned int prev_state = 0x00;  // compare to curr to see if you should update

void print_magnet(unsigned int vout)
{
    printf(vout ?  "magnet out of range\n" : "magnet detected\n" );
}

void test_sensor(const unsigned sensor_pin) {
    // verify that it's not the problem of the Hall Effect sensor
    // similar to Pat's lecture code demo on 2/25
    gpio_init();
    uart_init();
  	gpio_set_function(sensor_pin, GPIO_FUNC_INPUT);
  	gpio_set_pullup(sensor_pin);

    // vout is 1 when the magnet is out of range of the sensor
    print_magnet(1);
	while(1) {
  		while(gpio_read(sensor_pin) == 1) {} // wait for low
		print_magnet(0);
  		while(gpio_read(sensor_pin) == 0) {} // wait for high
		print_magnet(1);
	}
}

void print_magnets(void) {
    // print only when the magnets are updated
    unsigned int state_copy = state;
    unsigned int prev_copy = prev_state;
    unsigned int sensor_count = 0;
    for (int i = 0; i < NUM_SENSORS; i++) {
        int reg_count = (NUM_SENSORS - sensor_count - 1)/8;  // like the row!
        int sen_count = sensor_count % 8;  // like the col!
        if ((prev_copy & 1) == 0 && (state_copy & 1) == 1)
            printf("magnet out of range on reg %d sensor %d\n", reg_count, sen_count);
        else if ((prev_copy & 1) == 1 && (state_copy & 1) == 0)
            printf("magnet detected on reg %d sensor %d\n", reg_count, sen_count);
        else
            printf("no change on reg %d sensor %d\n", reg_count, sen_count);
        state_copy >>= 1;
        prev_copy >>= 1;
        sensor_count++;
    }
    printf("\n");
}

void test_print_magnets(void) {
    // this should alternate the magnet detected/out of range on sensor 1
    while (1) {
        prev_state = state;
        int last_bit = (state & 1);
        state &= 0;
        state |= !last_bit;
        print_magnets();
        timer_delay(1);
    }
}

void sleep(void) {
    timer_delay_ms(1);
}

void update(void) {
    prev_state = state;
    state = 0;

    //gpio_write(clockinhibit_pin, 1);  // disable clocking (not totally needed)
    gpio_write(pload_pin, 0);  // set to low to read data inputs
    sleep();
    gpio_write(pload_pin, 1);  // set to high to send data to Qh
    //gpio_write(clockinhibit_pin, 0);  // enable clocking

    // read all bits and bit shift to build the current state
    for (int i = 0; i < NUM_SENSORS; i++) {
        // the first data pin has already been read
        // Qa is at the LSB in state
        state |= gpio_read(data_pin) << (NUM_SENSORS  - 1 - i);
        gpio_write(clock_pin, 0);
        gpio_write(clock_pin, 1);  // set up for the next pin
    }
}

void test_update(void) {
    // this continuously waits for changing input and prints the changes
    gpio_write(clockinhibit_pin, 0);
    while (1) {
        if (state != prev_state)
            print_magnets();
        update();
    }
}

void test_one_round(void) {
    // I simulated one loop based on the logic diagram
    // https://domoticx.com/wp-content/uploads/2020/01/SN74HC165-datasheet-Texas-Instruments.pdf#page=12
    prev_state = state;
    state = 0;

    gpio_write(pload_pin, 1);
    gpio_write(clockinhibit_pin, 1);
    gpio_write(clock_pin, 0);
    
    sleep();
    gpio_write(clock_pin, 1);
    sleep();
    gpio_write(clock_pin, 0);
    gpio_write(pload_pin, 0);  // data has been loaded now
    sleep();
    gpio_write(clock_pin, 1);
    gpio_write(pload_pin, 1);

    // wait
    sleep();
    gpio_write(clock_pin, 0);
    sleep();
    gpio_write(clock_pin, 1);
    sleep();
    gpio_write(clock_pin, 0);
    sleep();

    // read data
    gpio_write(clock_pin, 1);
    gpio_write(clockinhibit_pin, 0);

    sleep();
    gpio_write(clock_pin, 0);
    state |= gpio_read(data_pin) << 0;
    sleep();

    gpio_write(clock_pin, 1);
    sleep();
    gpio_write(clock_pin, 0);
    state |= gpio_read(data_pin) << 1;
    sleep();
    

    gpio_write(clock_pin, 1);
    state |= gpio_read(data_pin) << 2;
    sleep();

    gpio_write(clock_pin, 0);
    sleep();
    gpio_write(clock_pin, 1);
    state |= gpio_read(data_pin) << 3;
    sleep();

    gpio_write(clock_pin, 0);
    gpio_write(clock_pin, 1);
    state |= gpio_read(data_pin) << 4;
    gpio_write(clock_pin, 0);
    gpio_write(clock_pin, 1);
    state |= gpio_read(data_pin) << 5;
    gpio_write(clock_pin, 0);
    gpio_write(clock_pin, 1);
    state |= gpio_read(data_pin) << 6;
    gpio_write(clock_pin, 0);
    gpio_write(clock_pin, 1);
    state |= gpio_read(data_pin) << 7;

    printf("state: %02x\n", state);
    timer_delay(2);
    //uart_putchar(EOT);
}

void main(void) {
    gpio_init();
    uart_init();
    printf("state: %x\n", state);
    printf("previ: %x\n", prev_state);

    gpio_set_output(pload_pin);
    gpio_set_output(clockinhibit_pin);
    gpio_set_output(clock_pin);
    gpio_set_input(data_pin);
    gpio_set_pullup(data_pin);

    timer_delay(1);

    //test_sensor(GPIO_PIN2);  // works
    //test_print_magnets();  // works
    //for (int i = 0; i < 3; i++)  // works
        //test_one_round();
    test_update();  // works
}

