// By Erico Bayani (ebayani@ucsc.edu)
// and Venkat Vetsa (vvetsa@ucsc.edu)

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
      friend class window; // lets decisions be made about window
   private:
      shared_ptr<shape> pshape;
      vertex center;
      rgbcolor color;
   public:      
      bool focus = false;
      int label;
      object (shared_ptr<shape>, vertex, rgbcolor, int);
      void draw(bool isFocus);
      void move (GLfloat delta_x, GLfloat delta_y);
};

class mouse {
      friend class window;
   private:
      int xpos {0};
      int ypos {0};
      int entered {GLUT_LEFT};
      int left_state {GLUT_UP};
      int middle_state {GLUT_UP};
      int right_state {GLUT_UP};
   private:
      void set (int x, int y) { xpos = x; ypos = y; }
      void state (int button, int state);
      void draw();
};


class window {
      friend class object; // lets decisions be made about window
      friend class mouse;
   private:
      static int width;         // in pixels
      static int height;        // in pixels
      static vector<object> objects;
      static size_t selected_obj;
      static mouse mus;
   public:   
      static int move_speed;      
      static rgbcolor border_color;
      static int border_size;
   private:
      static void close();
      static void entry (int mouse_entered);
      static void display();
      static void reshape (int width, int height);
      static void keyboard (GLubyte key, int, int);
      static void special (int key, int, int);
      static void motion (int x, int y);
      static void passivemotion (int x, int y);
      static void mousefn (int button, int state, int x, int y);
   public:
      static void push_back (const object& obj) {
                  objects.push_back (obj); }
      static void setwidth (int width_) { width = width_; }
      static void setheight (int height_) { height = height_; }
      
      static void setmove_speed (int _move_speed) { move_speed = _move_speed; }
      static void setborder_color (rgbcolor _border_color)
      { border_color = _border_color; }
      static void setborder_width (int _border_size)
      { border_size = _border_size; }
      static void main();
};



#endif
