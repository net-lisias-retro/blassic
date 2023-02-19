// sysvar.h

#ifndef SYSVAR_H_
#define SYSVAR_H_

namespace sysvar {

void init ();
size_t address ();

void set (size_t var, char value);
void set16 (size_t var, short value);
void set32 (size_t var, long value);

unsigned long get32 (size_t var);

const size_t
	GraphicsWidth= 0,
	GraphicsHeight= 2,
	NumArgs= 4,
	VersionMajor= 6,
	VersionMinor= 8,
	VersionRelease= 10,
	AutoInit= 12,
	AutoInc= 16,
	CharGen= 20,
	ShellResult= 24,
	// 1 byte reserved
	EndSysVar= 26;

} // namespace sysvar

#endif

// Fin de sysvar.h
