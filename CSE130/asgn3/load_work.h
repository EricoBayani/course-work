// load_work.h

#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h> // write
#include <string.h> // memset
#include <stdlib.h> // atoi
#include <stdbool.h> // true, false

#include <err.h> // for warn
#include <errno.h>
#include <pthread.h>

#include <signal.h>
#include <time.h>

#include "loadbalancer.h"
//#include "logger.h"

#define BAD_DEBUG  1 // My own debuf flag cuz I'm too lazy to explore better options
#define DEBUG_STR "##DEBUG##: " // the string I want at the beginning of my debug print statements

#define INTERNAL_ERROR "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n"
#define MAX_SERVERS 65535

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


/* struct httpObject { */
/*   /\* */
/*     Create some object 'struct' to keep track of all */
/*     the components related to a HTTP message */
/*     NOTE: There may be more member variables you would want to add */
/*     NOTE from Erico: I overloaded the crap out of this thing... */
/*   *\/ */
/*   char method[5];         // PUT, HEAD, GET */
/*   char filename[28];      // what is the file we are worried about */
/*   char httpversion[9];    // HTTP/1.1 */
/*   ssize_t content_length; // example: 13 */
/*   int status_code;        // this variable I will say has two purposes */
/*                           // if it's request, populate iwth my enum value. If it's a response, populate it with */
/*                           // the actual status code.  */
/*   uint8_t buffer[BUFFER_SIZE]; */
/* }; */

typedef struct server_info{
  
  uint16_t port; // identifying port
  uint32_t total_requests; // first metric of health, lower is better
  uint32_t failed_requests; // second metric of health, lower is better
  int dead; // true if healthcheck request is not good eg not 200, malformed, no response
  
} ServerInfo;

int client_connect(uint16_t connectport);
int server_listen(int port);
int bridge_connections(int fromfd, int tofd, uint8_t thread_id);



void bridge_loop(int client_fd, int server_fd, uint8_t thread_id);

//int bridge_health(int fromfd, int tofd);
void health_loop(int server_fd, ServerInfo* info);

int32_t best_server(ServerInfo servers[], uint8_t nServers);
