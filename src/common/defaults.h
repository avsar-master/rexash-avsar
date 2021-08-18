/*
defaults.h - set up default configuration
Copyright (C) 2016 Mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

/*
=========================================================================

Default build-depended cvar and constant values

=========================================================================
*/

#define DEFAULT_PRIMARY_MASTER "ms.xash.su:27010"
#define DEFAULT_SECONDARY_MASTER "ms2.xash.su:27010"
// Set ForceSimulating to 1 by default for dedicated, because AMXModX timers require this
// TODO: enable simulating for any server?

#ifdef XASH_DEDICATED
	#define DEFAULT_SV_FORCESIMULATING "1"
#else
	#define DEFAULT_SV_FORCESIMULATING "0"
#endif

// allow override for developer/debug builds
#ifndef DEFAULT_DEV
	#define DEFAULT_DEV 0
#endif

#ifndef DEFAULT_FULLSCREEN
#define DEFAULT_FULLSCREEN 1
#endif

#define DEFAULT_CON_MAXFRAC "1"

#endif // DEFAULTS_H
