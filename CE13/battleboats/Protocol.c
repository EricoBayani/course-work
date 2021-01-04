/*
 * File:   Protocol.c
 * Author:   Erico Bayani and Brian Lucht
 *
 */
// **** Include libraries here ****
// Standard libraries
#include <stdlib.h>
//CMPE13 Support Library
#include "BOARD.h"

// Microchip libraries
#include <xc.h>
#include <plib.h>

// User libraries
#include "Protocol.h"

// **** Set any macros or preprocessor directives here ****
#define ASCII_NUM_OFFSET 48
#define ASCII_HEXCHAR_OFFSET 87 //hexchars are 10 more than normal chars
#define INVALID_CHAR 255

// **** Declare any data types here ****

// enum for FSM states for Protocol Decode

typedef enum {
    WAITING,
    RECORDING,
    FIRST_CHECKSUM_HALF,
    SECOND_CHECKSUM_HALF,
    NEWLINE
} PDStates;

typedef struct protocolDecodeData {
    char protocolMessage[PROTOCOL_MAX_MESSAGE_LEN]; // string that holds Message
    uint8_t protoMessIndex; // index of message
    PDStates protocolDecodeStates; // states of decode FSM
    uint8_t checkSum; // checksum
} PD_Data;

// **** Define any module-level, global, or external variables here ****

static PD_Data pdData;
// **** Declare any function prototypes here ****

// static helper function prototypes


static uint8_t CheckSumCreate(char *payload);
static uint8_t AsciiToByte(char value);

int ProtocolEncodeCooMessage(char *message, const GuessData *data)
{
    int count = 0;

    // payload string to hold payload data
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(payload, PAYLOAD_TEMPLATE_COO, data->row, data->col);

    // getting payload checksum
    uint8_t plCheckSum = CheckSumCreate(payload);


    // storing whole message and payload into message string
    sprintf(message, MESSAGE_TEMPLATE, payload, plCheckSum);

    // count the characters
    count = strlen(message);

    return count;
}

int ProtocolEncodeHitMessage(char *message, const GuessData *data)
{
    int count = 0;

    // payload string to hold payload data
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(payload, PAYLOAD_TEMPLATE_HIT, data->row, data->col, data->hit);

    // getting payload checksum
    uint8_t plCheckSum = CheckSumCreate(payload);


    // storing whole message and payload into message string
    sprintf(message, MESSAGE_TEMPLATE, payload, plCheckSum);

    // count the characters
    count = strlen(message);

    return count;
}

int ProtocolEncodeChaMessage(char *message, const NegotiationData *data)
{
    int count = 0;

    // payload string to hold payload data
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(payload, PAYLOAD_TEMPLATE_CHA, data->encryptedGuess, data->hash);

    // getting payload checksum
    uint8_t plCheckSum = CheckSumCreate(payload);


    // storing whole message and payload into message string
    sprintf(message, MESSAGE_TEMPLATE, payload, plCheckSum);

    // count the characters
    count = strlen(message);

    return count;
}

int ProtocolEncodeDetMessage(char *message, const NegotiationData *data)
{
    int count = 0;

    // payload string to hold payload data
    char payload[PROTOCOL_MAX_PAYLOAD_LEN];
    sprintf(payload, PAYLOAD_TEMPLATE_DET, data->guess, data->encryptionKey);

    // getting payload checksum
    uint8_t plCheckSum = CheckSumCreate(payload);


    // storing whole message and payload into message string
    sprintf(message, MESSAGE_TEMPLATE, payload, plCheckSum);

    // count the characters
    count = strlen(message);

    return count;
}

ProtocolParserStatus ProtocolDecode(char in, NegotiationData *nData, GuessData *gData)
{
    switch (pdData.protocolDecodeStates) {

    default:
        pdData.protocolDecodeStates = WAITING;

    case WAITING:
        // waiting for $ and clear protocol message
        memset(pdData.protocolMessage, '\0', pdData.protoMessIndex);
        if (in == '$') {

            // reset string index
            pdData.protoMessIndex = 0;
            pdData.protocolDecodeStates = RECORDING;
            return PROTOCOL_PARSING_GOOD;
        } else {
            return PROTOCOL_WAITING;
        }
        break;

    case RECORDING:
        // check if '*' for end of protocol message and begin check sum
        if (in == '*') {
            pdData.protocolDecodeStates = FIRST_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        } else {
            // if not, add the next char to protocol message
            pdData.protocolMessage[pdData.protoMessIndex] = in;
            pdData.protoMessIndex++;
            return PROTOCOL_PARSING_GOOD;
        }
        break;

    case FIRST_CHECKSUM_HALF:
        // check if character is valid
        if (AsciiToByte(in) != INVALID_CHAR) {
            // store first hex value as top 4 bits
            pdData.checkSum = AsciiToByte(in) << 4;
            pdData.protocolDecodeStates = SECOND_CHECKSUM_HALF;
            return PROTOCOL_PARSING_GOOD;
        } else {
            // if character is invalid, fail
            pdData.protocolDecodeStates = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;

    case SECOND_CHECKSUM_HALF:
        // check if character is valid
        if (AsciiToByte(in) != INVALID_CHAR) {
            // store second hex value
            pdData.checkSum = pdData.checkSum ^ AsciiToByte(in);
            pdData.protocolMessage[pdData.protoMessIndex] = NULL;
            pdData.protocolDecodeStates = NEWLINE;
            return PROTOCOL_PARSING_GOOD;
        } else {
            // if character is invalid, fail
            pdData.protocolDecodeStates = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;


    case NEWLINE:
        // check for new line
        if (in == '\n') {
            if (strstr(pdData.protocolMessage, "COO") != NULL) { // check if COO
                sscanf(pdData.protocolMessage, PAYLOAD_TEMPLATE_COO, // stores COO data
                        &gData->row, &gData->col);
                pdData.protocolDecodeStates = WAITING;
                return PROTOCOL_PARSED_COO_MESSAGE;
            } else if (strstr(pdData.protocolMessage, "HIT") != NULL) { // check if HIT
                sscanf(pdData.protocolMessage, PAYLOAD_TEMPLATE_HIT, //stores HIT data
                        &gData->row, &gData->col,
                        &gData->hit);
                pdData.protocolDecodeStates = WAITING;
                return PROTOCOL_PARSED_HIT_MESSAGE;
            } else if (strstr(pdData.protocolMessage, "CHA") != NULL) { // check if CHA
                sscanf(pdData.protocolMessage, PAYLOAD_TEMPLATE_CHA, // stores CHA data
                        &nData->encryptedGuess, &nData->hash);
                pdData.protocolDecodeStates = WAITING;
                return PROTOCOL_PARSED_CHA_MESSAGE;
            } else if (strstr(pdData.protocolMessage, "DET") != NULL) { // check if DET
                sscanf(pdData.protocolMessage, PAYLOAD_TEMPLATE_DET, // stores DET data
                        &nData->guess, &nData->encryptionKey);
                pdData.protocolDecodeStates = WAITING;
                return PROTOCOL_PARSED_DET_MESSAGE;
            } else {
                // if not COO, HIT, CHA, or DET -- fail
                pdData.protocolDecodeStates = WAITING;
                return PROTOCOL_PARSING_FAILURE;
            }
        } else {
            // if no newline found, fail
            pdData.protocolDecodeStates = WAITING;
            return PROTOCOL_PARSING_FAILURE;
        }
        break;
    }
    return PROTOCOL_PARSING_GOOD;
}

void ProtocolGenerateNegotiationData(NegotiationData *data)
{
    // guess and key are gonna be 16 bit random numbers (lilend)
    data->guess = rand() & 0x0000FFFF;
    data->encryptionKey = rand() & 0x0000FFFF;

    data->encryptedGuess = data->guess ^ data->encryptionKey;

    // get the individual bytes of the guesses and the key to store in temporary variables
    uint8_t guessFirstByte = data->guess >> 8;
    uint8_t guessSecondByte = data->guess & 0x00FF;
    uint8_t encryptFirstByte = data->encryptionKey >> 8;
    uint8_t encryptSecondByte = data->encryptionKey & 0x00FF;

    // the XOR ops that makes the hash
    data->hash = guessFirstByte ^ guessSecondByte;
    data->hash ^= encryptFirstByte;
    data->hash ^= encryptSecondByte;

}

uint8_t ProtocolValidateNegotiationData(const NegotiationData *data)
{

    // by default we assume they sent us bad data
    uint8_t result = FALSE;

    // decrypts the guess using the given key
    uint32_t decryptGuess = data->encryptedGuess ^ data->encryptionKey;

    // get the individual bytes of the guesses and the key to store in temporary variables
    uint8_t guessFirstByte = decryptGuess >> 8;
    uint8_t guessSecondByte = decryptGuess & 0x00FF;
    uint8_t encryptFirstByte = data->encryptionKey >> 8;
    uint8_t encryptSecondByte = data->encryptionKey & 0x00FF;

    // the XOR ops that makes the hash
    uint32_t ourHash = guessFirstByte ^ guessSecondByte;
    ourHash ^= encryptFirstByte;
    ourHash ^= encryptSecondByte;


    if (ourHash == data->hash) {
        result = TRUE;
    }
    return result;
}

TurnOrder ProtocolGetTurnOrder(const NegotiationData *myData, const NegotiationData *oppData)
{
    int turnChecker;

    // XOR the guesses
    turnChecker = myData->guess ^ oppData->guess;

    // If LSB is 1 then largest guess starts
    if (turnChecker & 0x1) {
        if (myData->guess > oppData->guess) {
            return TURN_ORDER_START;
        } else if (myData->guess < oppData->guess) {
            return TURN_ORDER_DEFER;
        } else {
            // Tie
            return TURN_ORDER_TIE;
        }
        // If LSB is 0 then smallest guess starts
    } else {
        if (myData->guess < oppData->guess) {
            return TURN_ORDER_START;
        } else if (myData->guess > oppData->guess) {
            return TURN_ORDER_DEFER;
        } else {
            // Tie
            return TURN_ORDER_TIE;
        }
    }
}

//Helper Functions

static uint8_t CheckSumCreate(char *payload)
{
    //unsigned char is 8 bits
    uint8_t sum = 0;

    int payIndex;
    for (payIndex = 0; payload[payIndex] != '\0'; payIndex++) {
        //XOR everything

        sum ^= payload[payIndex];
    }

    return sum;
}

static uint8_t AsciiToByte(char value)
{
    uint8_t result;

    //makes sure it is a number
    if (value >= '0' && value <= '9') {
        //convert
        result = value - ASCII_NUM_OFFSET;

        //or if hexchar
    } else if (value >= 'a' && value <= 'f') {
        //convert
        result = value - ASCII_HEXCHAR_OFFSET;

    } else {
        //if not valid hex, return invalid flag number
        return INVALID_CHAR;
    }
    return result;
}