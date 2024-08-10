/* Hannah Clay & Eban Ebssa
 * CS107E
 * 16 March 2022
 *
 * battleship.c contains code for running battleship game
 */

#include "gl.h"
#include "board.h"
#include "timer.h"
#include "mouse.h"
#include "interrupts.h"
#include "sensors.h"

typedef enum { START_SCREEN = 0, PLAY_SCREEN, END_SCREEN } screen_t;

static int screen = START_SCREEN;
static int board_x = 70;
static int board_y = 100;
static int board_width = 360;
static int board_height = 360;
static int square_width = 45;  // board_width / 8;
static int square_height = 45; // board_height / 8;
static int marker_width = 15;  // square_width / 3;

static int stats_x = 15;   // top left corner of the stats string (how many ships left to sink)
static int stats_y = 475;   
static color_t sunken = 0xff8a0a1f; // deep red - color used for sunken squares
static color_t background = GL_SILVER; 
static color_t board_color;

static int boat_squares = 19;
static int throws = 0;

static int mouse_x = 0;
static int mouse_y = 0;
#define cursor_width 4
static color_t prev_colors[cursor_width*cursor_width]; // stores colors around the mouse

static bool display_hit(int x, int y);
static void store_cursor_colors(int x, int y);

/* set_start_screen draws the background, title, and play button for the opening screen on
 * battledart
 */
static void set_start_screen(void) {
    // draw background
    gl_clear(background);
    
    // cool title
    color_t title = gl_color(12, 47, 105);
    color_t dark_gray = gl_color(25, 25, 25);
    gl_draw_rect(90, 10, 320, 80, title);
    gl_draw_rect(100, 20, 300, 60, dark_gray);
    gl_draw_rect(110, 30, 280, 40, title);
    gl_draw_string(180, 40, "BATTLEDART", GL_WHITE);
   
    // draw play button
    color_t board_color = gl_color(64, 117, 201);
    gl_draw_rect(150, 225, 200, 50, board_color); // button_x, y, width + height
    gl_draw_string(215, 240, "PLAY", GL_WHITE);

    store_cursor_colors(mouse_x, mouse_y);
}

/* centered_text_x takes in the number of chars of a string and returns the x of that string
 * centered on the screen
 */
static int centered_text_x(int num_chars) {
    return (gl_get_width() / 2) - ((num_chars * gl_get_char_width()) / 2);
}

/* stat_to_string takes in an array of size 3 buf and an int stat which represents a game statistic
 * and fills buf with char versions of the game statistic's digits
 */
static void stat_to_string(char buf[], int stat) {
    buf[0] = ((stat / 10) % 10) + '0';
    buf[1] = (stat % 10) + '0';
    buf[2] = '\0'; 
}

/* set_end_screen clears the game, displays game over and the number of tries it took to win. 
 * It also displays the retry button.                                       
 */                                                                         
static void set_end_screen(void) {                                           
    // erase everything below the title                                      
    gl_draw_rect(0, 100, gl_get_width(), gl_get_height() - 100, background);
                                                                            
    int game_over_x = centered_text_x(9);                                  
    gl_draw_string(game_over_x, 100, "GAME OVER", sunken);                   
                                                                               
    int throws_x = centered_text_x(27);                                      
    gl_draw_string(throws_x, 350, "ALL SHIPS SUNK IN    THROWS", GL_WHITE);  
                                                                             
    char stat[3];                                                            
    stat_to_string(stat, throws);                                       
    gl_draw_string(throws_x + (18 * gl_get_char_width()), 350, stat, sunken); 

    // draw retry button                                                            
    color_t board_color = gl_color(64, 117, 201);                                 
    gl_draw_rect(150, 225, 200, 50, board_color);  // button_x, y, width and height
    gl_draw_string(215, 240, "RETRY", GL_WHITE);

    store_cursor_colors(mouse_x, mouse_y);
 }        

/* correct_bounds takes in a coordinate and corrects it so that it is no longer outside of the
 * bounds of the screen (less than 0 or greater than 499) and returns the new coordinate. If the
 * coordinate is already in bounds, it returns the original coordinate. 
 */
static int correct_bounds(int coord) {
    coord = coord >= 0 ? coord : 0;    // min coordinate on the screen
    coord = coord < 500 ? coord : 499; // max coordinate on the screen
    return coord;
}

/* on_button returns true if the given (x, y) coord is on the button on the screen
 */
static bool on_button(int x, int y) {
    int button_x = 150;
    int button_y = 225;
    int button_width = 200;
    int button_height = 50;

    return button_x <= x && x <= (button_x + button_width) &&
           button_y <= y && y <= (button_y + button_height);
}

/* in_bounds returns true if the given (x, y) coord is located on the screen
 */
static bool in_bounds(int x, int y) {
    return x >= 0 && x < gl_get_width() && y >= 0 && y < gl_get_height();
}

/* store_cursor updates the prev_colors array with the colors around the rect
 * centered at (x,y) with radius cursor_width (from top left to bottom right).
 * If parts of the rect aren't on the board, add a `0` at that location.
 */
static void store_cursor_colors(int x, int y) {
    int radius = cursor_width / 2;
    int start_x = x;
    int start_y = y;
    int i = 0;
    for (int r = start_y; r < start_y + radius * 2; r++) {
        for (int c = start_x; c < start_x + radius * 2; c++) {
            if (in_bounds(c, r))
                prev_colors[i] = gl_read_pixel(c, r);
            else
                prev_colors[i] = 0;
            i++;
        }
    }
}

/* draw_cursor draws the prev_colors array in the rect centered at (x,y).
 * If you try to draw a `0` color, draw nothing at that pixel.
 */
static void draw_cursor(int x, int y) {
    int radius = cursor_width / 2;
    int start_x = x;
    int start_y = y;
    int i = 0;
    for (int r = start_y; r < start_y + radius * 2; r++) {
        for (int c = start_x; c < start_x + radius * 2; c++) {
            if (prev_colors[i] != 0)
                gl_draw_pixel(c, r, prev_colors[i]);
            i++;
        }
    }
}

/*  read_mouse uses mouse_read_event() to read mouse events from a PS2_mouse and determines the
 *  new location of the mouse. If the left button is clicked, the function determines if the
 *  mouse is on the board, and hits the appropriate square if so. 
 */
static void read_mouse(void) {
    if(screen == PLAY_SCREEN) return;

    mouse_event_t evt = mouse_read_event();
    draw_cursor(mouse_x, mouse_y);  // reads from prev_colors
    
    // update mouse location
    mouse_x += evt.dx;
    mouse_y -= evt.dy;
    mouse_x = correct_bounds(mouse_x);
    mouse_y = correct_bounds(mouse_y);

    store_cursor_colors(mouse_x, mouse_y);  // updates prev_colors array
    gl_draw_rect(mouse_x, mouse_y, cursor_width, cursor_width, GL_BLACK);

    // attempted hit -- left buttom is pressed
    if(evt.left && evt.action == MOUSE_BUTTON_PRESS && on_button(mouse_x, mouse_y)) {
        screen = (screen == START_SCREEN) ? PLAY_SCREEN : START_SCREEN;
    }
}

/* check_dart_hit ...
 */
void check_dart_hit(void) {
    setup();
    gpio_write(20, 0);  // gpio pin 20 = clock pin
    if (did_state_change()) {
        int i = find_magnet();
        if (i != -1) {
            display_hit(i % 8, i / 8);
        }
    }
    update();
}

/* set_start_screen sets up the starting screen of the battledart game which includes an empty
 * 8x8 grid, the title and the "ships left to sink" counter
 */
static void set_play_screen(void) {

    // draw stats
    gl_draw_string(stats_x, stats_y, "ship spaces: 19", sunken); 
    stats_x += gl_get_char_width() * 13; // stats_x now points to start of '20'
    gl_draw_string(stats_x + (gl_get_char_width() * 4), stats_y, "throws: 00", GL_WHITE);

    // draw board
    board_color = gl_color(64, 117, 201);
    gl_draw_rect(board_x, board_y, board_width, board_height, board_color);
    
    // add lines to the board
    for(int i = 0; i < 9; i++) {  
        int vx = board_x + (i * square_width);
        int vy1 = board_y;
        int vy2 = board_y + board_height;
        gl_draw_line(vx, vy1, vx, vy2, GL_BLACK); // draw vertical line
        
        int hx1 = board_x;
        int hx2 = board_x + board_width;
        int hy = board_y + (i * square_height);
        gl_draw_line(hx1, hy, hx2, hy, GL_BLACK); // draw horizontal line
    }

    // store init previous colors
    for (int i = 0; i < cursor_width * cursor_width; i++) {
        prev_colors[i] = background;
    }

    store_cursor_colors(mouse_x, mouse_y);

}

/* update_stats updates the "ships to sink" counter to be the current amount of ships left on
 * the board
 */
static void update_stats(void) {
    int throw_stat_x = stats_x + (gl_get_char_width() * 12);

    // erase current stats
    gl_draw_rect(stats_x, stats_y, gl_get_char_width() * 2, gl_get_char_height(), background);
    gl_draw_rect(throw_stat_x, stats_y, gl_get_char_width() * 2, gl_get_char_height(), background);

    // draw new stats
    char stat[3];
    stat_to_string(stat, boat_squares);
    gl_draw_string(stats_x, stats_y, stat, sunken);
    stat_to_string(stat, throws);
    gl_draw_string(throw_stat_x, stats_y, stat, GL_WHITE);
}

/* display_hit takes in the x and y of the square on the board that was hit and updates the display
 * accordingly. It returns a bool of whether or not the hit square was valid
 */
static bool display_hit(int x, int y) {
    char HIT = 'X';

    if(!hit_square(x, y)) {  //square was already hit or invalid
        return false;
    } else {  //there's a new hit or miss at the square at (x, y)
        char* board = get_current_board();
        char status = ((char (*) [8])board)[y][x];
        color_t marker = (status == HIT) ? sunken : GL_WHITE;
        if (status == HIT) boat_squares--;   
        throws++;
        update_stats();

        int square_x = board_x + (x * square_width);
        int square_y = board_y + (y * square_height);
        
        // redraw the blue square
        gl_draw_rect(square_x + 1, square_y + 1, square_width - 1, square_height - 1, board_color);

        // draw the marker
        square_x += marker_width;
        square_y += marker_width;
        gl_draw_rect(square_x, square_y, marker_width, marker_width, marker);
    }

    return true;
}

/* play runs the battledart game in a forever loop of start screen, game, end screen
 */
static void play(void) {
    boat_squares = 19;
    throws = 0;
    stats_x = 15;                  
                                   
    // start screen                
    set_start_screen();            
    while(screen == START_SCREEN) {
        read_mouse();
    }                 
                         
    // play screen       
    set_play_screen();   
    board_init();        
    create_start_board();         
    print_board(); 
                     
    while(!game_over()) {             
        check_dart_hit();
    };                  
                        
    // end screen                
    screen = END_SCREEN;         
    set_end_screen();            
                                 
    while(screen == END_SCREEN) {
       read_mouse();
    }             
 
    play(); // new game
}

void main(void) {
    interrupts_init();
    mouse_init(3, 4);       // 3 = clock gpio pin, 4 = data gpio pin
    interrupts_global_enable();
 
    gl_init(500, 500, GL_SINGLEBUFFER);
    play();
}
