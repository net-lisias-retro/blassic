// sysvar.cpp
// Revision 22-aug-2003

#include "blassic.h"
#include "sysvar.h"
#include "charset.h"

namespace {

BlChar system_vars [sysvar::EndSysVar];

} // namespace


void sysvar::init ()
{
	set16 (GraphicsWidth, 0);
	set16 (GraphicsHeight, 0);
	set16 (NumArgs, 0);
	set16 (VersionMajor, version::Major);
	set16 (VersionMinor, version::Minor);
	set16 (VersionRelease, version::Release);
	set32 (AutoInit, 10);
	set32 (AutoInc, 10);
	set32 (CharGen, reinterpret_cast <size_t> (charset::data) );
	set (ShellResult, 0);
	set (TypeOfVal, 0); // VAL simple, number only.
	set (TypeOfNextCheck, 0); // Strict next check
	set (TypeOfDimCheck, 0); // Need erase before dim already dimensioned
	set16 (MaxHistory, 100);
	set (Flags1, 0); // LOCATE style Microsoft (row, col).
	set32 (MaxFnLevel, 1000); // Seems a good limit.
	set16 (DebugLevel, 0);
}

size_t sysvar::address ()
{
	return reinterpret_cast <size_t> (system_vars);
}

void sysvar::set (size_t var, char value)
{
	system_vars [var]= value;
}

void sysvar::set16 (size_t var, short value)
{
	poke16 (system_vars + var, value);
}

void sysvar::set32 (size_t var, long value)
{
	poke32 (system_vars + var, value);
}

BlChar sysvar::get (size_t var)
{
	return system_vars [var];
}

unsigned short sysvar::get16 (size_t var)
{
	return static_cast <unsigned short> (peek16 (system_vars + var) );
}

unsigned long sysvar::get32 (size_t var)
{
	return peek32 (system_vars + var);
}

// Fin de sysvar.cpp
