// runnerline.h
// Revision 22-aug-2003

#ifndef RUNNERLINE_H_

#define RUNNERLINE_H_

#include "blassic.h"
#include "error.h"
#include "result.h"
#include "codeline.h"
#include "file.h"
#include "directory.h"

#include <list>
#include <map>

class Runner;
class Program;

class RunnerLine {
public:
	RunnerLine (Runner & runner, CodeLine & line, Program & program) :
		pline (& line),
		runner (runner),
		program (program),
		fInElse (false)
	{ }
	void setline (CodeLine & line)
	{
		pline= & line;
		fInElse= false;
	}
	void execute ();
	inline BlLineNumber number () const { return pline->number (); }
	inline ProgramPos getposactual () const
	{
		return ProgramPos (pline->number (), actualchunk);
	}
private:
	//CodeLine & line;
	CodeLine * pline;
	BlChunk actualchunk;
	Runner & runner;
	Program & program;
	bool fInElse;
	CodeLine::Token token;
	BlCode codprev;
	Directory directory;

	typedef bool (RunnerLine::* do_func) (void);
	typedef std::map <BlCode, do_func> mapfunc_t;
	static const mapfunc_t mapfunc;
	static const mapfunc_t::const_iterator mapend;
	static mapfunc_t initmapfunc ();

	inline BlFile & getfile (BlChannel channel);

	//inline void gettoken () { token= line.gettoken (); }
	inline void gettoken () { pline->gettoken (token); }
	inline bool endsentence () { return token.isendsentence (); }
	inline void require_endsentence () const // throw (BlErrNo)
	{
		if (! token.isendsentence () )
			throw ErrSyntax;
	}
        #if 0 // Use macros instead to avoid strange errors on hp-ux.
        inline void requiretoken (BlCode code) const throw (BlErrNo);
        inline void expecttoken (BlCode code) throw (BlErrNo);
        #endif
	void getnextchunk ();

	void parenarg (BlResult & result);
	void getparenarg (BlResult & result);
	void getparenarg (BlResult & result, BlResult & result2);
	BlFile & getparenfile ();

	void valnumericfunc (double (* f) (double), BlResult & result);
	void valnumericfunc2 (double (* f) (double, double),
		BlResult & result);
	void valtrigonometricfunc
		(double (* f) (double), BlResult & result);
	void valtrigonometricinvfunc
		(double (* f) (double), BlResult & result);
	void valasc (BlResult & result);
	void vallen (BlResult & result);
	void valpeek (BlResult & result);
	void valpeek16 (BlResult & result);
	void valpeek32 (BlResult & result);
	void valprogramptr (BlResult & result);
	void valrnd (BlResult & result);
	void valint (BlResult & result);
	void valinstr (BlResult & result, bool reverse);
	void valfindfirstlast (BlResult & result, bool first, bool yesno);
	void valusr (BlResult & result);
        void valval (BlResult & result);
	void valeof (BlResult & result);
	void valvarptr (BlResult & result);
	void valsgn (BlResult & result);
        void valcvi (BlResult & result);
        void valcvs (BlResult & result);
        void valcvd (BlResult & result);
        void valcvl (BlResult & result);
        void valmin (BlResult & result);
        void valmax (BlResult & result);
	void valtest (BlResult & result, bool relative);
	void valpos (BlResult & result);
	void valvpos (BlResult & result);

	void valmid_s (BlResult & result);
	void valleft_s (BlResult & result);
	void valright_s (BlResult & result);
	void valchr_s (BlResult & result);
	void valenviron_s (BlResult & result);
	void valstring_s (BlResult & result);
	void valosfamily_s (BlResult & result);
	void valosname_s (BlResult & result);
	void valhex_s (BlResult & result);
        void valspace_s (BlResult & result);
        void valupper_s (BlResult & result);
        void vallower_s (BlResult & result);
        void valstr_s (BlResult & result);
        void valoct_s (BlResult & result);
        void valbin_s (BlResult & result);
        void valinkey_s (BlResult & result);
	void valprogramarg_s (BlResult & result);
	void valdate_s (BlResult & result);
	void valtime_s (BlResult & result);
	void valinput_s (BlResult & result);
        void valmki_s (BlResult & result);
        void valmks_s (BlResult & result);
        void valmkd_s (BlResult & result);
        void valmkl_s (BlResult & result);
        void valtrim (BlResult & result);
	void valcopychr_s (BlResult & result);
	void valstrerr_s (BlResult & result);
	void valdec_s (BlResult & result);

        void valfn (BlResult & result);

	void valsubindex (const std::string & varname, BlResult & result);
	void valbase (BlResult & result);
	void valparen (BlResult & result);
	void valexponent (BlResult & result);
	void valmod (BlResult & result);
	void valunary (BlResult & result);
	void valmuldiv (BlResult & result);
	void valplusmin (BlResult & result);
	void valcomp (BlResult & result);
	void valorand (BlResult & result);

	void eval (BlResult & result);
	void expect (BlResult & result);

	inline BlNumber evalnum ();
	inline BlNumber expectnum ();
	inline BlInteger evalinteger ();
	inline BlInteger expectinteger ();
	inline BlChannel evalchannel ();
	inline BlChannel expectchannel ();
	inline std::string evalstring ();
	inline std::string expectstring ();
	inline VarPointer evalvarpointer ();
	typedef std::list <VarPointer> ListVarPointer;
	void evalmultivarpointer (ListVarPointer & lvp);
	inline VarPointer eval_let ();
	BlLineNumber evallinenumber ();
	void evallinerange (BlLineNumber & blnBeg, BlLineNumber & blnEnd);
        Dimension expectdims ();

	void errorifparam ();
        void gosub_line (BlLineNumber dest);

	void getinkparams ();
	void getdrawargs (BlInteger & y);
	void getdrawargs (BlInteger & x, BlInteger & y);

	void make_clear ();

	void print_using (BlFile & out);

	bool do_empty_sentence ();
	bool do_endline ();
	bool do_number ();
	bool do_end ();
	bool do_list ();
	bool do_rem ();
	bool do_load ();
	bool do_save ();
	bool do_new ();
	bool do_exit ();
	bool do_run ();
	bool do_print ();
	bool do_for ();
	bool do_next ();
	bool do_if ();
	bool do_tron ();
	bool do_troff ();
	void letsubindex (const std::string &varname);
	bool do_let ();
	bool do_goto_gosub ();
	bool do_stop ();
	bool do_cont ();
	bool do_clear ();
	bool do_return ();
	bool do_poke ();
	bool do_read ();
	bool do_data ();
	bool do_restore ();
	bool do_input ();
	void do_line_input ();
	bool do_line ();
	bool do_randomize ();
	bool do_auto ();
	bool do_dim ();
	bool do_system ();
        bool do_on ();
        bool do_error ();
        bool do_open ();
        bool do_close ();
	bool do_locate ();
	bool do_cls ();
        bool do_write ();
	bool do_mode ();
	bool do_move ();
	bool do_color ();
        bool do_get ();
	bool do_label ();
	bool do_delimiter ();
	bool do_repeat ();
	bool do_until ();
        bool do_while ();
        bool do_wend ();
	bool do_plot ();
        bool do_resume ();
	bool do_delete ();
	bool do_local ();
	bool do_put ();
        bool do_field ();
        bool do_lset ();
	bool do_socket ();
	bool do_mid_s ();
	bool do_draw ();
	bool do_def_fn ();
	void definevars (VarType type);
        bool do_def ();
        bool do_fn ();
        bool do_programarg_s ();
        bool do_erase ();
        bool do_swap ();
        bool do_symbol ();
        bool do_zone ();
        bool do_pop ();
	bool do_name ();
	bool do_kill ();
	bool do_files ();
	bool do_paper ();
	bool do_pen ();
	bool do_shell ();
	bool do_merge ();
	bool do_chdir ();
	bool do_mkdir ();
	bool do_rmdir ();
	bool do_synchronize ();
	bool do_pause ();
	bool do_chain ();
	bool do_environ ();
	bool do_edit ();
	bool do_drawr ();
	bool do_plotr ();
	bool do_mover ();
	bool do_poke16 ();
	bool do_poke32 ();
	bool do_renum ();
	bool do_circle ();
	bool do_mask ();
	bool do_window ();
	void do_graphics_pen ();
	void do_graphics_paper ();
	void do_graphics_cls ();
	bool do_graphics ();
	bool do_beep ();
	bool do_defint ();
	bool do_ink ();
	bool do_set_title ();
	bool do_tag_tagoff ();
	bool do_origin ();
	bool do_deg_rad ();
	bool do_inverse ();
	bool do_if_debug ();
};

#endif

// Fin de runnerline.h
