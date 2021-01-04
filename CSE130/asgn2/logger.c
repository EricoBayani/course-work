#include "logger.h"


void entry_init(Entry* entry, uint8_t entry_name[], ThreadArgs* args,
                struct httpObject* msg){

#if BAD_DEBUG        
        printf(DEBUG_STR "Initializing log entry\n");
#endif    
  ssize_t total_space; 
  total_space = calculate_total_space(entry, msg->content_length, strlen((char*)entry_name));
  entry->lines_sofar = 0;
  entry->log_fd = args->log_fd;
  entry->chopped = false;

  int log_lock = pthread_mutex_lock(args->log_mut);
  entry->log_offset = *(args->global_log_offset);
  *(args->global_log_offset) += total_space;
  log_lock = pthread_mutex_unlock(args->log_mut);

#if BAD_DEBUG        
        printf(DEBUG_STR "Log entry initialized\n");
#endif  
  
  
}
uint32_t calculate_total_space(Entry* entry, ssize_t length, ssize_t entry_name_size){
  
  uint32_t total_size;
  uint32_t whole_lines;
  uint32_t unfilled_line;
  if(length > 0){
  whole_lines = length / BYTES_PER_LINE;
  unfilled_line = length % BYTES_PER_LINE;
  entry->total_whole_lines = whole_lines;
  entry->partial_line = unfilled_line;  

  if(unfilled_line > 0){
    total_size = (whole_lines * (FORMATTED_LINE_LENGTH(BYTES_PER_LINE))) + (FORMATTED_LINE_LENGTH(unfilled_line))
      + entry_name_size + SIGNATURE_SIZE;
  }
  else{
    total_size = (whole_lines * (FORMATTED_LINE_LENGTH(BYTES_PER_LINE))) 
      + entry_name_size + SIGNATURE_SIZE;
  }
  
  }

  else{
    total_size = entry_name_size + SIGNATURE_SIZE;
  }
  return total_size;
  
}
void create_entry_name(struct httpObject* message, int status_code, char* entry_name){

#if BAD_DEBUG        
        printf(DEBUG_STR "Writing Log Name\n");
#endif      

        //  static char entry_name[LOG_ENTRY_NAME_LENGTH];
  memset(entry_name, '\0', LOG_ENTRY_NAME_LENGTH);

  if (status_code >399){
    sprintf(entry_name, "FAIL: %s /%s %s --- response %d\n",
            message->method, message->filename, message->httpversion, status_code);
  }
  else{
    sprintf(entry_name, "%s /%s length %ld\n",
            message->method, message->filename, message->content_length);
  }
  

  //  return entry_name;
}


// write to the log correctly
void logwriter(Entry* entry, ssize_t bytes_written, uint8_t* buffer){

#if BAD_DEBUG        
        printf(DEBUG_STR "Writing to log\n");
#endif
  
  ssize_t bytes_logged = 0;
  uint8_t line[200];
  ssize_t log_written_now = 0;
  while(bytes_logged < bytes_written){ 


    bytes_logged += bytes_loader(entry, bytes_logged, bytes_written, buffer);
    if(entry->chopped) break;
    //    bytes_logged += BYTES_PER_LINE;    
    memset(line, '\0', sizeof(line));
    sprintf((char*)line,"%08d ", entry->lines_sofar * BYTES_PER_LINE);
    char formatted_bytes[70];
    fill_bytes_line(entry, formatted_bytes);
    strncat((char*) line, formatted_bytes, strlen(formatted_bytes));

#if BAD_DEBUG        
    printf(DEBUG_STR "Line written: %s\n", (char*) line);
#endif    
    log_written_now = pwrite(entry->log_fd, line, strlen((char*) line),entry->log_offset);
    entry->log_offset += log_written_now;

#if BAD_DEBUG        
    printf(DEBUG_STR "bytes logged: %ld vs bytes written: %ld\n", bytes_logged, bytes_written);
#endif    
   
  }//while(bytes_logged < bytes_written);
  

}
void fill_bytes_line(Entry* entry, char* bytes_string){

#if BAD_DEBUG        
    printf(DEBUG_STR "Filling bytes_string\n");
#endif      
  uint8_t byte[BYTE_FORMAT_SIZE+2];
  //  static uint8_t bytes_string[70];

    
  memset(bytes_string, '\0', 70);

  int i;
  
  if (entry->lines_sofar < entry->total_whole_lines){

#if BAD_DEBUG        
    printf(DEBUG_STR "Filling whole_line\n");
#endif    
    for (i = 0; i < BYTES_PER_LINE-1; i++){

      sprintf((char* ) byte, "%02x ", entry->bytes[i]);
      strncat(bytes_string, (char* ) byte, 4);
        
    }
  sprintf(byte,"%02x\n", entry->bytes[i]);
  strncat((char* ) bytes_string, (char* ) byte, 4);    
  entry->lines_sofar++;  
  }
  else{
#if BAD_DEBUG        
    printf(DEBUG_STR "Filling partial_line\n");
#endif
    if (!entry->partial_line){return;}
    for (i = 0; i < entry->last_index-1; i++){
#if BAD_DEBUG        
      printf(DEBUG_STR "Filling partial_line from index %d of bytes until %d (last index)\n", i, entry->last_index);
#endif              
      sprintf((char* )byte, "%02x ", entry->bytes[i]);
      strncat((char*) bytes_string, (char* )  byte, 4);
        
    }
  sprintf(byte,"%02x\n", entry->bytes[i]);
  strncat((char* ) bytes_string, (char* ) byte, 4);
  }

  


  
#if BAD_DEBUG        
  printf(DEBUG_STR "wrote %d of %d whole_lines\n" , entry->lines_sofar, entry->total_whole_lines);
#endif
  
#if BAD_DEBUG        
    printf(DEBUG_STR "bytes_string filled\n");
#endif        
  //  return (char* ) bytes_string;
  
}
uint8_t   bytes_loader(Entry* entry, ssize_t logged, ssize_t written, uint8_t buffer[]){

#if BAD_DEBUG        
    printf(DEBUG_STR "Filling bytes\n");
#endif      


    uint8_t i_logged;
  if (!entry->chopped) {
    memset(entry->bytes, 0, BYTES_PER_LINE);
#if BAD_DEBUG        
    printf(DEBUG_STR "not chopped\n");
#endif      

    for(i_logged = 0; i_logged < BYTES_PER_LINE; i_logged++){
      if((logged + i_logged) >= written){

#if BAD_DEBUG        
        printf(DEBUG_STR "%ld >= %ld\n",(logged + i_logged) , written);
#endif
        entry->last_index = i_logged;

#if BAD_DEBUG        
        printf(DEBUG_STR "last index is now %d\n",(i_logged));
#endif        
        if(entry->lines_sofar != entry->total_whole_lines){
          entry->chopped = true;
          
          break;
          
        }
        //entry->last_index = 0;
        break;
      }
      entry->bytes[i_logged] = buffer[logged+i_logged];
      /* entry->bytes[i_logged] = buffer[i_logged+(logged-entry->last_index)]; */
    }
    
  }
  
  else {
    uint8_t j = 0;
    for(i_logged = entry->last_index; i_logged < BYTES_PER_LINE; i_logged++){
      entry->bytes[i_logged] = buffer[j++];
      /* entry->bytes[i_logged] = buffer[logged+(i_logged-entry->last_index)]; */
      
    }
    entry->last_index = 0;
    entry->chopped = false;
    i_logged = j;
  }

  return i_logged;
}

// write the end string of a log entry
void write_entry_name(Entry* info, char* entry_name){
  
  uint8_t name[LOG_ENTRY_NAME_LENGTH];
  strncpy((char*) name, entry_name, strlen(entry_name));
  ssize_t to_write;
  to_write = pwrite(info->log_fd, name, strlen(entry_name), info->log_offset);
  info->log_offset += to_write;
  
}
void write_entry_sig(Entry* info){

  uint8_t sign[SIGNATURE_SIZE];
  strncpy((char*) sign, SIGN_STRING, SIGNATURE_SIZE);
  pwrite(info->log_fd, sign, SIGNATURE_SIZE, info->log_offset);
  
}

// report the health of the server
void health(int log_fd, ssize_t end_offset, char* healthcheck){


#if BAD_DEBUG        
    printf(DEBUG_STR "Creating healthcheck response\n");
#endif        
  
  //  static char healthcheck[HEALTHCHECK_SIZE];
  memset(healthcheck, '\0', HEALTHCHECK_SIZE);

  uint8_t read_buffer[BUFFER_SIZE];
  ssize_t bytes_read = 0;
  ssize_t to_read;

  ssize_t total_requests = 0;
  ssize_t failed_requests = 0;

  uint8_t* search_ptr = NULL;
  while(bytes_read < end_offset){
    to_read = pread(log_fd, read_buffer, BUFFER_SIZE, bytes_read);
    if (to_read < 0){
      warn("health");
      exit(1);
    }
    bytes_read += to_read;

    search_ptr = strstr((char*) read_buffer, SIGN_STRING);
    while(search_ptr != NULL){
      total_requests++;
      search_ptr = strstr(search_ptr+SIGNATURE_SIZE, SIGN_STRING);
    }
    search_ptr = strstr((char*) read_buffer, "FAIL: ");
    while(search_ptr != NULL){
      failed_requests++;
      search_ptr = strstr(search_ptr+6, "FAIL: ");
    }
  }

  sprintf(healthcheck, "%ld\n%ld", failed_requests, total_requests);

  //  return healthcheck;
}
