/*
launcher.c - direct xash3d launcher
Copyright (C) 2015 Mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifdef SINGLE_BINARY

#include <stdlib.h>
#include <string.h>

char szGameDir[128]; // safe place to keep gamedir
int g_iArgc;

typedef void (*pfnChangeGame)(const char* progname);

void Host_Shutdown( void );
void *Com_LoadLibrary(char *, int );
int Host_Main(int argc, const char** argv, const char* progname, int bChangeGame, pfnChangeGame func);

char **g_pszArgv;

void Launcher_ChangeGame( const char *progname );

void Launcher_ChangeGame( const char *progname )
{
	strncpy( szGameDir, progname, sizeof( szGameDir ) - 1 );
	Host_Shutdown( );
	exit( Host_Main( g_iArgc, (const char**)g_pszArgv, szGameDir, 1, &Launcher_ChangeGame ) );
}

#ifdef XASH_NOCONHOST
#include <windows.h>
int __stdcall WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int nShow)
{
	int szArgc;
	char **szArgv;
	LPWSTR* lpArgv = CommandLineToArgvW(GetCommandLineW(), &szArgc);
	int size, i = 0;
	szArgv = (char**)сalloc(szArgc + 1,sizeof(char*));
	for (; i < szArgc; ++i)
	{
		size = wcslen(lpArgv[i]) + 1;
		szArgv[i] = (char*)malloc(size);
		wcstombs(szArgv[i], lpArgv[i], size);
	}
	LocalFree(lpArgv);
	main( szArgc, szArgv );
}
#endif


int main( int argc, char** argv )
{
	char gamedir_buf[32] = "";
	const char *gamedir = getenv("XASH3D_GAMEDIR");

	if(!gamedir)
		gamedir = "cstrike";
	else
	{
		strncpy( gamedir_buf, gamedir, 32 );
		gamedir = gamedir_buf;
	}

	g_iArgc = argc;
	g_pszArgv = argv;

	return Host_Main( g_iArgc, (const char**)g_pszArgv, gamedir, 0, &Launcher_ChangeGame );
}

#endif
