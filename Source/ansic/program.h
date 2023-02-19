//	program.h
// Revision 4-jun-2003

#ifndef PROGRAM_H_
#define PROGRAM_H_

#include "blassic.h"
#include "codeline.h"
#include "file.h"

enum StatusProgram {
	ProgramEnded,
	ProgramReadyToRun,
	ProgramRunning,
	ProgramStopped,
	ProgramJump,
	ProgramGoto
};

class Program {
public:
	Program ();
	~Program ();
	BlChar * programptr (); // { return program; }
	BlLineNumber getlabel (const std::string & str);
	CodeLine getfirstline ();
	//CodeLine getnextline (CodeLine & line);
	void getnextline (CodeLine & line);
	//BlLineNumber getnextnum (CodeLine & line);
	//CodeLine getline (BlLineNumber num);
	void getline (BlLineNumber num, CodeLine & line);
	void getline (ProgramPos pos, CodeLine & line);
	void insert (const CodeLine & code);
	void deletelines (BlLineNumber iniline, BlLineNumber endline);
	void list (BlLineNumber iniline, BlLineNumber endline,
                BlFile & out) const;
	void save (const std::string & name) const;
	void load (const std::string & name);
	void merge (const std::string & name);
	void renew ();
	void renum (BlLineNumber blnNew, BlLineNumber blnOld,
		BlLineNumber blnInc);
	class Internal;
	// Public because is used by TextLoader.
private:
	Program (const Program &); // Prohibido
	Program & operator= (const Program &); // Prohibido
	//class Internal;
	Internal * pin;
};

#endif

// Fin de program.h
