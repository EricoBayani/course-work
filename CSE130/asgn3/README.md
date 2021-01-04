# README
## To Run:
1. Run cmake in the directory that contains `loadbalancer.c and .h`, and ` load_work.c and .h`. The program needs to be linked with -lpthread and -lrt to work properly.
2. `Usage: loadbalancer [-N #ofParallelConnections default = 4] [-R #requestsBeforeHealthProbe, default 5] [BALANCER PORT] [HTTPSERVER PORT] ...\n`
3. The balancer will run forever, trying to periodically, not in between busy requests, connect to its specifed PORTS. 

## Issues: 
1. The balancer doesn't reroute to a good server until the next probe, which happens either after 4 seconds or R total requests. I didn't realize this was counter to the expected behaviour. 
