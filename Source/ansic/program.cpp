// program.cpp

#include <cassert>
#define ASSERT assert

#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <sstream>

using std::string;
// For debugging.
using std::cerr;
using std::endl;
using std::flush;

#include "blassic.h"

#include "token.h"
#include "keyword.h"
#include "var.h"
#include "program.h"
#include "error.h"
#include "sysvar.h"
#include "trace.h"

#ifndef USE_HASH_MAP

#include <map>
#define MAP std::map

#else

#include <hash_map>
#define MAP std::hash_map

template <> struct hash <std::string>
{
	hash () : hashstr (hash <const char *> () ) { }
	size_t operator () (const std::string & str) const
	{ return hashstr (str.c_str () ); }
private:
	hash <const char *> hashstr;
};

#endif

#ifdef __BORLANDC__
#pragma warn -inl
#endif

typedef unsigned long Position;

namespace {

inline BlLineNumber getLineNumber (const BlChar * pos)
{
	return peek32 (pos);
}

inline void setLineNumber (BlChar * pos, BlLineNumber n)
{
	poke32 (pos, n);
}

inline BlLineLength getLineLength (BlChar * pos)
{
	return peek32 (pos);
}

inline void setLineLength (BlChar * pos, BlLineLength n)
{
	poke32 (pos, n);
}

} // namespace

//**********************************************************
//		Program::Internal
//**********************************************************

class Program::Internal {
public:
	Internal () :
		program (0),
		size (0)
	{ }
	inline BlChar * programptr () { return program; }
	inline BlLineNumber getlabel (const std::string & str);
	inline CodeLine getfirstline ();
	//inline CodeLine getnextline (CodeLine & line);
	inline void getnextline (CodeLine & line);
	//BlLineNumber getnextnum (CodeLine & line);
	//inline CodeLine getline (BlLineNumber num);
	inline void getlineinpos (Position pos, CodeLine & line);
	inline void getline (BlLineNumber num, CodeLine & line);
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
private:
	BlChar * program;
	BlLineLength size;

	typedef MAP <BlLineNumber, Position> linecache_t;
	linecache_t linecache;
	typedef MAP <string, BlLineNumber> labelcache_t;
	labelcache_t labelcache;

	void clear_cache ();

	inline Position nextline (Position pos) const;
	inline BlLineLength sizeline (Position) const;
	inline BlLineNumber numline (Position pos) const;
	inline const BlChar * linecontent (Position pos) const;
	inline BlChar * linecontent (Position pos);
	static const size_t BLOCK= 16 * 1024;
	inline static size_t blockrounded (size_t size)
	{
		return ( (size + BLOCK - 1) / BLOCK) * BLOCK;
	}
	void loadtext (std::istream & is);
	void loadbinary (std::istream & is);
};

inline BlLineLength Program::Internal::sizeline (Position pos) const
{
	BlChar * aux= program + pos + sizeof (BlLineNumber);
	return getLineLength (aux);
}

inline Position Program::Internal::nextline (Position pos) const
{
	return pos + sizeline (pos) +
		sizeof (BlLineNumber) + sizeof (BlLineLength);
}

inline BlLineNumber Program::Internal::numline (Position pos) const
{
	return getLineNumber (program + pos);
}

inline const BlChar * Program::Internal::linecontent (Position pos) const
{
	return program + pos + sizeof (BlLineNumber) + sizeof (BlLineLength);
}

inline BlChar * Program::Internal::linecontent (Position pos)
{
	return program + pos + sizeof (BlLineNumber) + sizeof (BlLineLength);
}

void Program::Internal::clear_cache ()
{
        linecache.clear ();
	labelcache.clear ();
}

#if 0
void Program::setlabel (const std::string & label, BlLineNumber number)
{
	labelcache [label]= number;
}
#endif

inline BlLineNumber Program::Internal::getlabel (const std::string & label)
{
	labelcache_t::iterator  it= labelcache.find (label);
	if (it != labelcache.end () )
		return it->second;
	CodeLine line= getfirstline ();
	CodeLine::Token token;
	while (line.number () != 0)
	{
		//CodeLine::Token token= line.gettoken ();
		line.gettoken (token);
		if (token.code == keyLABEL)
		{
			//token= line.gettoken ();
			line.gettoken (token);
			if (label == token.str)
			{
				BlLineNumber n (line.number () );
				labelcache [label]= n;
				// Probably the next action is jump to it,
				// then we put it in the cache.
				linecache [n]= line.content () - program
					- sizeof (BlLineNumber)
					- sizeof (BlLineLength);
;
				return line.number ();
			}
		}
		//line= getnextline (line);
		getnextline (line);
	}
	return 0;
}

inline CodeLine Program::Internal::getfirstline ()
{
	if (size == 0)
		return CodeLine (0, 0, 0);
	return CodeLine (
		//program + sizeof (BlLineNumber) + sizeof (BlLineLength),
		linecontent (0),
		numline (0), sizeline (0) );
}

//inline CodeLine Program::Internal::getnextline (CodeLine & line)
inline void Program::Internal::getnextline (CodeLine & line)
{
	if (line.number () == 0)
	{
		//return CodeLine (0, 0, 0);
		line.assign (0, 0, 0);
		return;
	}
	Position pos= line.content () - program;
	pos+= line.length ();
	if (pos >= size)
	{
		//return CodeLine (0, 0, 0);
		line.assign (0, 0, 0);
		return;
	}
	//return CodeLine (
	//	//program + pos + sizeof (BlLineNumber) + sizeof (BlLineLength),
	//	linecontent (pos),
	//	numline (pos), sizeline (pos) );
	line.assign (linecontent (pos), numline (pos), sizeline (pos) );
}

#if 0
inline BlLineNumber Program::Internal::getnextnum (CodeLine & line)
{
	if (line.number () == 0)
		return 0;
	Position pos= line.content () - program;
	pos+= line.length ();
	if (pos >= size)
		return 0;
	return numline (pos);
}
#endif

inline void Program::Internal::getlineinpos (Position pos, CodeLine & line)
{
	line.assign (linecontent (pos), numline (pos), sizeline (pos) );
}

//inline CodeLine Program::Internal::getline (BlLineNumber num)
inline void Program::Internal::getline (BlLineNumber num, CodeLine & line)
{
	Position pos= 0;

        linecache_t::iterator it= linecache.find (num);
        if (it != linecache.end () )
                pos= it->second;
        else
        {
        	while (pos < size && numline (pos) < num)
		        pos= nextline (pos);
	        if (pos >= size)
		{
			//return CodeLine (0, 0, 0);
			line.assign (0, 0, 0);
			return;
		}
                linecache [num]= pos;
        }
	//return CodeLine (
	//	//program + pos + sizeof (BlLineNumber) + sizeof (BlLineLength),
	//	linecontent (pos),
	//	numline (pos), sizeline (pos) );
	//line.assign (
	//	//program + pos + sizeof (BlLineNumber) + sizeof (BlLineLength),
	//	linecontent (pos),
	//	numline (pos), sizeline (pos) );
	getlineinpos (pos, line);
}

void Program::Internal::insert (const CodeLine & code)
{
        clear_cache ();
	Position pos= 0;
	while (pos < size && numline (pos) < code.number () )
		pos= nextline (pos);

	const BlChar * strnew= code.content ();
	BlLineLength linesize= code.length () +
		sizeof (BlLineNumber) + sizeof (BlLineLength);

	unsigned long osize= size;
	Position destpos= pos + linesize;
	Position origpos;
	if (pos < size && numline (pos) == code.number () )
	{
		origpos= nextline (pos);
		//destpos= pos + sizenew;
		size+= code.length () - sizeline (pos);
	}
	else
	{
		origpos= pos;
		//destpos= pos + sizenew;
		size+= linesize;
	}

	if (destpos > origpos)
	{
		ASSERT (size > osize);
		size_t newblock= blockrounded (size);
		if (newblock != blockrounded (osize) )
		{
			unsigned char * newprog=
				(unsigned char *) realloc (program, newblock);
			if (! newprog)
				throw ErrOutMemory;
			program= newprog;
		}
		if (pos < osize)
		{
			memmove (program + destpos,
				program + origpos,
				osize - origpos);
		}
	}
	else if (destpos < origpos)
	{
		ASSERT (size < osize);
		memmove (program + destpos,
			program + origpos,
			osize - origpos);
		size_t newblock= blockrounded (size);
		if (newblock != blockrounded (osize) )
		{
			unsigned char * newprog=
				(unsigned char *) realloc (program, newblock);
			if (! newprog)
				throw ErrOutMemory;
			program= newprog;
		}
	}

	setLineNumber (program + pos, code.number () );
	setLineLength (program + pos + sizeof (BlLineNumber), code.length () );
	memcpy (linecontent (pos), strnew, code.length () );
}

void Program::Internal::deletelines
	(BlLineNumber iniline, BlLineNumber endline)
{
	clear_cache ();
	Position pos= 0;
	while (pos < size && numline (pos) < iniline)
		pos= nextline (pos);
	if (pos >= size)
		return;
	Position posend= pos;
	while (posend < size && numline (posend) <= endline)
		posend= nextline (posend);
	if (posend == pos)
		return;
	#if 0
	cout << "Deleting from " << numline (pos) << " to ";
	if (posend < size)
		cout << "(not including) " << numline (posend);
	else
		cout << "the end";
	cout << endl;
	#endif

	if (posend < size)
		memmove (program + pos,
			program + posend,
			size - posend);
	size_t osize= size;
	size-= posend - pos;
	if (size > 0)
	{
		size_t newblock= blockrounded (size);
		if (newblock != blockrounded (osize) )
			realloc (program, newblock);
	}
	else
	{
		free (program);
		program= NULL;
	}
}

void Program::Internal::list
	(BlLineNumber iniline, BlLineNumber endline, BlFile & out) const
{
	Position pos= 0;
	while (pos < size && numline (pos) < iniline)
		pos= nextline (pos);
	while (pos < size)
	{
		BlLineNumber number= numline (pos);
		if (number > endline)
			break;
		BlLineLength linesize= sizeline (pos);
		out << /*std::setw (7) << */ number << ' ';
		const BlChar * aux= linecontent (pos);
		pos= nextline (pos);
                std::string line;
		for (unsigned long i= 0; i < linesize; ++i)
		{
			unsigned char c= aux [i];
                        if (c == '\0') // Skip garbage
                                break;
			if (iskey (c) )
			{
				BlCode s= c;
				s<<= 8;
				s|= aux [++i];
                                line+= decodekeyword (s);
			}
                        else if (c == INTEGER_PREFIX)
                        {
                                //BlInteger n;
                                //n= * (BlInteger *) (aux + i + 1);
				BlInteger n= peek32 (aux + i + 1);
                                std::ostringstream oss;
                                oss << n;
                                line+= oss.str ();
                                i+= 4;
                        }
			else if (c == '"')
			{
				line+= c;
				while ( (c= aux [++i]) != 0)
					if (c == '"')
						line+= "\"\"";
					else
						line+= c;
				line+= '"';
			}
			else if (c == '\t')
			{
				const size_t l= line.size ();
				line.insert (l, 8 - l % 8, ' ');
			}
			else
				line+= c;
		}
                //out << line;
		//out << '\n';
		line+= '\n';
		out << line;
		if (fInterrupted)
			break;
	}
}

namespace {

bool hasblassicextension (const std::string & name)
{
	std::string::size_type l= name.size ();
	if (l < 4)
		return false;
	std::string ext= name.substr (l - 4);
	//#ifdef _Windows
	std::transform (ext.begin (), ext.end (), ext.begin (), tolower);
	//#endif
	if (ext == ".blc" || ext == ".bas")
		return true;
	return false;
}

void openblassicprogram (std::ifstream & is, const std::string & name)
{
        const std::ios::openmode mode= std::ios::binary | std::ios::in;
	is.open (name.c_str (), mode);
	if (! is)
	{
		if (! hasblassicextension (name) )
		{
			std::string namex= name;
			namex+= ".blc";
			is.clear ();
			is.open (namex.c_str (), mode);
			if (! is.is_open () )
			{
				namex= name;
				namex+= ".bas";
				is.clear ();
				is.open (namex.c_str (), mode);
				if (! is.is_open () )
					throw ErrFileNotFound;
			}
		}
		else
			throw ErrFileNotFound;
	}
}

const char signature []=
	{ 'B', 'l', 'a', 's', 's', 'i', 'c', '\0' };
const size_t lsig= sizeof (signature);

bool isblassicbinary (std::istream & is)
{
	char magicstring [lsig];
	is.read (magicstring, lsig);
	if (! is || memcmp (magicstring, signature, lsig) != 0)
		return false;
	return true;
}

inline void checkread (std::istream & is, size_t readed)
{
	if (! is || size_t (is.gcount () ) != readed)
		throw ErrFileRead;
}

const unsigned long endian_mark= 0x12345678;

class TextLoader {
public:
	TextLoader (Program::Internal & program) :
		program (program),
		nextnumline (sysvar::get32 (sysvar::AutoInit) ),
		incnumline (sysvar::get32 (sysvar::AutoInc) ),
		maxnumline (BlMaxLineNumber - incnumline)
	{ }
	bool directive (std::string str);
	void load (std::istream & is);
private:
	Program::Internal & program;
	BlLineNumber nextnumline;
	BlLineNumber incnumline;
	BlLineNumber maxnumline;
};

bool TextLoader::directive (std::string str)
{
	TraceFunc tr ("TextLoader::directive");

	static std::string include ("include");
	static const std::string::size_type linc= include.size ();
	if (str.substr (1, linc) == include)
	{
		str.erase (0, linc + 1);
		std::string::size_type l= str.find_first_not_of (" \t");
		if (l > 0)
			str.erase (0, l);
		if (str.empty () )
			return false;
		if (str [0] == '"')
		{
			l= str.find ('"',  1);
			str= str.substr (1, l - 1);
		}
		else if (str [0] == '<')
		{
			l= str.find ('>',  1);
			str= str.substr (1, l - 1);
		}
		else
		{
			l= str.find_first_of (" \t");
			if (l != std::string::npos)
				str.erase (l);
		}
		tr.message (str);
		std::ifstream is;
		openblassicprogram (is, str);
		load (is);
		return true;
	}
	return false;
}

void TextLoader::load (std::istream & is)
{
	TraceFunc tr ("TextLoader::load");

	std::string str;
	std::getline (is, str);
	if (!str.empty () && str [0] == '#')
	{
		str.erase ();
		std::getline (is, str);
	}
	CodeLine code;
	bool fExhausted= false;
	for ( ; is; std::getline (is, str) )
	{
		if (! str.empty () && str [str.size () - 1] == '\r')
			str.erase (str.size () - 1);

		// Quick & dirty implemantation of #include
		if (! str.empty () && str [0] == '#' && directive (str) )
			continue;

		code.scan (str);
		if (code.number () == 0)
		{
			if (fExhausted)
			{
				tr.message ("Line exhausted");
				throw ErrLineExhausted;
			}
			code.setnumber (nextnumline);
			fExhausted= nextnumline > maxnumline;
			nextnumline+= incnumline;
		}
		else
		{
			fExhausted= code.number () > maxnumline;
			nextnumline= code.number () + incnumline;
		}
		if (code.length () > 0)
			program.insert (code);
	}
}

} // namespace

void Program::Internal::save (const std::string & name) const
{
	std::ofstream os (name.c_str (), std::ios::binary | std::ios::out);
	if (! os)
		return;
	os.write (signature, lsig);
	COMPILE_ASSERT (sizeof (unsigned long) == 4);
	os.write ( (char *) & endian_mark, 4);
	BlChar caux [4];
	poke32 (caux, size);
	os.write ( (char *) caux, 4);
	os.write ( (char *) program, size);
	if (! os)
		throw ErrFileWrite;
}

void Program::Internal::loadtext (std::istream & is)
{
	TraceFunc tr ("Program::Internal::loadtext");

	is.clear ();
	is.seekg (0);
	TextLoader loader (* this);
	loader.load (is);
}

void Program::Internal::loadbinary (std::istream & is)
{
	// This was intended to check endianess, but is
	// currently unused.
	COMPILE_ASSERT (sizeof (unsigned long) == 4);
	unsigned long endian_check;
	is.read ( (char *) & endian_check, 4);
	checkread (is, 4);
	BlChar caux [4];
	is.read ( (char *) caux, 4);
	checkread (is, 4);
	unsigned long newsize= peek32 (caux);
	size_t newblock= blockrounded (newsize);
	util::auto_alloc <BlChar> newprog (newblock);
	is.read (reinterpret_cast <char *> (newprog.data () ),
                newsize);
	checkread (is, newsize);
	renew ();
	program= newprog;
	newprog.release ();
	size= newsize;
}

void Program::Internal::load (const std::string & name)
{
        clear_cache ();
	std::ifstream (is);
	openblassicprogram (is, name);

	if (! isblassicbinary (is) )
	{
		renew ();
		loadtext (is);
	}
	else
	{
		loadbinary (is);
	}
}

void Program::Internal::merge (const std::string & name)
{
        clear_cache ();
	std::ifstream is;
	openblassicprogram (is, name);

	if (! isblassicbinary (is) )
	{
		loadtext (is);
	}
	else
	{
		Program::Internal inload;
		inload.loadbinary (is);
		for (CodeLine line= inload.getfirstline ();
			line.number () != 0;
			//line= inload.getnextline (line)
			inload.getnextline (line)
		)
		{
			insert (line);
		}
	}
}

void Program::Internal::renew ()
{
	size= 0;
        clear_cache ();
	if (program)
	{
		free (program);
		program= 0;
	}
}

namespace {

typedef MAP <BlLineNumber, BlLineNumber> MapLine;

void showmappedline (const MapLine::value_type & linepair)
{
	cerr << linepair.first << " mappped to " << linepair.second << endl;
}

bool iscodewithnumber (BlCode code)
{
	return (code == keyGOTO || code == keyGOSUB || code == keyRUN ||
		code == keyRESTORE || code == keyRESUME ||
		code == keyDELETE || code == keyLIST ||
		code == keyEDIT ||
		code == keyTHEN || code == keyELSE);
}

void changeline (CodeLine & line, const MapLine & mapline)
{
	const BlLineLength l= line.length ();
	BlChar * s= line.content ();
	BlLineLength p= 0;
	BlChar c;
	while (p < l)
	{
		c= s [p];
		if (iskey (c) )
		{
			BlCode code= BlCode ( (BlCode (c) << 8 ) ) |
				BlCode (s [p+1] );
			p+= 2;
			//cerr << "Key " << decodekeyword (code) << flush;
			if (iscodewithnumber (code) )
			{
				//cerr << " analyzing" << flush;
				for (;;)
				{
					while (p < l && isspace (s [p] ) )
						++p;
					if (p >= l)
						break;
					c= s [p];
					if (c == INTEGER_PREFIX)
					{
						BlLineNumber old= getLineNumber
							(s + p + 1);
						//cerr << " Find " << old <<
						//	flush;
						MapLine::const_iterator it=
							mapline.find (old);
						if (it != mapline.end () )
						{
							//cerr << " Changed " <<
							//	it->second <<
							//	flush;
							setLineNumber
								(s + p + 1,
								it->second);
						}
						p+= 1 + sizeof (BlInteger);
					}
					else if (c == ':')
					{
						++p;
						break;
					}
					else if (c == '"')
					{
						while (s [++p] != '\0')
							continue;
						++p;
					}
					else if (iskey (c) )
						p+= 2;
					else if (c == '\'')
					{
						p= l;
						break;
					}
					else ++p;
				}
				//cerr << endl;
			}
			//else cerr << endl;
		}
		else if (c == INTEGER_PREFIX)
			p+= 1 + sizeof (BlInteger);
		else if (c == '"')
		{
			while (s [++p] != '\0')
				continue;
			++p;
		}
		else if (c == '\'')
			break;
		else ++p;
	}
}

}

void Program::Internal::renum (BlLineNumber blnNew, BlLineNumber blnOld,
	BlLineNumber blnInc)
{
	clear_cache ();

	Position pos= 0;
	MapLine mapline;

	// Find first line to renum.
	BlLineNumber previous= 0;
	while (pos < size && numline (pos) < blnOld)
	{
		previous= numline (pos);
		//cerr << "Skipping line " << previous << endl;
		pos= nextline (pos);
	}
	if (previous >= blnNew)
		throw ErrMismatch;

	// Change the line numbers.
	BlLineNumber actual;
	BlLineNumber blnMax= BlMaxLineNumber - blnInc;
	bool overflow= false;
	for ( ; pos < size; pos= nextline (pos) )
	{
		actual= numline (pos);
		if (actual != blnNew)
		{
			if (overflow)
				throw ErrLineExhausted;
			mapline [actual]= blnNew;
			//setLineNumber (program + pos, blnNew);
		}
		if (blnNew > blnMax)
			overflow= true;
		else
			blnNew+= blnInc;
	}

	//for_each (mapline.begin (), mapline.end (), showmappedline);

	// Change refereneces to lines.
	MapLine::iterator it, mapend= mapline.end ();
	CodeLine line;
	for (pos= 0; pos < size; pos= nextline (pos) )
	{
		actual= numline (pos);
		getlineinpos (pos, line);
		//cerr << "Exploring line " << actual << endl;
		it= mapline.find (actual);
		if (it != mapend)
		{
			//cerr << "Changing line " << actual <<
			//	" by " << it->second << endl;
			setLineNumber (program + pos, it->second);
		}
		changeline (line, mapline);
	}
}

//**********************************************************
//		Program
//**********************************************************

Program::Program () :
	pin (new Internal)
{
}

Program::~Program ()
{
	delete pin;
}

BlChar * Program::programptr ()
{
	return pin->programptr ();
}

BlLineNumber Program::getlabel (const std::string & str)
{
	return pin->getlabel (str);
}

CodeLine Program::getfirstline ()
{
	return pin->getfirstline ();
}

//CodeLine Program::getnextline (CodeLine & line)
void Program::getnextline (CodeLine & line)
{
	//return pin->getnextline (line);
	pin->getnextline (line);
}

#if 0
BlLineNumber Program::getnextnum (CodeLine & line)
{
	return pin->getnextnum (line);
}
#endif

#if 0
CodeLine Program::getline (BlLineNumber num)
{
	return pin->getline (num);
}
#endif

void Program::getline (BlLineNumber num, CodeLine & line)
{
	pin->getline (num, line);
}

void Program::getline (ProgramPos pos, CodeLine & line)
{
	BlLineNumber n= pos.getnum ();
	pin->getline (n, line);
	if (line.number () == n)
	{
		BlChunk ch= pos.getchunk ();
		if (ch != 0)
			line.gotochunk (ch);
	}
}

void Program::insert (const CodeLine & code)
{
	pin->insert (code);
}

void Program::deletelines (BlLineNumber iniline, BlLineNumber endline)
{
	pin->deletelines (iniline, endline);
}

void Program::list (BlLineNumber iniline, BlLineNumber endline,
	BlFile & out) const
{
	pin->list (iniline, endline, out);
}

void Program::save (const std::string & name) const
{
	pin->save (name);
}

void Program::load (const std::string & name)
{
	pin->load (name);
}

void Program::merge (const std::string & name)
{
	pin->merge (name);
}

void Program::renew ()
{
	pin->renew ();
}

void Program::renum (BlLineNumber blnNew, BlLineNumber blnOld,
	BlLineNumber blnInc)
{
	pin->renum (blnNew, blnOld, blnInc);
}

// Fin de program.cpp
