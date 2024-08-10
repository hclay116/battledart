#ifndef SENSORS_H
#define SENSORS_H

/* Eban Ebssa and Hannah Clay
 * CS1O7E
 * 16 March 2022
 *
 * shift-sensors.h ...
 */

#include <stdbool.h>

/* find_magnet
 *
 * @return...
 */
int find_magnet(void);

/* sleep...
 */
void sleep(void);

/* update...
 */
void update(void);

/* did_state_change
 *
 * @return...
 */
bool did_state_change(void);

/* setup...
 */
void setup(void);

#endif
