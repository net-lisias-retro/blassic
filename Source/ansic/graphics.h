// graphics.h
// Revision 5-aug-2003

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "blassic.h"

namespace graphics {

struct Point {
	int x, y;
	Point (int xx, int yy) : x (xx), y (yy) { }
        Point (): x (0), y (0) { }
};

void initialize (const char * progname);
void uninitialize ();

void origin (int x, int y);
void limits (int minx, int maxx, int miny, int maxy);

void ink (int inknum, int cpccolor);
void ink (int inknum, int r, int g, int b);
void clearink ();

void idle ();
void cls ();
void cls (BlChannel n);
void setmode (int width, int height, bool inverty,
	int zoomx= 1, int zoomy= 1);
void setmode (int mode);
void setmode (const std::string & mode);
bool ingraphicsmode ();

void setcolor (int color);
void setcolor (BlChannel ch, int color);
int getcolor (BlChannel ch);
void setbackground (int color);
void setbackground (BlChannel ch, int color);
int getbackground (BlChannel ch);

void settransparent (int transpmode);
void setdrawmode (int mode);

void line (int x, int y);
void liner (int x, int y);
void rectangle (Point org, Point dest);
void rectanglefilled (Point org, Point dest);
void move (int x, int y);
void mover (int x, int y);
void plot (int x, int y);
void plotr (int x, int y);
void plot (std::vector <Point> & points);
int test (int x, int y, bool relative);

void circle (int x, int y, int radius);
void arccircle (int x, int y, int radius,
	BlNumber arcbeg, BlNumber arcend);
void ellipse (int x, int y, int rx, int ry);
void arcellipse (int x, int y, int rx, int ry,
	BlNumber arcbeg, BlNumber arcend);

void mask (int m);
void maskdrawfirstpoint (bool f);
void draw (const std::string & str);
Point getlast ();
std::string getkey ();

void charout (char c);
void stringout (const std::string & str);
void charout (BlChannel ch, char c);
void stringout (BlChannel ch, const std::string & str);
std::string copychr (BlChannel ch);

void definewindow (BlChannel n, int x1, int x2, int y1, int y2);
void undefinewindow (BlChannel ch);

size_t getlinewidth ();
size_t getlinewidth (BlChannel ch);

//void locate (int row, int col);
void gotoxy (int x, int y);
void gotoxy (BlChannel ch, int x, int y);
void tab (size_t n);
void tab (BlChannel ch);
void tab (BlChannel ch, size_t x);
void movecharforward (size_t n);
void movecharforward (BlChannel ch, size_t n);
void movecharback (size_t n);
void movecharback (BlChannel ch, size_t n);
void movecharup (size_t n);
void movecharup (BlChannel ch, size_t n);
void movechardown (size_t n);
void movechardown (BlChannel ch, size_t n);

void symbolafter (int symbol);
void definesymbol (int symbol, const unsigned char (& byte) [8] );
void synchronize (bool mode);
void synchronize ();
void synchronize_suspend ();
void synchronize_restart ();

int xmouse ();
int ymouse ();

int xpos ();
int xpos (BlChannel ch);
int ypos ();
int ypos (BlChannel ch);

void tag (BlChannel ch);
void tagoff (BlChannel ch);
bool istagactive (BlChannel ch);

void showcursor ();
void hidecursor ();

void showcursor (BlChannel ch);
void hidecursor (BlChannel ch);

void inverse (BlChannel ch, bool active);
bool getinverse (BlChannel ch);

void clean_input ();

void ring ();

void set_title (const std::string & title);
void set_default_title (const std::string & title);

} // namespace graphics

#endif

// Fin de graphics.h
