// trace.h

#include <cstddef>
#include <string>

class TraceFunc {
public:
	TraceFunc (const char * strFuncName)
	#ifndef NDEBUG
		;
	#else
	{ }
	#endif
	~TraceFunc ()
	#ifndef NDEBUG
		;
	#else
	{ }
	#endif
	void message (const std::string & str)
	#ifndef NDEBUG
		;
	#else
	{ }
	#endif
	static void show (int);
private:
	#ifndef NDEBUG
	const char * strfunc;
	TraceFunc * * previous;
	TraceFunc * next;
	#endif
};

// Fin de trace.h
