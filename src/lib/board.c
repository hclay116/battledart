/* Hannah Clay and Eban Ebssa
 * CS107E
 * 15 March 2022
 *
 * board.c contains code for creating and keeping track of the state of a battleship board
 * represented as an 8x8 array of characters
 */

#include "board.h"
#include "strings.h"
#include "timer.h"
#include "printf.h"
#include "malloc.h"
#include <stdbool.h>

#define EMPTY '_'
#define HIT 'X'
#define MISS 'O'

static char *board;
static int size = 8;
static int boat_squares;
static int hit_count;

char board_1[64] = { '_', '_', '_', '_', '_', '_', '_', '5',
                     '_', '_', '2', '_', '_', '_', '_', '5',
                     '_', '_', '2', '_', '_', '_', '_', '5',
                     '_', '_', '2', '_', '_', '_', '_', '5',
                     '0', '_', '_', '_', '_', '_', '_', '_',
                     '0', '_', '_', '3', '3', '_', '4', '4',
                     '0', '_', '_', '_', '_', '_', '_', '_',
                     '0', '_', '1', '1', '1', '1', '_', '_' };

char board_2[64] = { '0', '_', '_', '_', '_', '4', '_', '3',
                     '0', '_', '_', '_', '_', '4', '_', '3',
                     '0', '_', '_', '_', '_', '4', '_', '3',
                     '0', '_', '_', '_', '_', '_', '_', '_',
                     '_', '_', '_', '_', '6', '_', '_', '_',
                     '1', '_', '2', '_', '6', '_', '_', '_',
                     '1', '_', '2', '_', '_', '_', '_', '_',
                     '_', '_', '_', '_', '_', '5', '5', '5' };

char board_3[64] = { '_', '1', '1', '1', '_', '_', '6', '6',
                     '_', '_', '_', '_', '_', '_', '_', '_',
                     '_', '2', '2', '2', '2', '_', '_', '_',
                     '_', '_', '_', '_', '_', '_', '_', '_',
                     '0', '_', '3', '_', '_', '_', '5', '_',
                     '0', '_', '3', '_', '4', '_', '5', '_',
                     '_', '_', '_', '_', '4', '_', '5', '_',
                     '_', '_', '_', '_', '4', '_', '_', '_' };

/* board_init creates a starting 8x8 battleship board on the display
 */
void board_init(void) {
    board = malloc(size * size);
    if(board == NULL) return;            //check for null malloc
    memset(board, EMPTY, size * size);   //initialize board with empty squares
    boat_squares = 19;
    hit_count = 0;
}

/* get_current_board returns the current board as a char* array
 */
char* get_current_board(void) {
    return board;
}

/* random_int returns a random integer between 1 and end
 */
static int random_int(int end) {
    return ((timer_get_ticks() * timer_get_ticks() - timer_get_ticks()) % end) + 1;
}

/* is_valid takes in an x and a y coordinate and returns if the resulting coordinate is a valid
 * coordinate (with in the bounds of the size by size board)
 */
static int is_valid(int x, int y) {
    return (0 <= x && x < size && 0 <= y && y < size);
}

/* is_boat returns whether a character represents a boat square or not
 */
bool is_boat(char square) {
    return square == '0' || square == '1' || square == '2' || square == '3' || square == '4' ||
           square == '5' || square == '6';
}

/* returns whether or not the current position of the boat on the board is a valid position
 * 
 * a position is valid if it doesn't overlap with any other boats or touch other boats on the board
 */
static bool boat_fits(boat_t boat) {
    int boat_x = boat.x;
    int boat_y = boat.y;

    //check that each location is valid
    //check that the area above + below the boat != boat
    //check that the sides of the boat are not boats
    for(int i = 0; i < boat.length; i++) {
        if(!is_valid(boat_x, boat_y)) return false;
        
        for(int x_inc = -1; x_inc < 2; x_inc++) {
            for(int y_inc = -1; y_inc < 2; y_inc++) {
                int square_x = boat_x + x_inc;
                int square_y = boat_y + y_inc;

                if(is_valid(square_x, square_y)) {
                    char square = ((char (*)[size])board)[square_y][square_x];
                    if(is_boat(square)) return false;
                }
            }
        }

        boat_x = boat.horizontal ? boat_x + 1 : boat_x;
        boat_y = boat.horizontal ? boat_y : boat_y + 1;
    }

    return true;
}

/* create_random_board() sets the board to be a random arrangement of ships based on boat_squares
 */ 
void create_random_board(void) {
    int lengths[] = {4, 3, 3, 3, 2, 2, 2}; // lengths of the boats to be added

    for(int i = 0; i < 7; i++) {  // 7 = number of boats
        //randomize the starting info of the boat -- start square, orientation
        boat_t boat = {};
        boat.length = lengths[i];
        boat.hits = 0;
        boat.horizontal = (bool)(random_int(2) - 1);
        boat.x = random_int(size) - 1;
        boat.y = random_int(size) - 1;

        while(!boat_fits(boat)) {
            boat.horizontal = (bool)(random_int(2) - 1);
            boat.x = random_int(size) - 1;
            boat.y = random_int(size) - 1;
        }

        int square_x = boat.x;
        int square_y = boat.y;
        
        //put the boat on the board
        for(int num = 0; num < boat.length; num++) {
            char square = ((char (*) [size])board)[square_y][square_x];
            if(is_boat(square)) printf("boat square passed\n");

            ((char (*) [size])board)[square_y][square_x] = i + '0';
            if(boat.horizontal)
                square_x++;
            else  //vertical
                square_y++;
        }

    }
} 

/* create_simple_board sets the board to be one of three pre-made start boards at random
 */
void create_simple_board(void) {
    int board_num = random_int(3); //board_num = random integer btw 1 and 3

    switch (board_num) {
        case 1:
            board = (char *)board_1;
            break;
        case 2:
            board = (char *)board_2;
            break;
        case 3:
            board = (char *)board_3;
    }
}

/* print_board() is a helper function for testing which prints the current characters
 * stored in bored in an 8x8 formation
 */
void print_board(void) {
    for(int x = 0; x < size; x++) {
        printf("|");
        for(int y = 0; y < size; y++) {
            char square = ((char (*) [size])board)[x][y];
            printf("%c|", square);
        }
        printf("\n");
    }
    printf("\n");
}

/* create_start_board fills the board with hidden boats based on the number of boat squares and the
 * size of the board.
 */
void create_start_board() {
    create_random_board();
    //create_simple_board();
}

/* hit_square takes in the x and y coordinates of the square the player hit and marks the
 * corresponding square as a hit or miss based on whether or not a boat is there. hit_square
 * returns true if the square has not been hit before and false otherwise.
 */
bool hit_square(int x, int y) {
   if(!is_valid(x, y)) return false;    //invalid coordinates

   char square = ((char (*) [size])board)[y][x];
   if(is_boat(square)) {
        ((char (*) [size])board)[y][x] = HIT;
        hit_count++;

        return true;
   } else if(square == EMPTY) {
        ((char (*) [size])board)[y][x] = MISS;
        return true;
   } else { //square has already been hit before
        return false;
   }
}

/* game_over returns true if the player has sunken all of the ships and the game is over and false
 * otherwise
 */
bool game_over(void) {
    return hit_count == boat_squares;
}

