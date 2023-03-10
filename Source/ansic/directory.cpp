// directory.cpp
// Revision 18-aug-2003

#include "directory.h"
#include "error.h"

#include <errno.h>

// We use the unix style even on windows under cygwin.
#if defined __unix__ || defined __linux__ || defined __NetBSD__
#define USE_UNIX
#endif

#ifdef USE_UNIX
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <glob.h>
#else
#include <windows.h>
#include <dir.h>
#include <dos.h>
#ifdef __MINGW32__
#include <io.h>
#else
using std::unlink;
#endif
#endif

// ********************* Directory::Internal *************************

class Directory::Internal {
public:
	Internal ();
	~Internal ();
	std::string findfirst (const std::string & str);
	std::string findnext ();
	void closesearch ();
private:
	bool finding;
	#ifdef USE_UNIX
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
	#ifdef USE_UNIX
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
	#ifdef USE_UNIX
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
	#ifdef USE_UNIX
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

// ********************* Other functions *************************

void remove_file (const std::string & filename)
{
	if (unlink (filename.c_str () ) != 0)
	{
		switch (errno)
		{
		case ENOENT:
			throw ErrFileNotFound;
		default:
			throw ErrOperatingSystem;
		}
	}
}

void change_dir (const std::string & dirname)
{
	if (chdir (dirname.c_str () ) != 0)
		throw ErrOperatingSystem;
}

void make_dir (const std::string & dirname)
{
	#ifdef USE_UNIX
	int r= mkdir (dirname.c_str (), 0777);
	#else
	int r= mkdir (dirname.c_str () );
	#endif
	if (r != 0)
		throw ErrOperatingSystem;
}

void remove_dir (const std::string & dirname)
{
	if (rmdir (dirname.c_str () ) != 0)
		throw ErrOperatingSystem;
}

// End of directory.cpp
