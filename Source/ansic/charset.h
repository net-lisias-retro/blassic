// charset.h
// Revision 16-may-2003

#ifndef CHARSET_H_
#define CHARSET_H_

namespace charset {

typedef unsigned char chardata [8];

extern const chardata default_data [256];
extern chardata data [256];

}

#endif

// End of charset.h
