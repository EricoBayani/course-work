// dog.c by Erico Bayani created at 4-10-20
// I got the read/write and open loop from K&R


// includes I got from from K&R chapter 8.2-8.3
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <string.h>

#define PERMS 0666 // from K&R chapter 8.2-8.3

#define STDIN 0
#define STDOUT 1
#define STDERR 2


int main(int argc, char* argv[]){

  int exit_code = EXIT_SUCCESS;

  if (argc == 1) {
    uint8_t buffer[BUFSIZ];
    ssize_t to_read;
    while ((to_read = read(STDIN, buffer, BUFSIZ)) > 0){
      if (to_read <= -1) {
        warn("stdin:");
        exit_code = EXIT_FAILURE;
        break;
      }
      ssize_t to_write = write(STDOUT, buffer, to_read);
      if (to_write <= -1){
        warn("stdin:");
        exit_code = EXIT_FAILURE;
        break;
      }
    }
  }
  
  for (uint8_t i = argc - 1; i >= 1; i--){
    
	int dsctr;
    if (*argv[i] == '-') dsctr = STDIN;
    else dsctr = open(argv[i], O_RDONLY, PERMS);

    if (dsctr <= -1) {
      warn("%s", argv[i]);
      exit_code = EXIT_FAILURE;
      continue;
    }
    
    uint8_t buffer[BUFSIZ];
    ssize_t to_read;
    while ((to_read = read(dsctr, buffer, BUFSIZ)) > 0){
      if (to_read <= -1) {
        warn("%s", argv[i]);
        exit_code = EXIT_FAILURE;
        break;
      }
      ssize_t to_write = write(STDOUT, buffer, to_read);
      if (to_write <= -1){
        warn("%s", argv[i]);
        exit_code = EXIT_FAILURE;
        if (errno == ENOSPC) exit(1);
        break;
      }
    }
    
    if (dsctr == STDIN) continue; // keeps program from closing STDIN
    int to_close = close (dsctr);
    if (to_close <= -1){
     warn("%s", argv[i]);
     exit_code = EXIT_FAILURE;
     continue;
    }
  }

  return exit_code;
}
