/*	Clip.h	Release : 2.2	Date : 12/09/93	by sk	*/
/*-----------------------------------------------
 *	Clip.h
 *
 *	Headerfile zum Clipping nach Liang-Barsky
 */

#ifndef _Dt_CLIP_H
#define _Dt_CLIP_H

#ifdef __cplusplus
extern "C" {
#endif

#define CLIP_DRAW 1
#define CLIP_NODRAW 0

/*  Der Rœckgabewert der Funktion ist entweder CLIP_DRAW, dann
 *  muﬁ etwas gezeichnet werden und die Pointer auf die
 *  Koordinaten zeigen auf die korrigierten Werte, 
 *	oder CLIP_NODRAW , dann sind die Pointerinhalte undefiniert,
 *  weil nichts zu zeichnen ist
 */

	extern short DtClipLine(float xmin, float ymin,	/* Linke untere Ecke des Clip-Bereichs */
				float xmax, float ymax,	/* Rechte obere Ecke des Clip-Bereichs */
				float *x1, float *y1,	/* Pointer auf Startpunkt der Linie */
				float *x2, float *y2);	/* Pointer auf Endpunkt der Linie */

	extern short DtClipFilledBox(float xmin, float ymin,	/* Linke untere Ecke des Clip-Bereichs */
				     float xmax, float ymax,	/* Rechte obere Ecke des Clip-Bereichs */
				     float *x1, float *y1,	/* Pointer auf Startpunkt der Box */
				     float *x2, float *y2);	/* Pointer auf Endpunkt der Box */

#ifdef __cplusplus
}
#endif
#endif
