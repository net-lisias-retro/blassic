// blassic.h

#ifndef BLASSIC_H_
#define BLASSIC_H_

#ifdef _Windows

#define BLASSIC_USE_WINDOWS

#ifndef __MT__
#define __MT__
#endif

#endif

#if ! defined __BORLANDC__ && __GNUC__ < 3
#define USE_HASH_MAP
#endif

#if defined __unix__ || defined __linux__ // Kylix defines only __linux__

#define BLASSIC_USE_X

#endif

#if defined __unix__ || defined __linux__ || defined __APPLE__

#define BLASSIC_USE_TERMINFO

#endif

#ifdef __linux__

// Uncomment next #define if you want to use the svgalib option
// or tell it to configure.
// Support for svgalib is currently outdated.
//#define BLASSIC_USE_SVGALIB

#endif

// Borland define _M_IX86, gcc define __i386__
#if defined (_M_IX86) || defined (__i386__)

#define BLASSIC_INTEL

// In other processor used the endianess is different and / or
// there are restrictions of alignment.
#endif

#ifdef __BORLANDC__
#pragma warn -8027
#endif

#include <string>
#include <vector>

typedef unsigned char BlChar;
typedef unsigned short BlCode;
typedef double BlNumber;
typedef long BlInteger;
const BlInteger BlIntegerMax= 2147483647;
typedef unsigned long BlLineNumber;
// We limit the max line number as if it were signed.
//const BlLineNumber BlMaxLineNumber= 0xFFFFFFFF;
const BlLineNumber BlMaxLineNumber= BlIntegerMax;
typedef unsigned long BlLineLength;
typedef unsigned short BlChunk;
typedef unsigned short BlErrNo;
typedef unsigned short BlChannel;

class ProgramPos {
public:
	ProgramPos () :
		num (0), chunk (0)
	{ }
	ProgramPos (BlLineNumber num) :
		num (num), chunk (0)
	{ }
	ProgramPos (BlLineNumber num, BlChunk chunk) :
		num (num), chunk (chunk)
	{ }
	void operator= (BlLineNumber num)
	{
		this->num= num;
		chunk= 0;
	}
	void nextchunk () { ++chunk; }
	void nextline () { ++num; chunk= 0; }
	operator bool () { return num != 0 || chunk != 0; }
	BlLineNumber getnum () const { return num; }
	BlChunk getchunk () const { return chunk; }
	void setchunk (BlChunk n) { chunk= n; }
private:
	BlLineNumber num;
	BlChunk chunk;
};

// Global variables:

//extern BlLineNumber blnAuto, blnAutoInc;

extern bool fInterrupted;

extern const std::string strPrompt;

// version.cpp
namespace version {
extern const unsigned short Major, Minor, Release;
}

class Exit {
public:
	Exit (int ncode) : iCode (ncode) { }
	int code () const { return iCode; }
private:
	int iCode;
};

std::string getprogramarg (size_t n);
void setprogramargs (const std::vector <std::string> & nargs);

inline BlInteger peek16 (const BlChar * p)
{
	#ifdef BLASSIC_INTEL
	return * reinterpret_cast <const unsigned short *> (p);
	#else
	return p [0] | (static_cast <unsigned short> (p [1]) << 8);
	#endif
}

inline void poke16 (BlChar * p, short n)
{
	#ifdef BLASSIC_INTEL
	* reinterpret_cast <short *> (p)= n;
	#else
	p [0]= BlChar (n & 0xFF);
	p [1]= BlChar ( (n >> 8) & 0xFF);
	#endif
}

inline BlInteger peek32 (const BlChar * p)
{
	#ifdef BLASSIC_INTEL
	return * reinterpret_cast <const BlInteger *> (p);
	#else
	return p [0] |
		(BlInteger (p [1]) << 8) |
		(BlInteger (p [2]) << 16) |
		(BlInteger (p [3]) << 24);
	#endif
}

inline void poke32 (BlChar * p, BlInteger n)
{
	#ifdef BLASSIC_INTEL
	* reinterpret_cast <BlInteger *> (p)= n;
	#else
	p [0]= BlChar (n & 0xFF);
	p [1]= BlChar ( (n >> 8) & 0xFF);
	p [2]= BlChar ( (n >> 16) & 0xFF);
	p [3]= BlChar (n >> 24);
	#endif
}

inline std::string makestring (const BlInteger & n)
{
	#ifdef BLASSIC_INTEL
	return std::string (reinterpret_cast <const char *> (& n), 4);
	#else
	return std::string (1, char (n & 0xFF) ) +
		char ( (n >> 8) & 0xFF) +
		char ( (n >> 16) & 0xFF) +
		char (n >> 24);
	#endif
}

#endif // BLASSIC_H_

// Fin de blassic.h
