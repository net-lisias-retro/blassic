// cursor.cpp

#include "cursor.h"
#include "graphics.h"
#include "key.h"
#include "util.h"
#include "trace.h"

#ifdef __WIN32__

#include <windows.h>

#else

#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <errno.h>
#include <cassert>
#define ASSERT assert

#include <ncurses.h>
#include <term.h>

#endif

#include <iostream>
#include <map>

namespace {

#ifndef __WIN32__

bool fInit= true;

int background= 8;

const char * strCls, * strCup,
	* strCursorNormal, * strCursorInvisible,
	* strForeground, * strBackground,
	* strEnterBold, * strExitBold,
	* strMoveForward, * strMoveBack,
	* strMoveForwardN, * strMoveBackN,
	* strMoveUp, * strMoveDown,
	* strMoveUpN, * strMoveDownN,
	* strSaveCursorPos, * strRestoreCursorPos;

const char * newstr (const char * str)
{
	if (str == NULL)
		return NULL;
	size_t l= strlen (str);
	char *n= new char [l + 1];
	strcpy (n, str);
	return n;
}

#if 0
inline const char * mytgetstr (const char * id)
{
	return newstr (tgetstr ( (char *) id, 0) );
}
#endif

inline const char * calltigetstr (const char * id)
{
	const char * str= tigetstr ( (char *) id);
	if (str == (char *) -1)
		return NULL;
	return str;
}

inline const char * mytigetstr (const char * id)
{
        return newstr (calltigetstr (id) );
}

int putfunc (int ic)
{
	char c= ic;
	write (STDOUT_FILENO, & c, 1);
	return c;
}

void calltputs (const char * str)
{
	tputs (str, 1, putfunc);
}

void initkeytable ();

void init ()
{
	TraceFunc tr ("init");

	fInit= false;
	int errret;
	setupterm (0, 1, & errret);

	if (isatty (STDOUT_FILENO) )
	{
		#if 0
		//calltputs (tgetstr ( (char *) "smcup", 0) );
		const char * str= calltigetstr ("smcup");
		if (str != 0)
			calltputs (str);
		#endif

		const char * str_keypad_xmit= calltigetstr ("smkx");
		if (str_keypad_xmit != 0)
			calltputs (str_keypad_xmit);
	}

	//strCls= mytgetstr ("clear");
	strCls= mytigetstr ("clear");

	strCup= mytigetstr ("cup");
	//strCup= newstr (tgetstr ("cup", 0) );

	//strCursorNormal= mytgetstr ("ve");
	strCursorNormal= mytigetstr ("cnorm");
	//strCursorInvisible= mytgetstr ("vi");
	strCursorInvisible= mytigetstr ("civis");

	//strForeground= newstr (tigetstr ("setf") ); // Sf
	//strBackground= newstr (tigetstr ("setb") ); // Sb
	strForeground= mytigetstr ("setaf"); // Sf
	strBackground= mytigetstr ("setab"); // Sb

	strEnterBold= mytigetstr ("bold");
	strExitBold= mytigetstr ("sgr0");

	strMoveForward= mytigetstr ("cuf1");
	strMoveBack= mytigetstr ("cub1");
	strMoveForwardN= mytigetstr ("cuf");
	strMoveBackN= mytigetstr ("cub");

	strMoveUp= mytigetstr ("cuu1");
	strMoveDown= mytigetstr ("cud1");
	strMoveUpN= mytigetstr ("cuu");
	strMoveDownN= mytigetstr ("cud");

	strSaveCursorPos= mytigetstr ("sc");
	strRestoreCursorPos= mytigetstr ("rc");

	initkeytable ();
}

inline void checkinit ()
{
	if (fInit)
		init ();
}

#endif

} // namespace

void initconsole ()
{
	TraceFunc tr ("initconsole");

	cursorinvisible ();
}

void quitconsole ()
{
	TraceFunc tr ("quitconsole");

	cursorvisible ();

	#ifndef __WIN32__

	if (! fInit)
	{
		if (isatty (STDOUT_FILENO) )
		{
			#if 0
			//calltputs (tgetstr ( (char *) "rmcup", 0) );
			const char * str= calltigetstr ("rmcup");
			if (str != 0)
				calltputs (str);
			#endif

			const char * str_keypad_local= calltigetstr ("rmkx");
			if (str_keypad_local != 0)
				calltputs (str_keypad_local);
		}

	}

	#endif
}

void cursorvisible ()
{
	// checkinit not needed, is done by showcursor
	showcursor ();

	#ifndef __WIN32__

	struct termios ter;
	tcgetattr (STDIN_FILENO, & ter);
	//ter.c_lflag|= (ECHO | ICANON | PENDIN);
	ter.c_lflag|= (ECHO | ICANON);
	tcsetattr (STDIN_FILENO, TCSANOW, & ter);

        #endif
}

void cursorinvisible ()
{
	// checkinit not needed, is done by hidecursor
	hidecursor ();

	#ifndef __WIN32__

	struct termios ter;
	tcgetattr (STDIN_FILENO, & ter);
	ter.c_lflag&= ~ (ECHO | ICANON);
	tcsetattr (STDIN_FILENO, TCSANOW, & ter);

        #endif
}

void showcursor ()
{
	#ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo (h, & info);
        info.bVisible= TRUE;
        SetConsoleCursorInfo (h, & info);

	#else

	checkinit ();

	if (isatty (STDOUT_FILENO) )
		calltputs (strCursorNormal );

	#endif
}

void hidecursor ()
{
	#ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        GetConsoleCursorInfo (h, & info);
        info.bVisible= FALSE;
        SetConsoleCursorInfo (h, & info);

	#else

	checkinit ();

	if (isatty (STDOUT_FILENO) )
		calltputs (strCursorInvisible);

	#endif
}

#ifdef __WIN32__

const WORD init_attributes=
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
WORD attributes= init_attributes;

#endif

void cls ()
{
	#ifdef __WIN32__

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        if (h != INVALID_HANDLE_VALUE)
        {
                CONSOLE_SCREEN_BUFFER_INFO info;
                if (GetConsoleScreenBufferInfo (h, & info) )
                {
                        DWORD l= info.dwSize.X * info.dwSize.Y;
                        COORD coord= {0, 0};
                        DWORD notused;
                        FillConsoleOutputAttribute (h,
                                attributes,
                                l, coord, & notused);
                        FillConsoleOutputCharacter (h,
                        	' ', l, coord, & notused);
                        SetConsoleCursorPosition (h, coord);
                }
        }

	#else

	checkinit ();

	if (strCls)
		calltputs (strCls);

	#endif
}

void locate (int row, int col)
{
	#ifdef BLASSIC_USE_WINDOWS

        COORD coord= { SHORT (col - 1), SHORT (row - 1) };
        SetConsoleCursorPosition (GetStdHandle (STD_OUTPUT_HANDLE), coord);

	#else

	checkinit ();

	if (strCup)
		calltputs (tgoto (strCup, col - 1, row - 1) );

	#endif
}

void movecharforward ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharforward (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movecharforward (1);

	#else

	if (strMoveForward)
		calltputs (strMoveForward);

	#endif
}

void movecharback ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharback (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movecharback (1);

	#else

	if (strMoveBack)
		calltputs (strMoveBack);

	#endif
}

void movecharup ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharup (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movecharup (1);

	#else

	if (strMoveUp)
		calltputs (strMoveUp);

	#endif
}

void movechardown ()
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movechardown (1);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        movechardown (1);

	#else

	if (strMoveDown)
		calltputs (strMoveDown);

	#endif
}

namespace {

#if defined __unix__ || defined __linux__

inline void auxmovechar (const char * strN, const char * str, size_t n)
{
	if (n != 0)
	{
		if (strN)
			calltputs (tparm (strN, n) );
		else
			if (str)
				for (size_t i= 0; i < n; ++i)
					calltputs (str);
	}
}

#endif

#ifdef __WIN32__



#endif

} // namespace

void movecharforward (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharforward (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.X+= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#else

	auxmovechar (strMoveForwardN, strMoveForward, n);

	#endif
}

void movecharback (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharback (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.X-= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#else

	auxmovechar (strMoveBackN, strMoveBack, n);

	#endif
}

void movecharup (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movecharup (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.Y-= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#else

	auxmovechar (strMoveUpN, strMoveUp, n);

	#endif
}

void movechardown (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::movechardown (n);
		return;
	}

	#ifdef BLASSIC_USE_WINDOWS

        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo (h, & info) )
        {
                info.dwCursorPosition.Y+= SHORT (n);
                SetConsoleCursorPosition (h, info.dwCursorPosition);
        }

	#else

	auxmovechar (strMoveDownN, strMoveDown, n);

	#endif
}

void savecursorpos ()
{
	#ifdef BLASSIC_USE_WINDOWS

	#else

	if (strSaveCursorPos)
		calltputs (strSaveCursorPos);

	#endif
}

void restorecursorpos ()
{
	#ifdef BLASSIC_USE_WINDOWS

	#else

	if (strRestoreCursorPos)
		calltputs (strRestoreCursorPos);

	#endif
}


#ifndef __WIN32__

namespace {

static const int newcolor []=
	{ 0, 4, 2, 6, 1, 5, 3, 7};

inline int mapcolor (int n)
{
	// Intensity bit unchanged
	return newcolor [n & 7] | (n & 8);
}

}

#endif

void textcolor (int color)
{
        #ifdef __WIN32__
        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        attributes= (attributes & WORD (0xF0) ) | WORD (color & 0x0F);
        SetConsoleTextAttribute (h, attributes);

	#else

	color= mapcolor (color & 0xF);
	bool intensity= color > 7;
	if (intensity)
	{
		color&= 7;
		if (strEnterBold)
			calltputs (strEnterBold);
	}
	else
	{
		if (strExitBold)
		{
			calltputs (strExitBold);
			// sgr0 reset the background, then we need to set it.
			textbackground (background);
		}
	}
	if (strForeground)
		calltputs (tparm ( (char *) strForeground, color) );
        #endif
}

void textbackground (int color)
{
        #ifdef __WIN32__
        HANDLE h= GetStdHandle (STD_OUTPUT_HANDLE);
        attributes= (attributes & WORD (0xF) ) | WORD ( (color & 0xF) << 4);
        SetConsoleTextAttribute (h, attributes);

	#else

	background= color;
	color= mapcolor (color & 0xF);
	if (strBackground)
		calltputs (tparm ( (char *) strBackground, color) );
        #endif
}

namespace {

enum ReadType { ReadWait, ReadNoWait };

#ifdef __WIN32__

std::string string_from_key_event (const KEY_EVENT_RECORD & kr)
{
        char c= kr.uChar.AsciiChar;
        if (c != '\0')
                return std::string (1, c);
        WORD k= kr.wVirtualKeyCode;
        std::string str= string_from_key (k);
        if (! str.empty () )
                return str;

        if (k != VK_SHIFT &&
                k != VK_CONTROL &&
                k != VK_MENU &&
                k != VK_CAPITAL &&
                k != VK_NUMLOCK &&
                k != VK_SCROLL)
        {
                std::string str (1, '\0');
                str+= char (kr.wVirtualScanCode);
                return str;
        }
        return std::string ();
}

std::string string_from_input (const INPUT_RECORD & input)
{
        std::string str;
        if (input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown)
        {
                str= string_from_key_event (input.Event.KeyEvent);
        }
        return str;
}

std::string readkey (ReadType type)
{
        std::string str;

        HANDLE h= GetStdHandle (STD_INPUT_HANDLE);
        DWORD mode, orgmode;
        GetConsoleMode (h, & mode);
        orgmode= mode;
        //mode&= ~ (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
        mode= 0;
        SetConsoleMode (h, mode);
        DWORD n= 0;
        INPUT_RECORD input;
        if (type == ReadNoWait)
                PeekConsoleInput (h, & input, 1, & n);
        else
                n= 1;
        if (n)
        {
        sigue:
                ReadConsoleInput (h, & input, 1, & n);
                str= string_from_input (input);
                if (type == ReadWait && str.empty () )
                        goto sigue;
        }
        SetConsoleMode (h, orgmode);

        return str;
}

#else

class MapSpecial {
public:
	enum Result { NoMapped, Found, MoreNeeded };
	void addkey (const string & str, string::size_type pos,
		const string & keyname)
	{
		TraceFunc tr ("MapSpecial::addkey");

		ASSERT (pos < str.size () );
		char c= str [pos];
		if (pos == str.size () - 1)
			kname [c]= keyname;
		else
		{
			//if (kmap.find (c) == kmap.end () )
			//	kmap [c]= MapSpecial ();
			kmap [c].addkey (str, pos + 1, keyname);
		}
	}
	Result findkey (const string & str, string::size_type pos,
		string & keyname, string::size_type & consumed)
	{
		if (pos >= str.size () )
			return MoreNeeded;
		char c= str [pos];
		//cout << "Buscando: " << c << endl;
		{
			map <char, std::string>::iterator it= kname.find (c);
			if (it != kname.end () )
			{
				keyname= it->second;
				consumed= pos;
				return Found;
			}
		}
		map <char, MapSpecial>::iterator it= kmap.find (c);
		if (it != kmap.end () )
			return it->second.findkey
				(str, pos + 1, keyname, consumed);
		else
			return NoMapped;
	}
private:
	map <char, std::string> kname;
	map <char, MapSpecial> kmap;
};

struct KeyDescription {
	const char * tiId;
	//const char * blName;
	const std::string & blName;
};

const std::string
	strMULT ("*"),
	strMINUS ("-"),
	strPLUS ("+"),
	strDIV ("/");

const KeyDescription keyname [] = {
	{ "kpp",   strPAGEUP },    // previous-page key
	{ "knp",   strPAGEDOWN },  // next-page key
	{ "kend",  strEND },       // end key
	{ "kslt",  strEND },       // select key
	{ "kc1",   strEND },       // lower left of keypad
	{ "khome", strHOME },      // home key
	{ "kfnd",  strHOME },      // find key
	{ "ka1",   strHOME },      // upper left of keypad
	{ "kcub1", strLEFT },      // left-arrow key
	{ "kcuu1", strUP },        // up-arrow key
	{ "kcuf1", strRIGHT },     // right-arrow key
	{ "kcud1", strDOWN },      // down-arrow key
	{ "kich1", strINSERT },    // insert-character key
	{ "kdch1", strDELETE },    // delete-character key
	{ "kent",  strENTER },     // enter/send key
	{ "kf1",   strF1 },        // F1 function key
	{ "kf2",   strF2 },        // F2 function key
	{ "kf3",   strF3 },        // F3 function key
	{ "kf4",   strF4 },        // F4 function key
	{ "kf5",   strF5 },        // F5 function key
	{ "kf6",   strF6 },        // F6 function key
	{ "kf7",   strF7 },        // F7 function key
	{ "kf8",   strF8 },        // F8 function key
	{ "kf9",   strF9 },        // F9 function key
	{ "kf10",  strF10 },       // F10 function key
	{ "kf11",  strF11 },       // F11 function key
	{ "kf12",  strF12 },       // F12 function key
	{ "kf54",  strDIV },       // F54 function key, / in xterm
	{ "kf55",  strMULT },      // F55 function key, * in xterm
	{ "kf56",  strMINUS },     // F56 function key, - in xterm
	{ "kf57",  strPLUS },      // f57 function key, + in xterm
};

#ifndef NDEBUG

bool checktable ()
{
	const size_t nkeys= util::dim_array (keyname);

	for (size_t i= 0; i < nkeys - 1; ++i)
		for (size_t j= i + 1; j < nkeys; ++j)
			if (strcmp (keyname [i].tiId, keyname [j].tiId) == 0)
			{
				std::cerr << "Code repeated in keyname: " <<
					keyname [i].tiId << std::endl;
				throw 1;
			}
	return true;
}

bool tablechecked= checktable ();

#endif

MapSpecial ms;

void initkeytable ()
{
	TraceFunc tr ("initkeytable");

	const size_t nkeys= util::dim_array (keyname);

	for (size_t i= 0; i < nkeys; ++i)
	{
		const KeyDescription & keydesc= keyname [i];
		const char * const strkey= keydesc.tiId;
		const char * str= calltigetstr (strkey);
		if (str != NULL)
		{
			#if 0
			cerr << keydesc.blName << "=";
			for (size_t i= 0, l= strlen (str); i < l; ++i)
			{
				char c= str [i];
				if (c >= 32) cerr << c;
				else cerr << "\\(" << hex << int (c) << ')';
			}
			cerr << endl;
			#endif
			tr.message (std::string ("Adding ") + keydesc.blName);
			ms.addkey (str, 0, keydesc.blName);
		}
	}

}

class PollInput {
public:
	PollInput ()
	{
		pfd.fd= STDIN_FILENO;
		pfd.events= POLLIN | POLLERR | POLLNVAL;
	}
	int poll ()
	{
		return ::poll (& pfd, 1, 100);
	}
private:
	struct pollfd pfd;
};

void wait_event ()
{
	PollInput pi;
	int r;
	do {
		graphics::idle ();
	} while ( (r= pi.poll () ) == 0);
	if (r < 0)
	{
		cerr << "Error in poll: " << strerror (errno) << endl;
	}
}

void do_poll ()
{
	PollInput ().poll ();
}

std::string readkey (ReadType type)
{
	checkinit ();

	static std::string charpending;
        std::string str;
	bool reset_blocking_mode= false;

	if (! charpending.empty () )
		goto check_it;

        if (type == ReadWait)
        {
                //cursorvisible ();
		//fcntl (STDIN_FILENO, F_SETFL, 0);
		wait_event ();
	}
	else
	{
		fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
		reset_blocking_mode= true;
	}

	int l;
	char c;

	//read_another:

	l= read (STDIN_FILENO, & c, 1);
	if (l == 1)
		str+= c;

	read_another:
	
	charpending+= str;
	str.erase ();

	check_it:

	std::string keyname;
	std::string::size_type pos;
	if (! charpending.empty () )
	{
		MapSpecial::Result r=
			ms.findkey (charpending, 0, keyname, pos);
		switch (r)
		{
		case MapSpecial::NoMapped:
			str= charpending [0];
			charpending.erase (0, 1);
			break;
		case MapSpecial::Found:
			str= keyname;
			charpending.erase (0, pos + 1);
			break;
		case MapSpecial::MoreNeeded:
			fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
			reset_blocking_mode= true;
			do_poll ();
			l= read (STDIN_FILENO, & c, 1);
			if (l == 1)
			{
				str= c;
				goto read_another;
			}
			str= charpending [0];
			charpending.erase (0, 1);
			break;
		}
	}

        if (type == ReadWait)
                cursorinvisible ();

	if (reset_blocking_mode)
		fcntl (STDIN_FILENO, F_SETFL, 0);

        return str;
}

#endif

} // namespace

std::string inkey ()
{
        if (graphics::ingraphicsmode () )
                return graphics::getkey ();
        return readkey (ReadNoWait);
}

std::string getkey ()
{
        if (graphics::ingraphicsmode () )
        {
                std::string str;
                do {
                        str= graphics::getkey ();
                } while (str.empty () );
                return str;
        }
        return readkey (ReadWait);
}

#ifdef __WIN32__

void clean_input ()
{
        Sleep (100);
        HANDLE h= GetStdHandle (STD_INPUT_HANDLE);
        INPUT_RECORD input;
        DWORD n= 0;
        PeekConsoleInput (h, & input, 1, & n);
        if (n && input.EventType == KEY_EVENT &&
                        ! input.Event.KeyEvent.bKeyDown)
                ReadConsoleInput (h, & input, 1, & n);
}

#endif

// Fin de cursor.cpp
