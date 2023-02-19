// directory.h
// Revision 23-may-2003

#include <string>

class Directory {
public:
	Directory ();
	~Directory ();
	std::string findfirst (const std::string & str);
	std::string findnext ();
private:
	Directory (const Directory &); // Forbidden
	Directory operator = (const Directory &); // Forbidden
	class Internal;
	Internal * pin;
};

// End of directory.h
