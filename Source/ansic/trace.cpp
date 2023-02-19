// trace.cpp

#include "trace.h"

#if __BORLANDC__ >= 0x0560
#pragma warn -8091
#endif

#include <iostream>
using std::cerr;
using std::endl;
using std::ostream;
#include <fstream>
using std::ofstream;
#include <string>
using std::string;
#include <string.h>

#include <cstdlib>
using std::getenv;

#include <stdexcept>

namespace {

#ifndef NDEBUG

ostream * pout= 0;
bool flag= true;
size_t indent= 0;

TraceFunc * initial= NULL;
TraceFunc * * lastpos= & initial;

ostream * opentracefile (const char * str)
{
	std::ofstream * pof=
		new ofstream (str, std::ios::ate | std::ios::out);
	if (! pof->is_open () )
	{
		cerr << "Error al abrir " << str << endl;
		delete pof;
		pof= 0;
	}
	pof->clear ();
	return pof;
}

#endif

} // namespace

#ifndef NDEBUG

TraceFunc::TraceFunc (const char * strFuncName) :
	strfunc (strFuncName),
	next (NULL)
{
	if (flag)
	{
		flag= false;
		char * aux= getenv ("TRACEFUNC");
		if (aux)
		{
			if (strcmp (aux, "-") == 0)
				pout= & std::cerr;
			else
				pout= opentracefile (aux);
		}
	}

	//tracelist.push_back (this);
	previous= lastpos;
	* lastpos= this;
	lastpos= & next;

	if (pout)
		* pout << string (indent, ' ') << "Entra " << strfunc << endl;
	++indent;
}

TraceFunc::~TraceFunc ()
{
	--indent;
	if (pout)
	{
		* pout << string (indent, ' ') << "Sale ";
		if (std::uncaught_exception () )
			* pout << "(throwing) ";
		* pout << strfunc << endl;
	}
	if (lastpos != & next)
		throw std::logic_error ("Bad use of TraceFunc");
	lastpos= previous;
	* lastpos= NULL;
}

void TraceFunc::message (const std::string & strMes)
{
	if (pout)
	{
		* pout << string (indent, ' ') << strfunc <<
			": " << strMes << endl;
	}
}

#endif

void TraceFunc::show (int)
{
	cerr << "\r\n";

	#ifndef NDEBUG

	if (initial == NULL)
		cerr << "TraceFunc: no calls.";
	else
	{
		cerr << "TraceFunc dump of calls: \r\n";
		for (TraceFunc * act= initial; act != NULL; act= act->next)
			cerr << act->strfunc << "\r\n";
		cerr << "TraceFunc dump ended.";
	}

	#else

	cerr << "Debug info not compiled";

	#endif

	cerr << "\r\n";
}

// Fin de trace.cpp
