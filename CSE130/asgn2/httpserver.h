// httpserver.h, contains data structures for multithreading and request queue

//#include "server_work.h"
#define DEFAULT_THREADS 4
#define MAX_REQUESTS DEFAULT_THREADS * 4
#define ARGS_CAST (ThreadArgs* )


// struct for request node
typedef struct Req_Node{
  
  uint32_t request_id;
  int client_fd;
  struct Req_Node* next;
  
} Request;

// struct for request queue
typedef struct Req_List{
  
  Request* head;
  Request* last;
  ssize_t current_size;
  ssize_t max_size;
  
} Request_Queue;

// struct containing the thread arguments
typedef struct T_Args {
  
  uint8_t id;
  Request_Queue* requests;
  pthread_mutex_t* req_mut;
  pthread_cond_t* pend_req;
  pthread_mutex_t* log_mut;
  ssize_t* global_log_offset;
  int log_fd;
  
} ThreadArgs;
