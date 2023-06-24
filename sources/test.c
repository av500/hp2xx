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
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

void PError(const char *msg)
{
	perror(msg);
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

/**************************************************************************
 **
 ** HPGL_to_TMP ():
 **
 ** This call opens a single HP-GL input file, scans and interprets
 ** its commands, and writes elementary move/draw commands into
 ** a temporary file.
 **	The input file is closed after returning, but the temp. file
 ** is kept open. You may re-use it multiple times. Close it finally!
 **	Calling this function invalidates later processing stages like
 ** the picture buffer.
 **/

int HPGL_to_TMP(GEN_PAR * pg, IN_PAR * pi)
{
  /**
   ** Clean up previous leftovers (if any)
   **/

	__cleanup_g(pg);
  /**
   ** Open HP-GL input file. Use stdin if selected.
   **/

	if (*pi->in_file == '-')
		pi->hd = stdin;
	else if (pi->hd == NULL) {
		if ((pi->hd = fopen(pi->in_file, READ_BIN)) == NULL) {
			PError("hp2xx (while opening HPGL file)");
			return ERROR;
		}
	}
  /**
   ** Open temporary intermediate file.
   **/

	if ((pg->td = tmpfile()) == NULL)
	{
		PError("hp2xx -- opening temporary file");
		return ERROR;
	}

  /**
   ** Convert HPGL data into compact temporary binary file, and obtain
   ** scaling data (xmin/xmax/ymin/ymax in plotter coordinates)
   **/
	n_commands = 0;
	read_HPGL(pg, pi);
	if (n_commands <= 1 && n_commands >= 0) {
		if (pi->hd != stdin) {
			fclose(pi->hd);
			pi->hd = NULL;
		}
		return ERROR;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	GEN_PAR Pg;
	IN_PAR Pi;
	
	__preset_par(&Pg, &Pi);

	Pi.in_file = argv[1];
	
	int err = HPGL_to_TMP(&Pg, &Pi);

	__cleanup(&Pg, &Pi);
	return NOERROR;
}