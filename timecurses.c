#include <stdlib.h>
#include <ncurses.h>
#include <sys/time.h>
#include <math.h>
#include "curslib.h"
#include "timecurses.h"

#define NP 2048  // number of points in deterministic pattern


// Completely random matrix of data
long write_matrix (int nrows, int ncols, int docolor)
{
   int r, c, attrb;

   attron (A_BOLD);
   for (r = 2; r < nrows - 2; ++r)
   {
      move (r, 0);
      for (c = 0; c < ncols; ++c)
      {
         if (docolor)
         {
            attrb = COLOR_PAIR((rand () & 0x000D) + 1);
            attron (attrb);
         }
         addch ((rand () & 0x3F) + 33);
      }
   }
   attroff (A_BOLD);

   // return frame bit count
   if (docolor)
   {
      return 64*nrows*ncols;
   }
   else
   {
      return 8*nrows*ncols;
   }
}

// Deterministic (compressible) matrix of data
long write_matrix_det (int nrows, int ncols, int docolor)
{
   int row, col, attrb;
   static double rs[NP], phis[NP];
   static int nrowslast, ncolslast;

   if ((nrows == nrowslast) && (ncols == ncolslast))
   {
      // rotate
      double speed = 0.0001, vorticity = 2.0;
      for (register int k = 0; k < NP; k++)
      {
         if (docolor)
            phis[k] += speed/(vorticity*rs[k]/ncols+speed);
         else
            phis[k] += speed/(vorticity*rs[k]/ncols+speed);
      }
   }
   else
   {
      // init
      double uniformrv;
      for (int k = 0; k < NP; k++)
      {
         uniformrv = (double) rand() / (double) RAND_MAX;
         rs[k] = uniformrv*ncols/2.0;
         phis[k] = rand_max(359) / 3.1456 * 180.0;
      }
      nrowslast = nrows; ncolslast = ncols;
   }

   // clear
   for (row = 2; row < nrows - 2; row++)
   {
      move (row, 0);
      clrtoeol();
   }

   // write field
   attron (A_BOLD);
   int cycletime;
   cycletime = (int) floor((double) NP / 8.0);
   for (register int k = 0; k < NP; k++)
   {
      col = (int) round(rs[k]*cos(phis[k]) + ncols/2.0);
      row = (int) round(rs[k]*sin(phis[k])/2.0 + nrows/2.0);
      move (limiter(row, 2, nrows - 3), limiter(col, 1, ncols));
      if (docolor && !(k % cycletime))
      {
         attrb = COLOR_PAIR((int) ceil((double) 8*k / (double) NP) + 1);
         attron(attrb);
      }
      if (qlimit (row, 1, nrows - 2))
         addch ('.');  // in view
      else
         addch (' ');  // not in view
   }
   attroff (A_BOLD);

   // return frame bit count
   if (docolor)
   {
      return 32*NP;
   }
   else
   {
      return 16*NP;
   }
}

// Track and display bitrate
void display_mbps (long bits, int nrows, int ncols, int docolor, int docomp, int reset)
{
   static int sec, us, secold, usold;
   struct timeval systime;
   double bps, dt;

   if (reset || secold == 0)  // init
   {
      gettimeofday (&systime, NULL);
      secold = systime.tv_sec;
      usold = systime.tv_usec;
      bps = 0;
   }
   else  // normal ops
   {
      gettimeofday (&systime, NULL);
      sec = systime.tv_sec;
      us = systime.tv_usec;
      dt = (double) (sec - secold) + (double) (us - usold)*1e-6;
      bps = bits/dt;
   }

   attron(COLOR_PAIR(1));
   move(0, 0);
   clrtoeol();
   attron(A_BOLD);
   if (bps == 0)
      printw ("---");
   else
      printw ("%d", (int) round(bps / 1024 / 1024));
   attroff(A_BOLD);
   printw (" Mbps");
   attroff(COLOR_PAIR(1));
   if (docomp)
   {
      move(0, ncols - 26);
      attron(COLOR_PAIR(6));
      printw("Bitrate may be unreliable.");
      attroff(COLOR_PAIR(6));
   }
}

void static_display (int nrows, int ncols, int docolor, int docomp, int verbose)
{
   attron(COLOR_PAIR(1));
   drawline (1, ncols);
   drawline (nrows - 2, ncols);
   move (nrows - 1, 0);
   printw ("Type ");
   attron (A_BOLD);
   addch ('q');
   attroff (A_BOLD);
   printw(" to to quit, ");
   attron (A_BOLD);
   addch ('r');
   attroff (A_BOLD);
   printw (" to toggle ");
   if (docomp)
   {
      printw ("random");
   }
   else
   {
      attron (COLOR_PAIR(16));
      printw("random");
      attron (COLOR_PAIR(1));
   }
   printw (", ");
   attron (A_BOLD);
   addch ('c');
   attroff (A_BOLD);
   printw(" to toggle ");
   if (docolor)
   {
      attron (COLOR_PAIR(16));
      printw("color");
      attron (COLOR_PAIR(1));
   }
   else
   {
      printw("color");
   }
   addch ('.');
   if (verbose)
   {
      printw(" chars: ");
      if (docomp)
         printw("%d", NP);
      else
         printw("%d", nrows*ncols);
      printw(" res: %d x %d", ncols, nrows);
   }
   attroff(COLOR_PAIR(1));
}
