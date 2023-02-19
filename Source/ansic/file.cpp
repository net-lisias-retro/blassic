// file.cpp
// Revision 18-aug-2003

#include "blassic.h"
#include "file.h"
#include "trace.h"
#include "error.h"
#include "var.h"
#include "cursor.h"
#include "edit.h"
#include "graphics.h"
#include "util.h"
using util::to_string;

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <iostream>
using std::cerr;
using std::endl;

#include <cassert>
#define ASSERT assert

//#ifndef _Windows
#ifndef BLASSIC_USE_WINDOWS
#include <unistd.h> // read, write
#else
#include <io.h> // isatty
#endif

//***********************************************
//		Auxiliary functions
//***********************************************

namespace {

class updateposchar {
public:
	updateposchar (int & pos) :
		pos (pos)
	{ }
	void operator () (const char c)
	{
		switch (c)
		{
		case '\r':
		case '\n':
			pos= 0;
			break;
		case '\b':
			if (pos > 0)
				--pos;
			break;
		case '\a':
			// Bell does not use space in screen.
			break;
		case '\t':
			pos= ( (pos / 8) + 1) * 8;
			break;
		default:
			++pos;
		}
	}
private:
	int & pos;
};

void updatepos (int & pos, const std::string & str)
{
	std::for_each (str.begin (), str.end (), updateposchar (pos) );
}

} // namespace

//***********************************************
//              BlFile
//***********************************************

BlFile::BlFile (OpenMode nmode) :
        mode (nmode),
	cDelimiter (','),
	cQuote ('"'),
	cEscape ('\0')
{
}

BlFile::~BlFile ()
{
}

bool BlFile::eof ()
	{ throw ErrFileMode; }
void BlFile::flush ()
	{ throw ErrFileMode; }
void BlFile::getline (std::string &)
	{ throw ErrFileMode; }
void BlFile::outstring (const std::string &)
	{ throw ErrFileMode; }
void BlFile::outchar (char)
	{ throw ErrFileMode; }

#if 0

void BlFile::outnumber (BlNumber n)
{
	//outstring (to_string (n) );
	std::ostringstream oss;
	oss << std::setprecision (16) << n;
	outstring (oss.str () );
}

void BlFile::outinteger (BlInteger n)
{
	outstring (to_string (n) );
}

void BlFile::outlinenumber (BlLineNumber l)
{
        std::ostringstream oss;
        oss << std::setw (7) << l;
        outstring (oss.str () );
}

#endif

BlFile & operator << (BlFile & bf, const std::string & str)
{
        bf.outstring (str);
        return bf;
}

BlFile & operator << (BlFile & bf, char c)
{
        bf.outchar (c);
        return bf;
}

BlFile & operator << (BlFile & bf, BlNumber n)
{
        //bf.outnumber (n);
	std::ostringstream oss;
	oss << std::setprecision (16) << n;
	bf.outstring (oss.str () );
        return bf;
}

BlFile & operator << (BlFile & bf, BlInteger n)
{
	//bf.outinteger (n);
	bf.outstring (to_string (n) );
	return bf;
}

BlFile & operator << (BlFile & bf, BlLineNumber l)
{
	//bf.outlinenumber (l);
	std::ostringstream oss;
	oss << std::setw (7) << l;
	bf.outstring (oss.str () );
        return bf;
}

BlFile & operator << (BlFile & bf, unsigned short n)
{
	bf.outstring (to_string (n) );
	return bf;
}

void BlFile::putspaces (size_t n)
{
	outstring (std::string (n, ' ') );
}

void BlFile::tab ()
{
	outchar ('\t');
}

void BlFile::tab (size_t n)
{
	// Provisional
	outstring (std::string (n, ' ') );
}

void BlFile::endline ()
{
	outchar ('\n');
}

void BlFile::put (size_t)
{ throw ErrFileMode; }

void BlFile::get (size_t)
{ throw ErrFileMode; }

void BlFile::field (const std::vector <field_element> &)
{ throw ErrFileMode; }

// assign doesn't throw because we call it for all open files,
// those that are no random files or does nor have the var in
// their fields just ignore it.
void BlFile::assign (const std::string &, const std::string &, Align)
{ }

size_t BlFile::getwidth ()
{ throw ErrFileMode; }

void BlFile::movecharforward ()
{ throw ErrFileMode; }

void BlFile::movecharforward (size_t)
{ throw ErrFileMode; }

void BlFile::movecharback ()
{ throw ErrFileMode; }

void BlFile::movecharback (size_t)
{ throw ErrFileMode; }

void BlFile::movecharup ()
{ throw ErrFileMode; }

void BlFile::movecharup (size_t)
{ throw ErrFileMode; }

void BlFile::movechardown ()
{ throw ErrFileMode; }

void BlFile::movechardown (size_t)
{ throw ErrFileMode; }

void BlFile::showcursor ()
{ throw ErrFileMode; }

void BlFile::hidecursor ()
{ throw ErrFileMode; }

std::string BlFile::getkey ()
{ throw ErrFileMode; }

std::string BlFile::read (size_t)
{ throw ErrFileMode; }

void BlFile::gotoxy (int, int)
{ throw ErrFileMode; }

void BlFile::setcolor (int)
{ throw ErrFileMode; }

int BlFile::getcolor ()
{ throw ErrFileMode; }

void BlFile::setbackground (int)
{ throw ErrFileMode; }

int BlFile::getbackground ()
{ throw ErrFileMode; }

void BlFile::cls ()
{ throw ErrFileMode; }

std::string BlFile::copychr ()
{ throw ErrFileMode; }

int BlFile::pos ()
{ throw ErrFileMode; }

int BlFile::vpos ()
{ throw ErrFileMode; }

void BlFile::tag ()
{ throw ErrFileMode; }

void BlFile::tagoff ()
{ /* Ignored */ }

bool BlFile::istagactive ()
{ return false; }

void BlFile::inverse (bool)
{ throw ErrFileMode; }

bool BlFile::getinverse ()
{ throw ErrFileMode; }

//***********************************************
//              BlFileConsole
//***********************************************

BlFileConsole::BlFileConsole (std::istream & nin, std::ostream & nout) :
        BlFile (OpenMode (Input | Output) ),
        in (nin),
        out (nout),
        ttyin (isatty (0) ),
        ttyout (isatty (1) )
        //#ifndef _Windows
        #ifndef BLASSIC_USE_WINDOWS
        , xpos (0)
        #endif
{
}

bool BlFileConsole::eof ()
{
	int c= in.get ();
	if (! in || c == EOF)
		return true;
	in.unget ();
	return false;
}

void BlFileConsole::flush ()
{
        out << std::flush;
}

size_t BlFileConsole::getwidth ()
{
	return ::getwidth ();
}

void BlFileConsole::movecharforward ()
{
	::movecharforward ();
}

void BlFileConsole::movecharforward (size_t n)
{
	::movecharforward (n);
}

void BlFileConsole::movecharback ()
{
	::movecharback ();
}

void BlFileConsole::movecharback (size_t n)
{
	::movecharback (n);
}

void BlFileConsole::movecharup ()
{
	::movecharup ();
}

void BlFileConsole::movecharup (size_t n)
{
	::movecharup (n);
}

void BlFileConsole::movechardown ()
{
	::movechardown ();
}

void BlFileConsole::movechardown (size_t n)
{
	::movechardown (n);
}

void BlFileConsole::showcursor ()
{
	::showcursor ();
}

void BlFileConsole::hidecursor ()
{
	::hidecursor ();
}

std::string BlFileConsole::getkey ()
{
	TraceFunc tr ("BlFileConsole::getkey");

	std::string str= ::getkey ();

	#ifdef __WIN32__

	if (ttyin && str.size () == 1)
	{
		char c= str [0];
		OemToCharBuff (& c, & c, 1);
		return std::string (1, c);
	}
	
	#endif

	return str;
}

void BlFileConsole::getline (std::string & str)
{
	TraceFunc tr ("BlFileConsole::getline");

	//#ifdef __WIN32__

	if (ttyin)
	{
		std::string auxstr;
		//int inicol= getcursorx ();
		int inicol= pos ();
		while (! editline (* this, auxstr, 0, inicol) )
			continue;
		swap (str, auxstr);
	}
	else
		std::getline (in, str);

	//#else

	//std::getline (in, str);

	//#endif

	if (fInterrupted)
	{
		in.clear ();
		str.erase ();
		return;
	}

        #ifdef __WIN32__

	if (ttyin)
	{
	        size_t l= str.size ();
		util::auto_buffer <char> aux (l);
	        OemToCharBuff (str.data (), aux, l);
        	str= std::string (aux, l);
        }

        #endif
}

std::string BlFileConsole::read (size_t n)
{
	util::auto_buffer <char> buf (n);
	in.read (buf, n);
	return std::string (buf, n);
}

void BlFileConsole::tab (size_t n)
{
	if (graphics::ingraphicsmode () )
	{
		graphics::tab (n);
	}
	else
	{
		int p= pos ();
		if (p > static_cast <int> (n) )
		{
			outchar ('\n');
			p= pos ();
		}
		outstring (std::string (n - p, ' ') );
	}
}

void BlFileConsole::outstring (const std::string & str)
{
        if (graphics::ingraphicsmode () )
        {
                graphics::stringout (str);
                return;
        }

        #ifdef __WIN32__

	if (ttyout)
	{
		size_t l= str.size ();
		util::auto_buffer <char> aux (l + 1);
		CharToOemBuff (str.data (), aux, l);
		aux [l]= 0;
		out << aux;
	}
	else
		out << str;

        #else

        out << str;
        updatepos (xpos, str);

        #endif

	if (! out)
	{
		out.clear ();
		//throw std::runtime_error ("Al diablo");
	}
}

void BlFileConsole::outchar (char c)
{
        if (graphics::ingraphicsmode () )
        {
                graphics::charout (c);
                return;
        }
        #ifdef __WIN32__
	if (ttyout)
	        CharToOemBuff (& c, & c, 1);
	#endif

	if (c == '\n')
		out << endl;
	else
		out << c;

	//#ifndef _Windows
        #ifndef BLASSIC_USE_WINDOWS
	updateposchar (xpos).operator () (c);
	#endif

	if (! out)
	{
		out.clear ();
		//throw std::runtime_error ("Al diablo");
	}
}

#if 0

void BlFileConsole::outnumber (BlNumber n)
{
	if (graphics::ingraphicsmode () )
		graphics::stringout (to_string (n) );
	else
		out << n;
}

void BlFileConsole::outinteger (BlInteger n)
{
	if (graphics::ingraphicsmode () )
		graphics::stringout (to_string (n) );
	else
		out << n;
}

#endif

void BlFileConsole::gotoxy (int x, int y)
{
	::gotoxy (x, y);
	//#ifndef _Windows
	#ifndef BLASSIC_USE_WINDOWS
	xpos= x;
	#endif
}

void BlFileConsole::setcolor (int color)
{
	textcolor (color);
}

void BlFileConsole::setbackground (int color)
{
	textbackground (color);
}

void BlFileConsole::cls ()
{
	::cls ();
	//#ifndef _Windows
	#ifndef BLASSIC_USE_WINDOWS
	xpos= 0;
	#endif
}

int BlFileConsole::pos ()
{
	//#ifdef _Windows
	#ifndef BLASSIC_USE_WINDOWS
	return getcursorx ();
	#else
	return xpos;
	#endif
}

//***********************************************
//		BlFileWindow
//***********************************************

BlFileWindow::BlFileWindow (BlChannel ch) :
        BlFile (OpenMode (Input | Output) ),
	ch (ch)
{
	if (ch != 0)
		throw ErrBlassicInternal;
}

BlFileWindow::BlFileWindow (BlChannel ch, int x1, int x2, int y1, int y2) :
        BlFile (OpenMode (Input | Output) ),
	ch (ch)
{
	graphics::definewindow (ch, x1, x2, y1, y2);
}

BlFileWindow::~BlFileWindow ()
{
	graphics::undefinewindow (ch);
}

void BlFileWindow::reset (int x1, int x2, int y1, int y2)
{
	graphics::definewindow (ch, x1, x2, y1, y2);
}

bool BlFileWindow::eof ()
{
	return false;
}

void BlFileWindow::flush ()
{
	// Nothing to do
}

size_t BlFileWindow::getwidth ()
{
	return graphics::getlinewidth (ch);
}

void BlFileWindow::movecharforward ()
{
	graphics::movecharforward (ch, 1);
}

void BlFileWindow::movecharforward (size_t n)
{
	graphics::movecharforward (ch, n);
}

void BlFileWindow::movecharback ()
{
	graphics::movecharback (ch, 1);
}

void BlFileWindow::movecharback (size_t n)
{
	graphics::movecharback (ch, n);
}

void BlFileWindow::movecharup ()
{
	graphics::movecharup (ch, 1);
}

void BlFileWindow::movecharup (size_t n)
{
	graphics::movecharup (ch, n);
}

void BlFileWindow::movechardown ()
{
	graphics::movechardown (1);
}

void BlFileWindow::movechardown (size_t n)
{
	graphics::movechardown (n);
}

void BlFileWindow::showcursor ()
{
	graphics::showcursor (ch);
}

void BlFileWindow::hidecursor ()
{
	graphics::hidecursor (ch);
}

std::string BlFileWindow::getkey ()
{
	std::string str;
	do
	{
		str= graphics::getkey ();
	} while (str.empty () );
	return str;
}

void BlFileWindow::getline (std::string & str)
{
	std::string auxstr;
	int inicol= graphics::xpos (ch);
	while (! editline (* this, auxstr, 0, inicol) )
		continue;
	swap (str, auxstr);
}

std::string BlFileWindow::read (size_t)
{
	throw ErrNotImplemented;
}

void BlFileWindow::tab ()
{
	graphics::tab (ch);
}

void BlFileWindow::tab (size_t n)
{
	graphics::tab (ch, n);
}

void BlFileWindow::endline ()
{
	outstring ("\r\n");
}

void BlFileWindow::gotoxy (int x, int y)
{
	graphics::gotoxy (ch, x, y);
}

void BlFileWindow::setcolor (int color)
{
	graphics::setcolor (ch, color);
}

int BlFileWindow::getcolor ()
{
	return graphics::getcolor (ch);
}

void BlFileWindow::setbackground (int color)
{
	graphics::setbackground (ch, color);
}

int BlFileWindow::getbackground ()
{
	return graphics::getbackground (ch);
}

void BlFileWindow::outstring (const std::string & str)
{
	graphics::stringout (ch, str);
}

void BlFileWindow::outchar (char c)
{
	graphics::charout (ch, c);
}

void BlFileWindow::cls ()
{
	graphics::cls (ch);
}

std::string BlFileWindow::copychr ()
{
	return graphics::copychr (ch);
}

int BlFileWindow::pos ()
{
	return graphics::xpos (ch);
}

int BlFileWindow::vpos ()
{
	return graphics::ypos (ch);
}

void BlFileWindow::tag ()
{
	graphics::tag (ch);
}

void BlFileWindow::tagoff ()
{
	graphics::tagoff (ch);
}

bool BlFileWindow::istagactive ()
{
	return graphics::istagactive (ch);
}

void BlFileWindow::inverse (bool active)
{
	graphics::inverse (ch, active);
}

bool BlFileWindow::getinverse ()
{
	return graphics::getinverse (ch);
}

//***********************************************
//		BlFileOut
//***********************************************

BlFileOut::BlFileOut () : BlFile (Output)
{ }

BlFileOut::BlFileOut (OpenMode mode) : BlFile (mode)
{ }

void BlFileOut::flush ()
{
	ofs () << std::flush;
}

void BlFileOut::outstring (const std::string & str)
{
	ofs () << str;
}

void BlFileOut::outchar (char c)
{
	ofs () << c;
}

#if 0

void BlFileOut::outnumber (BlNumber n)
{
	ofs () << n;
}

void BlFileOut::outinteger (BlInteger n)
{
	ofs () << n;
}

void BlFileOut::outlinenumber (BlLineNumber l)
{
	ofs () << std::setw (7) << l;
}

#endif

//***********************************************
//		BlFileOutString
//***********************************************

BlFileOutString::BlFileOutString ()
{ }

std::string BlFileOutString::str ()
{
	return oss.str ();
}

std::ostream & BlFileOutString::ofs ()
{
	return oss;
}

//***********************************************
//		BlFileOutput
//***********************************************

BlFileOutput::BlFileOutput (std::ostream & os) :
	os (os)
{
}

std::ostream & BlFileOutput::ofs ()
{
	return os;
}

//***********************************************
//              BlFileRegular
//***********************************************

BlFileRegular::BlFileRegular (const std::string & name, OpenMode nmode) :
        BlFileOut (nmode)
{
        using std::ios;

        ios::openmode mode= ios::in;
        switch (nmode)
        {
        case Input:
                mode= ios::in; break;
        case Output:
                mode= ios::out; break;
        case Append:
                mode= ios::out | ios::ate; break;
	default:
		throw ErrBlassicInternal;
        }
        fs.open (name.c_str (), mode);
        if (! fs.is_open () )
        	throw ErrFileNotFound;
}

bool BlFileRegular::eof ()
{
	int c= fs.get ();
	if (! fs || c == EOF)
		return true;
	fs.unget ();
	return false;
}

void BlFileRegular::flush ()
{
        fs << std::flush;
}

void BlFileRegular::getline (std::string & str)
{
	std::getline (fs, str);
	if (! fs)
	{
		if (fs.eof () )
			throw ErrPastEof;
		else
			throw ErrFileRead;
	}
}

std::string BlFileRegular::read (size_t n)
{
	util::auto_buffer <char> buf (n);
	fs.read (buf, n);
	if (! fs)
	{
		if (fs.eof () )
			throw ErrPastEof;
		else
			throw ErrFileRead;
	}
	return std::string (buf, n);
}

std::ostream & BlFileRegular::ofs ()
{
	return fs;
}

//***********************************************
//              BlFileRandom
//***********************************************

inline void BlFileRandom::field_chunk::getvar (char * buf) const
{
	assignvarstring (name, std::string (buf + pos, size) );
}

BlFileRandom::BlFileRandom (const std::string & name, size_t record_len) :
	BlFile (Random),
	len (record_len),
	actual (0),
	buf (record_len)
{
        using std::ios;

	fs.open (name.c_str (), ios::in | ios::out | ios::binary);
	if (! fs.is_open () )
		throw "Al cuerno con todo";
	std::fill_n (buf.begin (), len, '\0');
}

void BlFileRandom::put (size_t pos)
{
	if (pos != 0)
	{
		actual= pos - 1;
		fs.seekp (actual * len);
	}
	fs.write (buf, len);
	++actual;
}

namespace {

class GetVar {
public:
	GetVar (char * buf) : buf (buf) { }
	void operator () (const BlFileRandom::field_chunk & chunk)
	{
		chunk.getvar (buf);
	}
private:
	char * buf;
};

} // namespace

void BlFileRandom::get (size_t pos)
{
        using std::string;

	if (pos != 0)
		actual= pos - 1;
        fs.clear ();
        fs.seekg (actual * len);
	fs.read (buf, len);
	std::streamsize r= fs.gcount ();
	if (r < std::streamsize (len) )
		std::fill_n (buf.begin () + r, len - r, ' ');
	std::for_each (chunk.begin (), chunk.end (), GetVar (buf) );
	++actual;
}

namespace {

class MakeChunk {
public:
	MakeChunk (size_t len) :
		len (len),
		pos (0)
	{ }
	BlFileRandom::field_chunk operator ()
		(const BlFile::field_element & elem)
	{
		size_t size= elem.size;
		BlFileRandom::field_chunk fc;
		fc.name= elem.name;
		fc.pos= pos;
		fc.size= size;
		pos+= size;
		if (pos > len)
			throw ErrFieldOverflow;
		return fc;
	}
private:
	const size_t len;
	size_t pos;
};

} // namespace

void BlFileRandom::field (const std::vector <field_element> & elem)
{
        chunk.clear ();
	std::transform (elem.begin (), elem.end (),
		std::back_inserter (chunk), MakeChunk (len) );
}

namespace {

class name_is {
public:
	name_is (const std::string & name) : name (name)
	{ }
	bool operator () (const BlFileRandom::field_chunk & chunk)
	{
		return chunk.name == name;
	}
private:
	const std::string & name;
};

} // namespace

void BlFileRandom::assign (const std::string & name,
                const std::string & value, Align align)
{
	vchunk::iterator pe= std::find_if (chunk.begin (), chunk.end (),
		name_is (name) );
	if (pe == chunk.end () )
		return;
        char * init= buf + pe->pos;
        std::string str;
        std::string::size_type l= value.size ();
        if (align == AlignLeft)
        {
                if (l < pe->size)
                        str= value + std::string (pe->size - l, ' ');
                else
                        str= value.substr (0, pe->size);
        }
        else
        {
                if (l < pe->size)
                        str= std::string (pe->size - l, ' ') + value;
                else
                        str= value.substr (l - pe->size);
        }
        ASSERT (str.size () == pe->size);
	std::copy (str.begin (), str.end (), init);
}

//***********************************************
//              BlFilePopen
//***********************************************

BlFilePopen::BlFilePopen (const std::string & str, OpenMode nmode) :
        BlFile (nmode),
        bufpos (0),
        bufread (0)
{
        TraceFunc tr ("BlFilePopen::BlFilePopen");

        //#ifdef _Windows
	#ifdef BLASSIC_USE_WINDOWS

        std::string command;
        {
                const char * strCommand= getenv ("COMSPEC");
                if (strCommand == NULL)
                        strCommand= "C:\\COMMAND.COM";
                command= strCommand;
        }
        command+= " /C ";
        command+= str;

        HANDLE hread, hwrite;
        HANDLE hchild;
	#if 0
        const HANDLE haux= INVALID_HANDLE_VALUE;
	#else
	const HANDLE haux= CreateFile ("NUL",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	#endif
        SECURITY_ATTRIBUTES sec;
        sec.nLength= sizeof sec;
        sec.lpSecurityDescriptor= NULL;
        sec.bInheritHandle= TRUE;
        if (CreatePipe (& hread, & hwrite, & sec, 0) == 0)
                throw "Al diablo";
        STARTUPINFO start;
        start.cb= sizeof start;
        GetStartupInfo (& start);
        start.dwFlags= STARTF_USESTDHANDLES;
        DWORD creationflags= 0;
        switch (nmode)
        {
        case BlFile::Input:
                hpipe= hread;
                hchild= hwrite;
                start.hStdInput= haux;
                start.hStdOutput= hwrite;
                start.hStdError= hwrite;
                break;
        case BlFile::Output:
        	creationflags= DETACHED_PROCESS;
                hpipe= hwrite;
                hchild= hread;
                start.hStdInput= hread;
                start.hStdOutput= haux;
                start.hStdError= haux;
                break;
        default:
		cerr << "Bad file mode in popen" << endl;
		throw ErrBlassicInternal;
        }

        PROCESS_INFORMATION procinfo;
        BOOL createresult= CreateProcess (
                NULL, (char *) command.c_str (),
                NULL, NULL,
                TRUE, creationflags,
                NULL, NULL, & start, & procinfo);

        CloseHandle (hchild);
	CloseHandle (haux);

        if (createresult == 0)
        {
                CloseHandle (hpipe);
                throw "Cuernos";
        }
        CloseHandle (procinfo.hProcess);
        CloseHandle (procinfo.hThread);

	#else // No windows

	const char * type;
	switch (nmode)
	{
	case BlFile::Input:
		type= "r";
		break;
	case BlFile::Output:
		type= "w";
		break;
	default:
		cerr << "Bad file mode in popen" << endl;
		throw ErrBlassicInternal;
	}
	hpipe= popen (str.c_str (), type);
        if (hpipe == NULL)
                throw "Cuernos";

        #endif
}

BlFilePopen::~BlFilePopen ()
{
        TraceFunc tr ("BlFilePopen:~BlFilePopen");

	//#ifdef _Windows
	#ifdef BLASSIC_USE_WINDOWS

        CloseHandle (hpipe);

	#else

	pclose (hpipe);

	#endif
}

void BlFilePopen::readbuffer ()
{
        TraceFunc tr ("BlFilePopen::readbuffer");

	//#ifdef _Windows
	#ifdef BLASSIC_USE_WINDOWS

        DWORD bytesread= 0;
        ReadFile (hpipe, buffer, bufsize, & bytesread, NULL);

	#else

	size_t bytesread= ::read (fileno (hpipe), buffer, bufsize);
	if (bytesread == size_t (-1) )
		bytesread= 0;

	#endif

        bufread= bytesread;
        bufpos= 0;
}

char BlFilePopen::getcharfrombuffer ()
{
        if (bufpos >= bufread)
        {
                readbuffer ();
                if (bufread == 0)
                        return '\0';
        }
        return buffer [bufpos++];
}


bool BlFilePopen::eof ()
{
        TraceFunc tr ("BlFilePopen::eof");

        if (bufpos < bufread)
	        return false;
        readbuffer ();
        return bufread == 0;
}

void BlFilePopen::flush ()
{
}

void BlFilePopen::getline (std::string & str)
{
        TraceFunc tr ("BlFilePopen::getline");

        str= std::string ();
        char c;
        while ( (c= getcharfrombuffer () ) != '\r' && c != '\n' && c != '\0')
                str+= c;

        //#ifdef _Windows
	#ifdef BLASSIC_USE_WINDOWS

        if (c == '\r')
                getcharfrombuffer ();
        //Sleep (10);

        #endif
}

std::string BlFilePopen::read (size_t n)
{
	std::string str;
	for (size_t i= 0; i < n; ++i)
		str+= getcharfrombuffer ();
	return str;
}

void BlFilePopen::outstring (const std::string & str)
{
	TraceFunc tr ("BlFilePopen::outstring");

	const char * to= str.data ();
        std::string::size_type l= str.size ();

        //#ifdef _Windows
	#ifdef BLASSIC_USE_WINDOWS

        DWORD written;
        WriteFile (hpipe, to, l, & written, NULL);

	#else

	write (fileno (hpipe), to, l);

        #endif
}

void BlFilePopen::outchar (char c)
{
        //#ifdef _Windows
	#ifdef BLASSIC_USE_WINDOWS

        DWORD written;
        WriteFile (hpipe, & c, 1, & written, NULL);

	#else

	write (fileno (hpipe), & c, 1);

        #endif
}

//***********************************************
//              BlFileSocket
//***********************************************

BlFileSocket::BlFileSocket (const std::string & host, short port) :
	BlFile (OpenMode (Input | Output) ),
	socket (host, port)
{
}

BlFileSocket::~BlFileSocket ()
{
}

bool BlFileSocket::eof ()
{
	return socket.eof ();
}

void BlFileSocket::flush ()
{
}

void BlFileSocket::getline (std::string & str)
{
	str= socket.readline ();
}

std::string BlFileSocket::read (size_t n)
{
	util::auto_buffer <char> buf (n);
	int r= socket.read (buf, n);
	std::string result;
	if (r > 0)
		result.assign (buf, r);
	return result;
}

void BlFileSocket::outstring (const std::string & str)
{
	socket.write (str);
}

void BlFileSocket::outchar (char c)
{
	socket.write (& c, 1);
}

// Fin de file.cpp
