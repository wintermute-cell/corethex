#include <ncurses.h>

/*********************
    DATA STRUCTURES
**********************/
struct vector2 {
    int x;
    int y;
};
struct vector3 {
    int x;
    int y;
    int z;
};

// a structure carrying all the runtime-relevant data.
struct rundata {
    struct vector2 screen_size; // the available screen size.
    struct vector2 wscreen_size; // the working-screen size defines the size without toolsbars an alerts.
    char user_input; // the user input char.
    WINDOW * win_status_bar;
    WINDOW * win_alert;
    WINDOW * win_wscreen;
};

/**********************
    FUNCTION HEADERS
***********************/
// initializer function that runs before anyting else.
int init_program(struct rundata* rd);

// initializer for ncurses, gets run after program initializer.
int init_ncurses(struct rundata* rd);

// this contains the main loop for the program.
int program_loop(struct rundata* rd);

// this contains the main loop for the program.
int refresh_ncurses(struct rundata* rd);
