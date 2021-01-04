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

#define BAD_DEBUG  0 // My own debuf flag cuz I'm too lazy to explore better options

#define BUFFER_SIZE 4096

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define PERMS 0644 
#define FLAGS 0 // don't know any flags so far that would be useful

struct httpObject {
  /*
    Create some object 'struct' to keep track of all
    the components related to a HTTP message
    NOTE: There may be more member variables you would want to add
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
             PUT,
             HEAD,
             GET
}; // This enum makes adding new cases easier to check

uint8_t* status_message(int status_code){
  // the "string" here is static so I can avoid using heap memory
  static uint8_t status_message[BUFFER_SIZE+1]; // I'm not sure what the max length of descriptrions are
  memset(status_message, '\0', sizeof(status_message)); // refresh the message string after each use
  switch(status_code){
    // I counted the size of these messages
  case 200:
    strncpy((char*)status_message, "OK", 2); 
    break;
  case 201:
    strncpy((char*)status_message, "Created", 7); 
    break;
  case 400:
    strncpy((char*)status_message, "Bad Request", 11); 
    break;
  case 403:
    strncpy((char*)status_message, "Forbidden", 9); 
    break;
  case 404:
    strncpy((char*)status_message, "Not Found", 9); 
    break;    
  default: // the case when I don't know what went wrong 
    strncpy((char*)status_message, "Internal Server Error", 22); // 22 is the size of (Internal Server Error)
    break;
  }
  
  return status_message;
  
}

// a function to handle general reading/writing operation, that would have the same functionality as dog
void write_to_d(int read_from, int write_to, int method, uint32_t content_length){
  uint8_t buffer[BUFFER_SIZE];
    ssize_t to_read;
    if(read_from < 0 || write_to < 0) return; // bad file descriptors, or bad name, just don't do anything
    uint32_t bytes_read = 0;     
    switch (method) {      
    case (PUT):

      while (bytes_read != content_length){
        to_read = recv(read_from, buffer, BUFFER_SIZE, FLAGS);
        if (to_read < 0){
          warn("reading");
          break;
        }
#if BAD_DEBUG == 1        
        printf("%ld bytes left to write\n", to_read);
#endif        
        bytes_read += to_read;
        ssize_t to_write = write(write_to, buffer, to_read);
        if (to_write <= -1){
          warn("Writing");
          if (errno == ENOSPC) exit(1);
          break;
        }
      }
      break;
      
    case GET:
      while (bytes_read != content_length){
        to_read = read(read_from, buffer, BUFFER_SIZE);
        if (to_read < 0){
          warn("reading");
          break;
        }
#if BAD_DEBUG == 1        
        printf("%ld bytes left to write\n", to_read);
#endif        
        bytes_read += to_read;
        ssize_t to_write = send(write_to, buffer, to_read, FLAGS);
        if (to_write <= -1){
          warn("Writing");
          if (errno == ENOSPC) exit(1);
          break;
        }
      }                  
      break;
      
    default:
      printf("Frick\n");
      break; // do nothing
    }

  return;
}

uint8_t filename_checker(char* filename){

  ssize_t length = strlen(filename);
  if (length > 27 || length <= 0){
    return 1;
  }
    
  
  for (int i = 0; i < length; i++){
    // if not [[alphnum]-_]
    if (    !( (filename[i] == '-')
            || (filename[i] == '_')
/* #if BAD_DEBUG == 1 */
/*             || (filename[i] == '.') */
/* #endif */
            || (filename[i]>='0' && filename[i] <= '9')
            || (filename[i]>='a' && filename[i] <= 'z')
            || (filename[i]>='A' && filename[i] <= 'Z')   )){
      
      return filename[i];
      
    }
    
  }
  return 0;
}

// brief 1. Want to read in the HTTP message/ data coming in from socket
// param client_sockd - socket file descriptor
// param message - object we want to 'fill in' as we read in the HTTP message

void read_http_response(ssize_t client_sockd, struct httpObject* request) {
#if BAD_DEBUG == 1  
  printf("This function will take care of reading message\n");
#endif  

  //Start constructing HTTP request based off data from socket

  // First loop receives message pushed through given socket
  uint8_t read_buffer[BUFFER_SIZE];
    ssize_t to_recv;
    uint8_t message_buffer[BUFFER_SIZE];
    memset(message_buffer, '\0', sizeof(message_buffer)); // get rid of lingering msgs
    memset(read_buffer, '\0', sizeof(read_buffer)); // get rid of lingering msgs
    
    // Guarantees the entire message is read into my message reading buffer
    while ((strstr((char*) message_buffer, "\r\n\r\n") == NULL)){

      to_recv = recv(client_sockd, read_buffer, BUFFER_SIZE, FLAGS);
        strcat((char*)message_buffer, (char*) read_buffer);
    }
    
#if BAD_DEBUG == 1
    printf("The whole message after loop:\n%s\n"
           "This message was %ld bytes long, not including the null\n",
           message_buffer, strlen((char*)message_buffer));
#endif
    // now that the message is in the buffer, let's glean the info we need:   
    // now the entire message is in the buffer. I want to tokensize by line
    uint8_t* buffer_lines[BUFFER_SIZE];
    uint8_t* empty_accum = NULL;
    uint8_t* line = strtok_r((char*)message_buffer, "\r\n", (char**)&empty_accum) ;
    ssize_t lines_count;
    for (lines_count = 0; line != NULL; lines_count++) {

        buffer_lines[lines_count] = line;
        line = strtok_r(NULL, "\r\n", (char**)&empty_accum);
    }

    // the following pieces of code glean and populate the message I'm concerned with
    empty_accum = NULL;

    uint8_t* info = strtok_r((char*) buffer_lines[0], " ", (char**)&empty_accum); // split first line by space
    strncpy(request->method, info, 5); // first part is method
    info = strtok_r(NULL, " ", (char**)&empty_accum); // next token 
    strncpy(request->filename, info+1, strlen(info)); // next is filename
    info = strtok_r(NULL, " ", (char**)&empty_accum); // next
    strncpy(request->httpversion, info, strlen(info));// last is httpversion
    // loop through the lines I got to get content length
    for(int i = 1; i < lines_count; i++){
      if ((info = strstr((char*) buffer_lines[i], "Content-Length:")) != NULL){
        info = strchr((char*) info, ' ')+1;
        request->content_length = atoi(info);
        break;
      }
    }
    

    
    return;
}


// brief 2. Want to process the message we just recieved

int process_request(struct httpObject* request, struct httpObject* response) {
#if BAD_DEBUG == 1
  printf("Processing Request\n");
#endif
  int descriptor = -2; ; // default to an error other than -1
  // check what message we're working with

  if ((filename_checker(request->filename)) != 0){
    response->status_code = 400; // bad request
    response->content_length = 0;
    return descriptor;
  }
  
  if (strcmp(request->method, "PUT") == 0){
    request->status_code = PUT;
    struct stat check;
        
    int exists = stat(request->filename, &check);
    if (exists < 0){
      switch(errno){
      case ENOENT:
        response->status_code = 201; // create if file doesn't exist
        descriptor = creat(request->filename, PERMS);
        
        break;
     /* case EACCES: */
     /*   response->status_code = 403; // access denied */
     /*   break; */
     /* case EBADF: */
     /*   response->status_code = 403; // access denied */
     /*   break; */
     /* case EFAULT: */
     /*   response->status_code = 403; // access denied */
     /*   break;         */
      default:
        response->status_code = 400; // bad request ... generic error
        if (!(check.st_mode & S_IWUSR)) response->status_code = 403;// check explicitly if we have perms
        break;
      }
      
    }
    else{
      response->status_code = 200;
      descriptor = open(request->filename, O_WRONLY, PERMS);
    }
    
  }
  

    

 else if (strcmp(request->method, "GET") == 0 ){
#if BAD_DEBUG == 1
   printf("processing GET\n");
#endif
   request->status_code = GET;
   struct stat check;
   memset(&check, 0, sizeof(check));
   
  
   int exists = stat(request->filename, &check);
   if (exists < 0){
     switch(errno){
     case ENOENT:
       response->status_code = 404; // file isn't found
       break;
     case EACCES:
       response->status_code = 403; // access denied
       break;
     case EBADF:
       response->status_code = 403; // access denied
       break;
     case EFAULT:
       response->status_code = 403; // access denied
       break;       
     default:
       response->status_code = 400; // bad request ... generic error
       if (!(check.st_mode & S_IRUSR)) response->status_code = 403; // check explicitly if we have perms
       break;
     }
     perror(request->filename);
     
   }
   else{
     descriptor = open(request->filename, O_RDONLY, PERMS);
     response->status_code = 200;
     response->content_length = check.st_size; // seems like you can stat as whoever you want     
   }
   
 }

  else if (strcmp(request->method, "HEAD") == 0 ){
    request->status_code = HEAD;
    struct stat check;
           
    int exists = stat(request->filename, &check);
    if (exists < 0){
     switch(errno){
     case ENOENT:
       response->status_code = 404; // file isn't found
       break;
     /* case EACCES: */
     /*   response->status_code = 403; // access denied */
     /*   break; */
     /* case EBADF: */
     /*   response->status_code = 403; // access denied */
     /*   break; */
     /* case EFAULT: */
     /*   response->status_code = 403; // access denied */
     /*   break;        */
     default:
       response->status_code = 400; // bad request ... generic error
       break;
     }
     perror(request->filename);
    }
    else {
      /* descriptor = open(request->filename, O_RDONLY, S_IRUSR); */
      response->status_code = 200;    
      response->content_length = check.st_size;
      /* close(descriptor); */
    }
    
  }
  else{ // unsupported method
    request->status_code = 500; // internal server error, generic error on my end
  }

  return descriptor;
}


// brief 3. Construct some response based on the HTTP request you recieved

void construct_http_response(struct httpObject* request, struct httpObject* response) {
  #if BAD_DEBUG == 1
  printf("Constructing Response\n");
  #endif
  memset(response->buffer, '\0', sizeof(response->buffer));
  sprintf((char*) response->buffer, "%s %d %s\r\nContent-Length: %ld\r\n\r\n",
          request->httpversion,
          response->status_code,
          (char*) status_message(response->status_code),
          response->content_length);

  
  return;
}




int main(int argc, char** argv) {
  
  // Create sockaddr_in with server information

  //check number of command-line arguments
  if (argc != 2){
    fprintf(stderr, "Usage: httpserver [PORT]\n");
    exit(1);
  }
  char* port = argv[1];
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(port));
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  socklen_t addrlen = sizeof(server_addr);

  // create server socket
  int server_sockd = socket(AF_INET, SOCK_STREAM, 0);

  // Need to check if server_sockd < 0, meaning an error
  if (server_sockd < 0) {
    perror("socket");
  }

  // configure server socket
  int enable = 1;

  // This allows you to avoid: 'Bind: Address Already in Use' error
  int ret = setsockopt(server_sockd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

  //  Bind server address to socket that is open
  ret = bind(server_sockd, (struct sockaddr *) &server_addr, addrlen);

  // Listen for incoming connections
  ret = listen(server_sockd, 5); // 5 should be enough, if not use SOMAXCONN

  if (ret < 0) {
    return EXIT_FAILURE;
  }

  // Connecting with a client
  struct sockaddr client_addr;
  socklen_t client_addrlen = sizeof(client_addr); // gleaned from discord grond and jake armendariz

  struct httpObject request_message;
  struct httpObject response_message;
  //  enum methods do;
  while (true) {
    printf("[+] server is waiting...\n");

    // 1. Accept Connection
    int client_sockd = accept(server_sockd, &client_addr, &client_addrlen);
    // Errors WILL happen, meaning do a continue after checking
    if (client_sockd < 0){
      warn("client_sockd:");
      // close bad desrciptor possible? 
      close(client_sockd);
      continue;
    }

    // 2. Read http message
    memset(&request_message, 0, sizeof(request_message));
    memset(&response_message, 0, sizeof(response_message));
    read_http_response(client_sockd, &request_message);

#if BAD_DEBUG == 1
    printf("The message I got after reading http request: concerned with is:\n"
           "method: %s\n"
           "filename: %s\n"
           "httpversion: %s\n"
           "content length: %ld\n",
           // no status code in request,
           request_message.method, request_message.filename,request_message.httpversion,
           request_message.content_length);
#endif

    
    // 3. Process Request
    int descriptor = process_request(&request_message, &response_message);

    
    // 4. Construct Response
    construct_http_response(&request_message, &response_message);
  
    // 5. Send Response

    ssize_t to_write;
    
    switch(request_message.status_code){
      
    case PUT:
      write_to_d(client_sockd, descriptor, PUT, request_message.content_length);     
      to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);
      
      break;
    case GET:
      to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);      
      write_to_d(descriptor, client_sockd, GET, response_message.content_length);
      
      break;
    case HEAD:
      // don't write
      to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);      
      
      break;
    default:
      break;
    }
    
    close(descriptor);
    close(client_sockd);
    printf("Response Sent\n");

    
  }

  return EXIT_SUCCESS;
}
