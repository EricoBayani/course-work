// load_work.c

#include "load_work.h"



int client_connect(uint16_t connectport) {
    int connfd;
    struct sockaddr_in servaddr;

    connfd=socket(AF_INET,SOCK_STREAM,0);
    if (connfd < 0)
        return -1;
    memset(&servaddr, 0, sizeof servaddr);

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(connectport);

    /* For this assignment the IP address can be fixed */
    inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));

    if(connect(connfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
        return -1;
    return connfd;
}

/*
 * server_listen takes a port number and creates a socket to listen on 
 * that port.
 * port: the port number to receive connections
 * returns: valid socket if successful, -1 otherwise
 */

// This probably never gets called in my loadbalancer, since the
// server port is already opened in main. 
int server_listen(int port) {
    int listenfd;
    int enable = 1;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        return -1;
    memset(&servaddr, 0, sizeof servaddr);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
        return -1;
    if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof servaddr) < 0)
        return -1;
    if (listen(listenfd, 500) < 0)
        return -1;
    return listenfd;
}

/*
 * bridge_connections send up to 100 bytes from fromfd to tofd
 * fromfd, tofd: valid sockets
 * returns: number of bytes sent, 0 if connection closed, -1 on error
 */
int bridge_connections(int fromfd, int tofd, uint8_t thread_id) {
    uint8_t recvline[BUFFER_SIZE];
    int n = recv(fromfd, recvline, BUFFER_SIZE, 0);
    if (n < 0) {
        printf("connection error receiving\n");
        return -1;
    } else if (n == 0) {
      printf("receiving connection ended on thread %d\n", thread_id);
        return 0;
    } 
#ifdef BAD_DEBUG   
    /* recvline[n] = '\0'; */
    /* printf("%s", recvline); */
    /* sleep(1); */
#endif
    
    n = send(tofd, recvline, n, 0);
    if (n < 0) {
        printf("connection error sending\n");
        return -1;
    } else if (n == 0) {
      printf("sending connection ended on thread %d\n", thread_id);
        return 0;
    }
    return n;
}

/*
 * bridge_loop forwards all messages between both sockets until the connection
 * is interrupted. It also prints a message if both channels are idle.
 * sockfd1, sockfd2: valid sockets
 */
void bridge_loop(int client_fd, int server_fd, uint8_t id_number) {
    fd_set set;
    struct timespec timeout;
    //sigset_t sigmask; // not needed?
    

    int fromfd, tofd;
    while(1) {
        // set for select usage must be initialized before each select call
        // set manages which file descriptors are being watched
        FD_ZERO (&set);
        FD_SET (client_fd, &set);
        FD_SET (server_fd, &set);

        // same for timeout
        // max time waiting, 5 seconds, 0 microseconds
        timeout.tv_sec = 4;
        timeout.tv_nsec = 0;

        // select return the number of file descriptors ready for reading in set
        switch (pselect(FD_SETSIZE, &set, NULL, NULL, &timeout, NULL)) {
            case -1:
                printf("error during select, exiting\n");
                return;
            case 0:
              printf("a channel in thread %d are idle, sending 500 to client\n", id_number);
              char* internal_error = INTERNAL_ERROR;
              send(client_fd, internal_error, strlen(internal_error), 0);
              return;
            default:
                if (FD_ISSET(client_fd, &set)) {
                    fromfd = client_fd;
                    tofd = server_fd;
                } else if (FD_ISSET(server_fd, &set)) {
                    fromfd = server_fd;
                    tofd = client_fd;
                } else {
                    printf("this should be unreachable\n");
                    return;
                }
        }
        if (bridge_connections(fromfd, tofd, id_number) <= 0)
            return;
    }
}



void health_loop(int server_fd, ServerInfo* info) {
    fd_set set;
    struct timespec timeout;
    //sigset_t sigmask; // not needed?

    uint8_t buffer[BUFFER_SIZE];

    int tofd;

    ssize_t to_send;
    ssize_t to_recv;

    uint8_t mini_buffer[BUFFER_SIZE];
    
    memset(buffer, '\0', sizeof(buffer));
    memset(mini_buffer, '\0', sizeof(buffer));

    char* healthcheck = "GET /healthcheck HTTP/1.1\r\n\r\n";
    
    strncat((char*) buffer, healthcheck, strlen(healthcheck));


    // send the healthcheck
    to_send = send(server_fd, buffer, strlen(buffer)+1, 0);

    memset(buffer, 0, BUFFER_SIZE);
    
#ifdef BAD_DEBUG
    printf("Sent %ld bytes to server on port %d\n", to_send, info->port);
#endif
    
    if(to_send < 0){
      err(1, "problem sending request for healthcheck");
    }

    /* // getting the response */
    /* to_recv = recv(server_fd, buffer, BUFFER_SIZE, 0); */

    
    // check for timeout in getting response
    FD_ZERO (&set);
    FD_SET (server_fd, &set);

    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;

    switch (pselect(FD_SETSIZE, &set, NULL, NULL, &timeout, NULL)) {
    case -1:
      printf("error during select, exiting\n");
      return;
    case 0:
      printf("server on port %d is unresponsive\n", info->port);
      info->dead = true;
      return;
    default:
      if (FD_ISSET(server_fd, &set)) {
        printf("Got response from server on port %d\n", info->port);
      } else {
        printf("this should be unreachable\n");
        return;
      }
    }
    // getting the response
    //    to_recv = recv(server_fd, buffer, BUFFER_SIZE, FLAGS);
    
    do {
      to_recv = recv(server_fd, mini_buffer, BUFFER_SIZE, FLAGS);
        strcat((char*)buffer, (char*) mini_buffer);
    } while(to_recv);
#ifdef BAD_DEBUG
      printf(DEBUG_STR "Got response: %s\n", (char*)buffer);
#endif
    printf("Received some type of response\n");
    // check response
    if(to_recv < 0){
      err(1, "problem receiving response for healthcheck");
    }    

    if(strstr((char*) buffer, "200 OK") == NULL){
      printf("server on port %d is unresponsive\n",info->port);
      info->dead = true;
      return;
    }

    if(strstr((char*) buffer, "Content-Length: 0") != NULL){
      info->total_requests = 0;
      info->failed_requests = 0;
      info->dead = false;
      return;
    }
    
    /* FD_ZERO (&set); */
    /* FD_SET (server_fd, &set); */

    /* timeout.tv_sec = 1; */
    /* timeout.tv_nsec = 0; */
    
    /* switch (pselect(FD_SETSIZE, &set, NULL, NULL, &timeout, NULL)) { */
    /* case -1: */
    /*   printf("error during select, exiting\n"); */
    /*   return; */
    /* case 0: */
    /*   printf("server on port %d is unresponsive\n", info->port); */
    /*   info->dead = true; */
    /*   return; */
    /* default: */
    /*   if (FD_ISSET(server_fd, &set)) { */
    /*     printf("Got content from server on port %d\n", info->port); */
    /*   } else { */
    /*     printf("this should be unreachable\n"); */
    /*     return; */
    /*   } */
    /* } */


    /* memset(buffer, 0, sizeof(buffer)); */

    
    /* //    to_recv = recv(server_fd, buffer, BUFFER_SIZE, FLAGS); */
    /* do { */
    /*   to_recv = recv(server_fd, mini_buffer, BUFFER_SIZE, FLAGS); */
    /*     strcat((char*)buffer, (char*) mini_buffer); */
    /* } while(to_recv); */
    

    if(to_recv < 0){
      err(1, "problem receiving content for healthcheck");
    }

    /* else if(to_recv == 0){ */
    /*   info->total_requests = 0; */
    /*   info->failed_requests = 0; */
    /*   info->dead = false; */
    /*   return; */
    /* } */

    else {

#ifdef BAD_DEBUG
      printf(DEBUG_STR "Got Content: %s\n", (char*)buffer);
#endif
      
      // taken from my asgn1 and 2 code for processing requests
      uint8_t* empty_accum = NULL;
      uint8_t* line = strtok_r((char*) buffer, "\n", (char**)&empty_accum) ;
      ssize_t lines_count;
      uint32_t content[5];
      for (lines_count = 0; line != NULL; lines_count++) {

        content[lines_count] = atoi(line);
        line = strtok_r(NULL, "\n", (char**)&empty_accum);
      }

      info->total_requests = content[4];
      info->failed_requests = content[3];
      info->dead = false;

#ifdef BAD_DEBUG
      printf(DEBUG_STR "Port %d: %d failed, %d total\n", info->port, content[3], content[4]);
#endif
      
      return;
      
    }

    return;

    
}

int32_t best_server(ServerInfo servers[], uint8_t nServers){

#ifdef BAD_DEBUG
      printf(DEBUG_STR "Determining best server...\n");
#endif
  
  ServerInfo same_total[nServers];
  ServerInfo same_failed[nServers];
  int total_tie_index = 0;
  int failed_tie_index = 0;

  uint16_t dead_servers = 0;

  for(int i = 0; i < nServers; i++){
    if(servers[i].dead){
      dead_servers++;
      
    }
  }

  if(dead_servers == nServers){
    fprintf(stderr,"All servers are dead or unresponsive\n");
    return -1;
  }
  
  uint32_t min = servers[dead_servers].total_requests;
  
  for(int i = 0; i < nServers; i++){
    if((servers[i].total_requests <= min) && !servers[i].dead){
      min = servers[i].total_requests;
      same_total[total_tie_index++] = servers[i];
#ifdef BAD_DEBUG
      printf(DEBUG_STR "server on port %d is good so far...\n", same_total[i].port);
#endif      
    }
  }

  /* min = same_total[0].failed_requests; */

  /* for(int i = 0; i < total_tie_index; i++){ */
  /*   if(same_total[i].total_requests <= min){ */
  /*     min = same_total[i].failed_requests; */
  /*     same_failed[failed_tie_index++] = same_failed[i]; */
  /*   } */
  /* } */


  return same_total[0].port;
}
