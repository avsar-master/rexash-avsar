/*
cl_game.c - client dll interaction
Copyright (C) 2008 Uncle Mike

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef XASH_DEDICATED

#include "common.h"
#include "client.h"
#include "const.h"
#include "triangleapi.h"
#include "r_efx.h"
#include "demo_api.h"
#include "ivoicetweak.h"
#include "pm_local.h"
#include "cl_tent.h"
#include "input.h"
#include "shake.h"
#include "sprite.h"
#include "gl_local.h"
#include "library.h"
#include "vgui_draw.h"
#include "sound.h"		// SND_STOP_LOOPING
#include "cdll_exp_api.h"

char			cl_textbuffer[MAX_TEXTCHANNELS][512];
client_textmessage_t	cl_textmessage[MAX_TEXTCHANNELS];

static dllfunc_t cdll_exports[] =
{
{ "Initialize", (void **)&clgame.dllFuncs.pfnInitialize },
{ "HUD_VidInit", (void **)&clgame.dllFuncs.pfnVidInit },
{ "HUD_Init", (void **)&clgame.dllFuncs.pfnInit },
{ "HUD_Shutdown", (void **)&clgame.dllFuncs.pfnShutdown },
{ "HUD_Redraw", (void **)&clgame.dllFuncs.pfnRedraw },
{ "HUD_UpdateClientData", (void **)&clgame.dllFuncs.pfnUpdateClientData },
{ "HUD_Reset", (void **)&clgame.dllFuncs.pfnReset },
{ "HUD_PlayerMove", (void **)&clgame.dllFuncs.pfnPlayerMove },
{ "HUD_PlayerMoveInit", (void **)&clgame.dllFuncs.pfnPlayerMoveInit },
{ "HUD_PlayerMoveTexture", (void **)&clgame.dllFuncs.pfnPlayerMoveTexture },
{ "HUD_ConnectionlessPacket", (void **)&clgame.dllFuncs.pfnConnectionlessPacket },
{ "HUD_GetHullBounds", (void **)&clgame.dllFuncs.pfnGetHullBounds },
{ "HUD_Frame", (void **)&clgame.dllFuncs.pfnFrame },
{ "HUD_PostRunCmd", (void **)&clgame.dllFuncs.pfnPostRunCmd },
{ "HUD_Key_Event", (void **)&clgame.dllFuncs.pfnKey_Event },
{ "HUD_AddEntity", (void **)&clgame.dllFuncs.pfnAddEntity },
{ "HUD_CreateEntities", (void **)&clgame.dllFuncs.pfnCreateEntities },
{ "HUD_StudioEvent", (void **)&clgame.dllFuncs.pfnStudioEvent },
{ "HUD_TxferLocalOverrides", (void **)&clgame.dllFuncs.pfnTxferLocalOverrides },
{ "HUD_ProcessPlayerState", (void **)&clgame.dllFuncs.pfnProcessPlayerState },
{ "HUD_TxferPredictionData", (void **)&clgame.dllFuncs.pfnTxferPredictionData },
{ "HUD_TempEntUpdate", (void **)&clgame.dllFuncs.pfnTempEntUpdate },
{ "HUD_DrawNormalTriangles", (void **)&clgame.dllFuncs.pfnDrawNormalTriangles },
{ "HUD_DrawTransparentTriangles", (void **)&clgame.dllFuncs.pfnDrawTransparentTriangles },
{ "HUD_GetUserEntity", (void **)&clgame.dllFuncs.pfnGetUserEntity },
{ "Demo_ReadBuffer", (void **)&clgame.dllFuncs.pfnDemo_ReadBuffer },
{ "CAM_Think", (void **)&clgame.dllFuncs.CAM_Think },
{ "CL_IsThirdPerson", (void **)&clgame.dllFuncs.CL_IsThirdPerson },
{ "CL_CameraOffset", (void **)&clgame.dllFuncs.CL_CameraOffset },
{ "CL_CreateMove", (void **)&clgame.dllFuncs.CL_CreateMove },
{ "IN_ActivateMouse", (void **)&clgame.dllFuncs.IN_ActivateMouse },
{ "IN_DeactivateMouse", (void **)&clgame.dllFuncs.IN_DeactivateMouse },
{ "IN_MouseEvent", (void **)&clgame.dllFuncs.IN_MouseEvent },
{ "IN_Accumulate", (void **)&clgame.dllFuncs.IN_Accumulate },
{ "IN_ClearStates", (void **)&clgame.dllFuncs.IN_ClearStates },
{ "V_CalcRefdef", (void **)&clgame.dllFuncs.pfnCalcRefdef },
{ "KB_Find", (void **)&clgame.dllFuncs.KB_Find },
{ NULL, NULL }
};

// optional exports
static dllfunc_t cdll_new_exports[] = 	// allowed only in SDK 2.3 and higher
{
{ "HUD_GetStudioModelInterface", (void **)&clgame.dllFuncs.pfnGetStudioModelInterface },
{ "HUD_DirectorMessage", (void **)&clgame.dllFuncs.pfnDirectorMessage },
{ "HUD_VoiceStatus", (void **)&clgame.dllFuncs.pfnVoiceStatus },
{ "HUD_ChatInputPosition", (void **)&clgame.dllFuncs.pfnChatInputPosition },
{ "HUD_GetPlayerTeam", (void **)&clgame.dllFuncs.pfnGetPlayerTeam },
{ NULL, NULL }
};

/*
====================
CL_GetServerTime

don't clamped time that come from server
====================
*/
float CL_GetServerTime( void )
{
	return cl.mtime[0];
}

/*
====================
CL_GetLerpFrac

returns current lerp fraction
====================
*/
float CL_GetLerpFrac( void )
{
	return cl.lerpFrac;
}

/*
====================
CL_IsThirdPerson

returns true if thirdperson is enabled
====================
*/
qboolean CL_IsThirdPerson( void )
{
	return cl.thirdperson;
}

/*
====================
CL_GetPlayerInfo

get player info by render request
====================
*/
player_info_t *CL_GetPlayerInfo( int playerIndex )
{
	if( playerIndex < 0 || playerIndex >= cl.maxclients )
		return NULL;

	return &cl.players[playerIndex];
}

/*
====================
CL_CreatePlaylist

Create a default valve playlist
====================
*/
void CL_CreatePlaylist( const char *filename )
{
	file_t	*f;

	f = FS_Open( filename, "w", false );
	if( !f ) return;

	// make standard cdaudio playlist
	FS_Print( f, "blank\n" );		// #1
	FS_Print( f, "Half-Life01.mp3\n" );	// #2
	FS_Print( f, "Prospero01.mp3\n" );	// #3
	FS_Print( f, "Half-Life12.mp3\n" );	// #4
	FS_Print( f, "Half-Life07.mp3\n" );	// #5
	FS_Print( f, "Half-Life10.mp3\n" );	// #6
	FS_Print( f, "Suspense01.mp3\n" );	// #7
	FS_Print( f, "Suspense03.mp3\n" );	// #8
	FS_Print( f, "Half-Life09.mp3\n" );	// #9
	FS_Print( f, "Half-Life02.mp3\n" );	// #10
	FS_Print( f, "Half-Life13.mp3\n" );	// #11
	FS_Print( f, "Half-Life04.mp3\n" );	// #12
	FS_Print( f, "Half-Life15.mp3\n" );	// #13
	FS_Print( f, "Half-Life14.mp3\n" );	// #14
	FS_Print( f, "Half-Life16.mp3\n" );	// #15
	FS_Print( f, "Suspense02.mp3\n" );	// #16
	FS_Print( f, "Half-Life03.mp3\n" );	// #17
	FS_Print( f, "Half-Life08.mp3\n" );	// #18
	FS_Print( f, "Prospero02.mp3\n" );	// #19
	FS_Print( f, "Half-Life05.mp3\n" );	// #20
	FS_Print( f, "Prospero04.mp3\n" );	// #21
	FS_Print( f, "Half-Life11.mp3\n" );	// #22
	FS_Print( f, "Half-Life06.mp3\n" );	// #23
	FS_Print( f, "Prospero03.mp3\n" );	// #24
	FS_Print( f, "Half-Life17.mp3\n" );	// #25
	FS_Print( f, "Prospero05.mp3\n" );	// #26
	FS_Print( f, "Suspense05.mp3\n" );	// #27
	FS_Print( f, "Suspense07.mp3\n" );	// #28
	FS_Close( f );
}

/*
====================
CL_InitCDAudio

Initialize CD playlist
====================
*/
void CL_InitCDAudio( const char *filename )
{
	char	*afile, *pfile;
	string	token;
	int	c = 0;

	if( !FS_FileExists( filename, false ))
	{
		// create a default playlist
		CL_CreatePlaylist( filename );
	}

	afile = (char *)FS_LoadFile( filename, NULL, false );
	if( !afile ) return;

	pfile = afile;

	// format: trackname\n [num]
	while(( pfile = COM_ParseFile( pfile, token )) != NULL )
	{
		if( !Q_stricmp( token, "blank" )) token[0] = '\0';
		Q_strncpy( clgame.cdtracks[c], token, sizeof( clgame.cdtracks[0] ));

		if( ++c > MAX_CDTRACKS - 1 )
		{
			MsgDev( D_WARN, "CD_Init: too many tracks %i in %s (only %d allowed)\n", c, filename, MAX_CDTRACKS );
			break;
		}
	}

	Mem_Free( afile );
}

/*
====================
CL_PointContents

Return contents for point
====================
*/
int CL_PointContents( const vec3_t p )
{
	int cont = CL_TruePointContents( p );

	if( cont <= CONTENTS_CURRENT_0 && cont >= CONTENTS_CURRENT_DOWN )
		cont = CONTENTS_WATER;
	return cont;
}

/*
====================
StudioEvent

Event callback for studio models
====================
*/
void CL_StudioEvent( struct mstudioevent_s *event, cl_entity_t *pEdict )
{
	clgame.dllFuncs.pfnStudioEvent( event, pEdict );
}

/*
=============
CL_AdjustXPos

adjust text by x pos
=============
*/
static int CL_AdjustXPos( float x, int width, int totalWidth )
{
	int	xPos;
	float scale;

	scale = scr_width->value / (float)clgame.scrInfo.iWidth;

	if( x == -1 )
	{
		xPos = ( clgame.scrInfo.iWidth - width ) * 0.5f;
	}
	else
	{
		if ( x < 0 )
			xPos = (1.0f + x) * clgame.scrInfo.iWidth - totalWidth;	// Alight right
		else // align left
			xPos = x * clgame.scrInfo.iWidth;
	}

	if( xPos + width > clgame.scrInfo.iWidth )
		xPos = clgame.scrInfo.iWidth - width;
	else if( xPos < 0 )
		xPos = 0;

	return xPos * scale;
}

/*
=============
CL_AdjustYPos

adjust text by y pos
=============
*/
static int CL_AdjustYPos( float y, int height )
{
	int	yPos;
	float scale;

	scale = scr_height->value / (float)clgame.scrInfo.iHeight;

	if( y == -1 ) // centered?
	{
		yPos = ( clgame.scrInfo.iHeight - height ) * 0.5f;
	}
	else
	{
		// Alight bottom?
		if( y < 0 )
			yPos = (1.0f + y) * clgame.scrInfo.iHeight - height; // Alight bottom
		else // align top
			yPos = y * clgame.scrInfo.iHeight;
	}

	if( yPos + height > clgame.scrInfo.iHeight )
		yPos = clgame.scrInfo.iHeight - height;
	else if( yPos < 0 )
		yPos = 0;

	return yPos * scale;
}

/*
=============
CL_CenterPrint

print centerscreen message
=============
*/
void CL_CenterPrint( const char *text, float y )
{
	byte	*s;
	int	width = 0;
	int	length = 0;
	float yscale = 1;

	clgame.centerPrint.lines = 1;
	clgame.centerPrint.totalWidth = 0;
	clgame.centerPrint.time = cl.mtime[0]; // allow pause for centerprint
	Q_strncpy( clgame.centerPrint.message, text, sizeof( clgame.centerPrint.message ));
	s = (byte*)clgame.centerPrint.message[0];

	// count the number of lines for centering
	while( *s )
	{
		if( *s == '\n' )
		{
			clgame.centerPrint.lines++;
			if( width > clgame.centerPrint.totalWidth )
				clgame.centerPrint.totalWidth = width;
			width = 0;
		}
		else width += clgame.scrInfo.charWidths[*s] * yscale;
		s++;
		length++;
	}

	clgame.centerPrint.totalHeight = ( clgame.centerPrint.lines * clgame.scrInfo.iCharHeight ); 
	clgame.centerPrint.y = CL_AdjustYPos( y, clgame.centerPrint.totalHeight );
}

/*
====================
SPR_AdjustSize

draw hudsprite routine
====================
*/
void SPR_AdjustSize( float *x, float *y, float *w, float *h )
{
	float	xscale, yscale;

	ASSERT( x || y || w || h );

	// scale for screen sizes
	xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	yscale = scr_height->value / (float)clgame.scrInfo.iHeight;

	if( x ) *x *= xscale;
	if( y ) *y *= yscale;

	if( w ) *w *= xscale;
	if( h ) *h *= yscale;
}

/*
====================
TextAdjustSize

draw hudsprite routine
====================
*/
void TextAdjustSize( int *x, int *y, int *w, int *h )
{
	float	xscale, yscale;

	ASSERT( x || y || w || h );

	if( !clgame.ds.adjust_size ) return;

	// scale for screen sizes
	xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	yscale = scr_height->value / (float)clgame.scrInfo.iHeight;

	if( x ) *x *= xscale;
	if( y ) *y *= yscale;
	if( w ) *w *= xscale;
	if( h ) *h *= yscale;
}

void TextAdjustSizeReverse(int* x, int* y, int* w, int* h)
{
	float	xscale, yscale;

	ASSERT(x || y || w || h);

	if (!clgame.ds.adjust_size) return;

	// scale for screen sizes
	xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	yscale = scr_height->value / (float)clgame.scrInfo.iHeight;

	if (x) *x /= xscale;
	if (y) *y /= yscale;
	if (w) *w /= xscale;
	if (h) *h /= yscale;
}

/*
====================
PictAdjustSize

draw hudsprite routine
====================
*/
void PicAdjustSize( float *x, float *y, float *w, float *h )
{
	float	xscale, yscale;

	if( !clgame.ds.adjust_size ) return;
	if( !x && !y && !w && !h ) return;

	// scale for screen sizes
	xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	yscale = scr_height->value / (float)clgame.scrInfo.iHeight;

	if( x ) *x *= xscale;
	if( y ) *y *= yscale;
	if( w ) *w *= xscale;
	if( h ) *h *= yscale;
}

/*
=============
CL_DrawCenterPrint

called each frame
=============
*/
void CL_DrawCenterPrint( void )
{
	char	*pText;
	int	i, j, x, y;
	int	width, lineLength;
	byte	*colorDefault, line[MAX_LINELENGTH];
	int	charWidth, charHeight;

	if( !clgame.centerPrint.time )
		return;

	if(( cl.time - clgame.centerPrint.time ) >= scr_centertime->value )
	{
		// time expired
		clgame.centerPrint.time = 0.0f;
		return;
	}

	y = clgame.centerPrint.y; // start y
	colorDefault = g_color_table[7];
	pText = clgame.centerPrint.message;
	Con_DrawCharacterLen( 0, NULL, &charHeight );
	
	for( i = 0; i < clgame.centerPrint.lines; i++ )
	{
		lineLength = 0;
		width = 0;

		while( *pText && *pText != '\n' && lineLength < MAX_LINELENGTH )
		{
			byte c = *pText;
			line[lineLength] = c;
			Con_DrawCharacterLen( c, &charWidth, NULL );
			width += charWidth;
			lineLength++;
			pText++;
		}

		if( lineLength == MAX_LINELENGTH )
			lineLength--;

		pText++; // Skip LineFeed
		line[lineLength] = 0;

		x = CL_AdjustXPos( -1, width, clgame.centerPrint.totalWidth );

		for( j = 0; j < lineLength; j++ )
		{
			if( x >= 0 && y >= 0 && x <= clgame.scrInfo.iWidth )
				x += Con_DrawCharacter( x, y, line[j], colorDefault );
		}
		y += charHeight;
	}
}

/*
=============
CL_DrawScreenFade

fill screen with specfied color
can be modulated
=============
*/
void CL_DrawScreenFade( void )
{
	screenfade_t	*sf = &clgame.fade;
	int		iFadeAlpha, testFlags;

	// keep pushing reset time out indefinitely
	if( sf->fadeFlags & FFADE_STAYOUT )
		sf->fadeReset = cl.time + 0.1f;
		
	if( sf->fadeReset == 0.0f && sf->fadeEnd == 0.0f )
		return;	// inactive

	// all done?
	if(( cl.time > sf->fadeReset ) && ( cl.time > sf->fadeEnd ))
	{
		Q_memset( &clgame.fade, 0, sizeof( clgame.fade ));
		return;
	}

	testFlags = (sf->fadeFlags & ~FFADE_MODULATE);

	// fading...
	if( testFlags == FFADE_STAYOUT )
	{
		iFadeAlpha = sf->fadealpha;
	}
	else
	{
		iFadeAlpha = sf->fadeSpeed * ( sf->fadeEnd - cl.time );
		if( sf->fadeFlags & FFADE_OUT ) iFadeAlpha += sf->fadealpha;
		iFadeAlpha = bound( 0, iFadeAlpha, sf->fadealpha );
	}

	pglColor4ub( sf->fader, sf->fadeg, sf->fadeb, iFadeAlpha );

	if( sf->fadeFlags & FFADE_MODULATE )
		GL_SetRenderMode( kRenderTransAdd );
	else GL_SetRenderMode( kRenderTransTexture );
	R_DrawStretchPic( 0, 0, scr_width->integer, scr_height->integer, 0, 0, 1, 1, cls.fillImage );
	pglColor4ub( 255, 255, 255, 255 );
}

/*
=============
pfnPlayerTraceExt

=============
*/
void CL_PlayerTraceExt( float *start, float *end, int traceFlags, int (*pfnIgnore)( physent_t *pe ), pmtrace_t *tr )
{
	if( !tr ) return;
	*tr = PM_PlayerTraceExt( clgame.pmove, start, end, traceFlags, clgame.pmove->numphysent, clgame.pmove->physents, -1, pfnIgnore );
	//clgame.pmove->usehull = clgame.old_trace_hull;	// restore old trace hull
}

/*
=============
pfnSetTraceHull

=============
*/
void CL_SetTraceHull( int hull )
{
	//clgame.old_trace_hull = clgame.pmove->usehull;
	clgame.pmove->usehull = bound( 0, hull, 3 );
}

/*
=============
CL_FindModelIndex

=============
*/
int CL_FindModelIndex( const char *m )
{
	int	i;

	if( !m || !m[0] )
		return 0;

	for( i = 1; i < MAX_MODELS && cl.model_precache[i][0]; i++ )
	{
		if( !Q_stricmp( cl.model_precache[i], m ))
			return i;
	}

	if( cls.state == ca_active && Q_strnicmp( m, "models/player/", 14 ))
	{
		// tell user about problem (but don't spam console about playermodel)
		MsgDev( D_NOTE, "CL_ModelIndex: %s not precached\n", m );
	}
	return 0;
}

/*
=============
pfnTextMessageGet

returns specified message from titles.txt
=============
*/
client_textmessage_t* CL_TextMessageGet( const char *pName )
{
	int	i;

	// first check internal messages
	for( i = 0; i < MAX_TEXTCHANNELS; i++ )
	{
		if( !Q_strcmp( pName, va( TEXT_MSGNAME, i )))
			return cl_textmessage + i;
	}

	// find desired message
	for( i = 0; i < clgame.numTitles; i++ )
	{
		if( !Q_stricmp( pName, clgame.titles[i].pName ))
			return clgame.titles + i;
	}
	return NULL; // found nothing
}

/*
====================
CL_InitTitles

parse all messages that declared in titles.txt
and hold them into permament memory pool 
====================
*/
static void CL_InitTitles( const char *filename )
{
	fs_offset_t	fileSize;
	byte	*pMemFile;
	int	i;

	// initialize text messages (game_text)
	for( i = 0; i < MAX_TEXTCHANNELS; i++ )
	{
		cl_textmessage[i].pName = _copystring( clgame.mempool, va( TEXT_MSGNAME, i ), __FILE__, __LINE__ );
		cl_textmessage[i].pMessage = cl_textbuffer[i];
	}

	// clear out any old data that's sitting around.
	if( clgame.titles ) Mem_Free( clgame.titles );

	clgame.titles = NULL;
	clgame.numTitles = 0;

	pMemFile = FS_LoadFile( filename, &fileSize, false );
	if( !pMemFile ) return;

	CL_TextMessageParse( pMemFile, (int)fileSize );
	Mem_Free( pMemFile );
}

/*
=============
pfnPlayerTrace

=============
*/
void CL_PlayerTrace( float *start, float *end, int traceFlags, int ignore_pe, pmtrace_t *tr )
{
	if( !tr ) return;
	*tr = PM_PlayerTraceExt( clgame.pmove, start, end, traceFlags, clgame.pmove->numphysent, clgame.pmove->physents, ignore_pe, NULL );
	//clgame.pmove->usehull = clgame.old_trace_hull;	// restore old trace hull
}

/*
====================
CL_ParseTextMessage

Parse TE_TEXTMESSAGE
====================
*/
void CL_ParseTextMessage( sizebuf_t *msg )
{
	static int		msgindex = 0;
	client_textmessage_t	*text;
	int			channel;

	// read channel ( 0 - auto)
	channel = BF_ReadByte( msg );

	if( channel <= 0 || channel > ( MAX_TEXTCHANNELS - 1 ))
	{
		// invalid channel specified, use internal counter		
		if( channel != 0 ) MsgDev( D_ERROR, "HudText: invalid channel %i\n", channel );
		channel = msgindex;
		msgindex = (msgindex + 1) & (MAX_TEXTCHANNELS - 1);
	}	

	// grab message channel
	text = &cl_textmessage[channel];

	text->x = (float)(BF_ReadShort( msg ) / 8192.0f);
	text->y = (float)(BF_ReadShort( msg ) / 8192.0f);
	text->effect = BF_ReadByte( msg );
	text->r1 = BF_ReadByte( msg );
	text->g1 = BF_ReadByte( msg );
	text->b1 = BF_ReadByte( msg );
	text->a1 = BF_ReadByte( msg );
	text->r2 = BF_ReadByte( msg );
	text->g2 = BF_ReadByte( msg );
	text->b2 = BF_ReadByte( msg );
	text->a2 = BF_ReadByte( msg );
	text->fadein = (float)(BF_ReadShort( msg ) / 256.0f );
	text->fadeout = (float)(BF_ReadShort( msg ) / 256.0f );
	text->holdtime = (float)(BF_ReadShort( msg ) / 256.0f );

	if( text->effect == 2 )
		text->fxtime = (float)(BF_ReadShort( msg ) / 256.0f );
	else text->fxtime = 0.0f;

	// to prevent grab too long messages
	Q_strncpy( (char *)text->pMessage, BF_ReadString( msg ), 512 ); 		

	// NOTE: a "HudText" message contain only 'string' with message name, so we
	// don't needs to use MSG_ routines here, just directly write msgname into netbuffer
	CL_DispatchUserMessage( "HudText", Q_strlen( text->pName ) + 1, (void *)text->pName );
}

static qboolean SPR_Scissor( float *x, float *y, float *width, float *height, float *u0, float *v0, float *u1, float *v1 )
{
	float	dudx, dvdy;

	// clip sub rect to sprite
	if(( width == 0 ) || ( height == 0 ))
		return false;

	if( *x + *width <= clgame.ds.scissor_x )
		return false;
	if( *x >= clgame.ds.scissor_x + clgame.ds.scissor_width )
		return false;
	if( *y + *height <= clgame.ds.scissor_y )
		return false;
	if( *y >= clgame.ds.scissor_y + clgame.ds.scissor_height )
		return false;

	dudx = (*u1 - *u0) / *width;
	dvdy = (*v1 - *v0) / *height;

	if( *x < clgame.ds.scissor_x )
	{
		*u0 += (clgame.ds.scissor_x - *x) * dudx;
		*width -= clgame.ds.scissor_x - *x;
		*x = clgame.ds.scissor_x;
	}

	if( *x + *width > clgame.ds.scissor_x + clgame.ds.scissor_width )
	{
		*u1 -= (*x + *width - (clgame.ds.scissor_x + clgame.ds.scissor_width)) * dudx;
		*width = clgame.ds.scissor_x + clgame.ds.scissor_width - *x;
	}

	if( *y < clgame.ds.scissor_y )
	{
		*v0 += (clgame.ds.scissor_y - *y) * dvdy;
		*height -= clgame.ds.scissor_y - *y;
		*y = clgame.ds.scissor_y;
	}

	if( *y + *height > clgame.ds.scissor_y + clgame.ds.scissor_height )
	{
		*v1 -= (*y + *height - (clgame.ds.scissor_y + clgame.ds.scissor_height)) * dvdy;
		*height = clgame.ds.scissor_y + clgame.ds.scissor_height - *y;
	}

	return true;
}

/*
====================
SPR_DrawGeneric

draw hudsprite routine
====================
*/
void SPR_DrawGeneric( int frame, float x, float y, float width, float height, const wrect_t *prc )
{
	float	s1, s2, t1, t2;
	int	texnum;

	if( width == -1 && height == -1 )
	{
		int	w, h;

		// assume we get sizes from image
		R_GetSpriteParms( &w, &h, NULL, frame, clgame.ds.pSprite );

		width = w;
		height = h;
	}

	if( prc )
	{
		wrect_t	rc;

		rc = *prc;

		// Sigh! some stupid modmakers set wrong rectangles in hud.txt 
		if( rc.left <= 0 || rc.left >= width ) rc.left = 0;
		if( rc.top <= 0 || rc.top >= height ) rc.top = 0;
		if( rc.right <= 0 || rc.right > width ) rc.right = width;
		if( rc.bottom <= 0 || rc.bottom > height ) rc.bottom = height;

		// calc user-defined rectangle
		s1 = (float)rc.left / width;
		t1 = (float)rc.top / height;
		s2 = (float)rc.right / width;
		t2 = (float)rc.bottom / height;
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
	}
	else
	{
		s1 = t1 = 0.0f;
		s2 = t2 = 1.0f;
	}

	// pass scissor test if supposed
	if( clgame.ds.scissor_test && !SPR_Scissor( &x, &y, &width, &height, &s1, &t1, &s2, &t2 ))
		return;

	// scale for screen sizes
	SPR_AdjustSize( &x, &y, &width, &height );
	texnum = R_GetSpriteTexture( clgame.ds.pSprite, frame );
	pglColor4ubv( clgame.ds.spriteColor );
	R_DrawStretchPic( x, y, width, height, s1, t1, s2, t2, texnum );
}

/*
=========
SPR_DisableScissor

=========
*/
static void SPR_DisableScissor( void )
{
	clgame.ds.scissor_x = 0;
	clgame.ds.scissor_width = 0;
	clgame.ds.scissor_y = 0;
	clgame.ds.scissor_height = 0;
	clgame.ds.scissor_test = false;
}

/*
====================
CL_DrawCrosshair

Render crosshair
====================
*/
void CL_DrawCrosshair( void )
{
	int		x, y, width, height;
	cl_entity_t	*pPlayer;

	if( !crosshair_state.pCrosshair || cl.refdef.crosshairangle[2] || !cl_crosshair->integer )
		return;

	pPlayer = CL_GetLocalPlayer();

	if( cl.frame.client.deadflag != DEAD_NO || cl.frame.client.flags & FL_FROZEN )
		return;

	// any camera on
	if( cl.refdef.viewentity != pPlayer->index )
		return;

	// get crosshair dimension
	width = crosshair_state.rcCrosshair.right - crosshair_state.rcCrosshair.left;
	height = crosshair_state.rcCrosshair.bottom - crosshair_state.rcCrosshair.top;

	x = clgame.scrInfo.iWidth / 2; 
	y = clgame.scrInfo.iHeight / 2;

	// g-cont - cl.refdef.crosshairangle is the autoaim angle.
	// if we're not using autoaim, just draw in the middle of the screen
	if( !VectorIsNull( cl.refdef.crosshairangle ))
	{
		vec3_t	angles;
		vec3_t	forward;
		vec3_t	point, screen;

		VectorAdd( cl.refdef.viewangles, cl.refdef.crosshairangle, angles );
		AngleVectors( angles, forward, NULL, NULL );
		VectorAdd( cl.refdef.vieworg, forward, point );
		R_WorldToScreen( point, screen );

		x += 0.5f * screen[0] * scr_width->value + 0.5f;
		y += 0.5f * screen[1] * scr_height->value + 0.5f;
	}

	clgame.ds.pSprite = crosshair_state.pCrosshair;

	GL_SetRenderMode( kRenderTransTexture );
	*(int *)clgame.ds.spriteColor = *(int *)crosshair_state.rgbaCrosshair;

	SPR_EnableScissor( x - 0.5f * width, y - 0.5f * height, width, height );
	SPR_DrawGeneric( 0, x - 0.5f * width, y - 0.5f * height, -1, -1, &crosshair_state.rcCrosshair );
	SPR_DisableScissor();
}

/*
=============
CL_DrawLoading

draw loading progress bar
=============
*/
static void CL_DrawLoading( float percent )
{
	int	x, y, width, height, right;
	float	xscale, yscale, step, s2;

	R_GetTextureParms( &width, &height, cls.loadingBar );
	x = ( clgame.scrInfo.iWidth - width ) >> 1;
	y = ( clgame.scrInfo.iHeight - height) >> 1;

	xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	yscale = scr_height->value / (float)clgame.scrInfo.iHeight;

	x *= xscale;
	y *= yscale;
	width *= xscale;
	height *= yscale;

	if( cl_allow_levelshots->integer )
	{
		pglColor4ub( 128, 128, 128, 255 );
		GL_SetRenderMode( kRenderTransTexture );
		R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cls.loadingBar );

		step = (float)width / 100.0f;
		right = (int)ceil( percent * step );
		s2 = (float)right / width;
		width = right;
	
		pglColor4ub( 208, 152, 0, 255 );
		GL_SetRenderMode( kRenderTransTexture );
		R_DrawStretchPic( x, y, width, height, 0, 0, s2, 1, cls.loadingBar );
		pglColor4ub( 255, 255, 255, 255 );
	}
	else
	{
		pglColor4ub( 255, 255, 255, 255 );
		GL_SetRenderMode( kRenderTransTexture );
		R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cls.loadingBar );
	}
}

/*
=============
CL_DrawPause

draw pause sign
=============
*/
static void CL_DrawPause( void )
{
	int	x, y, width, height;
	float	xscale, yscale;

	R_GetTextureParms( &width, &height, cls.pauseIcon );
	x = ( clgame.scrInfo.iWidth - width ) >> 1;
	y = ( clgame.scrInfo.iHeight - height) >> 1;

	xscale = scr_width->value / (float)clgame.scrInfo.iWidth;
	yscale = scr_height->value / (float)clgame.scrInfo.iHeight;

	x *= xscale;
	y *= yscale;
	width *= xscale;
	height *= yscale;

	pglColor4ub( 255, 255, 255, 255 );
	GL_SetRenderMode( kRenderTransTexture );
	R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cls.pauseIcon );
}

void CL_DrawHUD( int state )
{
	if( state == CL_ACTIVE && !cl.video_prepped )
		state = CL_LOADING;

	if( state == CL_ACTIVE && cl.refdef.paused )
		state = CL_PAUSED;

	switch( state )
	{
	case CL_ACTIVE:
		CL_DrawScreenFade ();
		CL_DrawCrosshair ();
		CL_DrawCenterPrint ();
		clgame.dllFuncs.pfnRedraw( cl.time, cl.refdef.intermission );
		break;
	case CL_PAUSED:
		CL_DrawScreenFade ();
		CL_DrawCrosshair ();
		CL_DrawCenterPrint ();
		clgame.dllFuncs.pfnRedraw( cl.time, cl.refdef.intermission );
		CL_DrawPause();
		break;
	case CL_LOADING:
		CL_DrawLoading( scr_loading->value );
		break;
	case CL_CHANGELEVEL:
		if( cls.draw_changelevel )
		{
			CL_DrawLoading( 100.0f );
			cls.draw_changelevel = false;
		}
		break;
	}
}

static void CL_ClearUserMessage( char *pszName, int svc_num )
{
	int i;

	for( i = 0; i < MAX_USER_MESSAGES && clgame.msg[i].name[0]; i++ )
		if( ( clgame.msg[i].number == svc_num ) && Q_strcmp( clgame.msg[i].name, pszName ) )
			clgame.msg[i].number = 0;
}

void CL_LinkUserMessage( char *pszName, const int svc_num, int iSize )
{
	int	i;

	if( !pszName || !*pszName )
		Host_Error( "CL_LinkUserMessage: bad message name\n" );

	if( svc_num < svc_lastmsg )
		Host_Error( "CL_LinkUserMessage: tried to hook a system message \"%s\"\n", svc_strings[svc_num] );	

	// see if already hooked
	for( i = 0; i < MAX_USER_MESSAGES && clgame.msg[i].name[0]; i++ )
	{
		// NOTE: no check for DispatchFunc, check only name
		if( !Q_strcmp( clgame.msg[i].name, pszName ))
		{
			clgame.msg[i].number = svc_num;
			clgame.msg[i].size = iSize;
			CL_ClearUserMessage( pszName, svc_num );
			return;
		}
	}

	if( i == MAX_USER_MESSAGES ) 
	{
		Host_Error( "CL_LinkUserMessage: MAX_USER_MESSAGES hit!\n" );
		return;
	}

	// register new message without DispatchFunc, so we should parse it properly
	Q_strncpy( clgame.msg[i].name, pszName, sizeof( clgame.msg[i].name ));
	clgame.msg[i].number = svc_num;
	clgame.msg[i].size = iSize;
	CL_ClearUserMessage( pszName, svc_num );
}

void CL_FreeEntity( cl_entity_t *pEdict )
{
	ASSERT( pEdict );
	R_RemoveEfrags( pEdict );
	CL_KillDeadBeams( pEdict );
}

void CL_ClearWorld( void )
{
	cl.world = clgame.entities;
	cl.world->curstate.modelindex = 1;	// world model
	cl.world->curstate.solid = SOLID_BSP;
	cl.world->curstate.movetype = MOVETYPE_PUSH;
	cl.world->model = cl.worldmodel;
	cl.world->index = 0;

	clgame.ds.cullMode = GL_FRONT;
	clgame.numStatics = 0;
}

void CL_InitEdicts( void )
{
	ASSERT( clgame.entities == NULL );
	if( !clgame.mempool )
		return; // Host_Error without client

	CL_UPDATE_BACKUP = ( cl.maxclients == 1 ) ? SINGLEPLAYER_BACKUP : MULTIPLAYER_BACKUP;
	cls.num_client_entities = CL_UPDATE_BACKUP * 64;
	cls.packet_entities = (entity_state_t*)Z_Realloc( cls.packet_entities, sizeof( entity_state_t ) * cls.num_client_entities );
	clgame.entities = (cl_entity_t*)Mem_Alloc( clgame.mempool, sizeof( cl_entity_t ) * clgame.maxEntities );
	clgame.static_entities = (cl_entity_t*)Mem_Alloc( clgame.mempool, sizeof( cl_entity_t ) * MAX_STATIC_ENTITIES );
	clgame.numStatics = 0;

	if(( clgame.maxRemapInfos - 1 ) != clgame.maxEntities )
	{
		CL_ClearAllRemaps (); // purge old remap info
		clgame.maxRemapInfos = clgame.maxEntities + 1; 
		clgame.remap_info = (remap_info_t **)Mem_Alloc( clgame.mempool, sizeof( remap_info_t* ) * clgame.maxRemapInfos );
	}
}

void CL_FreeEdicts( void )
{
	Z_Free( clgame.entities );
	clgame.entities = NULL;

	Z_Free( clgame.static_entities );
	clgame.static_entities = NULL;

	Z_Free( cls.packet_entities );
	cls.packet_entities = NULL;

	cls.num_client_entities = 0;
	cls.next_client_entities = 0;
	clgame.numStatics = 0;
}

void CL_ClearEdicts( void )
{
	if( clgame.entities != NULL )
		return;

	// in case we stopped with error
	clgame.maxEntities = 2;
	CL_InitEdicts();
}

/*
===============================================================================
	CGame Builtin Functions

===============================================================================
*/
qboolean CL_LoadHudSprite( const char *szSpriteName, model_t *m_pSprite, qboolean mapSprite, uint texFlags )
{
	byte	*buf;
	fs_offset_t	size;
	qboolean	loaded;

	ASSERT( m_pSprite != NULL );

	buf = FS_LoadFile( szSpriteName, &size, false );
	if( !buf ) return false;

	Q_strncpy( m_pSprite->name, szSpriteName, sizeof( m_pSprite->name ));
	m_pSprite->flags = 256; // it's hud sprite, make difference names to prevent free shared textures

	if( mapSprite ) Mod_LoadMapSprite( m_pSprite, buf, (size_t)size, &loaded );
	else Mod_LoadSpriteModel( m_pSprite, buf, &loaded, texFlags );		

	Mem_Free( buf );

	if( !loaded )
	{
		Mod_UnloadSpriteModel( m_pSprite );
		return false;
	}
	return true;
}

void CL_UnloadProgs( void )
{
	if( !clgame.hInstance ) return;

	CL_FreeEdicts();
	CL_FreeTempEnts();
	CL_FreeViewBeams();
	CL_FreeParticles();
	CL_ClearAllRemaps();
	Mod_ClearUserData();


	// NOTE: HLFX 0.5 has strange bug: hanging on exit if no map was loaded
	if( !( !Q_stricmp( GI->gamefolder, "hlfx" ) && GI->version == 0.5f ))
		clgame.dllFuncs.pfnShutdown();

	Cvar_FullSet( "cl_background", "0", CVAR_READ_ONLY );
	Cvar_FullSet( "host_clientloaded", "0", CVAR_INIT );

	Com_FreeLibrary( clgame.hInstance );
	VGui_Shutdown();
	Mem_FreePool( &cls.mempool );
	Mem_FreePool( &clgame.mempool );
	Q_memset( &clgame, 0, sizeof( clgame ));

	Cvar_Unlink();
	Cmd_Unlink( CMD_CLIENTDLL );
}

void Sequence_Init( void );

qboolean CL_LoadProgs( const char *name )
{
	static playermove_t		gpMove;
	const dllfunc_t		*func;
	CL_EXPORT_FUNCS		F; // export 'F'
	qboolean			critical_exports = true;

	if( clgame.hInstance ) CL_UnloadProgs();

	// setup globals
	cl.refdef.movevars = &clgame.movevars;

	// initialize PlayerMove
	clgame.pmove = &gpMove;

	cls.mempool = Mem_AllocPool( "Client Static Pool" );
	clgame.mempool = Mem_AllocPool( "Client Edicts Zone" );
	clgame.entities = NULL;

	// NOTE: important stuff!
	// vgui must startup BEFORE loading client.dll to avoid get error ERROR_NOACESS
	// during LoadLibrary
	VGui_Startup (menu.globals->scrWidth, menu.globals->scrHeight);
	
	clgame.hInstance = Com_LoadLibrary( name, false );
	if( !clgame.hInstance ) return false;

	// clear exports
	for( func = cdll_exports; func && func->name; func++ )
		*func->func = NULL;

	// trying to get single export named 'F'
	if(( *(void**)&F = Com_GetProcAddress( clgame.hInstance, "F" )) != NULL )
	{
		MsgDev( D_NOTE, "CL_LoadProgs: found single callback export\n" );		

		// trying to fill interface now
		F( &clgame.dllFuncs );

		// check critical functions again
		for( func = cdll_exports; func && func->name; func++ )
		{
			if( func->func == NULL )
				break; // BAH critical function was missed
		}

		// because all the exports are loaded through function 'F"
		if( !func || !func->name )
			critical_exports = false;
	}

	for( func = cdll_exports; func && func->name != NULL; func++ )
	{
		if( *func->func != NULL )
			continue;	// already get through 'F'

		// functions are cleared before all the extensions are evaluated
		if(!( *func->func = (void *)Com_GetProcAddress( clgame.hInstance, func->name )))
		{
			MsgDev( D_NOTE, "CL_LoadProgs: failed to get address of %s proc\n", func->name );

			if( critical_exports )
			{
				Com_FreeLibrary( clgame.hInstance );
				clgame.hInstance = NULL;
				return false;
			}
		}
	}

	// it may be loaded through 'F' so we don't need to clear them
	if( critical_exports )
	{
		// clear new exports
		for( func = cdll_new_exports; func && func->name; func++ )
			*func->func = NULL;
	}

	for( func = cdll_new_exports; func && func->name != NULL; func++ )
	{
		if( *func->func != NULL )
			continue;	// already get through 'F'

		// functions are cleared before all the extensions are evaluated
		// NOTE: new exports can be missed without stop the engine
		if(!( *func->func = (void *)Com_GetProcAddress( clgame.hInstance, func->name )))
			MsgDev( D_NOTE, "CL_LoadProgs: failed to get address of %s proc\n", func->name );
	}

	if( !clgame.dllFuncs.pfnInitialize( &cl_enginefuncs, CLDLL_INTERFACE_VERSION ))
	{
		Com_FreeLibrary( clgame.hInstance );
		MsgDev( D_NOTE, "CL_LoadProgs: can't init client API\n" );
		clgame.hInstance = NULL;
		return false;
	}

	Cvar_Get( "cl_nopred", "1", CVAR_ARCHIVE|CVAR_USERINFO, "disable client movement predicting" );
	cl_lw = Cvar_Get( "cl_lw", "0", CVAR_ARCHIVE|CVAR_USERINFO, "enable client weapon predicting" );
	Cvar_Get( "cl_lc", "0", CVAR_ARCHIVE|CVAR_USERINFO, "enable lag compensation" );
	Cvar_FullSet( "host_clientloaded", "1", CVAR_INIT );

	clgame.maxRemapInfos = 0; // will be alloc on first call CL_InitEdicts();
	clgame.maxEntities = 2; // world + localclient (have valid entities not in game)

	CL_InitCDAudio( "media/cdaudio.txt" );
	CL_InitTitles( "titles.txt" );
	CL_InitParticles ();
	CL_InitViewBeams ();
	CL_InitTempEnts ();
	CL_InitEdicts ();	// initailize local player and world
	CL_InitClientMove(); // initialize pm_shared

	Sequence_Init();

	// NOTE: some usermessages are handled into the engine
	pfnHookUserMsg( "ScreenFade", CL_ParseScreenFade );
	pfnHookUserMsg( "ScreenShake", CL_ParseScreenShake );

	// initialize game
	clgame.dllFuncs.pfnInit();

	CL_InitStudioAPI( );

	return true;
}
#endif // XASH_DEDICATED
