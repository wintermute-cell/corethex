#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// enumerating directories
#include <sys/types.h>
#include <dirent.h>

// used for interacting with external editor
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "main.h"
#include "config.h"
#include "../lib/dirops.h"

/**********************
   GLOBAL HELPER VARS
***********************/
// stores allocation size for dynamic arrays
int allocSize;

// for masking SIGWINCH signals
sigset_t sigwinch_mask;

// loop iterator
int i;

/**********************
    INIT FUNCTIONS
***********************/
int init_program(struct rundata* rd) {
    // block SIGWINCH
    sigemptyset (&sigwinch_mask);
    sigaddset(&sigwinch_mask, SIGWINCH);
    sigprocmask(SIG_BLOCK, &sigwinch_mask, NULL);

    // set locale (for wide characters)
    setlocale(LC_ALL, "");

    // set the editor
    if(getenv("EDITOR") == NULL) {
        rd->editor = malloc(4);
        if(rd->editor == NULL) {
            printf("%s\n", "couldn't initialize editor");
            exit(1);
        }
        snprintf(rd->editor, 3, "%s", "vi");
    }
    else {
        allocSize = snprintf(NULL, 0, "%s", getenv("EDITOR"));
        rd->editor = malloc(allocSize + 1);
        if(rd->editor == NULL) {
            printf("%s\n", "couldn't initialize editor");
            exit(1);
        }
        snprintf(rd->editor, allocSize+1, "%s", getenv("EDITOR"));
    }

    // get the current working dir.
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        allocSize = snprintf(NULL,0,"%s",cwd);


        rd->dir = malloc(allocSize+1);
        if(rd->dir == NULL) {
            printf("%s\n", "Couldn't initialize dir!\n");
            exit(1);
        }
        snprintf(rd->dir,allocSize+1,"%s",cwd);
    }
    else {
        printf("Couldn't open current directory!\n");
        exit(1);
    }


    // initialize runtime data
    rd->quit_flag = 0;
    rd->user_input = ' ';
    strcpy(rd->current_alert, WELCOME_MSG);

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

// open a file in the $EDITOR or else in vi
void open_file(struct rundata* rd, char* filepath)
{
    endwin();
    // Make a child process to edit file
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        execlp(rd->editor, rd->editor, filepath, (char *)0);
        exit(1);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
        return;
    }
}

// comprehending and acting on the user input character.
int process_input(struct rundata* rd) {
    switch (rd->user_input) {
        // meta keys
        case KEYMAP_QUIT:
            rd->quit_flag = 1;
            break;
        case ':':
            alert(rd, ":");
            // TODO: implement text input in alert bar.

        // movement
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

        // file operations
        case KEYMAP_OPEN:
            alert(rd, "Opening file!");
            open_file(rd, "./testfile.txt");
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

// parses and acts on the cl args.
// returns 1 when program should end after processing args.
int parse_args(struct rundata* rd, int argc, char *argv[]) {
    // check if there is only one arg.
    if (argc > 2) { printf("Too many args!\n"); exit(1); }

    // loop over every arg (should only be one).
    for (i = 1; i < argc; i+=1) {
        // initializing a new corethex project.
        if (strcmp(argv[i], "init") == 0) {
            // make sure the dir is empty
            if (dirop_count(rd->dir) > 0) { printf("Directory not empty, can't init!\n"); exit(1); }

            // run initialisation
            FILE* initfile;
            initfile  = fopen (".corethexpg", "w");
            fprintf(initfile, "#crthx v0.0.1\n.pagetree\n");
            fclose(initfile);
            return 1;
        // compiling an entire corethex project.
        } else if (strcmp(argv[i], "compile") == 0) {
            printf("COMPILE!\n");
            return 1;
        }
        // catch illegal arguments and exit the program.
        else { printf("Illegal arg: %s\n", argv[i]); exit(1); }
    }
    return 0;
}

/**********************
    MAIN FUNCTION
***********************/
int main(int argc, char *argv[]) {
    struct rundata rd;
    init_program(&rd);

    if(parse_args(&rd, argc, argv) != 1) {
        init_ncurses(&rd);
        program_loop(&rd);

        endwin();
    }

    return 0;
}
