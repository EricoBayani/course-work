// By Erico Bayani (ebayani@ucsc.edu) and Venkat Vetsa (vvetsa@ucsc.edu)

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdio>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

// includes needed to work
#include <sys/stat.h> 
#include <regex>
#include <iomanip>
#include <cstring>
#include <fstream>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
  {"exit", cix_command::EXIT},
    {"help", cix_command::HELP},
      {"get", cix_command::GET},
        {"put", cix_command::PUT},
          {"rm", cix_command::RM},
            {"ls"  , cix_command::LS},
              // ack nak? 
              };

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
  cout << help;
}

void cix_ls (client_socket& server) {
  // create the header for command
  cix_header header;
  header.command = cix_command::LS;
  // send the header
  send_packet (server, &header, sizeof header);
  // wait for a packet and receive
  recv_packet (server, &header, sizeof header);
  outlog << "received header " << header << endl;
   
  // do something with the header given

  // if header isnt expected, print an error message
  // and do nothing
  if (header.command != cix_command::LSOUT) {
    outlog << "sent LS, server did not return LSOUT" << endl;
    outlog << "server returned " << header << endl;
  }
  // else accept the payload using a buffer 
  else {
    // recieve the packet according to its info
    size_t host_nbytes = ntohl (header.nbytes);
    auto buffer = make_unique<char[]> (host_nbytes + 1);
    recv_packet (server, buffer.get(), host_nbytes);
    // output the result of the command, if needed
    buffer[host_nbytes] = '\0';
    cout << buffer.get();
  }
}

// To Erico: Useful snippets found in textfile Snips

// I gotta make command for put get rm
// put: create a file on the server named filename
void cix_put (client_socket& server, string commandInput){

  // get the filename from command string

  string name;
  if(commandInput.length() < 3){
    cout << "ERROR: No files specifed" << endl;
    return;
  }
  name = commandInput.substr(2);
  if (name.find(' ') != string::npos){
    cout << 
"ERROR: Too many files specifed, only 1 file allowed" 
<< endl;
    return;
  }
  else if (name.find('\\') != string::npos || 
   name.find('/') != string::npos){
    cout << "ERROR: invalid character in name" << endl;
    return;
  }
  else if (name.length() >= 58) {
    cout << "ERROR: filename too long" << endl;
    return;
  }

  // create the header for command
  cix_header header;
  memset (header.filename, 0, FILENAME_SIZE);  
  header.command = cix_command::PUT;

  struct stat stat_buf;
  int status = stat (name.c_str(), &stat_buf);
  if (status != 0) {
    cout << "ERROR: " << name << " doesn't exist" << endl;
    return;
  }
  strcpy(header.filename, name.c_str());
  if (stat_buf.st_size <= 0) {
      
    header.nbytes = 0;
    send_packet (server, &header, sizeof header);
    
  }
  else {
    //create the payload

    ifstream file(name);
   
    string file_contents;
    char buffer[0x1000];

    file.read(buffer, stat_buf.st_size);

    file_contents = buffer; 
    header.nbytes = htonl(file_contents.size());
    // send the header
    send_packet (server, &header, sizeof header);
    send_packet (server, file_contents.c_str(), file_contents.size());
  }
  
  // wait for a packet and receive
  recv_packet (server, &header, sizeof header);
  outlog << "received header " << header << endl;
   
  // do something with the header given
  if (header.command != cix_command::ACK) {
    outlog << "sent PUT, server did not return ACK" << endl;
    outlog << "server returned " << header << endl;
  }

}


void cix_get (client_socket& server, string commandInput){

  string name;
  if(commandInput.length() < 3){
    cout << "ERROR: No files specifed" << endl;
    return;
  }
  name = commandInput.substr(2);
  if (name.find(' ') != string::npos){
    cout << 
"ERROR: Too many files specifed, only 1 file allowed" 
<< endl;
    return;
  }
  else if (name.find('\\') != string::npos || 
  name.find('/') != string::npos){
    cout << "ERROR: invalid character in name" << endl;
    return;
  }
  else if (name.length() >= 58) {
    cout << "ERROR: filename too long" << endl;
    return;
  }

  // create the header for command
  cix_header header;
  header.command = cix_command::GET;
  strcpy(header.filename, name.c_str());
   
  // send the header
  send_packet (server, &header, sizeof header);

  // wait for a packet and receive

  recv_packet (server, &header, sizeof header);
  outlog << "received header " << header << endl;
   
  // do something with the header given

  // if header isnt expected, print an error message
  // and do nothing
  if (header.command != cix_command::FILEOUT) {
    outlog << "sent GET, server did not return FILEOUT" << endl;
    outlog << "server returned " << header << endl;
  }
  // else accept the payload using a buffer 
  else {
    // same filewriting scheme from cixd
    ofstream putFile(header.filename, ofstream::out|ofstream::trunc);
    if (!putFile) {
      outlog << "failed to open file for get" << endl;
    }

    // read data from socket
    size_t payloadSize = ntohl(header.nbytes);
    if (payloadSize == 0) {
      putFile.close();
    }
    else{
      auto buffer = make_unique<char[]>(payloadSize+1);
      recv_packet(server, buffer.get(), payloadSize);

      // write data to file
      putFile << buffer.get();
      putFile.close();
    }
  }     
}

void cix_rm (client_socket& server, string commandInput){

  // resolve name based on substring of commandline argument
  string name;
  if(commandInput.length() < 2){
    cout << "ERROR: No files specifed" << endl;
    return;
  }
  name = commandInput.substr(2);
  if (name.find(' ') != string::npos){
    cout << 
"ERROR: Too many files specifed, only 1 file allowed" 
<< endl;
    return;
  }
  else if (name.find('\\') != string::npos || 
  name.find('/') != string::npos){
    cout << "ERROR: invalid character in name" << endl;
    return;
  }
  else if (name.length() >= 58) {
    cout << "ERROR: filename too long" << endl;
    return;
  }
  // create the header for command
  cix_header header;
  header.command = cix_command::RM;
  strcpy(header.filename, name.c_str());
   
  // send the header
  send_packet (server, &header, sizeof header);
   
  // wait for a packet and receive
  recv_packet (server, &header, sizeof header);
  outlog << "received header " << header << endl;
   
  // do something with the header given

  if (header.command != cix_command::ACK) {
    outlog << "sent RM, server did not return ACK" << endl;
    outlog << "server returned " << header << endl;
  }
  // if header isnt expected, print an error message
  // and do nothing

}

void usage() {
  cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
  throw cix_exit();
}

int main (int argc, char** argv) {
  outlog.execname (basename (argv[0]));
  outlog << "starting" << endl;
  vector<string> args (&argv[1], &argv[argc]);
  if (args.size() > 2) usage();
  string host = get_cix_server_host (args, 0);
  in_port_t port = get_cix_server_port (args, 1);
  outlog << to_string (hostinfo()) << endl;
  try {
    outlog << "connecting to " << host << " port " << port << endl;
    client_socket server (host, port);
    outlog << "connected to " << to_string (server) << endl;
    for (;;) {
      string line;
      getline (cin, line);
      if (cin.eof()) throw cix_exit();
      const auto& itor1 = command_map.find (line.substr(0,2));
      const auto& itor2 = command_map.find (line.substr(0,3));
      const auto& itor3 = command_map.find (line.substr(0,4));
      cix_command cmd;
      if (itor3 != command_map.end()){
        cmd = itor3->second;
      }
      else if (itor2 != command_map.end()){
        cmd = itor2->second;
      }
      else if (itor1 != command_map.end()){
        cmd = itor1->second;
      }
      else {
        cmd = cix_command::ERROR;
      }
      switch (cmd) {
      case cix_command::EXIT:
        throw cix_exit();
        break;
      case cix_command::HELP:
        cix_help();
        break;
      case cix_command::LS:
        cix_ls (server);
        break;
      case cix_command::PUT:
        cix_put (server, line.substr(2));
        break;
      case cix_command::GET:
        cix_get (server, line.substr(2));
        break;
      case cix_command::RM:
        cix_rm (server, line.substr(1));
        break;                              
      default:
        outlog << line << ": invalid command" << endl;
        break;
      }
    }
  }catch (socket_error& error) {
    outlog << error.what() << endl;
  }catch (cix_exit& error) {
    outlog << "caught cix_exit" << endl;
  }
  outlog << "finishing" << endl;
  return 0;
}

