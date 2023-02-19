// edit.h

#include "blassic.h"
#include "file.h"
#include "program.h"

bool editline (BlFile & bf, std::string & str, size_t npos, size_t inicol= 0);
bool editline (BlFile & bf, Program & program, BlLineNumber bln,
	std::string & str);
bool editline (BlFile & bf, Program & program, BlLineNumber bln);

// End of edit.h
