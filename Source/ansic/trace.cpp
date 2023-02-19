// trace.cpp

#include "trace.h"

#if __BORLANDC__ >= 0x0560
#pragma warn -8091
#endif

#include <iostream>
using std::cerr;
using std::endl;
#include <fstream>
using std::ofstream;
#include <string>
using std::string;

#include <cstdlib>
using std::getenv;

#include <stdexcept>

namespace {

#ifndef NDEBUG

ofstream * pof= 0;
bool flag= true;
size_t indent= 0;

TraceFunc * initial= NULL;
TraceFunc * * lastpos= & initial;

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
			pof= new ofstream (aux, std::ios::ate | std::ios::out);
			if (! pof->is_open () )
			{
				cerr << "Error al abrir " << aux << endl;
				delete pof;
				pof= 0;
			}
			pof->clear ();
		}
	}

	//tracelist.push_back (this);
	previous= lastpos;
	* lastpos= this;
	lastpos= & next;

	if (pof)
		* pof << string (indent, ' ') << "Entra " << strfunc << endl;
	++indent;
}

TraceFunc::~TraceFunc ()
{
	--indent;
	if (pof)
	{
		* pof << string (indent, ' ') << "Sale ";
		if (std::uncaught_exception () )
			* pof << "(throwing) ";
		* pof << strfunc << endl;
	}
	if (lastpos != & next)
		throw std::logic_error ("Bad use of TraceFunc");
	lastpos= previous;
	* lastpos= NULL;
}

void TraceFunc::message (const std::string & strMes)
{
	if (pof)
	{
		* pof << string (indent, ' ') << strfunc <<
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
