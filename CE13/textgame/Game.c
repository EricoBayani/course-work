// **** Include libraries here ****
// Standard libraries
#include <string.h>
#include <math.h>
#include <stdio.h>

//CMPE13 Support Library
#include "UNIXBOARD.h"
#include "Game.h"
#include "Player.h"


// User libraries


// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

typedef struct rData {
    char title[GAME_MAX_ROOM_TITLE_LENGTH];
    char desc[GAME_MAX_ROOM_DESC_LENGTH];
    uint8_t north;
    uint8_t east;
    uint8_t south;
    uint8_t west;
} RoomData;
// **** Define any global or external variables here ****

static RoomData data = {"\0", "\0", 0, 0, 0, 0};

static int titleSize;
static int descSize;
static int itemsReq;
static int itemsCont;


// **** Declare any function prototypes here ****

int GameGoNorth(void)
{

	// checks if the direction is a valid exit
	if (!data.north)
		return STANDARD_ERROR;
    // clear the title and description data

    memset(data.title, '\0', GAME_MAX_ROOM_TITLE_LENGTH);
    memset(data.desc, '\0', GAME_MAX_ROOM_DESC_LENGTH);

    // make an array to hold the file name
    char fileName[21];

    // create our key based on direction's room number, and set a direction
    int direction = data.north;
    int key = DECRYPTION_BASE_KEY + direction;

    // array for storing the title, encrypted at first
    char titleContents[GAME_MAX_ROOM_TITLE_LENGTH];

    // getting the proper file name
    sprintf(fileName, "RoomFiles/room%d.txt", direction);
    FILE *room = fopen(fileName, "rb");

    // check if there is such file
    if (room == NULL)
        return STANDARD_ERROR;

    // get the size of the title
    titleSize = fgetc(room) ^ key;

    // decrypt the title, and store it in the data struct
    fread(titleContents, 1, titleSize, room);
    int i;
    for (i = 0; i <= titleSize; i++) {
        titleContents[i] ^= key;
    }
    strncpy(data.title, titleContents, titleSize);

    // checks to see how many items we need
    itemsReq = fgetc(room) ^ key;

    // loop until we have the needed items, or we don't need items
    while (itemsReq) {

        // these variables and the loop checks if we have to items needed
        int itemHave = 0;
        int itemNeed = 0;
        int itemGood = 0;
        for (i = 1; i <= itemsReq; i++) {
            itemNeed = fgetc(room) ^ key;
            itemHave = FindInInventory(itemNeed);
            if (itemHave == SUCCESS)
                itemGood++;
        }

        // if we have enough items, we break out of the loop and continue to data parsing
        if (itemGood == itemsReq)
            break;

        // since we didn't have the items needed, we skip to the next version of the room

        // first by the length of the description
        descSize = fgetc(room) ^ key;
        fseek(room, descSize, SEEK_CUR);

        // then by how many items it contains
        itemsCont = fgetc(room) ^ key;
        fseek(room, itemsCont, SEEK_CUR);

        // then by the exits it contains, which is just 4, since there are only 4 exits
        fseek(room, 4, SEEK_CUR);

        // now we should be in another version of the room, so we grab the items required again
        itemsReq = fgetc(room) ^ key;

    }

    // by now, we should have the required items, or we don't need items, so we parse the room data
    char descContents[GAME_MAX_ROOM_DESC_LENGTH];
    descSize = fgetc(room) ^ key;

    // decrypt the description, and store it in the data struct
    fread(descContents, 1, descSize, room);
    for (i = 0; i <= descSize; i++) {
        descContents[i] ^= key;
    }
    strncpy(data.desc, descContents, descSize);

    // after the description, we see what items the room has, then take them
    int itemGot = 0;
    int item = 0;
    itemsCont = fgetc(room) ^ key;
    for (i = 1; i <= itemsCont; i++) {
        item = fgetc(room)^key;
        itemGot = AddToInventory(item);
    }

    // by this time, we should be where the exits are, so we store the exits into the struct
    data.north = fgetc(room) ^ key;
    data.east = fgetc(room) ^ key;
    data.south = fgetc(room) ^ key;
    data.west = fgetc(room) ^ key;


    // close the file, if we can't, then throw an error
    int closeFail = fclose(room);
    if (closeFail)
        return STANDARD_ERROR;

    return SUCCESS;
}

int GameGoEast(void)
{
	// checks if the direction is a valid exit
	if (!data.east)
		return STANDARD_ERROR;
	
    // clear the title and description data
    memset(data.title, '\0', GAME_MAX_ROOM_TITLE_LENGTH);
    memset(data.desc, '\0', GAME_MAX_ROOM_DESC_LENGTH);

    // make an array to hold the file name
    char fileName[21];

    // create our key based on direction's room number, and set a direction
    int direction = data.east;
    int key = DECRYPTION_BASE_KEY + direction;

    // array for storing the title, encrypted at first
    char titleContents[GAME_MAX_ROOM_TITLE_LENGTH];

    // getting the proper file name
    sprintf(fileName, "RoomFiles/room%d.txt", direction);
    FILE *room = fopen(fileName, "rb");

    // check if there is such file
    if (room == NULL)
        return STANDARD_ERROR;

    // get the size of the title
    titleSize = fgetc(room) ^ key;

    // decrypt the title, and store it in the data struct
    fread(titleContents, 1, titleSize, room);
    int i;
    for (i = 0; i <= titleSize; i++) {
        titleContents[i] ^= key;
    }
    strncpy(data.title, titleContents, titleSize);

    // checks to see how many items we need
    itemsReq = fgetc(room) ^ key;

    // loop until we have the needed items, or we don't need items
    while (itemsReq) {

        // these variables and the loop checks if we have to items needed
        int itemHave = 0;
        int itemNeed = 0;
        int itemGood = 0;
        for (i = 1; i <= itemsReq; i++) {
            itemNeed = fgetc(room) ^ key;
            itemHave = FindInInventory(itemNeed);
            if (itemHave == SUCCESS)
                itemGood++;
        }

        // if we have enough items, we break out of the loop and continue to data parsing
        if (itemGood == itemsReq)
            break;

        // since we didn't have the items needed, we skip to the next version of the room

        // first by the length of the description
        descSize = fgetc(room) ^ key;
        fseek(room, descSize, SEEK_CUR);

        // then by how many items it contains
        itemsCont = fgetc(room) ^ key;
        fseek(room, itemsCont, SEEK_CUR);

        // then by the exits it contains, which is just 4, since there are only 4 exits
        fseek(room, 4, SEEK_CUR);

        // now we should be in another version of the room, so we grab the items required again
        itemsReq = fgetc(room) ^ key;

    }

    // by now, we should have the required items, or we don't need items, so we parse the room data
    char descContents[GAME_MAX_ROOM_DESC_LENGTH];
    descSize = fgetc(room) ^ key;

    // decrypt the description, and store it in the data struct
    fread(descContents, 1, descSize, room);
    for (i = 0; i <= descSize; i++) {
        descContents[i] ^= key;
    }
    strncpy(data.desc, descContents, descSize);

    // after the description, we see what items the room has, then take them
    int itemGot = 0;
    int item = 0;
    itemsCont = fgetc(room) ^ key;
    for (i = 1; i <= itemsCont; i++) {
        item = fgetc(room)^key;
        itemGot = AddToInventory(item);
    }

    // by this time, we should be where the exits are, so we store the exits into the struct
    data.north = fgetc(room) ^ key;
    data.east = fgetc(room) ^ key;
    data.south = fgetc(room) ^ key;
    data.west = fgetc(room) ^ key;


    // close the file, if we can't, then throw an error
    int closeFail = fclose(room);
    if (closeFail)
        return STANDARD_ERROR;

    return SUCCESS;
}

int GameGoSouth(void)
{
	
	// checks if the direction is a valid exit
	if (!data.south)
		return STANDARD_ERROR;
    // clear the title and description data

    memset(data.title, '\0', GAME_MAX_ROOM_TITLE_LENGTH);
    memset(data.desc, '\0', GAME_MAX_ROOM_DESC_LENGTH);

    // make an array to hold the file name
    char fileName[21];

    // create our key based on direction's room number, and set a direction
    int direction = data.south;
    int key = DECRYPTION_BASE_KEY + direction;

    // array for storing the title, encrypted at first
    char titleContents[GAME_MAX_ROOM_TITLE_LENGTH];

    // getting the proper file name
    sprintf(fileName, "RoomFiles/room%d.txt", direction);
    FILE *room = fopen(fileName, "rb");

    // check if there is such file
    if (room == NULL)
        return STANDARD_ERROR;

    // get the size of the title
    titleSize = fgetc(room) ^ key;

    // decrypt the title, and store it in the data struct
    fread(titleContents, 1, titleSize, room);
    int i;
    for (i = 0; i <= titleSize; i++) {
        titleContents[i] ^= key;
    }
    strncpy(data.title, titleContents, titleSize);

    // checks to see how many items we need
    itemsReq = fgetc(room) ^ key;

    // loop until we have the needed items, or we don't need items
    while (itemsReq) {

        // these variables and the loop checks if we have to items needed
        int itemHave = 0;
        int itemNeed = 0;
        int itemGood = 0;
        for (i = 1; i <= itemsReq; i++) {
            itemNeed = fgetc(room) ^ key;
            itemHave = FindInInventory(itemNeed);
            if (itemHave == SUCCESS)
                itemGood++;
        }

        // if we have enough items, we break out of the loop and continue to data parsing
        if (itemGood == itemsReq)
            break;

        // since we didn't have the items needed, we skip to the next version of the room

        // first by the length of the description
        descSize = fgetc(room) ^ key;
        fseek(room, descSize, SEEK_CUR);

        // then by how many items it contains
        itemsCont = fgetc(room) ^ key;
        fseek(room, itemsCont, SEEK_CUR);

        // then by the exits it contains, which is just 4, since there are only 4 exits
        fseek(room, 4, SEEK_CUR);

        // now we should be in another version of the room, so we grab the items required again
        itemsReq = fgetc(room) ^ key;

    }

    // by now, we should have the required items, or we don't need items, so we parse the room data
    char descContents[GAME_MAX_ROOM_DESC_LENGTH];
    descSize = fgetc(room) ^ key;

    // decrypt the description, and store it in the data struct
    fread(descContents, 1, descSize, room);
    for (i = 0; i <= descSize; i++) {
        descContents[i] ^= key;
    }
    strncpy(data.desc, descContents, descSize);

    // after the description, we see what items the room has, then take them
    int itemGot = 0;
    int item = 0;
    itemsCont = fgetc(room) ^ key;
    for (i = 1; i <= itemsCont; i++) {
        item = fgetc(room)^key;
        itemGot = AddToInventory(item);
    }

    // by this time, we should be where the exits are, so we store the exits into the struct
    data.north = fgetc(room) ^ key;
    data.east = fgetc(room) ^ key;
    data.south = fgetc(room) ^ key;
    data.west = fgetc(room) ^ key;


    // close the file, if we can't, then throw an error
    int closeFail = fclose(room);
    if (closeFail)
        return STANDARD_ERROR;

    return SUCCESS;
}

int GameGoWest(void)
{
	// checks if the direction is a valid exit
	if (!data.west)
		return STANDARD_ERROR;
	
    // clear the title and description data
    memset(data.title, '\0', GAME_MAX_ROOM_TITLE_LENGTH);
    memset(data.desc, '\0', GAME_MAX_ROOM_DESC_LENGTH);

    // make an array to hold the file name
    char fileName[21];

    // create our key based on direction's room number, and set a direction
    int direction = data.west;
    int key = DECRYPTION_BASE_KEY + direction;

    // array for storing the title, encrypted at first
    char titleContents[GAME_MAX_ROOM_TITLE_LENGTH];

    // getting the proper file name
    sprintf(fileName, "RoomFiles/room%d.txt", direction);
    FILE *room = fopen(fileName, "rb");

    // check if there is such file
    if (room == NULL)
        return STANDARD_ERROR;

    // get the size of the title
    titleSize = fgetc(room) ^ key;

    // decrypt the title, and store it in the data struct
    fread(titleContents, 1, titleSize, room);
    int i;
    for (i = 0; i <= titleSize; i++) {
        titleContents[i] ^= key;
    }
    strncpy(data.title, titleContents, titleSize);

    // checks to see how many items we need
    itemsReq = fgetc(room) ^ key;

    // loop until we have the needed items, or we don't need items
    while (itemsReq) {

        // these variables and the loop checks if we have to items needed
        int itemHave = 0;
        int itemNeed = 0;
        int itemGood = 0;
        for (i = 1; i <= itemsReq; i++) {
            itemNeed = fgetc(room) ^ key;
            itemHave = FindInInventory(itemNeed);
            if (itemHave == SUCCESS)
                itemGood++;
        }

        // if we have enough items, we break out of the loop and continue to data parsing
        if (itemGood == itemsReq)
            break;

        // since we didn't have the items needed, we skip to the next version of the room

        // first by the length of the description
        descSize = fgetc(room) ^ key;
        fseek(room, descSize, SEEK_CUR);

        // then by how many items it contains
        itemsCont = fgetc(room) ^ key;
        fseek(room, itemsCont, SEEK_CUR);

        // then by the exits it contains, which is just 4, since there are only 4 exits
        fseek(room, 4, SEEK_CUR);

        // now we should be in another version of the room, so we grab the items required again
        itemsReq = fgetc(room) ^ key;

    }

    // by now, we should have the required items, or we don't need items, so we parse the room data
    char descContents[GAME_MAX_ROOM_DESC_LENGTH];
    descSize = fgetc(room) ^ key;

    // decrypt the description, and store it in the data struct
    fread(descContents, 1, descSize, room);
    for (i = 0; i <= descSize; i++) {
        descContents[i] ^= key;
    }
    strncpy(data.desc, descContents, descSize);

    // after the description, we see what items the room has, then take them
    int itemGot = 0;
    int item = 0;
    itemsCont = fgetc(room) ^ key;
    for (i = 1; i <= itemsCont; i++) {
        item = fgetc(room)^key;
        itemGot = AddToInventory(item);
    }

    // by this time, we should be where the exits are, so we store the exits into the struct
    data.north = fgetc(room) ^ key;
    data.east = fgetc(room) ^ key;
    data.south = fgetc(room) ^ key;
    data.west = fgetc(room) ^ key;


    // close the file, if we can't, then throw an error
    int closeFail = fclose(room);
    if (closeFail)
        return STANDARD_ERROR;

    return SUCCESS;
}

int GameInit(void)
{

    // ghost room that has nothing but an exit to STARTING_ROOM
    data.north = STARTING_ROOM;

    // go into starting room which is north
    int goSuc = GameGoNorth();

    // check for if function was successful, it would fail if there were no rooms.
    if (goSuc == STANDARD_ERROR)
        return STANDARD_ERROR;

    return SUCCESS;
}

int GameGetCurrentRoomTitle(char *title)
{

    // simply copies string from data struct to passed pointer
    strncpy(title, data.title, titleSize);

    return titleSize;
}

int GameGetCurrentRoomDescription(char *desc)
{

    // simply copies string from data struct to passed pointer
    strncpy(desc, data.desc, descSize);

    return descSize;
}

uint8_t GameGetCurrentRoomExits(void)
{
    // if there are is an exit in data.* then xor with corresponding enum value, then return value
    uint8_t exits = 0b0000;
    if (data.north)
        exits ^= GAME_ROOM_EXIT_NORTH_EXISTS;
    if (data.east)
        exits ^= GAME_ROOM_EXIT_EAST_EXISTS;
    if (data.south)
        exits ^= GAME_ROOM_EXIT_SOUTH_EXISTS;
    if (data.west)
        exits ^= GAME_ROOM_EXIT_WEST_EXISTS;
    return exits;
}








