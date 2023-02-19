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
#include <list>

#include <cstdlib>
using std::getenv;

#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <functional>

namespace {

#ifndef NDEBUG

ofstream * pof= 0;
bool flag= true;
size_t indent= 0;

// Changed to not use std::list, seems to be unusable during
// initialization of statics, at least in some gcc versions.

//typedef std::list <TraceFunc *> TraceList;
//TraceList tracelist;

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
			pof= new ofstream (aux, std::ios::ate);
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
	lastpos= & this->next;

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
		#if 0
		if (uncaught_exception () )
			* pof << "(throwing) ";
		#endif
		* pof << strfunc << endl;
	}
	//if (tracelist.back () != this)
	//	throw std::logic_error ("Bad use of TraceFunc");
	//tracelist.pop_back ();
	if (lastpos != & this->next)
		throw std::logic_error ("Bad use of TraceFunc");
	lastpos= previous;
	* lastpos= NULL;
}

void TraceFunc::message (const std::string & strMes)
{
	if (pof)
	{
		* pof << string (indent + 2, ' ') << strfunc <<
			": " << strMes << endl;
	}
}

#endif

void TraceFunc::show (int)
{
	cerr << "\r\n";

	#ifndef NDEBUG

	//if (tracelist.size () == 0)
	if (initial == NULL)
		cerr << "TraceFunc: no calls.";
	else
	{
		cerr << "TraceFunc dump of calls: \r\n";
		#if 0
		//std::ostream_iterator <const char *> oit (cerr, "\r\n");
		//std::transform (tracelist.begin (), tracelist.end (), oit,
		//	std::mem_fun (& TraceFunc::strfunc) );
		for (TraceList::iterator it= tracelist.begin ();
			it != tracelist.end ();
			++it)
		{
			cerr << (* it)->strfunc << "\r\n";
		}
		#else
		for (TraceFunc * act= initial; act != NULL; act= act->next)
			cerr << act->strfunc << "\r\n";
		#endif
		cerr << "TraceFunc dump ended.";
	}

	#else

	cerr << "Debug info not compiled";

	#endif

	cerr << "\r\n";
}

// Fin de trace.cpp
