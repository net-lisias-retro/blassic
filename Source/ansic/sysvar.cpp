// sysvar.cpp

#include "blassic.h"
#include "sysvar.h"
#include "charset.h"

namespace {

BlChar system_vars [sysvar::EndSysVar];

} // namespace


void sysvar::init ()
{
	set16 (VersionMajor, version::Major);
	set16 (VersionMinor, version::Minor);
	set16 (VersionRelease, version::Release);
	set32 (AutoInit, 10);
	set32 (AutoInc, 10);
	set32 (CharGen, reinterpret_cast <size_t> (charset::data) );
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

unsigned long sysvar::get32 (size_t var)
{
	return peek32 (system_vars + var);
}

// Fin de sysvar.cpp
