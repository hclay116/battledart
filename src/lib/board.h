#ifndef BOARD_H
#define BOARD_H

/* board.h defines the board class used to keep track of the state of the battledart board
 *
 * the battledart board is an 8x8 array of chars used to signify, boat, empty, hit or miss
 *
 * Author: Hannah Clay
 * Author: Eban Ebssa  
 */

#include <stdbool.h>

typedef struct {
    int length;
    bool horizontal;
    int x;  //starting x coordinate
    int y;  //starting y coordinate
    int hits;
} boat_t;

/* board_init should initialize the board to be an 8x8 array of characters and fill the board
 * with empty squares ('_')
 */
void board_init(void);

/* create_start_board fills the board 20 boat squares which represent the ships the player is to
 * sink. The boats should be of various lengths ranging from 1-5 and various orientations, whether
 * vertical or horizontal. 
 */
void create_start_board(void);

/* hit_square: updates the board to show that the square associated with the given coordinate
 * (x, y) has an attempted hit (hit if there's a boat at that square, miss otherwise) 
 *
 * @param x     x coordinate of the square that is hit on the board
 * @param y     y coordinate of the square that is hit on the board
 * @return      true if this the squares first hit, false if it's been hit already or if (x, y) is
 *              an invalid coordinate
 */
bool hit_square(int x, int y);

/* game_over: function for reporting whether or not the game has been won
 *
 * @return      true if the player has won and all ships are sunken and false otherwise
 */
bool game_over(void);

#endif
