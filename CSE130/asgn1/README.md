# README
### To Run:
- run cmake in the directory that contains `httpserver.c` and it's associated `Makefile`.
- ./httpserver [PORT NUMBER]
- The server will run forever assuming it can open a socket. 
### Issues:
- httpserver run in sudo mode will say that a requst on a file it shouldn't have permission to will succeed. 