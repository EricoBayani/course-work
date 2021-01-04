// **** Include libraries here ****
// Standard libraries
#include <string.h>
#include <math.h>
#include <stdio.h>

//CMPE13 Support Library
#include "UNIXBOARD.h"



// User libraries
#include "Player.h"

// **** Set any macros or preprocessor directives here ****

// **** Declare any data types here ****

// **** Define any global or external variables here ****

static int inventory[INVENTORY_SIZE];
static int inventoryIndex = 0; // the first inventory space is the first element of the array above

// **** Declare any function prototypes here ****

int AddToInventory(uint8_t item)
{

    // first check to see if there's enough space, if there isn't, return STANDARD_ERROR
    if (inventoryIndex > (INVENTORY_SIZE - 1))
        return STANDARD_ERROR;

    // adds the item to the inventory
    inventory[inventoryIndex] = item;

    return SUCCESS;
}

int FindInInventory(uint8_t item)
{
	
    // first check to see if there's enough space, if there isn't, return STANDARD_ERROR
    if (inventoryIndex > (INVENTORY_SIZE - 1))
        return STANDARD_ERROR;

    // loop through our inventory checking for the specific item
    int i;
    for (i = 0; i <= (INVENTORY_SIZE - 1); i++) {
        if (inventory[i] == item)
            return SUCCESS;
    }

    // if we end up here, that means we didn't find the item
    return STANDARD_ERROR;
}