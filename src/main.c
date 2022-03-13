#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "config.h"

int init_program(struct rundata* rd) {
    rd->quit_flag = 0;

    return 0;
};

int init_ncurses(struct rundata* rd) {
    initscr();
    noecho();
    curs_set(0);

    // check for color support.
    if (has_colors() == FALSE) {
        endwin();
        printf("the terminal does not support color.\n");
        exit(1);
    }
    else if (can_change_color() == FALSE) {
        printf("the terminal does not support changing colors.\n");
    }

    // register colors.
    start_color();
    use_default_colors();
    init_pair(1,
            COLOR_PRIMARY_FG,
            COLOR_PRIMARY_BG);
    init_pair(2,
            COLOR_SECONDARY_FG,
            COLOR_SECONDARY_BG);
    init_pair(3,
            COLOR_TERTIARY_FG,
            COLOR_TERTIARY_BG);

    return 0;
}

int refresh_ncurses(struct rundata* rd) {

    // get size of terminal.
    getmaxyx(stdscr, rd->screen_size.y, rd->screen_size.x);

    // save last two rows for status_win.
    rd->wscreen_size.x = rd->screen_size.x;
    rd->wscreen_size.y = rd->screen_size.y-2;

    // create ncurses windows.
    rd->win_status_bar = newwin( // status bar
            1,                      // height
            rd->screen_size.x,      // width
            rd->screen_size.y-2,    // ypos
            0);                     // xpos

    rd->win_alert = newwin( // alert
            1,                      // height
            rd->screen_size.x,      // width
            rd->screen_size.y-1,      // ypos
            0);                     // xpos

    rd->win_wscreen = newwin( // wscreen
            rd->wscreen_size.y,     // height
            rd->wscreen_size.x,     // width
            0,                      // ypos
            0);                     // xpos

    refresh();
    return 0;
}

/**********************
 WINDOW DRAW FUNCTIONS
***********************/
int draw_statusbar(struct rundata* rd) {
    wbkgd(rd->win_status_bar, COLOR_PAIR(2));
    wattron(rd->win_status_bar, A_DIM);
    mvwprintw(
            rd->win_status_bar,
            0, 0, "Status bar...");
    wattroff(rd->win_status_bar, A_DIM);

    wrefresh(rd->win_status_bar);
    return 0;
}
int draw_alert(struct rundata* rd) {
    wclear(rd->win_alert);
    mvwprintw(
            rd->win_alert,
            0, 0, "%s", rd->current_alert);

    wrefresh(rd->win_alert);
    return 0;
}
int draw_wscreen(struct rundata* rd) {
    mvwprintw(
            rd->win_wscreen,
            0, 0, "Working Screen");
    box(rd->win_wscreen, 0, 0);

    wrefresh(rd->win_wscreen);
    return 0;
}

/**********************
   UTILITY FUNCTIONS
***********************/
// use this to print an alert. (max 512 characters!)
int alert(struct rundata* rd, char* alert){
    strcpy(rd->current_alert, alert);
    return 0;
}

// comprehending and acting on the user input character.
int process_input(struct rundata* rd) {
    switch (rd->user_input) {
        // meta keys
        case KEYMAP_QUIT:
            rd->quit_flag = 1;
            break;
        case KEYMAP_UP:
            alert(rd, "Going up!");
            break;
        case KEYMAP_DOWN:
            alert(rd, "Going down!");
            break;
        case KEYMAP_LEFT:
            alert(rd, "Going left!");
            break;
        case KEYMAP_RIGHT:
            alert(rd, "Going right!");
            break;
    }
    return 0;
}

/**********************
   MAIN PROGRAM LOOP
***********************/
int program_loop(struct rundata* rd) {
    do {
        // refresh ncurses (recreating the windows at the correct size)
        refresh_ncurses(rd);

        // draw windows
        draw_statusbar(rd);
        draw_alert(rd);
        draw_wscreen(rd);

        // get the user input (also triggered by the resize event)
        rd->user_input = getch();

        // process user input
        process_input(rd);
    } while (rd->quit_flag != 1);

    return 0;
}

/**********************
    MAIN FUNCTION
***********************/
int main(int argc, char *argv[]) {
    struct rundata rd;
    init_program(&rd);
    init_ncurses(&rd);
    program_loop(&rd);

    endwin();

    return 0;
}
