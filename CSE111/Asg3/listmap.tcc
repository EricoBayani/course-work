// listmap.tcc by Venkat Vetsa (vvetsa@ucsc.edu)
// and Erico Bayani (ebayani@ucsc.edu)

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));
   
   anchor_.next = static_cast<node*> (&anchor_);
   anchor_.prev = static_cast<node*> (&anchor_);

   it = new iterator(static_cast<node*> (&anchor_));
}

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::~listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));

   iterator del = begin();
   
   while(end() != del) {
      del = erase(del);
   }
   
   delete it;   
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::insert (const value_type& pair) {
   DEBUGF ('l', &pair << "->" << pair);

   // itialize the iterator
   node *nodeIt = anchor()->next;
   node *found = NULL;

   while (nodeIt != anchor()) {
      if (!less(pair.first,nodeIt->value.first) &&
          !less(nodeIt->value.first,pair.first)) {
         found = nodeIt;
         break;
      } else if (less(pair.first,nodeIt->value.first)) {
         break;
      }
      nodeIt = nodeIt->next;
   }

   if (NULL == found) {
      found = new node(nodeIt, nodeIt->prev, pair);
      nodeIt->prev->next = found;
      nodeIt->prev = found;
   } else {
      found->value.second = pair.second;
   }

   it->where = found;

   return *it;
}

//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::find (const key_type& that) {
   DEBUGF ('l', that);

   // define starting point
   node *finding = anchor()->next;

   while (finding != anchor()) {
      if (!less(finding->value.first,that) &&
          !less(that,finding->value.first)) {
         break;
      }
      finding = finding->next;
   }

   it->where = finding;

   return *it;

   //return finding;
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::erase (iterator position) {
   DEBUGF ('l', &*position);

   /**
   if (end() == position) {
      cerr << "invalid erase value" << endl;
   }
   
   listmap<key_t,mapped_t,less_t>::iterator returnVal = position->next;
   node *toDel = position.where;

   toDel->prev->next = toDel->next;
   toDel->next->prev = toDel->prev;

   delete toDel;
   **/

   if (end() == position) {
      cerr << "invalid argument to erase" << endl;
      return iterator();
   }

   node *toDel = position.where;
   node *toRet = toDel->next;

   toDel->prev->next = toDel->next;
   toDel->next->prev = toDel->prev;

   delete toDel;

   it->where = toRet;

   return *it;
}


