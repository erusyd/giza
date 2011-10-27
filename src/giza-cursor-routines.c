/* giza - a scientific plotting layer built on cairo
 *
 * This file is (or was) part of GIZA, a scientific plotting
 * layer built on cairo.
 * GIZA comes with ABSOLUTELY NO WARRANTY.
 * This is free software; and you are welcome to redistribute
 * it under the terms of the GNU General Public License
 * (see LICENSE file for details) and the provision that
 * this notice remains intact. If you modify this file, please
 * note section 2a) of the GPLv2 states that:
 *
 *  a) You must cause the modified files to carry prominent notices
 *     stating that you changed the files and the date of any change.
 *
 * Copyright (C) 2010-2011 James Wetter and Daniel Price. All rights reserved.
 * Contact: wetter.j@gmail.com, daniel.price@monash.edu
 *
 */

#include "giza-private.h"
#include "giza-io-private.h"
#include "giza-drivers-private.h"
#include "giza-cursor-private.h"
#include <giza.h>
#include <string.h>
#define GIZA_MARK_ORDERED 1
#define GIZA_MARK_UNORDERED 0



/**
 *  _giza_mark_with_cursor
 *
 *  This is the general (currently internal) routine to which
 *  giza_mark_points, giza_mark_lines etc are merely interfaces
 *
 */
void
_giza_mark_with_cursor (int maxpts, int *npts, double* xpts, double* ypts,
                        int symbol, int ordered, int mode)
{
  if(!_giza_check_device_ready ("_giza_mark_with_cursor")) /* should be done in parent routines anyway */
    return;

  if (maxpts <= 0)
    {
      _giza_error("_giza_mark_with_cursor","maxpts < 0 on input");
      return;
    }

  double x, y, xanc, yanc;
  double xmin,xmax,ymin,ymax;
  giza_get_window(&xmin,&xmax,&ymin,&ymax);

  if (*npts > 0)
    {
      xanc = xpts[*npts-1];
      yanc = ypts[*npts-1];
      /* plot previously drawn line segments/points */
      if (mode == GIZA_BAND_LINE)
        {
          giza_line(*npts,xpts,ypts);
        } else {
          giza_points(*npts,xpts,ypts,symbol);
        }
    } else {
      /* PGPLOT default is to put the cursor
         at the centre of the current viewport */
      xanc = 0.5*(xmin + xmax);
      yanc = 0.5*(ymin + ymax);
    }
  char ch[2] = "A";
  int err = 0;
  int moveCurs = 0;
  int modein = 0;

  while(err == 0) {
     err = _giza_get_key_press (modein, moveCurs, xanc, yanc, &x, &y, ch);
     modein = mode;

     if (!strcmp(ch,"A") || !strcmp(ch,"a")) /* || !strcmp(ch,GIZA_LEFT_CLICK)) */
       {
         if (*npts < maxpts-1)
           {
             *npts += 1;
             xpts[*npts-1] = x;
             ypts[*npts-1] = y;
             if (mode == GIZA_BAND_LINE && *npts > 1)
               {
                 giza_move(xpts[*npts-2],ypts[*npts-2]);
                 giza_draw(xpts[*npts-1],ypts[*npts-1]);
               } else {
                 giza_single_point(x,y,symbol);
               }
           } else {
             _giza_message("reached array limits, cannot add more points");
           }
       } else if (!strcmp(ch,"D") || !strcmp(ch,"d") || *ch == 8 ) {

         if (*npts > 0)
           {
             /* erase the previous point by colouring over it in the background colour */
             int ci;
             double lw;
             giza_get_colour_index(&ci);
             giza_set_colour_index(GIZA_BACKGROUND_COLOUR);
             giza_get_line_width(&lw);
             giza_set_line_width(2.*lw);
             if (mode == GIZA_BAND_LINE && *npts > 1)
               {
                 giza_move(xpts[*npts-1],ypts[*npts-1]);
                 giza_draw(xpts[*npts-2],ypts[*npts-2]);
               } else {
                 giza_single_point(xpts[*npts-1],ypts[*npts-1],17);
               }
             giza_set_colour_index(GIZA_FOREGROUND_COLOUR);
             giza_set_line_width(lw);

             *npts -= 1;
             if (*npts > 0)
               {
                 x = xpts[*npts - 1];
                 y = ypts[*npts - 1];
               } else {
                 /* reset back to starting conditions */
                 x = 0.5*(xmin + xmax);
                 y = 0.5*(ymin + ymax);
                 modein = 0;
               }
               giza_move(x,y);
           } else {
             _giza_message("no points left to delete");
           }

       } else if (!strcmp(ch,"X") || !strcmp(ch,"x") || *ch == 13) {

         return;

       } else if (!strcmp(ch,"q") || *ch == 27 ) {
         /* quit without saving */
         *npts = 0;
         return;

       } else {

         /*printf("char = %i %s\n",*ch,ch); */
         _giza_message("unknown command, use (a)dd (d)elete/backspace, e(x)it or (q)uit/Esc");

       }

     xanc = x;
     yanc = y;

  }

}

/**
 * _giza_mark_with_cursor_float
 *
 * Synopsis: Same functionality as _giza_mark_with_cursor, but takes floats
 *
 * See Also: _giza_mark_with_cursor
 */
void
_giza_mark_with_cursor_float (int maxpts, int *npts, float* xpts, float* ypts,
                              int symbol, int ordered, int mode)
{
   double xptsd[maxpts], yptsd[maxpts];
   int i;
   for (i = 0; i < *npts; i++)
     {
        xptsd[i] = (double) xpts[i];
        yptsd[i] = (double) ypts[i];
     }

   _giza_mark_with_cursor (maxpts, npts, xptsd, yptsd, symbol, ordered, mode);

   for (i = 0; i < *npts; i++)
     {
        xpts[i] = (float) xptsd[i];
        ypts[i] = (float) yptsd[i];
     }

}


/**
 * Interactive: giza_mark_points
 *
 * Synopsis: Mark a set of points using the cursor
 *
 * Input:
 *  -maxpts   :- maximum number of points that may be accepted
 *  -symbol   :- symbol code for drawing points
 *
 * Input/Output:
 *  -xpts     :- the x-coord of the points
 *  -ypts     :- the y-coord of the anchor point.
 *  -npts     :- number of points entered, should be zero on first call
 *
 * See Also: giza_points
 */
void
giza_mark_points (int maxpts, int *npts, double* xpts, double* ypts, int symbol)
{
  if(!_giza_check_device_ready ("giza_mark_points"))
    return;

  _giza_mark_with_cursor (maxpts, npts, xpts, ypts, symbol, GIZA_MARK_UNORDERED, GIZA_BAND_NONE);
}

/**
 * Interactive: giza_mark_points_float
 *
 * Synopsis: Same functionality as giza_mark_points, but takes floats
 *
 * See Also: giza_mark_points
 */
void
giza_mark_points_float (int maxpts, int *npts, float* xpts, float* ypts, int symbol)
{
   if(!_giza_check_device_ready ("giza_mark_points_float"))
     return;

   _giza_mark_with_cursor_float (maxpts, npts, xpts, ypts, symbol, GIZA_MARK_UNORDERED, GIZA_BAND_NONE);
}

/**
 * Interactive: giza_mark_points_ordered
 *
 * Synopsis: Mark a set of points using the cursor
 *
 * Input:
 *  -maxpts   :- maximum number of points that may be accepted
 *  -symbol   :- symbol code for drawing points
 *
 * Input/Output:
 *  -xpts     :- the x-coord of the points
 *  -ypts     :- the y-coord of the anchor point.
 *  -npts     :- number of points entered, should be zero on first call
 *
 * Note:
 *  Points are returned sorted in order according to the x coordinate
 *
 * See Also: giza_points
 */
void
giza_mark_points_ordered (int maxpts, int *npts, double* xpts, double* ypts, int symbol)
{
  if(!_giza_check_device_ready ("giza_mark_points_ordered"))
    return;

  _giza_mark_with_cursor (maxpts, npts, xpts, ypts, symbol, GIZA_MARK_ORDERED, GIZA_BAND_NONE);
}

/**
 * Interactive: giza_mark_points_ordered_float
 *
 * Synopsis: Same functionality as giza_mark_points_ordered, but takes floats
 *
 * See Also: giza_mark_points
 */
void
giza_mark_points_ordered_float (int maxpts, int *npts, float* xpts, float* ypts, int symbol)
{
   if(!_giza_check_device_ready ("giza_mark_points_ordered_float"))
     return;

   _giza_mark_with_cursor_float (maxpts, npts, xpts, ypts, symbol, GIZA_MARK_ORDERED, GIZA_BAND_NONE);
}

/**
 * Interactive: giza_mark_line
 *
 * Synopsis: Mark a set of points using the cursor
 *
 * Input:
 *  -maxpts   :- maximum number of points that may be accepted
 *
 * Input/Output:
 *  -xpts     :- the x-coord of the points
 *  -ypts     :- the y-coord of the anchor point.
 *  -npts     :- number of points entered, should be zero on first call
 *
 * See Also: giza_mark_points
 */
void
giza_mark_line (int maxpts, int *npts, double* xpts, double* ypts)
{
  if(!_giza_check_device_ready ("giza_mark_line"))
    return;

  _giza_mark_with_cursor (maxpts, npts, xpts, ypts, 1, GIZA_MARK_UNORDERED, GIZA_BAND_LINE);
}

/**
 * Interactive: giza_mark_points_float
 *
 * Synopsis: Same functionality as giza_mark_line, but takes floats
 *
 * See Also: giza_mark_line, giza_mark_points, giza_mark_line_ordered
 */
void
giza_mark_line_float (int maxpts, int *npts, float* xpts, float* ypts)
{
   if(!_giza_check_device_ready ("giza_mark_line_float"))
     return;

   _giza_mark_with_cursor_float (maxpts, npts, xpts, ypts, 1, GIZA_MARK_UNORDERED, GIZA_BAND_LINE);
}

/**
 * Interactive: giza_mark_line_ordered
 *
 * Synopsis: Mark a set of points using the cursor
 *
 * Input:
 *  -maxpts   :- maximum number of points that may be accepted
 *
 * Input/Output:
 *  -xpts     :- the x-coord of the points
 *  -ypts     :- the y-coord of the anchor point.
 *  -npts     :- number of points entered, should be zero on first call
 *
 * Note:
 *  Points are returned sorted in order according to the x coordinate
 *
 * See Also: giza_mark_line, giza_mark_points
 */
void
giza_mark_line_ordered (int maxpts, int *npts, double* xpts, double* ypts)
{
  if(!_giza_check_device_ready ("giza_mark_line_ordered"))
    return;

  _giza_mark_with_cursor (maxpts, npts, xpts, ypts, 1, GIZA_MARK_ORDERED, GIZA_BAND_LINE);
}

/**
 * Interactive: giza_mark_line_ordered_float
 *
 * Synopsis: Same functionality as giza_mark_line_ordered, but takes floats
 *
 * See Also: giza_mark_points
 */
void
giza_mark_line_ordered_float (int maxpts, int *npts, float* xpts, float* ypts)
{
   if(!_giza_check_device_ready ("giza_mark_line_ordered_float"))
     return;

   _giza_mark_with_cursor_float (maxpts, npts, xpts, ypts, 1, GIZA_MARK_ORDERED, GIZA_BAND_LINE);
}
