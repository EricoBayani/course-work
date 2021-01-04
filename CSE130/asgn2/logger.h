// logger.h, contains data structures for multithreading and request queue

/* #include <sys/socket.h> */
/* #include <sys/stat.h> */
/* #include <stdio.h> */
/* #include <netinet/in.h> */
/* #include <netinet/ip.h> */
/* #include <fcntl.h> */
/* #include <unistd.h> // write */
/* #include <string.h> // memset */
/* #include <stdlib.h> // atoi */
/* #include <stdbool.h> // true, false */

/* #include <err.h> // for warn */
/* #include <errno.h> */
/* #include <pthread.h> */


#include "server_work.h"
/* #include "httpserver.h" */

#define BYTES_PER_LINE 20
#define DIGITS_OF_LINE 8   // 8 digits of decimal
#define BYTE_FORMAT_SIZE 3 // 2 hex digits + space pr \n
#define LINE_FORMATTERS 1 // space after digit
#define FORMATTED_LINE_LENGTH(BYTES) (BYTES * BYTE_FORMAT_SIZE) + DIGITS_OF_LINE + LINE_FORMATTERS
#define SIGNATURE_SIZE 9 // 8 equalsigns + 1 \n
#define SIGN_STRING "========\n"
#define LOG_ENTRY_NAME_LENGTH 200
#define HEALTHCHECK_SIZE 43 // digits of largest unsigned integer * 2 + \n


typedef struct Log_Entry_Info {
  
  uint32_t total_whole_lines;
  uint32_t lines_sofar;
  uint32_t partial_line;
  int chopped;
  uint8_t bytes[BYTES_PER_LINE];
  uint32_t last_index;
  ssize_t log_offset;
  int log_fd;
  
} Entry;


// initialize entry struct
void entry_init(Entry* entry, uint8_t entry_name[], ThreadArgs* args,
                struct httpObject* msg);
uint32_t calculate_total_space(Entry* entry, ssize_t length, ssize_t entry_name_size);
void create_entry_name(struct httpObject* message, int status_code, char* entry_name);


// write to the log correctly
void logwriter(Entry* entry, ssize_t bytes_written, uint8_t* buffer);
void fill_bytes_line(Entry* info, char* bytes_string);
uint8_t bytes_loader(Entry* entry, ssize_t logged, ssize_t written, uint8_t buffer[]);

// write the end string of a log entry
void write_entry_name(Entry* info, char* entry_name);
void write_entry_sig(Entry* info);

// report the health of the server
void health(int log_fd, ssize_t end_offset, char* healthcheck);
