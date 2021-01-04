// By Erico Bayani (ebayani@ucsc.edu) and Venkat Vetsa (vvetsa@ucsc.edu)

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");

   inode_ptr fresh (new inode());
   root = fresh;
   cwd = fresh;
   path = vector<string>();
}

const string& inode_state::prompt() const { return prompt_; }


ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}
inode::inode() {
  next_inode_nr = 2;
  inode_nr = 1;
  contents = make_shared<directory>();
  inode_ptr imroot (this);
  contents->meroot(imroot);
  
}
inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&, const inode_ptr ) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {

   throw file_error ("is a " + error_file_type());
}


size_t plain_file::size() const {
  size_t size = data.size();;
   DEBUGF ('i', "size = " << size);
   for(unsigned int it = 0; it < data.size(); it++){
     size += data[it].size();
   }
   return (size == 0 ? 0 : size - 1);
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   return dirents.size();
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);

   if(dirents.end() == dirents.find(filename)){
     cerr << filename << " not found" << endl;
     return;
   }
   if (dirents.at(filename)->get_contents()->my_file_type()
       == "directory"){
       if(dirents.at(filename)->get_contents()->size() > 2){
    cerr << filename << " must be empty" << endl;
    return;
       }
       dirents.at(filename)->get_contents()->nuke();
     }
   
  
  dirents.erase(filename);
}
// overloading mkdir function
void directory::meroot (const inode_ptr me) {

   
  dirents["."] = me;
  dirents[".."] = me;

}

inode_ptr directory::mkdir (const string& dirname, const inode_ptr me) {
   DEBUGF ('i', dirname);
   // inode new_node = inode(file_type::DIRECTORY_TYPE);
   inode_ptr node (new inode(file_type::DIRECTORY_TYPE));
   
   shared_ptr<directory> contents = dynamic_pointer_cast<directory>
   (node->get_contents());
   contents->dirents["."] = node;
   contents->dirents[".."] = me;

   dirents[dirname] = node;
   return node;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   inode_ptr node (new inode(file_type::PLAIN_TYPE));

   dirents[filename] = node;
   return node;
}

