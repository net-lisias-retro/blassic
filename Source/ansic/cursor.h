// cursor.h

#ifndef CURSOR_H_
#define CURSOR_H_

#include <string>

void initconsole ();
void quitconsole ();

size_t getwidth ();

void cursorvisible ();
void cursorinvisible ();
void showcursor ();
void hidecursor ();

void cls ();

//void locate (int row, int col);
void gotoxy (int x, int y);
void movecharforward ();
void movecharback ();
void movecharforward (size_t n);
void movecharback (size_t n);
void movecharup ();
void movechardown ();
void movecharup (size_t n);
void movechardown (size_t n);
void savecursorpos ();
void restorecursorpos ();

void textcolor (int color);
void textbackground (int color);
std::string inkey ();
std::string getkey ();

#ifdef __WIN32__

void clean_input ();

#else

inline void clean_input () { }

#endif

#endif

// Fin de cursor.h

