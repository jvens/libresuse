/* resuse.c - process resource use library
   Copyright (C) 2016 Jeramie Vens
   Copyright (C) 1993, 1996 Free Software Foundation, Inc.

   Written by Jeramie Vens ported from GNU Time
   Written by David MacKenzie, with help from
   arnej@imf.unit.no (Arne Henrik Juul)
   and pinard@iro.umontreal.ca (Francois Pinard).

   This file is part of libresuse.

   libresuse is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libresuse is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libresuse.  If not, see <http://www.gnu.org/licenses/>.
*/ 

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "resuse.h"

#ifndef TICKS_PER_SEC
#define TICKS_PER_SEC 100
#endif

#define MSEC_PER_TICK 		(1000 / TICKS_PER_SEC)
#define MSEC_TO_TICKS(m)	((m) / MSEC_PER_TICK)
#define UL			unsigned long

static unsigned long ptok (unsigned long pages);

/**
 * Start resource usage collection information.
 * @details    This will save the start time and scope information
 *             for the resource usage collector.  This function should
 *             be called at the top of the scope for which resource
 *             information is being collected.
 * @param      resp
 *                  The `struct resuse` structure to save the resource
 *                  information to.
 * @param      scope
 *                  The scope to collect resources for.
 */
void resuse_start(struct resuse * resp, enum resuse_scope scope)
{
	gettimeofday (&resp->start, (struct timezone *) 0);
	resp->scope = scope;
}

/**
 * Stop collecting and save the current resource usage information.
 * @details    This function will save the current time and calculate
 *             the elapsed time since resuse_start() was called.  It
 *             will also query the kernel for information about the
 *             resources used by the process or thread depending on
 *             scope.
 * @param      resp
 *                  The `struct resuse` structure that was given to
 *                  resuse_start().
 */
void resuse_end(struct resuse * resp)
{
	int status;
	gettimeofday (&resp->elapsed, (struct timezone *) 0);

	// get resource usage from the kernel
	getrusage(resp->scope, &resp->ru);

	// calculate elapsed time
	resp->elapsed.tv_sec -= resp->start.tv_sec;
	if (resp->elapsed.tv_usec < resp->start.tv_usec)
	{
		// Manually carry a one from the seconds field.
		resp->elapsed.tv_usec += 1000000;
		--resp->elapsed.tv_sec;
	}
	resp->elapsed.tv_usec -= resp->start.tv_usec;
}

/**
 * Print the resource usage as a formated string to the file stream.
 * @details    This function mimics fprintf().  It uses the format string
 *             to format the collected resource information.  The format
 *             string uses `%' flags similar to printf() functions.  To
 *             see all supported flags look at the projectes README.md
 * @param      fp
 *                  The file stream to print the output to
 * @param      fmt
 *                  The format string to be used.  `%' flags will be
 *                  replaced with approriate information from @param resp.
 * @param      resp
 *                  The resuse structure that was passed to resuse_start()
 *                  and resuse_end().
 */
int resuse_fprint(FILE * fp, const char * fmt, const struct resuse * resp)
{
	unsigned long r;
	unsigned long v;

	r = resp->elapsed.tv_sec * 1000 + resp->elapsed.tv_usec / 1000;
	v = resp->ru.ru_utime.tv_sec * 1000 + resp->ru.ru_utime.TV_MSEC +
	    resp->ru.ru_stime.tv_sec * 1000 + resp->ru.ru_stime.TV_MSEC;

	while (*fmt)
	{
		switch(*fmt)
		{
			case '%':
				switch (*++fmt)
				{
					case '%':	// Literal '%'
						putc('%', fp);
						break;
					case 'D':	// Average unshared data size
						fprintf(fp, "%lu",
							MSEC_TO_TICKS(v) == 0 ? 0 :
							ptok ((UL) resp->ru.ru_idrss) / MSEC_TO_TICKS(v) +
							ptok ((UL) resp->ru.ru_isrss) / MSEC_TO_TICKS(v));
						break;
					case 'E':	// Elapsed real (wall clock) time.
						if (resp->elapsed.tv_sec >= 3600) // one hour -> h:m:s
							fprintf (fp, "%ld:%02ld:%02ld",
								resp->elapsed.tv_sec / 3600,
								(resp->elapsed.tv_sec % 3600) / 60,
								resp->elapsed.tv_sec %60);
						else // m:s.ms
							fprintf (fp, "%ld:%02ld.%02ld",
								resp->elapsed.tv_sec / 60,
								resp->elapsed.tv_sec % 60,
								resp->elapsed.tv_usec / 10000);
						break;
					case 'F':	// Major page faults
						fprintf (fp, "%ld", resp->ru.ru_majflt);
						break;
					case 'I':	// Inputs
						fprintf (fp, "%ld", resp->ru.ru_inblock);
						break;
					case 'K':	// Average mem usage == data+stack+text
						fprintf (fp, "%lu",
							ptok ((UL) resp->ru.ru_idrss) / MSEC_TO_TICKS (v) +
							ptok ((UL) resp->ru.ru_isrss) / MSEC_TO_TICKS (v) +
							ptok ((UL) resp->ru.ru_ixrss) / MSEC_TO_TICKS (v));
						break;
					case 'M':	// Maximum resident set size
						fprintf (fp, "%lu", ptok ((UL) resp->ru.ru_maxrss));
						break;
					case 'O':	// Outputs
						fprintf (fp, "%ld", resp->ru.ru_oublock);
						break;
					case 'P':	// Percent of CPU this job got.
						if (r > 0)
							fprintf (fp, "%lu%%", (v * 100 / r));
						else
							fprintf (fp, "?%%");
						break;
					case 'R':	// Minor page faults (reclaims)
						fprintf (fp, "%ld", resp->ru.ru_minflt);
						break;
					case 'S':	// System time
						fprintf (fp, "%ld.%02d",
							resp->ru.ru_stime.tv_sec,
							resp->ru.ru_stime.TV_MSEC / 10);
						break;
					case 'U':	// User time
						fprintf (fp, "%ld.%02ld",
							resp->ru.ru_utime.tv_sec,
							resp->ru.ru_utime.TV_MSEC / 10);
						break;
					case 'W':	// Times swapped out
						fprintf (fp, "%ld", resp->ru.ru_nswap);
						break;
					case 'X':	// Average shared text size
						fprintf (fp, "%lu",
							MSEC_TO_TICKS (v) == 0 ? 0 :
							ptok ((UL) resp->ru.ru_ixrss) / MSEC_TO_TICKS (v));
						break;
					case 'Z':	// Page size
						fprintf (fp, "%d", getpagesize());
						break;
					case 'c':	// Involuntary context switches
						fprintf (fp, "%ld", resp->ru.ru_nivcsw);
						break;
					case 'e':	// Elapsed real time in seconds
						fprintf (fp, "%ld.%02ld",
							resp->elapsed.tv_sec,
							resp->elapsed.tv_usec / 10000);
						break;
					case 'k':	// Signals delivered
						fprintf (fp, "%ld", resp->ru.ru_nsignals);
						break;
					case 'p':	// Average stack segment
						fprintf (fp, "%lu",
							MSEC_TO_TICKS (v) == 0 ? 0 :
							ptok ((UL) resp->ru.ru_isrss) / MSEC_TO_TICKS (v));
						break;
					case 'r':	// Incoming socket messages received
						fprintf (fp, "%ld", resp->ru.ru_msgrcv);
						break;
					case 's':	// Outgoing socket messages sent
						fprintf (fp, "%ld", resp->ru.ru_msgsnd);
						break;
					case 't':	// Average resident set size
						fprintf (fp, "%lu",
							MSEC_TO_TICKS (v) == 0 ? 0 :
							ptok ((UL) resp->ru.ru_idrss) / MSEC_TO_TICKS (v));
						break;
					case 'w':	// Voluntary context switches
						fprintf (fp, "%ld", resp->ru.ru_nvcsw);
						break;
					case '\0':
						putc ('?', fp);
						return -1;
					default:
						putc ('?', fp);
						putc (*fmt, fp);
				}
				++fmt;
				break;
			case '\\':	// Format escape
				switch(*++fmt)
				{
					case 't':
						putc ('\t', fp);
						break;
					case 'n':
						putc ('\n', fp);
						break;
					case '\\':
						putc ('\\', fp);
						break;
					default:
						putc ('?', fp);
						putc ('\\', fp);
						putc (*fmt, fp);
				}
				++fmt;
				break;
			default:
				putc (*fmt++, fp);	
		}

		//if (ferror (fp))
		//	error (stderr, errno, "write error");
	}
	return 0;
}

/**
 * Converte number of pages to size in KB
 * @param   pages
 *               The number of pages
 * @return  The size in KB of the given number of pages.
 */
static unsigned long ptok (unsigned long pages)
{
	static unsigned long ps = 0;
	unsigned long tmp;
	static long size = LONG_MAX;

	if (ps == 0)
		ps = (long) getpagesize();
	
	if (pages > (LONG_MAX / ps))
	{	// could overflow
		tmp = pages / 1024;	// smaller first
		size = tmp * ps;	// then larger
	}
	else
	{	// could underflow
		tmp = pages * ps;	// larger first
		size = tmp / 1024;	// then smaller
	}
	return size;
}




