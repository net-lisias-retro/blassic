// file.h
// Revision 13-aug-2003

#ifndef INCLUDE_FILE_H_
#define INCLUDE_FILE_H_

#ifdef __BORLANDC__
#pragma warn -8022
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#ifndef _Windows
#include <stdio.h> // FILE needed
#else
#include <windows.h> // HANDLE needed
#endif

#include "blassic.h"
#include "socket.h"
#include "util.h"

class BlFile {
public:
        enum OpenMode { Input= 1, Output= 2, Append= 6, Random= 8 };
        struct field_element {
                size_t size;
                std::string name;
                field_element (size_t n, const std::string & str) :
                        size (n), name (str)
                { }
        };
        enum Align { AlignRight, AlignLeft };

        BlFile (OpenMode nmode);
        virtual ~BlFile ();
	virtual bool eof ();
	virtual void flush ();
	virtual bool istextwindow () { return false; }
	virtual size_t getwidth ();
	virtual void movecharforward ();
	virtual void movecharforward (size_t n);
	virtual void movecharback ();
	virtual void movecharback (size_t n);
	virtual void movecharup ();
	virtual void movecharup (size_t n);
	virtual void movechardown ();
	virtual void movechardown (size_t n);
	virtual void showcursor ();
	virtual void hidecursor ();
	virtual std::string getkey ();
	virtual void getline (std::string & str);
	char delimiter () { return cDelimiter; }
	void delimiter (char delim) { cDelimiter= delim; }
	char quote () { return cQuote; }
	void quote (char qu) { cQuote= qu; }
	char escape () { return cEscape; }
	void escape (char esc) { cEscape= esc; }
        friend BlFile & operator << (BlFile & bf, const std::string & str);
        friend BlFile & operator << (BlFile & bf, char c);
        friend BlFile & operator << (BlFile & bf, BlNumber n);
        friend BlFile & operator << (BlFile & bf, BlInteger n);
        friend BlFile & operator << (BlFile & bf, BlLineNumber l);
        friend BlFile & operator << (BlFile & bf, unsigned short n);
	void putspaces (size_t n);
	virtual void tab ();
	virtual void tab (size_t n);
	virtual void endline ();
	virtual void put (size_t pos);
        virtual void get (size_t pos);
        virtual void field (const std::vector <field_element> & elem);
        virtual void assign (const std::string & name,
                const std::string & value, Align align);
	virtual std::string read (size_t n);
	virtual void gotoxy (int x, int y);
	virtual void setcolor (int color);
	virtual int getcolor ();
	virtual void setbackground (int color);
	virtual int getbackground ();
	virtual void cls ();
	virtual std::string copychr ();
	virtual int pos ();
	virtual int vpos ();
	virtual void tag ();
	virtual void tagoff ();
	virtual bool istagactive ();
	virtual void inverse (bool active);
	virtual bool getinverse ();
private:
        virtual void outstring (const std::string & str);
        virtual void outchar (char c);
        //virtual void outnumber (BlNumber n);
	//virtual void outinteger (BlInteger n);
        //virtual void outlinenumber (BlLineNumber l);

        OpenMode mode;
	char cDelimiter, cQuote, cEscape;
};

class BlFileConsole : public BlFile {
public:
        BlFileConsole (std::istream & nin, std::ostream & nout);
	virtual bool eof ();
	virtual void flush ();
	virtual size_t getwidth ();
	virtual void movecharforward ();
	virtual void movecharforward (size_t n);
	virtual void movecharback ();
	virtual void movecharback (size_t n);
	virtual void movecharup ();
	virtual void movecharup (size_t n);
	virtual void movechardown ();
	virtual void movechardown (size_t n);
	virtual void showcursor ();
	virtual void hidecursor ();
	virtual std::string getkey ();
	void getline (std::string & str);
	std::string read (size_t n);
	void tab (size_t n);
	void gotoxy (int x, int y);
	virtual void setcolor (int color);
	virtual void setbackground (int color);
	virtual void cls ();
	int pos ();
private:
        void outstring (const std::string & str);
        void outchar (char c);
	//void outnumber (BlNumber n);
	//void outinteger (BlInteger n);

        std::istream & in;
        std::ostream & out;
        bool ttyin, ttyout;
        #ifndef _Windows
        int xpos;
        #endif
};

class BlFileWindow : public BlFile {
public:
	BlFileWindow (BlChannel ch);
	BlFileWindow (BlChannel ch, int x1, int x2, int y1, int y2);
	~BlFileWindow ();
	void reset (int x1, int x2, int y1, int y2);
	virtual bool eof ();
	virtual void flush ();
	virtual size_t getwidth ();
	virtual void movecharforward ();
	virtual void movecharforward (size_t n);
	virtual void movecharback ();
	virtual void movecharback (size_t n);
	virtual void movecharup ();
	virtual void movecharup (size_t n);
	virtual void movechardown ();
	virtual void movechardown (size_t n);
	virtual void showcursor ();
	virtual void hidecursor ();
	virtual std::string getkey ();
	virtual bool istextwindow () { return true; }
	void getline (std::string & str);
	std::string read (size_t n);
	void tab ();
	void tab (size_t n);
	void endline ();
	void gotoxy (int x, int y);
	virtual void setcolor (int color);
	virtual int getcolor ();
	virtual void setbackground (int color);
	virtual int getbackground ();
	virtual void cls ();
	std::string copychr ();
	int pos ();
	int vpos ();
	void tag ();
	void tagoff ();
	bool istagactive ();
	void inverse (bool active);
	bool getinverse ();
private:
        void outstring (const std::string & str);
        void outchar (char c);

	BlChannel ch;
};

class BlFileOut : public BlFile {
public:
	BlFileOut ();
	BlFileOut (OpenMode mode);
	void flush ();
protected:
	virtual std::ostream & ofs ()= 0;
private:
	void outstring (const std::string & str);
	void outchar (char c);
	//void outnumber (BlNumber n);
	//void outinteger (BlInteger n);
	//void outlinenumber (BlLineNumber l);
};

class BlFileOutString : public BlFileOut {
public:
	BlFileOutString ();
	std::string str ();
private:
	std::ostream & ofs ();
	std::ostringstream oss;
};

class BlFileOutput : public BlFileOut {
public:
	BlFileOutput (std::ostream & os);
private:
	std::ostream & ofs ();
	std::ostream & os;
};

class BlFileRegular : public BlFileOut {
public:
        BlFileRegular (const std::string & name, OpenMode mode);
	void getline (std::string & str);
	bool eof ();
	void flush ();
	std::string read (size_t n);
private:
	std::ostream & ofs ();
        std::fstream fs;
};

class BlFileRandom : public BlFile {
public:
	BlFileRandom (const std::string & name, size_t record_len);
	void put (size_t pos);
	void get (size_t pos);
        void field (const std::vector <field_element> & elem);
        virtual void assign (const std::string & name,
                const std::string & value, Align align);
        struct field_chunk {
                std::string name;
                size_t pos;
                size_t size;
		inline void getvar (char * buf) const;
        };
        typedef std::vector <field_chunk> vchunk;
private:
	std::fstream fs;
	size_t len;
	size_t actual;
	//auto_array buf;
	util::auto_buffer <char> buf;
	vchunk chunk;
};

class BlFilePopen : public BlFile {
public:
        BlFilePopen (const std::string & str, OpenMode mode);
        ~BlFilePopen ();
	void getline (std::string & str);
	bool eof ();
	void flush ();
	std::string read (size_t n);
private:
        void outstring (const std::string & str);
        void outchar (char c);

        static const size_t bufsize= 4096;
        char buffer [bufsize];
        size_t bufpos, bufread;
        void readbuffer ();
        char getcharfrombuffer ();
        #ifdef _Windows
        HANDLE hpipe;
        #else
	FILE * hpipe;
        #endif
};

class BlFileSocket : public BlFile {
public:
	BlFileSocket (const std::string & host, short port);
	~BlFileSocket ();
	void getline (std::string & str);
	bool eof ();
	void flush ();
	std::string read (size_t n);
private:
	void outstring (const std::string & str);
	void outchar (char c);
	
	TcpSocketClient socket;
};

#endif

// Fin de file.h
