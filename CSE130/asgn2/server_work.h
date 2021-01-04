// Starter code taken from Michael C's section repo
// adapted for Erico Bayani's httpserver

#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <unistd.h> // write
#include <string.h> // memset
#include <stdlib.h> // atoi
#include <stdbool.h> // true, false

#include <err.h> // for warn
#include <errno.h>
#include <pthread.h>

#include "httpserver.h"
//#include "logger.h"

#define BAD_DEBUG  1 // My own debuf flag cuz I'm too lazy to explore better options
#define DEBUG_STR "##DEBUG##: " // the string I want at the beginning of my debug print statements

#define BUFFER_SIZE 4096

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define PERMS 0644 
#define FLAGS 0 // don't know any flags so far that would be useful



// typedefs for pesky type names
typedef int8_t OptFlag;
typedef pthread_mutex_t* MutexPtr;
typedef pthread_cond_t* CondPtr;


struct httpObject {
  /*
    Create some object 'struct' to keep track of all
    the components related to a HTTP message
    NOTE: There may be more member variables you would want to add
    NOTE from Erico: I overloaded the crap out of this thing...
  */
  char method[5];         // PUT, HEAD, GET
  char filename[28];      // what is the file we are worried about
  char httpversion[9];    // HTTP/1.1
  ssize_t content_length; // example: 13
  int status_code;        // this variable I will say has two purposes
                          // if it's request, populate iwth my enum value. If it's a response, populate it with
                          // the actual status code. 
  uint8_t buffer[BUFFER_SIZE];
};

enum methods{
             ERROR,
             PUT,
             HEAD,
             GET,
             HEALTH
             
}; // This enum makes adding new cases easier to check


// *_response helper functions


void status_message(int status_code, uint8_t* status_string);


//
void write_to_d(int read_from, int write_to, int method, struct httpObject* req_msg, struct httpObject* resp_msg,
                ThreadArgs* Args, OptFlag logging);


//
uint8_t filename_checker(char* filename);


// server actions
void read_http_response(ssize_t client_sockd, struct httpObject* request);
int process_request(struct httpObject* request, struct httpObject* response, ThreadArgs* args);
void construct_http_response(struct httpObject* request, struct httpObject* response);

