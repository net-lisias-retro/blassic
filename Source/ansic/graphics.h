// graphics.h

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "blassic.h"
#include <vector>

namespace graphics {

struct Point {
	int x, y;
	Point (int xx, int yy) : x (xx), y (yy) { }
        Point (): x (0), y (0) { }
};

void initialize (const char * progname);
void uninitialize ();
void idle ();
void cls ();
void setmode (int width, int height);
void setmode (int mode);
bool ingraphicsmode ();
void setcolor (int color);
void setbackground (int color);
void settransparent (int transpmode);
void setdrawmode (int mode);
void line (int x, int y);
void move (int x, int y);
void plot (int x, int y);
void plot (std::vector <Point> & points);
Point getlast ();
std::string getkey ();

void charout (char c);
void stringout (const std::string & str);

size_t getlinewidth ();

void locate (int row, int col);
void tab (size_t n);
void movecharforward (size_t n);
void movecharback (size_t n);
void movecharup (size_t n);
void movechardown (size_t n);

void definesymbol (int symbol, const unsigned char (& byte) [8] );
void synchronize (bool mode);
void synchronize ();

int xmouse ();
int ymouse ();

} // namespace graphics

#endif

// Fin de graphics.h
