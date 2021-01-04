// main.cpp by Erico Bayani (ebayani@ucsc.edu)
// and Venkat Vetsa (vvetsa@ucsc.edu)

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>

#include <regex>
#include <fstream>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
  opterr = 0;
  for (;;) {
    int option = getopt (argc, argv, "@:");
    if (option == EOF) break;
    switch (option) {
    case '@':
      debugflags::setflags (optarg);
      break;
    default:
      complain() << "-" << char (optopt) << ": invalid option"
                 << endl;
      break;
    }
  }
}

// void work_keyvalue(string& line)
// work_keyvalue will take in a line reference from a file 
// and parse through it using a
// regex, doing list map operations based on what it finds. 
void work_keyvalue(str_str_map& map, string& line){
  // regex lines taken from matchlines.cpp
  
  // looks for # at some point after a lot of whitespace
  regex comment_regex {R"(^\s*(#.*)?$)"};
  // looks for anything with a = in the middle
  regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
  // looks for no equal sign
  regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};

  smatch result;

  if (regex_search (line, result, comment_regex)) {
    // do nothing when finding a comment or empty line
  }
  else if (regex_search (line, result, key_value_regex)) {


    if (result[1].length() && result[2].length()){
      // logic for inserting pair
      str_str_pair pair(result[1], result[2]);
      map.insert(pair);
      str_str_map::iterator query = map.find(result[1]);
      cout << *query << endl;
    }
    else if (not result[1].length() && not result[2].length()){
      // logic for printing list
      for(auto query : map){
        cout << query << endl;
      }

    }
    else if (result[1].length() && not result[2].length()){
      // logic for deleting key and value

      str_str_pair pair(result[1], result[2]);
      str_str_map::iterator query = map.find(result[1]);
      map.erase(query);
    }
    else if (not result[1].length() && result[2].length()){
      // logic for printing keys with same value
      for(auto query : map){
        if(query.second == result[2]){
          cout << query << endl;
        }
      }
    }
    // this really shouldn't happen
    else{
      cout << "This shouldn't happen" << endl;
    }

    
  }
  else if (regex_search (line, result, trimmed_regex)) {
    // logic goes here for printing value from key
    str_str_map::iterator query = map.find(result[1]);
    if (query == map.end()){
      cout << result[1] << ": "<< "key not found" << endl;
    }
    else {
      cout << *query << endl;
    }
  }
}


// this void function will work on the filenames passed through it, 
// and will redirect to cin
// when necessary
void work_file (str_str_map& map, string filename){
  string line;
  if (filename == "-"){
    // do cin stuff
    for(;;){
      cout << filename << endl;
      cin.clear();
      getline (cin, line);
      if (cin.eof()) {
        cout << "^D";
        cout << endl;
        DEBUGF ('y', "EOF");
        
        break;
      }
      work_keyvalue(map, line); // option handling goes here
    }
  }
  else {
    ifstream file(filename);
    do {
      // this is where the line working is
      cout << filename << endl;
      work_keyvalue(map, line); // option handling goes here
    } while ( getline (file,line) );
    file.close();
  }
  
}

// needs to accept file arguments: open files, print line by line, 
// use regex to sift through
// line.
int main (int argc, char** argv) {
  sys_info::execname (argv[0]);
  scan_options (argc, argv);

  str_str_map test;
  // this loop handles the command line arguments   
  for (char** argp = &argv[optind]; argp != &argv[argc]; ++argp) {
    work_file(test, *argp);
    str_str_pair pair (*argp, to_string<int> (argp - argv));
  }

  cout << "EXIT_SUCCESS" << endl;
  return EXIT_SUCCESS;
}

