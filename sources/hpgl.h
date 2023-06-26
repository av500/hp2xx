#include "lindef.h"
#include "pendef.h"
#include "bresnham.h"
#include "hp2xx.h"

#define	MAX_LB_LEN	1024	/* Max num of chars per label   */

//#define MAXPOLY 2048		/* Size of polygon vertex buffer */
#define MAXPOLY 128		/* Size of polygon vertex buffer */

int read_float (float *pnum, void * hd);
void read_string (char *target, void *);

void Pen_to_tmpfile(int pen);
void Speed_to_tmpfile(int speed);
void Pt_to_tmpfile (PlotCmd cmd, const HPGL_Pt * pf);
void HPGL_Pt_to_tmpfile (PlotCmd cmd, const HPGL_Pt * pf);
void Pen_action_to_tmpfile(PlotCmd, const HPGL_Pt *, int);
void HPGL_Pt_to_polygon (const HPGL_Pt pf );

#if defined __TURBOC__ || defined __STRICT_ANSI__
#define	HYPOT(x,y)	sqrt((x)*(x)+(y)*(y))
#else
#define	HYPOT(x,y)	hypot(x,y)
#endif

extern short silent_mode;

extern long vec_cntr_w;
extern long n_commands;
extern short record_off;

int read_PE_flags(GEN_PAR *, int, void *, PE_flags *);
int read_PE_coord(int, void *, PE_flags *, float *);
int read_PE_pair(int, void *, PE_flags *, HPGL_Pt *);
void read_PE(GEN_PAR *, void *);
int decode_PE_char(int, PE_flags *);
int isPEterm(int, PE_flags *);

float ceil_with_tolerance(float, float);
void line(int relative, HPGL_Pt p);

void fill(HPGL_Pt polygon[], int numpoints, HPGL_Pt P1, HPGL_Pt P2,
	  int scale_flag, int filltype, float spacing, float hatchangle);
