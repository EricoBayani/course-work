# README
## To Run:
1. Run cmake in the directory that contains `httpserver.c and .h`, `server_work.c and .h`, and `logger.c and .h`.
2. `./httpserver [-N NUMBER OF THREADS, default is 4] [-l NAME OF LOG FILE] [PORT NUMBER]`
3. The server will run forever if it can open a socket

## Issues: 
1. If too many requests hit the server at the same time, there is a chance that the server locks up. I haven't been able to identify why. 
2. The server runs a lot slower with logging enabled. 
