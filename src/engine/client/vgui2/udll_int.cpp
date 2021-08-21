/*
dll_int.cpp - dll entry point
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


#include "vgui_int.h"
#include "cdll_int.h"
#include "menu_int.h"
#include "gameinfo.h"
#include "keydefs.h"
#include "netadr.h"

#include "public/FileSystem.h"
#include "BaseUI/IBaseUI.h"
#include "vgui/IInput.h"
#include "vgui/IInputInternal.h"
#include "vgui/KeyCode.h"
#include "vgui/MouseCode.h"
#include "BaseUISurface.h"

#include "GameUI/IGameUI.h"

#include "keydest.h"

static IBaseUI* staticUIFuncs;
extern IGameUI* staticGameUIFuncs;
extern cl_enginefunc_t cl_enginefuncs;
extern BaseUISurface* staticSurface;
extern qboolean g_bScissor;
extern vgui2::IInputInternal* g_pInputInternal;

void UI_UpdateUserinfo(void);
void Key_ClearStates(void);
void Key_EnableTextInput(qboolean enable, qboolean force);

namespace ui
{
	ui_enginefuncs_t engfuncs;
#ifndef XASH_DISABLE_FWGS_EXTENSIONS
	ui_textfuncs_t	textfuncs;
#endif
}

namespace ui
{
	ui_globalvars_t	*gpGlobals;
}

static bool pending_activate = false;

void UI_Init(void)
{
	CreateInterfaceFn pEngineFactory = Sys_GetFactoryThis();
	staticUIFuncs = (IBaseUI*)pEngineFactory(BASEUI_INTERFACE_VERSION, NULL);
}
int UI_VidInit(void)
{
	/*
	SCREENINFO screeninfo;
	screeninfo.iSize = sizeof(SCREENINFO);
	cl_enginefuncs.pfnGetScreenInfo(&screeninfo);
	staticSurface->SetScreenInfo(&screeninfo);
	*/ 

	// AVSARWHY?

	return 0;
}
void UI_Shutdown(void)
{

}
void UI_KeyEvent(int key, int down)
{
	if (!staticUIFuncs || !staticGameUIFuncs)
	{
		return;
	}
	
	bool want_text_input = staticUIFuncs->Key_Event(down, key, "");
	Key_EnableTextInput(want_text_input, false);
}
void UI_MouseMove(int x, int y)
{

}
void UI_SetActiveMenu(int fActive)
{
	if (!staticUIFuncs || !staticGameUIFuncs)
	{
		pending_activate = fActive;
		return;
	}
	pending_activate = false;

	Key_ClearStates();

	if (fActive)
		staticUIFuncs->ActivateGameUI();
	else
		staticUIFuncs->HideGameUI();
}
void UI_UpdateMenu(float flTime)
{
	if (pending_activate)
	{
		// real VidInit goes here
		UI_SetActiveMenu(true);
		staticUIFuncs->ShowConsole();
	}
	
	if (!staticUIFuncs) {
		return;
	}
	
	int wide, tall;
	staticSurface->GetScreenSize(wide, tall);
	g_bScissor = true;
	staticUIFuncs->Paint(0, 0, wide, tall);
	g_bScissor = false;
	UI_UpdateUserinfo();
}
void UI_AddServerToList(netadr_t adr, const char* info)
{

}
void UI_GetCursorPos(int* pos_x, int* pos_y)
{

}
void UI_SetCursorPos(int pos_x, int pos_y)
{

}
void UI_ShowCursor(int show)
{

}
static int utf8charlen(int in)
{
	if (in >= 0xF8)
		return 0;
	else if (in >= 0xF0)
		return 4;
	else if (in >= 0xE0)
		return 3;
	else if (in >= 0xC0)
		return 2;
	else if (in <= 0x7F)
		return 1; //ascii
}
void UI_CharEvent(int utf8)
{
	static int iCharCounter = 0;
	static int iInputIndex = 0;
	static char szInput[5] = {};

	if(!iCharCounter)
	{
		iCharCounter = utf8charlen(utf8);
		iInputIndex = 0;
		szInput[0] = 0;
	}

	if (!iCharCounter)
		return;
	
	szInput[iInputIndex] = (char)utf8;
	++iInputIndex;
	--iCharCounter;
	if(!iCharCounter)
	{
		wchar_t unicode[16] = {};
		V_UTF8ToUnicode(szInput, unicode, 16);
		for(int i = 0; unicode[i]; ++i)
			g_pInputInternal->InternalKeyTyped(unicode[i]);
	}
}
int UI_MouseInRect(void)
{
	return 0;
}
int UI_IsVisible(void)
{
	return staticUIFuncs->IsGameUIVisible();
}
int UI_CreditsActive(void)
{
	return 0;
}
void UI_FinalCredits(void)
{

}
