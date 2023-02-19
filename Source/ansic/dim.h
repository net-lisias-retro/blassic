// dim.h

#ifndef DIM_H_
#define DIM_H_

#include <iostream>
#include <cstddef>
#include <vector>

class Dimension {
	class Internal;
public:
	void add (size_t n) { dim.push_back (n); }
	size_t size () const { return dim.size (); }
	size_t elements () const;
	size_t operator [] (size_t n) const { return dim [n]; }
	size_t evalpos (const Dimension & d);
private:
	std::vector <size_t> dim;
};

std::ostream & operator << (std::ostream & os, const Dimension & d);

#endif

// Fin de dim.h
