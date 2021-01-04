// load_balancer.c, contains multithread stuff

#include "load_work.h"

// defines from:
// https://www.man7.org/linux/man-pages/man2/timer_create.2.html
#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

// static variables global to this file

static OptFlag nConns = DEFAULT_CONNECTIONS; // use default number of threads
static OptFlag nReqs = DEFAULT_REQUESTS;
static OptFlag probe = 0; // comapred against nReqs to trigger a probe


static ssize_t total_requests = 0; // fluff

static uint16_t nServers;
static ServerInfo servers[MAX_SERVERS]; // the max amount of servers is the largest unsigned 16-bit integer
static int32_t best_port;


static timer_t timerid;
static struct sigevent sev;
static struct itimerspec its;
//static long long freq_nanosecs;// not needed?
static sigset_t mask;
static struct sigaction sa;
void init_timer();
void start_timer();
void stop_timer();
static void handler(int sig, siginfo_t *si, void *uc);

Request_Queue* init_queue(OptFlag max_requests){
  
  Request_Queue* Requests;
  Requests = malloc(sizeof(Request_Queue));
  Requests->head = NULL;
  Requests->last = NULL;
  Requests->current_size = 0;
  Requests->max_size = max_requests; // placeholder
  
  return Requests;
}

// code straight lifted from
// http://www.cs.kent.edu/~ruttan/sysprog/lectures/multi-thread/thread-pool-server.c
// modified to fit this assignment
void add_request(int new_client_fd, uint16_t port, Request_Queue* requests,
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
    probe++; // increment the probe no matter what
    
    // maybe I should just reject requests passed the parallel connections threshold? 
        while(requests->current_size >= requests->max_size){
    //  rc = pthread_cond_signal(pend_req);
          printf("Waiting for for queue to empty\n");
        }
    /* lock the mutex, to assure exclusive access to the list */
    rc = pthread_mutex_lock(req_mut);

    //    if (requests->current_size >= requests->max_size) rc = pthread_cond_signal(pend_req);

    a_request->request_id = ++total_requests;
    a_request->client_fd = new_client_fd;
    a_request->prio_port = port;
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
    
#ifdef BAD_DEBUG
    printf(DEBUG_STR "There are %ld requests now\n", requests->current_size);
    fflush(stdout);
#endif
    /* unlock mutex */
    //rc = pthread_cond_signal(pend_req);              
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
Request* get_request(Request_Queue* requests, pthread_mutex_t* req_mut, pthread_cond_t* pend_req)
{
    int rc;	                    /* return code of pthreads functions.  */
    Request* a_request;      /* pointer to request.                 */

#ifdef BAD_DEBUG
    printf(DEBUG_STR "Getting request\n");
    fflush(stdout);
#endif /* DEBUG */            

    
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

#ifdef BAD_DEBUG
    printf(DEBUG_STR "There are %ld requests left\n", requests->current_size);
    fflush(stdout);
#endif /* DEBUG */            
    if (requests->current_size >= requests->max_size) rc = pthread_cond_signal(pend_req);
    
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

    
    while (true) {
    
      // get request goes here
      if(requests->current_size > 0){
        request = get_request(requests, req_mut, pend_req);

        }

      if (request){
      rc = pthread_mutex_unlock(req_mut);

      int connectport = request->prio_port;
      int server_fd;
      
      if ((server_fd = client_connect(connectport)) < 0)
        err(1, "failed connecting");

      int client_fd = request->client_fd;

      bridge_loop(client_fd, server_fd, thread_id);

      
      rc = pthread_mutex_lock(req_mut);
      }
      // the mutex is unlocked during the run of this function...
      printf("Thread is waiting for new connection...\n");

      if (requests->current_size > 0){
        rc = pthread_cond_signal(pend_req);
#ifdef BAD_DEBUG        
        printf(DEBUG_STR "Thread %d signalled another thread\n", thread_id);
#endif        
      }
      rc = pthread_cond_wait(pend_req, req_mut);

    

#ifdef BAD_DEBUG
          printf(DEBUG_STR"thread '%d' after pthread_cond_wait\n", thread_id);
          fflush(stdout);
#endif /* DEBUG */          
        }
}

    


// helper for parsing cli arguments
// shamelessly stolen from one of Clark's section videos
int parse_args(int argc, char** argv){

  int exit_value = 0;
  OptFlag opt;
  while(true){
    opt = getopt(argc, argv, "N:R:");
    switch(opt){
    case -1: // done
      return exit_value;
      break;
    case 'N': // number of connections
      nConns = atoi(optarg);
      break;
    case 'R': 
      nReqs = atoi(optarg);
      break;
    default:
      exit_value = -1; // bad argument
      break;
    }
  }
  
  return exit_value;
}


void find_best_port(uint16_t nServers){

        probe = 0; // reset the probing variable

      // reset the timer that would count
      
      uint8_t check_servers = 0;
      for(check_servers = 0; check_servers < nServers; check_servers++){
        // establish a connection with each server using
        // modified bridgeloop


        int health_client_fd = client_connect(servers[check_servers].port);        
        if (health_client_fd < 0){
          servers[check_servers].dead = true;
#ifdef BAD_DEBUG
          printf(DEBUG_STR "Server on port %d is dead\n", servers[check_servers].port);
          fflush(stdout);
#endif /* DEBUG */
          continue;
      
        }

        // start the GET healthcheck correspondence
        health_loop(health_client_fd, &servers[check_servers]);
        close(health_client_fd);
        
      }

      // find the best server based on the healthcheck probes
      best_port = best_server(servers, nServers);
#ifdef BAD_DEBUG
      printf(DEBUG_STR "The best port is %d\n", best_port);
      fflush(stdout);
#endif /* DEBUG */   

}

// handler and set up code taken from
// https://www.man7.org/linux/man-pages/man2/timer_create.2.html
// adapted to catch an alarm signal every X seconds, reset probe,
// find best port, and reset timer. 



static void handler(int sig, siginfo_t *si, void *uc){
  
#ifdef BAD_DEBUG
  printf("Caught signal %d\n", sig);
#endif
  find_best_port(nServers);
  start_timer();
  
  //  signal(sig, SIG_IGN);
}

void init_timer(){
  /* Establish handler for timer signal */

  printf("Establishing handler for signal %d\n", SIG);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIG, &sa, NULL) == -1)
    err(1, "sigaction");

  /* Create the timer */

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIG;
  sev.sigev_value.sival_ptr = &timerid;
  if (timer_create(CLOCKID, &sev, &timerid) == -1)
    err(1, "timer_create");
  
}

void start_timer(){
  
  its.it_value.tv_sec = SECONDS_UNTIL_PROBE;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;
  if (timer_settime(timerid, 0, &its, NULL) == -1)
    err(1, "timer_settime");
}
void stop_timer(){
  
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;
  if (timer_settime(timerid, 0, &its, NULL) == -1)
    err(1, "timer_settime");
}

int main(int argc, char** argv) {
  
  printf("Hello Asg3!\n");
  // put code here

  //check number of command-line arguments
  if (argc < 2){
    fprintf(stderr, "Usage: loadbalancer [-N #ofParallelConnections default = 4] [-R #requestsBeforeHealthProbe, default 5] [BALANCER PORT] [HTTPSERVER PORT] ...\n");
    exit(1);
  }
  if (0 > parse_args(argc, argv)){
    fprintf(stderr, "Bad Arguments\n");
    exit(1);
   }
  char* balancer_port = argv[optind++];

#ifdef BAD_DEBUG
  printf(DEBUG_STR "This loadbalancer is on port %s with %d parallel connections\n"
         DEBUG_STR "after %d requests, A health check probe is sent to servers on ports:\n",
         balancer_port, nConns, nReqs);
#endif

  // initalize the ServerInfo array

  nServers = argc - optind;
  //ServerInfo servers[nServers];
  for (uint8_t ports = 0; ports < nServers; ports++){
#ifdef BAD_DEBUG
  printf(DEBUG_STR"%s\n", argv[optind]);
#endif
  servers[ports].port = atoi(argv[optind++]);
  servers[ports].total_requests = 0;
  servers[ports].failed_requests = 0;
  servers[ports].dead = false;
  }
  
  // this code is from https://stackoverflow.com/a/7963765, used to
  // initialize a recursive mutex without using the macro, 
  pthread_mutex_t Request_Mutex;
  pthread_mutexattr_t Request_Mutex_attr; // mutex attribute

  pthread_mutexattr_init(&Request_Mutex_attr);
  pthread_mutexattr_settype(&Request_Mutex_attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&Request_Mutex, &Request_Mutex_attr);

  pthread_cond_t Pending_Request;
  pthread_cond_init(&Pending_Request, NULL); // default cond variable

  
  Request_Queue* Requests = init_queue(nReqs);

  uint32_t nThreads = nConns*nServers;
  
  pthread_t threads[nThreads];

  for (uint32_t i = 0; i < nThreads; i++){
    
    ThreadArgs* ArgsPtr = malloc(sizeof(ThreadArgs));
    ArgsPtr->id = i;
    ArgsPtr->requests = Requests;
    ArgsPtr->req_mut = &Request_Mutex;
    ArgsPtr->pend_req = &Pending_Request;

    pthread_create(&threads[i], NULL, worker_func, (void *) ArgsPtr);
    
  }

  // Create sockaddr_in with server information
  
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(atoi(balancer_port));
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
    err(1, "Failed listening");
    return EXIT_FAILURE;
  }

  // Connecting with a client
  struct sockaddr client_addr;
  socklen_t client_addrlen = sizeof(client_addr); // gleaned from discord grond and jake armendariz

  init_timer();
  start_timer();
  find_best_port(nServers);
  while(true){
    // IF not time to probe
  
    if(probe != nReqs){
    
      //port = servers[0].port; // take out when health probing is done
      /* while(Requests->current_size >= Requests->max_size){ */
      /*   pthread_cond_signal(&Pending_Request); */
      /* } */
      int client_sockd = accept(server_sockd, &client_addr, &client_addrlen);
      if (client_sockd < 0){
        warn("client_sockd:");
        close(client_sockd);
        start_timer();
        continue;
      } else if(best_port <= 0){
        // send a 500
#ifdef BAD_DEBUG
        printf(DEBUG_STR "Sending 500...\n");
        fflush(stdout);
#endif /* DEBUG */        
        char* internal_error = INTERNAL_ERROR;
        send(client_sockd, internal_error, strlen(internal_error), 0);
        continue;
      }
      else{ // add a request if nothing else is bad
        
      
#ifdef BAD_DEBUG
        printf(DEBUG_STR "Accepted a Request\n");
        fflush(stdout);
#endif /* DEBUG */
            
        add_request(client_sockd, best_port, Requests,  &Request_Mutex, &Pending_Request);
        
      }

    }
    // ELSE probe
    else{
      stop_timer();
      find_best_port(nServers);
      start_timer();
    }
    
  }

  return 0;
}
