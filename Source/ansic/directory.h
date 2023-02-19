// directory.h
// Revision 23-jun-2003

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

void remove_file (const std::string & filename);
void change_dir (const std::string & dirname);
void make_dir (const std::string & dirname);
void remove_dir (const std::string & dirname);

// End of directory.h
