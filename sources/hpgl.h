#include "lindef.h"
#include "pendef.h"
#include "bresnham.h"
#include "hp2xx.h"

int read_c(void *ctx);
void unread_c(int c, void *ctx);
int read_float (float *pnum, void * hd);
void read_string (char *target, void *);

int write_c(int c, void *ctx);
size_t write_bytes(const void *ptr, size_t size, size_t nmemb, void *ctx);

void PlotCmd_to_tmpfile (PlotCmd cmd);
void Pen_to_tmpfile(int pen);
void Speed_to_tmpfile(int speed);
void HPGL_Pt_to_tmpfile (const HPGL_Pt * pf);

void HPGL_Pt_to_polygon (const HPGL_Pt pf );

#ifdef EMF
void reset_tmpfile(void);
#endif

#if defined __TURBOC__ || defined __STRICT_ANSI__
#define	HYPOT(x,y)	sqrt((x)*(x)+(y)*(y))
#else
#define	HYPOT(x,y)	hypot(x,y)
#endif

extern short silent_mode;

extern long vec_cntr_w;
extern long n_commands;
extern short record_off;
extern FILE *temp_file;

