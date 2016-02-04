/* resuse.c - declarations for process resource use library
   Copyright (C) 2016 Jeramie Vens
   Copyright (C) 1993, 1996 Free Software Foundation, Inc.

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

#ifndef RESUSE_H
#define RESUSE_H

# include <sys/time.h>
#  include <sys/resource.h>

# define TV_MSEC tv_usec / 1000

enum resuse_scope
{
	RESUSE_SCOPE_PROC = RUSAGE_SELF,
#ifdef __USE_GNU
	RESUSE_SCOPE_THREAD = RUSAGE_THREAD,
#else
# warning RESUSE_SCOPE_THREAD Not Defined
#endif
};

/**
 * Resource Usesage Structure
 */
struct resuse
{
	// system resource usage
	struct rusage ru;
	// start and elapsed time
	struct timeval start, elapsed;
	// the scope of the resource collection
	enum resuse_scope scope;
};

void resuse_start(struct resuse *resp, enum resuse_scope scope);

void resuse_end(struct resuse *resp);

//int resuse_print(const char * fmt, const struct resuse * resp);
int resuse_fprint(FILE * fp, const char * fmt, const struct resuse * resp);
//int resuse_snprint(char * buffer, size_t n, const char * fmt, const struct resuse *resp);
//int resuse_asprint(char **ptr, const char *fmt, const struct resuse * resp);

#endif

