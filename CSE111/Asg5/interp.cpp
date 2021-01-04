// By Erico Bayani (ebayani@ucsc.edu)
// and Venkat Vetsa (vvetsa@ucsc.edu)

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

unordered_map<string,interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"border" , &interpreter::do_border },
   {"moveby" , &interpreter::do_moveby },     
};

unordered_map<string,interpreter::factoryfn>
interpreter::factory_map {
   {"text"     , &interpreter::make_text     },
   {"ellipse"  , &interpreter::make_ellipse  },
   {"circle"   , &interpreter::make_circle   },
   {"polygon"  , &interpreter::make_polygon  },
   {"rectangle", &interpreter::make_rectangle},
   {"square"   , &interpreter::make_square   },
   {"diamond"  , &interpreter::make_diamond  },
   {"triangle"  , &interpreter::make_triangle  },
     {"equilateral" , &interpreter::make_equilateral  },  
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

// sets order of drawn shapes, 0-indexed
static int objects_marker = 0;

interpreter::shape_map interpreter::objmap;

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   rgbcolor color {begin[0]};
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   // need to push_back ojbects_marker too
   window::push_back
     (object (itor->second, where, color, objects_marker++));
}

void interpreter::do_border (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   // shapes might need gl_LINE_LOOP
   // take the same shape,
   // draw functions should also include
   // border option enable
   if (end - begin != 2) throw runtime_error ("invalid # arguments");
   window::setborder_color(rgbcolor{begin[0]});
   window::setborder_width(from_string<int>(begin[1]));
   

}
void interpreter::do_moveby (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   // just give an option a new center
   if (end - begin != 1) throw runtime_error ("invalid # arguments");
   window::setmove_speed(from_string<int>(*begin));
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));
      if (end - begin < 2)
        throw runtime_error ("invalid amount of arguments");           
   string font = begin[0];

   unordered_map<string,void*>::iterator itor = fontcode.find (font);
   if (itor == fontcode.end()) {
      throw runtime_error (font + ": no such font");
   }
   void* selected_font = itor->second;
   string toPrint = begin[1];
   for(int index = 2; index < (end-begin); ++index){
     toPrint = toPrint + " " + begin[index];
   }
   return make_shared<text> (selected_font, toPrint.c_str());
   // just make sure the 
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
      if (end - begin != 2)
        throw runtime_error ("syntax error");        
   return make_shared<ellipse> (from_string<GLfloat> (*begin),
                                from_string<GLfloat> (*(begin+1)));
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1)
     throw runtime_error ("invalid number of arguments");   
   return make_shared<circle> (from_string<GLfloat> (*begin));
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (((end - begin ) % 2 !=  0) && not ((end - begin) > 0))
     throw runtime_error ("bad vertices");
   
   vertex_list list;
   while (begin != end){
     vertex parm;
     parm.xpos = from_string<GLfloat> (*begin++);
     parm.ypos = from_string<GLfloat> (*begin++);
     list.push_back(parm);
     
   }
   return make_shared<polygon> (list);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2)
     throw runtime_error ("invalid number of arguments");   
   return make_shared<rectangle> (from_string<GLfloat> (*begin),
                             from_string<GLfloat> (*(begin+1)));
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1)
     throw runtime_error ("invalid number of arguments");      
   return make_shared<square> (from_string<GLfloat> (*begin));
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 2)
     throw runtime_error ("invalid number of arguments");   
   return make_shared<diamond> (from_string<GLfloat> (*begin),
                                from_string<GLfloat> (*(begin+1)));
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   DEBUGF ('f', range (begin, end));
   if (end - begin != 6)
     throw runtime_error ("invalid number of arguments");
   
   vertex_list list;
   while (begin != end){
     vertex parm;
     parm.xpos = from_string<GLfloat> (*begin++);
     parm.ypos = from_string<GLfloat> (*begin++);
     list.push_back(parm);
     
   }
        return make_shared<triangle> (list[0], list[1], list[2]);
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 1)
     throw runtime_error ("invalid number of arguments");   
   return make_shared<equilateral> (from_string<GLfloat> (*begin));
}

