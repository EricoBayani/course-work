//#include "server_work.h"
#include "logger.h"


void status_message(int status_code, uint8_t* status_string){
  // the "string" here is static so I can avoid using heap memory
  //  uint8_t status_message // I'm not sure what the max length of descriptrions are
  //memset(status_message, '\0', BUFFER_SIZE/2); // refresh the message string after each use
  switch(status_code){
    // I counted the size of these messages
  case 200:
    strncpy((char*)status_string, "OK", 2); 
    break;
  case 201:
    strncpy((char*)status_string, "Created", 7); 
    break;
  case 400:
    strncpy((char*)status_string, "Bad Request", 11); 
    break;
  case 403:
    strncpy((char*)status_string, "Forbidden", 9); 
    break;
  case 404:
    strncpy((char*)status_string, "Not Found", 9); 
    break;    
  default: // the case when I don't know what went wrong 
    strncpy((char*)status_string, "Internal Server Error", 22); // 22 is the size of (Internal Server Error)
    break;
  }
  
  //return status_message;
  
}

// a function to handle general reading/writing operation, that would have the same functionality as dog
//void write_to_d(int read_from, int write_to, int method, uint32_t content_length){
void write_to_d(int read_from, int write_to, int method, struct httpObject* req_msg, struct httpObject* resp_msg,
                ThreadArgs* Args, OptFlag logging){

  
  uint8_t buffer[BUFFER_SIZE];
    ssize_t to_read;
    /* if(method != HEAD){ // if not head */
    /*   if(read_from < 0 || write_to < 0) return; // bad file descriptors, or bad name, just don't do anything */
    /* } */
    uint32_t bytes_read = 0;
    ssize_t content_length;

    Entry* entry = (Entry* ) malloc(sizeof(*entry));
    uint8_t entry_name[LOG_ENTRY_NAME_LENGTH];
      // write the log_entry name here
    if (logging == true){
#if BAD_DEBUG        
        printf(DEBUG_STR "Starting logging\n");
#endif    
      memset(entry_name, '\0', LOG_ENTRY_NAME_LENGTH);
      /* strncpy((char*) entry_name, create_entry_name(req_msg, resp_msg->status_code), LOG_ENTRY_NAME_LENGTH); */
      create_entry_name(req_msg, resp_msg->status_code, (char*) entry_name);
    }
      
    
    switch (method) {      
    case (PUT):
      content_length = req_msg->content_length;
      if (logging == true){
        //        memset(&entry, '\0', sizeof(entry));
        entry_init(entry, entry_name,  Args, req_msg);
        write_entry_name(entry, entry_name);
      }
      while (bytes_read != content_length){
        if(read_from < 0 || write_to < 0) break; // bad file descriptors, or bad name, just don't do anything
        to_read = recv(read_from, buffer, BUFFER_SIZE, FLAGS);
        if (to_read < 0){
          warn("reading");
          break;
        }
#if BAD_DEBUG        
        printf("%ld bytes left to write\n", to_read);
#endif        
        bytes_read += to_read;
        ssize_t to_write = write(write_to, buffer, to_read);
        if (to_write <= -1){
          warn("Writing");
          if (errno == ENOSPC) exit(1);
          break;
        }

        // log write goes here
        if (logging == true){
          logwriter(entry, to_write, buffer);
        }
        
      }
      break;
      
    case GET:
      content_length = resp_msg->content_length;
      
      if (logging == true){
        //        memset(entry, '\0', sizeof(entry));
        entry_init(entry, entry_name, Args, resp_msg);

        write_entry_name(entry, entry_name);
      }
      
      while (bytes_read != content_length){
        if(read_from < 0 || write_to < 0) break; // bad file descriptors, or bad name, just don't do anything
        
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

        // log write goes here
        if (logging == true){
          logwriter(entry, to_write, buffer);
        }
        
      }                  
      break;

    case HEAD:

      if (logging == true){
        //        memset(&entry, '\0', sizeof(entry));
        entry_init(entry, entry_name, Args, req_msg);
        write_entry_name(entry, entry_name);
      }
      
      if (logging == true){
      
      }
      
    break;
    case HEALTH:
      if (logging == true){
          entry_init(entry, entry_name, Args, resp_msg);
          write_entry_name(entry, entry_name);
          ssize_t to_write = send(write_to, req_msg->buffer, strlen(req_msg->buffer), FLAGS);
          if (to_write <= -1){
            warn("Writing");
            if (errno == ENOSPC) exit(1);
            break;
          }
            
          // log write goes here
          
            logwriter(entry, to_write, req_msg->buffer);
              
          }
           
          
        
      break;

      
    default:
      printf("Frick\n");
      break; // do nothing
    }


    // write the log entry signature here
    if (logging == true){
      write_entry_sig(entry);
    }
    //    free(entry->bytes);
    free(entry);
    
    
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
/* #if BAD_DEBUG == 1   */
/*   printf("This function will take care of reading message\n"); */
/* #endif   */

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
    
/* #if BAD_DEBUG == 1 */
/*     printf("The whole message after loop:\n%s\n" */
/*            "This message was %ld bytes long, not including the null\n", */
/*            message_buffer, strlen((char*)message_buffer)); */
/* #endif */
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

int process_request(struct httpObject* request, struct httpObject* response, ThreadArgs* args) {
#if BAD_DEBUG == 1
  printf("Processing Request\n");
#endif
  int descriptor = -2; ; // default to an error other than -1
  // check what message we're working with


  
  if (strcmp(request->method, "PUT") == 0){
    request->status_code = PUT;
    struct stat check;
    if ((filename_checker(request->filename)) != 0){
      response->status_code = 400; // bad request
      response->content_length = 0;
      request->content_length = 0;
      return descriptor;
    }
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
        if (!(check.st_mode & S_IWOTH)) response->status_code = 403;// check explicitly if we have perms
        break;
      }
      
    }
    else{
      response->status_code = 200;
      descriptor = open(request->filename, O_WRONLY | O_TRUNC, PERMS);
    }
    
  }
  

    

 else if (strcmp(request->method, "GET") == 0 ){
#if BAD_DEBUG == 1
   printf("processing GET\n");
#endif
   request->status_code = GET;

    if ((filename_checker(request->filename)) != 0){
      response->status_code = 400; // bad request
      
      response->content_length = 0;
      request->content_length = 0;

      return descriptor;
    }

    if((strcmp(request->filename,"healthcheck") == 0 ) && (args->log_fd < 0)) {

      request->status_code = HEALTH;
      response->status_code = 200;
      memset(request->buffer, 0, BUFFER_SIZE);
#if BAD_DEBUG == 1
   printf("processing HEALTHCHECK\n");
#endif            
      health(args->log_fd, *(args->global_log_offset), (char*)request->buffer);
      response->content_length = strlen((char*)request->buffer);
      request->content_length = strlen((char*)request->buffer);
      
      return descriptor;  
    }
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
       if (!(check.st_mode & S_IROTH)) response->status_code = 403; // check explicitly if we have perms
       break;
     }
     perror(request->filename);
     
   }
   else{
     
     descriptor = open(request->filename, O_RDONLY, PERMS);
     response->status_code = 200;
     response->content_length = check.st_size; // seems like you can stat as whoever you want
     request->content_length = check.st_size;
     
   }
   
 }

  else if (strcmp(request->method, "HEAD") == 0 ){
    request->status_code = HEAD;

    if ((filename_checker(request->filename)) != 0){
      response->status_code = 400; // bad request
      response->content_length = 0;
      request->content_length = 0;
      return descriptor;
    }    
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
       if (!(check.st_mode & S_IROTH)) response->status_code = 403; // check explicitly if we have perms
       break;
     }
     perror(request->filename);
    }
    else {
      /* descriptor = open(request->filename, O_RDONLY, S_IRUSR); */
      response->status_code = 200;    
      response->content_length = check.st_size;
      request->content_length = 0;
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
  uint8_t* status_string = calloc(1,BUFFER_SIZE/2);
  status_message(response->status_code, status_string);
  memset(response->buffer, '\0', sizeof(response->buffer));
  sprintf((char*) response->buffer, "%s %d %s\r\nContent-Length: %ld\r\n\r\n",
          request->httpversion,
          response->status_code,
          (char*) status_string ,
          response->content_length);
  #if BAD_DEBUG ==1
  printf("%s", (char*) response->buffer);
  #endif
  free(status_string);
  return;
}



