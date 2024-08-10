#include "printf.h"
#include "assert.h"
#include "board.h"
#include "timer.h"
#include "uart.h"

static void test_create_simple_board(void) {
    printf("Make sure that create_start_board has create_simple_board uncommented\n");
    board_init();
    create_start_board();
    print_board();
    printf("Compare the printed board with the three premade boards\n\n");
}

static void test_create_random_board(void) {
    printf("Make sure that create_start_board has create_random_board uncommented\n");
    board_init();
    create_start_board();
    print_board();
    printf("Make sure that the printed board is a valid board\n\n");
}

static void test_board(void) {
    board_init();
    create_start_board();
    printf("start board\n");
    print_board();

    //hit a bunch of random squares
    assert(hit_square(0,0));
    assert(hit_square(5, 6));
    assert(hit_square(1, 2));
    assert(hit_square(3, 4));
    assert(hit_square(7, 2));
    assert(hit_square(7, 4));
    assert(hit_square(6, 5));
    assert(hit_square(1, 1));
    assert(hit_square(3, 3));
    assert(hit_square(4, 7));

    printf("board after a series of hits\n");
    print_board();
    printf("\n");

    //invalid calls to hit_square return false
    assert(!hit_square(0,0));    // already been hit
    assert(!hit_square(-1, 6));  // outside of the bounds
    assert(!hit_square(8, 7));   // ^^

    printf("board should be unchanged\n");
    print_board();

    assert(!game_over());
}

void main(void)
{
    uart_init();
    printf("Running tests from file %s\n", __FILE__);
    //test_create_simple_board();
    test_create_random_board();
    test_board();
    uart_putchar(EOT);
}
