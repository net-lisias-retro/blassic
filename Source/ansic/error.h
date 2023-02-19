// error.h
// Revision 17-may-2003

#ifndef ERROR_H_
#define ERROR_H_

#include "blassic.h"

#include <string>
#include <iostream>

const BlErrNo
	ErrNoError=             0,
	ErrSyntax=              1,
	ErrMismatch=            2,
	ErrGosubWithoutReturn=  3,
	ErrReturnWithoutGosub=  4,
	ErrNextWithoutFor=      5,
	ErrNotImplemented=      6,
	ErrDivZero=             7,
	ErrDataExhausted=       8,
	ErrInvalidCommand=      9,
	ErrPolite=             10,
        ErrBadSubscript=       11,
        ErrOutMemory=          12,
        ErrAlreadyDim=         13,
        ErrNoContinue=         14,
        ErrFileNumber=         15,
	ErrFileMode=           16,
	ErrFileAlreadyOpen=    17,
	ErrFileRead=           18,
	ErrFileWrite=          19,
	ErrUntilWithoutRepeat= 20,
        ErrWendWithoutWhile=   21,
        ErrWhileWithoutWend=   22,
        ErrBlassicInternal=    23,
        ErrNoDynamicLibrary=   24,
        ErrNoDynamicSymbol=    25,
        ErrCannotResume=       26,
	ErrNoLabel=            27,
	ErrMisplacedLocal=     28,
        ErrFieldOverflow=      29,
	ErrFileNotFound=       30,
	ErrLineExhausted=      31,
        ErrFunctionNoDefined=  32,
        ErrIncompleteDef=      33,
        ErrInvalidDirect=      34,
        ErrBadRecord=          35,
        ErrFunctionCall=       36,
        ErrSocket=             37,
        ErrRenameFile=         38,
	ErrOperatingSystem=    39,
	ErrPastEof=            40,
	ErrNoGraphics=         41,
	ErrImproperArgument=   42,
	ErrDomain=             43,
	ErrRange=              44;

class BlError {
public:
	#if 0
	BlError (BlErrNo nerr, BlLineNumber nline, BlChunk nchunk= 0) :
		err (nerr),
		//line (nline),
                //chunk (nchunk)
                pos (nline, nchunk)
	{ }
	#endif
	BlError () :
		err (0),
		pos (0, 0)
	{ }
	BlError (BlErrNo nerr, ProgramPos pos) :
		err (nerr),
		pos (pos)
	{ }
        BlErrNo geterr () const { return err; }
        //BlLineNumber getline () { return line; }
        //BlChunk getchunk () { return chunk; }
	ProgramPos getpos () const { return pos; }
	friend std::ostream & operator << (std::ostream & os,
		const BlError & be);
private:
	BlErrNo err;
	//BlLineNumber line;
        //BlChunk chunk;
	ProgramPos pos;
};

class BlBreak { };

std::string ErrStr (BlErrNo err);

#endif

// Fin de error.h
