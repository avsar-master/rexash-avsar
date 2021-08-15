/*
build.c - returns a engine build number
Copyright (C) 2010 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"

#define XASH_GENERATE_BUILDNUM

#if defined(XASH_GENERATE_BUILDNUM)
static char *date = __DATE__;
static char *mon[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char mond[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
#endif

// returns days since Feb 13 2007
int Q_buildnum( void )
{
// do not touch this! Only author of Xash3D can increase buildnumbers!
// Xash3D SDL: HAHAHA! I TOUCHED THIS!
#if defined(XASH_GENERATE_BUILDNUM)
	int m = 0, d = 0, y = 0;
	static int b = 0;

	if( b != 0 ) return b;

	for( m = 0; m < 11; m++ )
	{
		if( !Q_strnicmp( &date[0], mon[m], 3 ))
			break;
		d += mond[m];
	}

	d += Q_atoi( &date[4] ) - 1;
	y = Q_atoi( &date[7] ) - 1900;
	b = d + (int)((y - 1) * 365.25f );

	if((( y % 4 ) == 0 ) && m > 1 )
	{
		b += 1;
	}
	//b -= 38752; // Feb 13 2007
	b -= 41728; // Apr 1 2015. Date of first release of crossplatform Xash3D

	return b;
#else
	return 1276; // Aug 13 2016
#endif
}

/*
============
Q_buildos

Returns current name of operating system. Without any spaces.
============
*/
const char *Q_buildos( void )
{
	const char *osname;

#if defined(_WIN32) && defined(_MSC_VER)
	osname = "Win32";
#elif defined(__linux__)
	osname = "Linux";
#elif defined(__APPLE__) && ( TARGET_OS_OSX || TARGET_OS_MAC )
	osname = "macOS";
#else
#error "Place your operating system name here! If this is a mistake, try to fix conditions above and report a bug"
#endif

	return osname;
}

/*
============
Q_buildos

Returns current name of operating system. Without any spaces.
============
*/
const char *Q_buildarch( void )
{
	const char *archname;

#if defined(__i386__) || defined(_X86_) || defined(_M_IX86)
	archname = "i386";
#else
#error "111!11!1!1!!1111!!!1!"
#endif

	return archname;
}

/*
=============
Q_buildcommit

Returns a short hash of current commit in VCS as string.
XASH_BUILD_COMMIT must be passed in quotes

if XASH_BUILD_COMMIT is not defined,
Q_buildcommit will identify this build as release or "notset"
=============
*/
const char *Q_buildcommit( void )
{
	return "custom";
}

/*
=============
Q_buildnum_compat

Returns a Xash3D build number. This is left for compatibility with original Xash3D.
IMPORTANT: this value must be changed ONLY after updating to newer Xash3D
IMPORTANT: this value must be acquired through "build" cvar.
=============
*/
int Q_buildnum_compat( void )
{
	return 3366;
}
