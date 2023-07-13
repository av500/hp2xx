#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "bresnham.h"
#include "pendef.h"
#include "hp2xx.h"
#include "hpgl.h"

void Eprintf(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void PError(const char *msg)
{
	printf("ERROR: %s", msg);
}

static void __reset_par(IN_PAR * pi)
/**
 ** Reset some parameter struct elements which may have been changed
 ** by action() to their defaults
 **/
{
	pi->x0 = 1e10;		/* HP7550A's range is about     */
	pi->x1 = -1e10;		/* [-2^24, 2^24], so we're safe */
	pi->y0 = 1e10;
	pi->y1 = -1e10;
}

static void __preset_par(GEN_PAR * pg, IN_PAR * pi)
/**
 ** Pre-set constant parameter struct elements with reasonable defaults
 **/
{
	int i;

	pi->aspectfactor = 1.0;
	pi->center_mode = FALSE;
	pi->height = 200.0;
	pi->width = 200.0;
	pi->xoff = 0.0;
	pi->yoff = 0.0;
	pi->truesize = FALSE;
	pi->hwcolor = FALSE;
	pi->hwsize = FALSE;
	pi->rotation = 0.0;
	pi->in_file = "";
	pi->hd = NULL;
	pi->first_page = 0;
	pi->last_page = 0;

	pi->hwlimit.x = 33600.;
	pi->hwlimit.y = 47520.;

	pg->logfile = "";
	pg->swapfile = "hp2xx.swp";
	pg->mode = "pre";
	pg->td = NULL;
	pg->xx_mode = XX_PRE;
	pg->nofill = FALSE;
	pg->no_ps = FALSE;
	pg->quiet = FALSE;
	pg->extraclip = 0;
	pg->maxpensize = 0.1;	/* 1/10 mm              */
	pg->maxcolor = 1;	/* max. color index             */
	pg->maxpens = 8;
	pg->is_color = FALSE;
	pg->mapzero = -1;

	pt.width[0] = 0.0;	/* 1/10 mm              */
	pt.color[0] = xxBackground;
	for (i = 1; i <= NUMPENS; i++) {
		pt.width[i] = 0.1;	/* 1/10 mm              */
		pt.color[i] = xxForeground;
	}
	pt.color[1] = xxForeground;
	pt.color[2] = xxRed;
	pt.color[3] = xxGreen;
	pt.color[4] = xxBlue;
	pt.color[5] = xxCyan;
	pt.color[6] = xxMagenta;
	pt.color[7] = xxYellow;
	set_color_rgb(xxBackground, 255, 255, 255);
	set_color_rgb(xxForeground, 0, 0, 0);
	set_color_rgb(xxRed, 255, 0, 0);
	set_color_rgb(xxGreen, 0, 255, 0);
	set_color_rgb(xxBlue, 0, 0, 255);
	set_color_rgb(xxCyan, 0, 255, 255);
	set_color_rgb(xxMagenta, 255, 0, 255);
	set_color_rgb(xxYellow, 255, 255, 0);

	__reset_par(pi);
}

/**************************************************************************
 **
 ** cleanup_x ():
 **
 ** Call these functions to close & remove the temp. and input file
 **	as well as to free the raster picture buffer.
 ** Calling is ok even if the buffer is already freed or the files
 **	are already closed, so just call them when in doubt.
 **/

static void __cleanup_g(GEN_PAR * pg)
{
	if (pg != NULL && pg->td != NULL) {
		fclose(pg->td);
		pg->td = NULL;
	}
}

static void __cleanup_i(IN_PAR * pi)
{
	if (pi != NULL && pi->hd != NULL) {
		if (pi->hd != stdin)
			fclose(pi->hd);
		pi->hd = NULL;
	}
}

static void __cleanup(GEN_PAR * pg, IN_PAR * pi)
{
	__cleanup_g(pg);
	__cleanup_i(pi);
}

#define DBG if(1)

void Init_to_tmpfile(void)
{
DBG printf("[%5d]  ", n_commands);
DBG printf("INIT\n");
}

void Pen_to_tmpfile(int pen)
{
	if (record_off)
		return;

DBG printf("[%5d]  ", n_commands);
DBG printf("PEN %d\n", pen);
}

void Speed_to_tmpfile(int speed)
{
	if (record_off)
		return;

DBG printf("[%5d]  ", n_commands);
DBG printf("VS  %d\n", speed);
}

void Pt_to_tmpfile(PlotCmd cmd, const HPGL_Pt * pf)
{
	if (record_off)		/* Wrong page!  */
		return;

DBG printf("[%5d]  ", n_commands);
DBG printf("CMD %d  Pt  %13.3f %13.3f\n", cmd, pf->x, pf->y);
}

void Line_Attr_to_tmpfile(LineAttrKind kind, int value)
{
	if (record_off)		/* return if current plot is not the selected one */
		return;		/* (of a multi-image file) */

	if (kind == LineAttrEnd)	/* save this so we may save/restore the current state before character draw */
		CurrentLineEnd = value;

DBG printf("[%5d]  ", n_commands);
DBG printf("DEF_LA  %d  %d\n", kind, value);
	return;
}

void Pen_Width_to_tmpfile(int pen, PEN_W width)
{
	if (record_off)		/* Wrong page!  */
		return;
	if (pen < 0)
		return;		/* Might happen when "current pen" is still
				   undefined */
DBG printf("[%5d]  ", n_commands);
DBG printf("DEF_PW %d  %f\n", pen, width);
}

void Pen_Color_to_tmpfile(int pen, int red, int green, int blue)
{
	if (record_off)		/* Wrong page!  */
		return;

DBG printf("[%5d]  ", n_commands);
DBG printf("DEF_PC %d %d %d\n", red, green, blue);
}

int  read_c(void *ctx);
void unread_c(int c, void *ctx);

int read_c(void *ctx)
{
	FILE *hd = (FILE*) ctx;
	int c = getc(hd);
	if (c == EOF)
		return EOF;
	return c;
}

void unread_c(int c, void *ctx)
{
	FILE *hd = (FILE*) ctx;
	ungetc(c, hd);
}

int main(int argc, char *argv[])
{
	GEN_PAR  Pg;
	GEN_PAR *pg = &Pg;
	IN_PAR   Pi;
	IN_PAR  *pi = &Pi;
	
	__preset_par(pg, pi);
	__cleanup_g(pg);

	init_HPGL(pg, pi);

	pi->in_file = argv[1];

	if (*pi->in_file == '-')
		pi->hd = stdin;
	else if (pi->hd == NULL) {
		if ((pi->hd = fopen(pi->in_file, READ_BIN)) == NULL) {
			PError("hp2xx (while opening HPGL file)");
			return ERROR;
		}
	}

	n_commands = 0;
	while(1) {
		if(read_HPGL(pg, pi)) {
			break;
		}
	}
	if (n_commands <= 1 && n_commands >= 0) {
		if (pi->hd != stdin) {
			fclose(pi->hd);
			pi->hd = NULL;
		}
		return ERROR;
	}

	__cleanup(pg, pi);
	return NOERROR;
}
