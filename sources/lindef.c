/*
   Copyright (c) 2001-2002 Andrew Bird  All rights reserved.
   Distributed by Free Software Foundation, Inc.

This file is part of HP2xx.

HP2xx is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY.  No author or distributor accepts responsibility
to anyone for the consequences of using it or for whether it serves any
particular purpose or works at all, unless he says so in writing.  Refer
to the GNU General Public License, Version 2 or later, for full details.

Everyone is granted permission to copy, modify and redistribute
HP2xx, but only under the conditions described in the GNU General Public
License.  A copy of this license is supposed to have been
given to you along with HP2xx so you can know your rights and
responsibilities.  It should be in a file named COPYING.  Among other
things, the copyright notice and this notice must be preserved on all
copies.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
*/

#include <stdarg.h>
#include <math.h>
#include "lindef.h"
#include "hpgl.h"


float CurrentLinePatLen;
/*LineType   CurrentLineType;*/
signed int CurrentLinePattern;

LINESTYLE lt;

LineAttr CurrentLineAttr;
LineEnds CurrentLineEnd;

/********************************************
 * Line Style
 ********************************************/

void set_line_style(SCHAR index, ...)
{
	SCHAR count;
	float factor, percentage;
	va_list ap;

	SCHAR val;

	va_start(ap, index);

	for (count = 0, percentage = 0; count < LT_ELEMENTS; count++) {

		val = va_arg(ap, int);

		if (val < 0) {
			break;
		} else {
			lt[index - LT_MIN][count] = (float) val;
			percentage += val;
		}
	}

	lt[index - LT_MIN][count] = -1;

	if (fabs(percentage - 100.) > 0.5) {
		factor = 100.0 / percentage;
		for (count = 0; count < LT_ELEMENTS; count++) {
			if (lt[index - LT_MIN][count] < 0) {
				break;
			} else {
				lt[index - LT_MIN][count] *= factor;
			}
		}
	}

	va_end(ap);

}

void set_line_style_by_UL(void *hd)
{
	SCHAR index, pos_index, neg_index, count, i;
	float factor, percentage;
	float tmp;

	if (read_float(&tmp, hd)) {
		set_line_style_defaults();	/* reset to defaults */
		return;
	} else {
		index = (int) tmp;
	}

	pos_index = index - LT_MIN;
	neg_index = (index * -1) - LT_MIN;

	for (count = 0, percentage = 0; (read_float(&tmp, hd) == 0); count++) {	/* while there is an argument */
		lt[pos_index][count] = (float) tmp;
		percentage += (int) tmp;
	}

	lt[pos_index][count] = -1;

	if (fabs(percentage - 100.) > 0.5) {
		factor = 100.0 / percentage;
		for (count = 0; count < LT_ELEMENTS; count++) {
			if (lt[pos_index][count] < 0) {
				break;
			} else {
				lt[pos_index][count] *= factor;
			}
		}
	}
	/* now derive the adaptive version */

	count--;

	if (count % 2) {	/* last value denotes a gap */
		lt[neg_index][0] = lt[pos_index][0] / 2;
		for (i = 1; i <= count; i++)
			lt[neg_index][i] = lt[pos_index][i];
		lt[neg_index][count + 1] = lt[pos_index][0] / 2;
		lt[neg_index][count + 2] = -1;
	} else {		/* last value denotes a line */
		lt[neg_index][0] =
		    (lt[pos_index][0] + lt[pos_index][count]) / 2;
		for (i = 1; i < count; i++)
			lt[neg_index][i] = lt[pos_index][i];
		lt[neg_index][count] = lt[neg_index][0];
		lt[neg_index][count + 1] = -1;
	}

}


void set_line_style_defaults()
{
/*                 Line  gap   Line  gap   Line   gap   Line  TERM        */

	set_line_style(-8, 25, 10, 0, 10, 10, 10, 0, 10, 25, -1);
	set_line_style(-7, 35, 10, 0, 10, 0, 10, 35, -1);
	set_line_style(-6, 25, 10, 10, 10, 10, 10, 25, -1);
	set_line_style(-5, 35, 10, 10, 10, 35, -1);
	set_line_style(-4, 40, 10, 0, 10, 40, -1);
	set_line_style(-3, 35, 30, 35, -1);
	set_line_style(-2, 25, 50, 25, -1);
	set_line_style(-1, 0, 100, 0, -1);
	set_line_style(0, 0, 100, -1);
	set_line_style(1, 0, 100, -1);
	set_line_style(2, 50, 50, -1);
	set_line_style(3, 70, 30, -1);
	set_line_style(4, 80, 10, 0, 10, -1);
	set_line_style(5, 70, 10, 10, 10, -1);
	set_line_style(6, 50, 10, 10, 10, 10, 10, -1);
	set_line_style(7, 70, 10, 0, 10, 0, 10, -1);
	set_line_style(8, 50, 10, 0, 10, 10, 10, 0, 10, -1);
}

/************************************************* 
 *  Line Attributes
 *************************************************/

void set_line_attr_defaults(void)
{
	Line_Attr_to_tmpfile(LineAttrEnd, LAE_butt);
#define LA_JOINS_LIMIT_SUPPORT 1
#ifdef LA_JOINS_LIMIT_SUPPORT
	Line_Attr_to_tmpfile(LineAttrJoin, LAJ_plain_miter);

	Line_Attr_to_tmpfile(LineAttrLimit, 5);	/* 5 times line width */
#endif
}

void set_line_attr(void * hd)
{
	float ftmp1;
	float ftmp2;

/*   LineEnds itmp;*/
	int itmp;

	if (read_float(&ftmp1, hd)) {	/* No kind found        */
		set_line_attr_defaults();
		return;
	}

	for (;;) {

		if (read_float(&ftmp2, hd)) {	/* No value found       */
			/* do_error */
			return;
		}
		itmp = (int) ftmp2;

		switch ((int) ftmp1) {
		case 1:
			if ((itmp >= LAE_butt) && (itmp <= LAE_round)) {
				Line_Attr_to_tmpfile(LineAttrEnd, itmp);
			} else {
				Line_Attr_to_tmpfile(LineAttrEnd,
						     LAE_butt);
			}
			break;
		case 2:
			if ((itmp >= LAJ_plain_miter)
			    && (itmp <= LAJ_nojoin)) {
				Line_Attr_to_tmpfile(LineAttrJoin, itmp);
			} else {
				Line_Attr_to_tmpfile(LineAttrJoin,
						     LAJ_plain_miter);
			}
			break;
		case 3:
			Line_Attr_to_tmpfile(LineAttrLimit, itmp);
			break;
		}
		if (read_float(&ftmp1, hd)) {	/* No kind found        */
			return;
		}
	}
	return;
}

#ifndef EMBEDDED
#include <stdlib.h>
#include <stdio.h>

int load_line_attr(FILE * td)
{
	LineAttrKind kind;
	int value;
	static int FoundJoin = 0;
	static int FoundLimit = 0;

	if (fread((void *) &kind, sizeof(kind), 1, td) != 1) {
		return (-1);
	}

	if (fread((void *) &value, sizeof(value), 1, td) != 1) {
		return (-1);
	}

	switch (kind) {
	case LineAttrEnd:
		CurrentLineAttr.End = value;
		break;
	case LineAttrJoin:
		if (!FoundJoin) {
			if (!silent_mode)
				fprintf(stderr,
					"\nLA - Joins not supported\n");
			FoundJoin = 1;
		}
		CurrentLineAttr.Join = value;
	case LineAttrLimit:
		if (!FoundLimit) {
			if (!silent_mode)
				fprintf(stderr,
					"\nLA - Limit not supported\n");
			FoundLimit = 1;
		}
		CurrentLineAttr.Limit = value;
		break;
	}

	return (0);
}
#endif