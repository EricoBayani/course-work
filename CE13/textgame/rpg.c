// **** Include libraries here ****
// Standard libraries
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

//CMPE13 Support Library
#include "UNIXBOARD.h"
#include "Game.h"



// User libraries


// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any global or external variables here ****
static char desc[GAME_MAX_ROOM_DESC_LENGTH];
static char title[GAME_MAX_ROOM_TITLE_LENGTH];
static char *north = "\x1b[31mNORTH\x1b[0m";
static char *east = "\x1b[31mEAST\x1b[0m";
static char *south = "\x1b[31mSOUTH\x1b[0m";
static char *west = "\x1b[31mWEST\x1b[0m";
static uint8_t exits = 0b0000;
// **** Declare any function prototypes here ****

int main()
{



    /******************************** Your custom code goes below here ********************************/

    // initializes game and checks if it worked, if it doesn't, stop the program
    int initSuc = GameInit();
    if (initSuc == STANDARD_ERROR) {
        puts("Init Failed\n");
        FATAL_ERROR();
    }
    // gets the description of the starting room and the title
    GameGetCurrentRoomTitle(title);
    GameGetCurrentRoomDescription(desc);

    // gets the exits of the starting room, this is really not needed, but it helps for the other
    // checks for the exits
    exits = GameGetCurrentRoomExits();

    if (exits & GAME_ROOM_EXIT_NORTH_EXISTS)
        north = "\x1b[32mNORTH\x1b[0m";
    else
        north = "\x1b[31mNORTH\x1b[0m";
    if (exits & GAME_ROOM_EXIT_EAST_EXISTS)
        east = "\x1b[32mEAST\x1b[0m";
    else
        east = "\x1b[31mEAST\x1b[0m";
    if (exits & GAME_ROOM_EXIT_SOUTH_EXISTS)
        south = "\x1b[32mSOUTH\x1b[0m";
    else
        south = "\x1b[31mSOUTH\x1b[0m";
    if (exits & GAME_ROOM_EXIT_WEST_EXISTS)
        west = "\x1b[32mWEST\x1b[0m";
    else
        west = "\x1b[31mWEST\x1b[0m";

    // initial priting
    printf("\n\x1b[2J\x1b[H");
    printf("\x1b[34m\x1b[41m%s\x1b[0m\n%s\n", title, desc);
    printf("\x1b[H\x1b[24B\x1b[10C%s\n%s\x1b[17C%s\n\x1b[10C%s\n", north, west, east, south);
    printf("Enter Direction to Travel (n, e, s, w) or q to quit followed by enter:");

    // event loop looking for directions or q
    while (TRUE) {
        switch (getchar()) {
        case 'n':
                GameGoNorth();
            break;
        case 'e':
                GameGoEast();
            break;
        case 's':
                GameGoSouth();
            break;
        case 'w':
                GameGoWest();
            break;
        case 'q':
            exit(SUCCESS);
            break;
        }
        // get the title and description of the room
        memset(title, '\0', GAME_MAX_ROOM_TITLE_LENGTH);
        memset(desc, '\0', GAME_MAX_ROOM_DESC_LENGTH);
        GameGetCurrentRoomTitle(title);
        GameGetCurrentRoomDescription(desc);

        // gets the exits of the room
        exits = GameGetCurrentRoomExits();
        if (exits & GAME_ROOM_EXIT_NORTH_EXISTS)
            north = "\x1b[32mNORTH\x1b[0m";
        else
            north = "\x1b[31mNORTH\x1b[0m";
        if (exits & GAME_ROOM_EXIT_EAST_EXISTS)
            east = "\x1b[32mEAST\x1b[0m";
        else
            east = "\x1b[31mEAST\x1b[0m";
        if (exits & GAME_ROOM_EXIT_SOUTH_EXISTS)
            south = "\x1b[32mSOUTH\x1b[0m";
        else
            south = "\x1b[31mSOUTH\x1b[0m";
        if (exits & GAME_ROOM_EXIT_WEST_EXISTS)
            west = "\x1b[32mWEST\x1b[0m";
        else
            west = "\x1b[31mWEST\x1b[0m";

        // subsequent printing
        printf("\n\x1b[2J\x1b[H");
        printf("\x1b[34m\x1b[41m%s\x1b[0m\n%s\n", title, desc);
        printf("\x1b[H\x1b[24B\x1b[10C%s\n%s\x1b[17C%s\n\x1b[10C%s\n", north, west, east, south);
        printf("Enter Direction to Travel (n, e, s, w) or q to quit followed by enter:");
    }
    /**************************************************************************************************/

}