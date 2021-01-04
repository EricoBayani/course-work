// **** Include libraries here ****
// Standard C libraries


//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Tree.h"

// **** Set any macros or preprocessor directives here ****


// **** Declare any data types here ****

// **** Define any module-level, global, or external variables here ****

// **** Declare any function prototypes here ****

Node *TreeCreate(int level, const char *data)
{

    Node *treeNode = malloc(sizeof (Node)); // allocates memory for node based on size of struct
    if (treeNode == NULL) {
        return NULL;
    }
    treeNode->data = *data; // assigns data to node

    // base case just returns node with data in it, no children
    if (level == 1) {
        return treeNode;
    }

    // falls through until last left child
    treeNode->leftChild = TreeCreate(level - 1, data + 1);
    // after the last left child, then right child gets made
    treeNode->rightChild = TreeCreate(level - 1, data + (2 << (level - 2)));


    return treeNode;


}