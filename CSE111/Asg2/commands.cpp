// By Erico Bayani (ebayani@ucsc.edu) and Venkat Vetsa (vvetsa@ucsc.edu)

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
   {"#"     , fn_comment }
};


void recursive_clear (inode_ptr rm);

// helper functions

inode_ptr resolvePath (inode_state& state, const string path){
  inode_ptr curr;
  if (path.empty() || path == "") {
    return state.getCWD();
  }
  else {
    if('/' == path[0]){
      curr = state.getRoot();
    }
    else {
      curr = state.getCWD();
    }
    wordvec names = split(path, "/");
    for (unsigned int it = 0; it < names.size(); it++){
     shared_ptr<directory> currDir = dynamic_pointer_cast<directory>
     (curr->get_contents());
     map<string,inode_ptr> currMap = currDir->getDirents();
     if (currMap.empty()){
       cerr << names[it] <<" is not directory" << endl;
       return nullptr;
     }
     if (currMap.find(names[it]) != currMap.end()) {
       curr = currMap.at(names[it]);
     }
     else {
       cerr << names[it] <<" not found" << endl;
       return nullptr;
     }
    }
  }
  return curr;
}

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string path = string();
   if (words.size() == 1){
     cerr << "no files specified" << endl;
     return;
   }
   for (unsigned int jt = 1; jt < words.size(); jt++){
     
     path = words[jt];
     
     inode_ptr toPrint = resolvePath(state, path);
     if (toPrint == nullptr){
       return;
     }

     shared_ptr<plain_file> contents = dynamic_pointer_cast<plain_file>
       (toPrint->get_contents());

     if (contents->my_file_type() == "directory"){
       cerr << words[jt] << " is a directory" << endl;
       return;
     }

     const wordvec& readWords = contents->readfile();
     string outstr = string();
     if (!readWords.empty()){
       outstr = readWords[0];
       for(unsigned int it = 1; it < readWords.size(); it++){
         outstr += " ";
         outstr += readWords[it];
       }
     }
   
     cout << outstr << endl;
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // currently cannot work when empty
   string path = string();
   if (words.size() > 1){
     path = words[1];
   } else {
      path = "/";
   }

   inode_ptr toGo = resolvePath(state, path);
   if(nullptr == toGo){
     cerr << path << "is not a valid path" << endl;
     return;
   }

   if (toGo->get_contents()->my_file_type() != "directory"){
     cerr << path << " is not a directory" << endl;
     return;
   }
   
   if ('/' == path.at(0)) {
      state.resetPath();
   }
   wordvec names = split(path, "/");
   for (unsigned int it = 0; it < names.size(); it++){
     if("." == names[it]){
       ;
     }
     else if (".." == names[it]){
       if (!(state.getPath().empty()))
         state.removeFromPath();
     }
     else {
       state.addToPath(names[it]);
     }
   }
   state.setCWD(toGo);
}

void fn_comment (inode_state& , const wordvec&){
  return;
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   shared_ptr<directory> rootConts = dynamic_pointer_cast<directory>
      (state.getRoot()->get_contents());
   map<string,inode_ptr> rootChildren = rootConts->getDirents();
   map<string,inode_ptr>::iterator myMap = rootChildren.begin();
   while (rootChildren.end() != myMap) {
      if (myMap->first != "." && myMap->first != "..") {
         if (myMap->second->get_contents()->my_file_type()
             == "directory") {
            recursive_clear(myMap->second);
         }
      }
      myMap++;
   }

   int exit_status = 0;

   if (1 < words.size()) {
      string pars = words.at(1);
      string::iterator strIt = pars.begin();
      while(pars.end() != strIt) {
         if (*strIt >= '0' && *strIt <= '9') {
            exit_status *= 10;
            exit_status += *strIt - '0';
         } else {
            exit_status = 127;
            break;
         }
         strIt++;
      }
   }
   
   exec::status(exit_status);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string path = string();
   string absPath = string();

   vector<string> modPath;
   vector<string>::iterator strIt;
   vector<string> workingPath = state.getPath();

   if (words.size() > 1){
     path = words[1];
     modPath = split(path, "/");

     if ('/' == path.at(0)) {
       workingPath.clear();
     }
   } else {
     modPath = vector<string>();
   }

   strIt = modPath.begin();
   while (modPath.end() != strIt) {
      if (*strIt == ".") {
      } else if (*strIt == "..") {
         if (!workingPath.empty()) {
            workingPath.pop_back();
         }
      } else {
         workingPath.push_back(*strIt);
      }
      strIt++;
   }

   if (0 < workingPath.size()) {
      strIt = workingPath.begin();
      while (workingPath.end() != strIt) {
         absPath += "/" + *strIt;
         strIt++;
      }
   } else {
      absPath += "/";
   }

   inode_ptr toPrint = resolvePath(state, path);
   
   if(nullptr == toPrint){
     return;
   }

   /*
   if (toPrint->get_contents()->my_file_type() == "directory"){
     cerr << path << " is not a directory" << endl;
     return;
   }
   */
   
   string name = string();
   int num;
   size_t size;

   if (toPrint->get_contents()->my_file_type() == "directory") {
     shared_ptr<directory> contents = dynamic_pointer_cast<directory>
       (toPrint->get_contents());
     map<string,inode_ptr> myMap = contents->getDirents();
     map<string,inode_ptr>::iterator it = myMap.begin(); 

     cout << absPath << ":" << endl;
     while (it != myMap.end()){
       name = it->first;
       if (name != "." && name != "..") {
         if (it->second->get_contents()->my_file_type()
             == "directory") {
           name += "/";
         }
       }
       num = it->second->get_inode_nr();
       size = (it->second->get_contents())->size();
       cout << setw(6) << num << setw(6) << size << " " << name << endl;
       it++;
     }
   } else {
     name = modPath.back();
     size = toPrint->get_contents()->size();
     num = toPrint->get_inode_nr();
     cout << absPath << ":" << endl;
     cout << setw(6) << num << setw(6) << size << " " << name << endl;
   }
}


void recursive_ls (inode_ptr listing, string path){

   if (listing->get_contents()->my_file_type() == "plain file"){
      return;
   }
   shared_ptr<directory> contents = dynamic_pointer_cast<directory>
   (listing->get_contents());
   map<string,inode_ptr> my_map = contents->getDirents();
   map<string,inode_ptr>::iterator it = my_map.begin();

   string name = string();
   int num;
   size_t size;

   // print directory name here pls
   if (0 == path.size()) {
      cout << "/:" << endl;
   } else {
      cout << path << ":" << endl;
   }
   while (it != my_map.end()){
     name = it->first;
     if (name != "." && name != "..") {
       if (it->second->get_contents()->my_file_type() == "directory") {
         name += "/";
       }
     }
     num = it->second->get_inode_nr();
     size = (it->second->get_contents())->size() ;
     cout << setw(6) << num << setw(6) << size << " " << name << endl;
     it++;
   }
   cout << endl;
   it = my_map.begin();
   while(my_map.end()!=it){
     if (it->first == "." ||it->first == ".."){
       it++;
       continue;
     }
     recursive_ls(it->second, path + "/" + it->first);
    it++;
  }
   
}
void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = string();
   string absPath = string();

   vector<string> modPath;
   vector<string> workingPath = state.getPath();
   vector<string>::iterator it;

   if (words.size() > 1){
      path = words[1];
      modPath = split(path, "/");

      if ('/' == path.at(0)) {
         workingPath.clear();
      }
   } else {
      modPath = vector<string>();
   }

   it = modPath.begin();
   while (modPath.end() != it) {
      if (*it == ".") {
      } else if (*it == "..") {
         if (!workingPath.empty()) {
            workingPath.pop_back();
         }
      } else {
         workingPath.push_back(*it);
      }
      it++;
   }

   if (0 < workingPath.size()) {
      it = workingPath.begin();
      while (workingPath.end() != it) {
         absPath += "/" + *it;
         it++;
      }
   }
   
   inode_ptr ls_file = resolvePath(state, path);

   recursive_ls(ls_file, absPath);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = string();

   if (words.size() > 1){
     path = words[1];
   }
   else {
     cerr << "not enough arguments" << endl;
     return;
   }

   wordvec surgery = split(path, "/");
   string filename = surgery.back();
   surgery.pop_back();

   path = string();
   for (unsigned int it = 0; it < surgery.size(); it++){
     path += surgery[it];
     path += "/";
   }
   inode_ptr location = resolvePath(state, path);
   if (location == nullptr){
     return;
   }

   shared_ptr<directory> contents = dynamic_pointer_cast<directory>
   (location->get_contents());
   if (contents->my_file_type() != "directory"){
     cerr << "invalid path" << endl;
     return;
   }
   inode_ptr newfile = contents->mkfile(filename);
   shared_ptr<plain_file> filecontents =
     dynamic_pointer_cast<plain_file>
   (newfile->get_contents());
   wordvec::const_iterator first = words.begin() + 2;
   wordvec::const_iterator last = words.end();
   wordvec toWrite(first, last);
   filecontents->writefile(toWrite);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string path = string();

   if (words.size() > 1){
     path = words[1];
   }
   else {
     cerr << "not enough arguments" << endl;
     return;
   }
   wordvec surgery = split(path, "/");

   string dirname = surgery.back();
   surgery.pop_back();

   path = string();
   for (unsigned int it = 0; it < surgery.size(); it++){
     path += surgery[it];
     path += "/";
   }
   inode_ptr location = resolvePath(state, path);

   // might need more error checking here
   if (location == nullptr){
     return;
   }

   shared_ptr<directory> contents = dynamic_pointer_cast<directory>
   (location->get_contents());
   if (contents->my_file_type() != "directory"){
     cerr << "invalid path" << endl;
     return;
   }
   contents->mkdir(dirname, location);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string new_prompt = string();
   if(1 < words.size()){
     for (unsigned int it = 1; it < words.size(); it++){
       new_prompt += (words[it]);
       new_prompt += (" ");
       
     }

     state.setPrompt(new_prompt);
   }
   
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   vector<string> curr_path = state.getPath();
   string out_str = string();
   // code
   if (curr_path.empty()){
     out_str += "/";
   }
   else {
     for (unsigned int i = 0; i < curr_path.size(); i++){
       out_str += "/";
       out_str += curr_path[i];
     }
   }

   cout << out_str << endl;
   
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   string path = string();

   if (words.size() > 1){
     path = words[1];
   } else {
     cerr << "rm needs argument <path>" << endl;
     return;
   } 

   wordvec surgery = split(path, "/");

   string filename = surgery.back();
   if (filename == ".." || filename == "."){
     cerr << "invalid characters: can't remove .. or ." << endl;
     return;
   }

   surgery.pop_back();

   path = string();
   for (unsigned int it = 0; it < surgery.size(); it++){
     path += surgery[it];
     path += "/";
   }
   
   inode_ptr wd = resolvePath(state, path);
   if(nullptr == wd){
     cerr << path << "is not a valid path" << endl;
     return;
   }

   if (wd->get_contents()->my_file_type() == "directory"){
     wd->get_contents()->remove(filename);
   }

}

// helper function for rmr

void recursive_clear (inode_ptr rm){
  if (rm->get_contents()->my_file_type() == "plain file"){
    return;
  }
  shared_ptr<directory> contents = dynamic_pointer_cast<directory>
     (rm->get_contents());
    map<string,inode_ptr> my_map = contents->getDirents();
  map<string,inode_ptr>::iterator it = my_map.begin();
  while(my_map.end()!=it){
    if (it->first == "." ||it->first == ".."){
      it++;
      continue;
    }
    recursive_clear(it->second);
    it++;
  }
  contents->nuke();
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = string();
   
   if (words.size() > 1){
     path = words[1];
   } else {
     cerr << "rmr needs argument <path>" << endl;
     return;
   }

   inode_ptr rm_file = resolvePath(state, path);
   
   wordvec surgery = split(path, "/");

   string filename = surgery.back();
   if (filename == ".." || filename == "."){
     cerr << "invalid characters: can't remove .. or ." << endl;
     return;
   }

   surgery.pop_back();

   path = string();
   for (unsigned int it = 0; it < surgery.size(); it++){
     path += surgery[it];
     path += "/";
   }
   
   inode_ptr here = resolvePath(state, path);
   recursive_clear(rm_file);

   here->get_contents()->remove(filename); 
}



