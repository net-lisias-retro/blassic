// directory.cpp
// Revision 23-may-2003

#include "directory.h"

#if defined __unix__ || defined __linux__
#include <glob.h>
#endif

class Directory::Internal {
public:
	Internal ();
	~Internal ();
	std::string findfirst (const std::string & str);
	std::string findnext ();
	void closesearch ();
private:
	bool finding;
	#if defined __unix__ || defined __linux__
	glob_t g;
	size_t n;
	#else
	WIN32_FIND_DATA fd;
	HANDLE h;
	#endif
};

Directory::Internal::Internal () :
	finding (false)
{
}

Directory::Internal::~Internal ()
{
	if (finding)
		closesearch ();
}

std::string Directory::Internal:: findfirst (const std::string & str)
{
	if (finding)
		closesearch ();
	finding= true;
	#if defined __unix__ || defined __linux__
	glob (str.c_str (), 0, NULL, & g);
	if (g.gl_pathc == 0)
	{
		closesearch ();
		return std::string ();
	}
	n= 1;
	return g.gl_pathv [0];
	#else
	h= FindFirstFile (str.c_str (), & fd);
	if (h == INVALID_HANDLE_VALUE)
	{
		closesearch ();
		return std::string ();
	}
	return fd.cFileName;
	#endif
}

std::string Directory::Internal:: findnext ()
{
	if (! finding)
		return std::string ();
	#if defined __unix__ || defined __linux__
	if (n >= g.gl_pathc)
	{
		closesearch ();
		return std::string ();
	}
	++n;
	return g.gl_pathv [n - 1];
	#else
	if (! FindNextFile (h, & fd) )
	{
		closesearch ();
		return std::string ();
	}
	return fd.cFileName;
	#endif
}

void Directory::Internal::closesearch ()
{
	#if defined __unix__ || defined __linux__
	globfree (& g);
	#else
	if (h != INVALID_HANDLE_VALUE)
		FindClose (h);
	#endif
	finding= false;
}

// ********************* Directory *************************

Directory::Directory () :
	pin (new Internal)
{
}

Directory::~Directory ()
{
	delete pin;
}

std::string Directory::findfirst (const std::string & str)
{
	return pin->findfirst (str);
}

std::string Directory::findnext ()
{
	return pin->findnext ();
}


// End of directory.cpp
