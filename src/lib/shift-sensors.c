/* Eban Ebssa and Hannah Clay
 * CS107E
 * 16 March 2022
 *
 * sensors.c...
*/ 

#include "sensors.h"
#include "gpio.h"                                                                                   
#include "gpio_extra.h"                                                                             
#include "uart.h"                                                                                   
#include "timer.h"                                                                                  
#include "printf.h"                                                                                 
#include "strings.h"                                                                                
#include <stdbool.h>                                                                                
                                                                                                     
static int pload_pin = 21;  // yellow SH/LD is at GPIO pin 21                                       
static int clockinhibit_pin = 20;  // orange CLK INH is at GPIO pin 20                              
static int clock_pin = 16;  // brown (or green) wired CLK is at GPIO pin 16                         
static int data_pin = 23;  // blue wired Qh at GPIO pin 23                                          
                                                                                                     
#define num_regs 8                                                                                  

// one byte per shift register (reg 0 thru 7) (could try to use a long long too)                    
static unsigned char state[num_regs] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};            
static unsigned char prev_state[num_regs] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/* find_magnet...
 */
int find_magnet(void) {
    // returns the index of which new magnet has been sensed
    unsigned char state_copies[num_regs];
    unsigned char prev_copies[num_regs];
    memcpy(state_copies, state, num_regs);
    memcpy(prev_copies, prev_state, num_regs);

    for (int r = 0; r < num_regs; r++) {
        unsigned char state_copy = state_copies[r];
        unsigned char prev_copy = prev_copies[r];
        for (int c = 0; c < num_regs; c++) {
            if ((r == num_regs - 1) && (c == 0)) { }// bug with this reg bit
            else if ((prev_copy & 1) == 0 && (state_copy & 1) == 1) {}
            else if ((prev_copy & 1) == 1 && (state_copy & 1) == 0)
                return (r * num_regs) + c;
            state_copy >>= 1;
            prev_copy >>= 1;
        }
    }
    return -1;
}

/* sleep delays for one milisecond
 */
void sleep(void) {
    timer_delay_ms(1);
}

/* update
 */
void update(void) {                                                                                 
    memcpy(prev_state, state, num_regs);                                                            
                                                                                                    
    gpio_write(pload_pin, 0);  // set to low to read data inputs                                    
    sleep();                                                                                        
    gpio_write(pload_pin, 1);  // set to high to send data to Qh                                    
                                                                                                    
     // read all bits and bit shift to build the current state                                       
     for (int r = 0; r < num_regs; r++) {                                                            
        unsigned char new_state = 0x00;                                                             
        for (int c = 0; c < num_regs; c++) {                                                        
            new_state |= gpio_read(data_pin) << (num_regs - 1 - c);                                 
            gpio_write(clock_pin, 0);                                                               
            gpio_write(clock_pin, 1);  // set up for the next pin                                   
        }                                                                                           
        state[r] = new_state;                                                                       
    }                                                                                               
}                                                                                                   
   
/* did_state_change
 */
bool did_state_change(void) {                                                                       
    for (int i = 0; i < num_regs; i++)                                                              
        if (state[i] != prev_state[i])  // mismatch means yes you did change                        
            return true;                                                                            
    return false;                                                                                   
}                                                                                                   

/* setup
 */
void setup(void) {                                                                                  
    gpio_init();                                                                                    
    uart_init();                                                                                    
                                                                                                    
    gpio_set_output(pload_pin);                                                                     
    gpio_set_output(clockinhibit_pin);                                                              
    gpio_set_output(clock_pin);                                                                     
    gpio_set_input(data_pin);                                                                       
    gpio_set_pullup(data_pin);                                                                      

    timer_delay_ms(500);                                                                            
}                                                                                                   

