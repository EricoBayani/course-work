// **** Include libraries here ****
// Standard C libraries


//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Buttons.h"
#include "Tree.h"
#include "Morse.h"
// **** Set any macros or preprocessor directives here ****


// **** Declare any data types here ****

static enum MorseStates {
    MORSE_WAITING,
    MORSE_DOT,
    MORSE_DASH,
    MORSE_INTER_LETTER
} MorseStates = MORSE_WAITING;
// **** Define any module-level, global, or external variables here ****

// char array containing the binary tree representation of the ascii alphanumerical characters
// and the nulls in between
static const char morseTree[63] = {
    '\0', 'E', 'I', 'S', 'H', '5', '4', 'V', '\0', '3', 'U', 'F', '\0', '\0', '\0', '\0', '2',
    'A', 'R', 'L', '\0', '\0', '\0', '\0', '\0', 'W', 'P', '\0', '\0', 'J', '\0', '1',
    'T', 'N', 'D', 'B', '6', '\0', 'X', '\0', '\0', 'K', 'C', '\0', '\0', 'Y', '\0', '\0',
    'M', 'G', 'Z', '7', '\0', 'Q', '\0', '\0', 'O', '\0', '8', '\0', '\0', '9', '0'

};

// static morse trees, morseRoot acts as a base to go back to after we're done traversing, 
// and morseTraverse is the tree we actually go down and traverse; we need two so that 
// we can reset one to the other since children don't know their parents, so there's no
// way to traverse back up the tree once we're down one
// NOTE: in reality, the traverse tree is just the pointer to the top of the root tree, and
// in essence, they're the same tree
static Node *morseRoot;
static Node *morseTraverse;

// static variable for holding the button event to be used in MorseCheckEvents
static uint8_t butEvent = 0;

// static variables for holding the morse events and the button counter to judge 
// how long a button was held down
static MorseEvent morEvent = MORSE_EVENT_NONE;
static short counter = 0;
// **** Declare any function prototypes here ****

int MorseInit(void)
{

    ButtonsInit(); // initializes buttons
    morseRoot = TreeCreate(6, morseTree); // creates morse tree decoder

    if (morseRoot == NULL) { // if there's not enough memory for a tree, then we have an error
        return STANDARD_ERROR;
    }

    // create a copy of the tree that we can freely traverse
    // morseRoot acts as the base that we can refer back to so we can reset morseTraverse
    // after traversing, since we children don't know their parents

    morseTraverse = morseRoot;

    return SUCCESS;
}

char MorseDecode(MorseChar in)
{
    // checks for the existence of either morse decoding tree, and returns STANDARD_ERROR
    // if there isn't
    if (morseRoot == NULL || morseTraverse == NULL) {
        return STANDARD_ERROR;
    }
    // switch statement handles the different MorseChar inputs
    switch (in) {
        // dot simply traverses the node to the left child
    case MORSE_CHAR_DOT:
        morseTraverse = morseTraverse->leftChild;
        break;

        // dash traverses the node to the right child
    case MORSE_CHAR_DASH:
        morseTraverse = morseTraverse->rightChild;
        break;

        // end of char will simply return the decoded character, or STANDARD_ERROR if there 
        // isn't one

    case MORSE_CHAR_END_OF_CHAR:
        if (morseTraverse->data == '\0') {
            return STANDARD_ERROR;
        } else {
            return morseTraverse->data;
        }
        break;

        // Reset will reset our traversal tree to our root tree, so we can start from the top
        // of our tree again

    case MORSE_CHAR_DECODE_RESET:
        morseTraverse = morseRoot;
        break;
        // ideally, there isn't a default case, this is included for completeness.
        // I'll put in a FATAL_ERROR here for now, since if we do hit this case, something really
        // must have gone wrong
    default:
        FATAL_ERROR();
        break;


    }
    return SUCCESS;
}

MorseEvent MorseCheckEvents(void)
{
    butEvent = ButtonsCheckEvents(); // first need to call ButtonsCheckEvents to get button events

    counter++; // counter increments every call to judge how long buttons are pressed for

    // switch statement will handle states transitions in the state machine of this function 
    // based on button events and counter compared to constants in Morse.h

    switch (MorseStates) {
    case MORSE_WAITING:
        // clears morse event flag after a transition to this state
        morEvent = MORSE_EVENT_NONE;
        // transition to dot when button is pressed
        if (butEvent & BUTTON_EVENT_4DOWN) {
            counter = 0;
            MorseStates = MORSE_DOT;
            butEvent = BUTTON_EVENT_NONE;
        }
        break;
    case MORSE_DOT:
        // clears morse event flag after a transition to this state
        morEvent = MORSE_EVENT_NONE;
        // transition to interletter when button is no longer pressed and qualifies as a dot
        if (butEvent & BUTTON_EVENT_4UP && counter < MORSE_EVENT_LENGTH_DOWN_DOT) {
            morEvent = MORSE_EVENT_DOT;
            MorseStates = MORSE_INTER_LETTER;
            butEvent = BUTTON_EVENT_NONE;
            // transition to dash when we press long enough    
        } else if (counter >= MORSE_EVENT_LENGTH_DOWN_DOT) {
            MorseStates = MORSE_DASH;
            butEvent = BUTTON_EVENT_NONE;
        }
        break;
    case MORSE_DASH:
        // transition to interletter when we release the button
        if (butEvent & BUTTON_EVENT_4UP) {
            counter = 0;
            morEvent = MORSE_EVENT_DASH;
            MorseStates = MORSE_INTER_LETTER;
            butEvent = BUTTON_EVENT_NONE;
        }
        break;
    case MORSE_INTER_LETTER:
        // prevents more than one dot or dash event from occuring 
        morEvent = MORSE_EVENT_NONE;
        // transition to waiting when nothing pressed for longer than 
        // MORSE_EVENT_LENGTH_UP_INTER_LETTER_TIMEOUT
        if (counter >= MORSE_EVENT_LENGTH_UP_INTER_LETTER_TIMEOUT) {
            morEvent = MORSE_EVENT_INTER_WORD;
            MorseStates = MORSE_WAITING;
            // transition back to dot when button 4 is pressed    
        } else if (butEvent & BUTTON_EVENT_4DOWN) {
            // transition back to dot where we can enter another letter of the word
            if (counter >= MORSE_EVENT_LENGTH_UP_INTER_LETTER) {
                counter = 0;
                morEvent = MORSE_EVENT_INTER_LETTER;
                MorseStates = MORSE_DOT;
                // transition back to dot where we can still change the current letter    
            } else {
                counter = 0;
                morEvent = MORSE_EVENT_NONE;
                MorseStates = MORSE_DOT;
            }
        }
        break;
        // we should never hit the default state
    default:
        FATAL_ERROR();
        break;
    }
    return morEvent;
}