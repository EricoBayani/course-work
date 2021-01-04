// httpserver.c, contains multithread stuff

//#include "httpserver.h"
#include "server_work.h"


// static variables global to this file

static OptFlag logging = false; // no logging by default
static char* log_name = "N/A";
static OptFlag nThreads = DEFAULT_THREADS; // use default number of threads

static ssize_t total_requests = 0;

Request_Queue* init_queue(){
  
  Request_Queue* Requests;
  Requests = malloc(sizeof(Request_Queue));
  Requests->head = NULL;
  Requests->last = NULL;
  Requests->current_size = 0;
  Requests->max_size = MAX_REQUESTS;
  
  return Requests;
}

// code straight lifted from
// http://www.cs.kent.edu/~ruttan/sysprog/lectures/multi-thread/thread-pool-server.c
// modified to fit this assignment
void add_request(int new_client_fd, Request_Queue* requests,
                 pthread_mutex_t* req_mut, pthread_cond_t* pend_req){
  
    int rc;	                    /* return code of pthreads functions.  */
    Request* a_request;      /* pointer to newly added request.     */

    /* create structure with new request */
    a_request = malloc(sizeof(Request));
    if (!a_request) { /* malloc failed?? */
	fprintf(stderr, "add_request: out of memory\n");
	exit(1);
    }

#ifdef BAD_DEBUG
    	    printf(DEBUG_STR "Adding Request\n");
    	    fflush(stdout);
#endif /* DEBUG */        
    while(requests->current_size > requests->max_size){}
    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(req_mut);

    a_request->request_id = ++total_requests;
    a_request->client_fd = new_client_fd;
    a_request->next = NULL;
    
    /* add new request to the end of the list, updating list */
    /* pointers as required */
    if (requests->current_size == 0) { /* special case - list is empty */
	requests->head = a_request;
	requests->last = a_request;
    }
    else {
	requests->last->next = a_request;
	requests->last = a_request;
    }
    requests->current_size++;
    

    /* unlock mutex */
    rc = pthread_mutex_unlock(req_mut);
    
    /* signal the condition variable - there's a new request to handle */
    rc = pthread_cond_signal(pend_req);  
#ifdef BAD_DEBUG
    	    printf(DEBUG_STR "Successfully added a request\n");
    	    fflush(stdout);
#endif /* DEBUG */  
  
}


// code straight lifted from
// http://www.cs.kent.edu/~ruttan/sysprog/lectures/multi-thread/thread-pool-server.c
// modified to fit this assignment
Request* get_request(Request_Queue* requests, pthread_mutex_t* req_mut)
{
    int rc;	                    /* return code of pthreads functions.  */
    Request* a_request;      /* pointer to request.                 */

    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(req_mut);

    if (requests->current_size > 0) {
	a_request = requests->head;
	requests->head = a_request->next;
	if (requests->head == NULL) { /* this was the last request on the list */
	    requests->last = NULL;
	}
	/* decrease the total number of pending requests */
	requests->current_size--;
    //    a_request->request_id = total_requests;
    }
    else { /* requests list is empty */
	a_request = NULL;
    }

    /* unlock mutex */
    rc = pthread_mutex_unlock(req_mut);

    /* return the request to the caller. */
    return a_request;
}


void* worker_func(void* Args){
  
    int rc;	                    /* return code of pthreads functions.  */
    Request* request;
    ThreadArgs* args = ARGS_CAST Args;
    
    uint8_t thread_id = args->id;  /* thread identifying number           */
    Request_Queue* requests = args->requests;
    MutexPtr req_mut = args->req_mut;
    CondPtr pend_req = args->pend_req;
    MutexPtr log_mut = args->log_mut;
    ssize_t* global_log_offset = args->global_log_offset;
    int log_fd = log_fd;
    
    

#ifdef BAD_DEBUG
    printf(DEBUG_STR "Starting thread '%d'\n", thread_id);
    fflush(stdout);
#endif 

    /* lock the mutex, to access the requests list exclusively. */
    rc = pthread_mutex_lock(req_mut);

#ifdef BAD_DEBUG
    printf(DEBUG_STR "thread '%d' after pthread_mutex_lock\n", thread_id);
    fflush(stdout);
#endif 

    struct httpObject request_message;
    struct httpObject response_message;
    
    while (true) {
    
    
#ifdef BAD_DEBUG
    	printf(DEBUG_STR "thread '%d' is fetching request\n", thread_id);
    	//fflush(stdout);
#endif 
        if (requests->current_size > 0) { /* a request is pending */
          request = get_request(requests, req_mut);
          if (request) { /* got a request - handle it and free it */

#ifdef BAD_DEBUG
            printf(DEBUG_STR "thread '%d' is now handling request #%d\n", thread_id, request->request_id);
    	//fflush(stdout);
#endif
        
            rc = pthread_mutex_unlock(req_mut); // unlock the mutex to let other threads access the queue 
        int client_sockd = request->client_fd;

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
            int descriptor = process_request(&request_message, &response_message, args);
            // 4. Construct Response
            construct_http_response(&request_message, &response_message);
            // 5. Send Response
            ssize_t to_write;
            switch(request_message.status_code){
      
            case PUT:
              write_to_d(client_sockd, descriptor, PUT, &request_message, &response_message, args, logging);
              to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);
      
              break;
            case GET:
              to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);
              write_to_d(descriptor, client_sockd, GET, &request_message, &response_message, args, logging);
      
              break;
            case HEAD:
              // don't write
              to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);
              write_to_d(descriptor, client_sockd, HEAD, &request_message, &response_message, args, logging);
      
              break;
            case HEALTH:
              to_write = send(client_sockd, response_message.buffer, strlen(response_message.buffer), FLAGS);
              write_to_d(descriptor, client_sockd, HEALTH, &request_message, &response_message, args, logging);
            default:
              break;
            }
    
            close(descriptor);
            close(client_sockd);

#if BAD_DEBUG    
            printf(DEBUG_STR "Response Sent\n");
#endif
            free(request);
            rc = pthread_mutex_lock(req_mut); // loch the mutex again
          }

        }
        else {
          // wait for a signal here
          printf("[+] server_thread %d is waiting...\n", thread_id);
#ifdef BAD_DEBUG
          printf(DEBUG_STR"thread '%d' before pthread_cond_wait\n", thread_id);
          fflush(stdout);
#endif /* DEBUG */
            
          // the mutex is unlocked during the run of this function...
          rc = pthread_cond_wait(pend_req, req_mut);

          /* rc = pthread_mutex_unlock(req_mut); */

#ifdef BAD_DEBUG
          printf(DEBUG_STR"thread '%d' after pthread_cond_wait\n", thread_id);
          fflush(stdout);
#endif /* DEBUG */          
        }
    }

    //return;
}

// helper for parsing cli arguments
// shamelessly stolen from one of Clark's section videos
int parse_args(int argc, char** argv){

  int exit_value = 0;
  OptFlag opt;
  while(true){
    opt = getopt(argc, argv, "N:l:");
    switch(opt){
    case -1: // done
      return exit_value;
      break;
    case 'N': // number of threads
      nThreads = atoi(optarg);
      break;
    case 'l': // we logging?
      logging = true;
      log_name = optarg;
      break;
    default:
      exit_value = -1; // bad argument
      break;
    }
  }
  

  return exit_value;
}



int main(int argc, char** argv) {
  
  printf("Hello Asg2!\n");
  // put code here

  
  

  //check number of command-line arguments
  if (argc < 2){
    fprintf(stderr, "Usage: httpserver -l [LOG_NAME] -N [NUMBER OF THREADS, default 4] [PORT]\n");
    exit(1);
  }
  if (0 > parse_args(argc, argv)){
    fprintf(stderr, "Bad Arguments\n");
    exit(1);
   }
  char* port = argv[optind];

#ifdef BAD_DEBUG
  printf(DEBUG_STR "This server is on port %s using %d threads\n" DEBUG_STR "Logging is %s, logging to file %s\n",
         port, nThreads, logging ? "ON":"OFF", log_name );
#endif
  
  // Use the info from the commandline arguments to create the threads

  // 1: Create sync devices in this function's scope
  
  // this code is from https://stackoverflow.com/a/7963765, used to
  // initialize a recursive mutex without using the macro, since
  // the recursive mutex was mentioned in the multithreading tutuorial
  // from the spec
  pthread_mutex_t Request_Mutex;
  pthread_mutexattr_t Request_Mutex_attr; // mutex attribute

  pthread_mutexattr_init(&Request_Mutex_attr);
  pthread_mutexattr_settype(&Request_Mutex_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&Request_Mutex, &Request_Mutex_attr);

  pthread_cond_t Pending_Request;
  pthread_cond_init(&Pending_Request, NULL); // default cond variable


  // 1.5: open the log file if active, set the global log offset
  int Log_FD = -1;
  ssize_t main_log_offset = 0;
  pthread_mutex_t Log_Mutex; // declared
  if (logging == true) {
    pthread_mutex_init(&Log_Mutex, NULL); // initialize default mutex for log
    Log_FD = open(log_name, O_RDWR | O_CREAT | O_TRUNC, PERMS);
  }

  // 2: allocate and initialize the list
  Request_Queue* Requests = init_queue();

  // 3: Allocate and Populate the ThreadArgs struct and create the worker threads
  pthread_t threads[nThreads];

  for (uint8_t i = 0; i < nThreads; i++){
    
    ThreadArgs* ArgsPtr = malloc(sizeof(ThreadArgs));
    ArgsPtr->id = i;
    ArgsPtr->requests = Requests;
    ArgsPtr->req_mut = &Request_Mutex;
    ArgsPtr->pend_req = &Pending_Request;
    ArgsPtr->log_mut = &Log_Mutex;
    ArgsPtr->global_log_offset = &main_log_offset;
    ArgsPtr->log_fd = Log_FD;
    pthread_create(&threads[i], NULL, worker_func, (void *) ArgsPtr);
    
  }

  // Create sockaddr_in with server information
  
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

  while(true){
  // 1. Accept Connection
    int client_sockd = accept(server_sockd, &client_addr, &client_addrlen);
    // Errors WILL happen, meaning do a continue after checking
    if (client_sockd < 0){
      warn("client_sockd:");
      // close bad desrciptor possible?
      close(client_sockd);
      continue;
    }
#ifdef BAD_DEBUG
    	    printf(DEBUG_STR "Accepted a Request\n");
    	    fflush(stdout);
#endif /* DEBUG */    
    add_request(client_sockd, Requests, &Request_Mutex, &Pending_Request);
  }

  return 0;
}
