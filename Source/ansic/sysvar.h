// sysvar.h
// Revision 27-aug-2003

#ifndef SYSVAR_H_
#define SYSVAR_H_

namespace sysvar {

void init ();
size_t address ();

void set (size_t var, char value);
void set16 (size_t var, short value);
void set32 (size_t var, long value);

BlChar get (size_t var);
unsigned short get16 (size_t var);
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
	TypeOfVal= 25,	// 0: simple, 1: expression evluation,
			// else unimplemented.
	TypeOfNextCheck= 26,	// 0: normal, else ZX-type
	TypeOfDimCheck= 27,	// 0: cannot dim already dimensioned
				// 1: Silently redim
	MaxHistory= 28,	// Max size of history buffer.
	Flags1= 30,	// Bit 0: LOCATE style. 0 Microsoft, 1 Amstrad CPC.
	PrinterLine=  31, 	// Type of printer line feed.
				// 0 LF only.
				// 1 CR + LF
				// 2 CR only.
	MaxFnLevel= 32, // Max level of FN calls.
	DebugLevel= 36, // Level for IF_DEBUG
	Zone= 38,	// Size of zone for the , separator of PRINT.
	EndSysVar= 40;

} // namespace sysvar

#endif

// Fin de sysvar.h
