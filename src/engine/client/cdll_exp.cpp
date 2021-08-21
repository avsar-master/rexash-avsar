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

// cdll_exp.cpp

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
#include "r_triangle.h"
#include "eventapi.h"
#include "cl_demoapi.h"
#include "voice.h"
#include "cdll_exp_api.h"

#include "vgui/ISurface.h"
#include "vgui_controls/controls.h"

/*
=========
pfnSPR_LoadExt

=========
*/
HSPRITE pfnSPR_LoadExt( const char *szPicName, uint texFlags )
{
	char	name[64];
	int	i;

	if( !szPicName || !*szPicName )
	{
		MsgDev( D_ERROR, "CL_LoadSprite: bad name!\n" );
		return 0;
	}

	Q_strncpy( name, szPicName, sizeof( name ));
	COM_FixSlashes( name );

	// slot 0 isn't used
	for( i = 1; i < MAX_IMAGES; i++ )
	{
		if( !Q_stricmp( clgame.sprites[i].name, name ))
		{
			// prolonge registration
			clgame.sprites[i].needload = clgame.load_sequence;
			return i;
		}
	}

	// find a free model slot spot
	for( i = 1; i < MAX_IMAGES; i++ )
	{
		if( !clgame.sprites[i].name[0] )
			break; // this is a valid spot
	}

	if( i >= MAX_IMAGES ) 
	{
		MsgDev( D_ERROR, "SPR_Load: can't load %s, MAX_HSPRITES limit exceeded\n", szPicName );
		return 0;
	}

	// load new model
	if( CL_LoadHudSprite( name, &clgame.sprites[i], false, texFlags ))
	{
		if( i < MAX_IMAGES - 1 )
		{
			clgame.sprites[i].needload = clgame.load_sequence;
		}
		return i;
	}
	return 0;
}

/*
=========
pfnSPR_Load

=========
*/
HSPRITE pfnSPR_Load( const char *szPicName )
{
	int texFlags = TF_NOPICMIP;
	if( cl_sprite_nearest->integer )
		texFlags |= TF_NEAREST;

	return pfnSPR_LoadExt( szPicName, texFlags );
}

/*
=========
pfnSPR_Frames

=========
*/
int pfnSPR_Frames( HSPRITE hPic )
{
	int	numFrames;

	R_GetSpriteParms( NULL, NULL, &numFrames, 0, CL_GetSpritePointer( hPic ));

	return numFrames;
}

/*
=========
pfnSPR_Height

=========
*/
int pfnSPR_Height( HSPRITE hPic, int frame )
{
	int	sprHeight;

	R_GetSpriteParms( NULL, &sprHeight, NULL, frame, CL_GetSpritePointer( hPic ));

	return sprHeight;
}

/*
=========
pfnSPR_Width

=========
*/
int pfnSPR_Width( HSPRITE hPic, int frame )
{
	int	sprWidth;

	R_GetSpriteParms( &sprWidth, NULL, NULL, frame, CL_GetSpritePointer( hPic ));

	return sprWidth;
}

/*
=========
pfnSPR_Set

=========
*/
void pfnSPR_Set( HSPRITE hPic, int r, int g, int b )
{
	clgame.ds.pSprite = CL_GetSpritePointer( hPic );
	clgame.ds.spriteColor[0] = bound( 0, r, 255 );
	clgame.ds.spriteColor[1] = bound( 0, g, 255 );
	clgame.ds.spriteColor[2] = bound( 0, b, 255 );
	clgame.ds.spriteColor[3] = 255;

	// set default state
	pglDisable( GL_BLEND );
	pglDisable( GL_ALPHA_TEST );
	pglTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
}


/*
=========
pfnSPR_Draw

=========
*/
void pfnSPR_Draw( int frame, int x, int y, const wrect_t *prc )
{
	pglEnable( GL_ALPHA_TEST );
	SPR_DrawGeneric( frame, x, y, -1, -1, prc );
}

/*
=========
pfnSPR_DrawHoles

=========
*/
void pfnSPR_DrawHoles( int frame, int x, int y, const wrect_t *prc )
{
	GL_SetRenderMode( kRenderTransAlpha );
	SPR_DrawGeneric( frame, x, y, -1, -1, prc );
}

/*
=========
pfnSPR_DrawAdditive

=========
*/
void pfnSPR_DrawAdditive( int frame, int x, int y, const wrect_t *prc )
{
	GL_SetRenderMode( kRenderTransAdd );
	SPR_DrawGeneric( frame, x, y, -1, -1, prc );
}

/*
=========
SPR_EnableScissor

=========
*/
void SPR_EnableScissor( int x, int y, int width, int height )
{
	// check bounds
	x = bound( 0, x, clgame.scrInfo.iWidth );
	y = bound( 0, y, clgame.scrInfo.iHeight );
	width = bound( 0, width, clgame.scrInfo.iWidth - x );
	height = bound( 0, height, clgame.scrInfo.iHeight - y );

	clgame.ds.scissor_x = x;
	clgame.ds.scissor_width = width;
	clgame.ds.scissor_y = y;
	clgame.ds.scissor_height = height;
	clgame.ds.scissor_test = true;
}

/*
=========
SPR_DisableScissor

=========
*/
void SPR_DisableScissor( void )
{
	clgame.ds.scissor_x = 0;
	clgame.ds.scissor_width = 0;
	clgame.ds.scissor_y = 0;
	clgame.ds.scissor_height = 0;
	clgame.ds.scissor_test = false;
}


/*
=========
pfnSPR_GetList

for parsing half-life scripts - hud.txt etc
=========
*/
client_sprite_t* pfnSPR_GetList( const char *psz, int *piCount )
{
	client_sprite_t	*pList;
	int		index, numSprites = 0;
	char		*afile, *pfile;
	string		token;
	byte		*pool;

	if( piCount ) *piCount = 0;

	if( !clgame.itemspath[0] )	// typically it's sprites\*.txt
		FS_ExtractFilePath( psz, clgame.itemspath );

	afile = (char *)FS_LoadFile( psz, NULL, false );
	if( !afile ) return NULL;

	pfile = afile;
	pfile = COM_ParseFile( pfile, token );
	numSprites = Q_atoi( token );

	if( !cl.video_prepped ) pool = cls.mempool;	// static memory
	else pool = com_studiocache;			// temporary

	// name, res, pic, x, y, w, h
	// NOTE: we must use com_studiocache because it will be purge on next restart or change map
	pList = (client_sprite_t*)Mem_Alloc( pool, sizeof( client_sprite_t ) * numSprites );

	for( index = 0; index < numSprites; index++ )
	{
		if(( pfile = COM_ParseFile( pfile, token )) == NULL )
			break;

		Q_strncpy( pList[index].szName, token, sizeof( pList[index].szName ));

		// read resolution
		pfile = COM_ParseFile( pfile, token );
		pList[index].iRes = Q_atoi( token );

		// read spritename
		pfile = COM_ParseFile( pfile, token );
		Q_strncpy( pList[index].szSprite, token, sizeof( pList[index].szSprite ));

		// parse rectangle
		pfile = COM_ParseFile( pfile, token );
		pList[index].rc.left = Q_atoi( token );

		pfile = COM_ParseFile( pfile, token );
		pList[index].rc.top = Q_atoi( token );

		pfile = COM_ParseFile( pfile, token );
		pList[index].rc.right = pList[index].rc.left + Q_atoi( token );

		pfile = COM_ParseFile( pfile, token );
		pList[index].rc.bottom = pList[index].rc.top + Q_atoi( token );

		if( piCount ) (*piCount)++;
	}

	if( index < numSprites )
		MsgDev( D_WARN, "SPR_GetList: unexpected end of %s (%i should be %i)\n", psz, numSprites, index );

	Mem_Free( afile );

	return pList;
}


/*
=============
pfnFillRGBA

=============
*/
void CL_FillRGBA( int x, int y, int width, int height, int r, int g, int b, int a )
{
	float x1 = x, y1 = y, w1 = width, h1 = height;
	r = bound( 0, r, 255 );
	g = bound( 0, g, 255 );
	b = bound( 0, b, 255 );
	a = bound( 0, a, 255 );
	pglColor4ub( r, g, b, a );

	SPR_AdjustSize( &x1, &y1, &w1, &h1 );

	GL_SetRenderMode( kRenderTransAdd );
	R_DrawStretchPic( x1, y1, w1, h1, 0, 0, 1, 1, cls.fillImage );
	pglColor4ub( 255, 255, 255, 255 );
}


/*
=============
pfnGetScreenInfo

get actual screen info
=============
*/
int pfnGetScreenInfo( SCREENINFO *pscrinfo )
{
	// setup screen info
	float scale_factor = hud_scale->value;
	clgame.scrInfo.iSize = sizeof( clgame.scrInfo );

	
	if( scale_factor && scale_factor != 1.0f)
	{
		clgame.scrInfo.iWidth = scr_width->value / scale_factor;
		clgame.scrInfo.iHeight = scr_height->value / scale_factor;
		clgame.scrInfo.iFlags |= SCRINFO_STRETCHED;
	}
	else
	{
		clgame.scrInfo.iWidth = scr_width->integer;
		clgame.scrInfo.iHeight = scr_height->integer;
		clgame.scrInfo.iFlags &= ~SCRINFO_STRETCHED;
	}

	if( !pscrinfo ) return 0;

	if( pscrinfo->iSize != clgame.scrInfo.iSize )
		clgame.scrInfo.iSize = pscrinfo->iSize;

	// copy screeninfo out
	Q_memcpy( pscrinfo, &clgame.scrInfo, clgame.scrInfo.iSize );

	return 1;
}

/*
=============
pfnSetCrosshair

setup crosshair
=============
*/
void pfnSetCrosshair( HSPRITE hspr, wrect_t rc, int r, int g, int b )
{
	crosshair_state.rgbaCrosshair[0] = (byte)r;
	crosshair_state.rgbaCrosshair[1] = (byte)g;
	crosshair_state.rgbaCrosshair[2] = (byte)b;
	crosshair_state.rgbaCrosshair[3] = (byte)0xFF;
	crosshair_state.pCrosshair = CL_GetSpritePointer( hspr );
	crosshair_state.rcCrosshair = rc;
}

/*
=============
pfnAddClientCommand

=============
*/
int pfnAddClientCommand( const char *cmd_name, xcommand_t func )
{
	if( !cmd_name || !*cmd_name )
		return 0;

	// NOTE: if( func == NULL ) cmd will be forwarded to a server
	Cmd_AddClientCommand( cmd_name, func );

	return 1;
}

/*
=============
pfnHookUserMsg

=============
*/
int pfnHookUserMsg( const char *pszName, pfnUserMsgHook pfn )
{
	int	i;

	// ignore blank names or invalid callbacks
	if( !pszName || !*pszName || !pfn )
		return 0;	

	for( i = 0; i < MAX_USER_MESSAGES && clgame.msg[i].name[0]; i++ )
	{
		// see if already hooked
		if( !Q_strcmp( clgame.msg[i].name, pszName ))
			return 1;
	}

	if( i == MAX_USER_MESSAGES ) 
	{
		Host_Error( "HookUserMsg: MAX_USER_MESSAGES hit!\n" );
		return 0;
	}

	// hook new message
	Q_strncpy( clgame.msg[i].name, pszName, sizeof( clgame.msg[i].name ));
	clgame.msg[i].func = pfn;

	return 1;
}


/*
=============
pfnServerCmd

=============
*/
int pfnServerCmd( const char *szCmdString )
{
	string buf;

	if( !szCmdString || !szCmdString[0] )
		return 0;

	// just like the client typed "cmd xxxxx" at the console
	Q_snprintf( buf, sizeof( buf ) - 1, "cmd %s\n", szCmdString );
	Cbuf_AddText( buf );

	return 1;
}

/*
=============
pfnClientCmd

=============
*/
int pfnClientCmd( const char *szCmdString )
{
	if( !szCmdString || !szCmdString[0] )
		return 0;

	Cbuf_AddText( szCmdString );
	Cbuf_AddText( "\n" );
	return 1;
}

/*
=============
pfnGetPlayerInfo

=============
*/
void pfnGetPlayerInfo( int ent_num, hud_player_info_t *pinfo )
{
	player_info_t	*player;
	cl_entity_t	*ent;
	qboolean		spec = false;

	ent = CL_GetEntityByIndex( ent_num );
	ent_num -= 1; // player list if offset by 1 from ents

	if( ent_num >= cl.maxclients || ent_num < 0 || !cl.players[ent_num].name[0] )
	{
		Q_memset( pinfo, 0, sizeof( *pinfo ));
		return;
	}

	player = &cl.players[ent_num];
	pinfo->thisplayer = ( ent_num == cl.playernum ) ? true : false;
	if( ent ) spec = ent->curstate.spectator;

	pinfo->name = player->name;
	pinfo->model = player->model;

	pinfo->spectator = spec;		
	pinfo->ping = player->ping;
	pinfo->packetloss = player->packet_loss;
	pinfo->topcolor = Q_atoi( Info_ValueForKey( player->userinfo, "topcolor" ));
	pinfo->bottomcolor = Q_atoi( Info_ValueForKey( player->userinfo, "bottomcolor" ));
}


/*
=============
pfnPlaySoundByName

=============
*/
void pfnPlaySoundByName( const char *szSound, float volume )
{
	int hSound = S_RegisterSound( szSound );
	S_StartSound( NULL, cl.refdef.viewentity, CHAN_ITEM, hSound, volume, ATTN_NORM, PITCH_NORM, SND_STOP_LOOPING );
}

/*
=============
pfnPlaySoundByIndex

=============
*/
void pfnPlaySoundByIndex( int iSound, float volume )
{
	int hSound;

	// make sure what we in-bounds
	iSound = bound( 0, iSound, MAX_SOUNDS );
	hSound = cl.sound_index[iSound];

	if( !hSound )
	{
		MsgDev( D_ERROR, "CL_PlaySoundByIndex: invalid sound handle %i\n", iSound );
		return;
	}
	S_StartSound( NULL, cl.refdef.viewentity, CHAN_ITEM, hSound, volume, ATTN_NORM, PITCH_NORM, SND_STOP_LOOPING );
}

// AVSARTODO: change with vgui2
/*
=============
pfnDrawCharacter

returns drawed chachter width (in real screen pixels)
=============
*/
int pfnDrawCharacter( int x, int y, int number, int r, int g, int b )
{
	if( !cls.creditsFont.valid )
		return 0;

	number &= 255;
	
	if( hud_utf8->integer )
		number = Con_UtfProcessChar( number );

	if( number < 32 ) return 0;
	if( y < -clgame.scrInfo.iCharHeight )
		return 0;

	clgame.ds.adjust_size = true;
	pfnPIC_Set( cls.creditsFont.hFontTexture, r, g, b, 255 );
	pfnPIC_DrawAdditive( x, y, -1, -1, &cls.creditsFont.fontRc[number] );
	clgame.ds.adjust_size = false;

	return clgame.scrInfo.charWidths[number];
}

/*
=============
pfnDrawConsoleString

drawing string like a console string 
=============
*/
int pfnDrawConsoleString( int x, int y, const char *string )
{
	int	drawLen;

	if( !string || !*string ) return 0; // silent ignore
	clgame.ds.adjust_size = true;
	Con_SetFont( con_fontsize->integer );
	drawLen = Con_DrawString(x, y, string, clgame.ds.textColor);
	Vector4Copy( g_color_table[7], clgame.ds.textColor );
	clgame.ds.adjust_size = false;
	Con_RestoreFont();

	return (x + drawLen); // exclude color prexfixes
}

/*
=============
pfnDrawSetTextColor

set color for anything
=============
*/
void pfnDrawSetTextColor( float r, float g, float b )
{
	//AVSARTODO
	vgui2::surface()->DrawSetTextColor(r * 255, g * 255, b * 255, 255);

	// bound color and convert to byte
	clgame.ds.textColor[0] = (byte)bound( 0, r * 255, 255 );
	clgame.ds.textColor[1] = (byte)bound( 0, g * 255, 255 );
	clgame.ds.textColor[2] = (byte)bound( 0, b * 255, 255 );
	clgame.ds.textColor[3] = (byte)0xFF;
}

/*
=============
pfnDrawConsoleStringLen

compute string length in screen pixels
=============
*/
void pfnDrawConsoleStringLen( const char *pText, int *length, int *height )
{
	Con_SetFont( con_fontsize->integer );
	clgame.ds.adjust_size = true;
	Con_DrawStringLen( pText, length, height );
	clgame.ds.adjust_size = false;
	Con_RestoreFont();
}

/*
=============
pfnConsolePrint

prints directly into console (can skip notify)
=============
*/
void pfnConsolePrint( const char *string )
{
	if( !string || !*string ) return;
	if( *string != 1 ) Con_Printf( "%s", string ); // show notify
	else Con_NPrintf( 0, "%s", (char *)string + 1 ); // skip notify
}


/*
=============
pfnCenterPrint

holds and fade message at center of screen
like trigger_multiple message in q1
=============
*/
void pfnCenterPrint( const char *string )
{
	if( !string || !*string ) return; // someone stupid joke
	CL_CenterPrint( string, 0.25f );
}

/*
=========
GetWindowCenterX

=========
*/
int pfnGetWindowCenterX( void )
{
	int x = 0;
	SDL_GetWindowPosition( host.hWnd, &x, NULL );
	return host.window_center_x + x;
}

/*
=========
GetWindowCenterY

=========
*/
int pfnGetWindowCenterY( void )
{
	int y = 0;
	SDL_GetWindowPosition( host.hWnd, NULL, &y );
	return host.window_center_y + y;
}

/*
=============
pfnGetViewAngles

return interpolated angles from previous frame
=============
*/
void pfnGetViewAngles( float *angles )
{
	if( angles ) VectorCopy( cl.refdef.cl_viewangles, angles );
}

/*
=============
pfnSetViewAngles

return interpolated angles from previous frame
=============
*/
void pfnSetViewAngles( float *angles )
{
	if( angles ) VectorCopy( angles, cl.refdef.cl_viewangles );
}

/*
====================
CL_GetMaxlients

Render callback for studio models
====================
*/
int CL_GetMaxClients( void )
{
	return cl.maxclients; // AVSARTODO
}


/*
=============
pfnPhysInfo_ValueForKey

=============
*/
const char* pfnPhysInfo_ValueForKey( const char *key )
{
	return Info_ValueForKey( cl.frame.client.physinfo, key );
}

/*
=============
pfnServerInfo_ValueForKey

=============
*/
const char* pfnServerInfo_ValueForKey( const char *key )
{
	return Info_ValueForKey( cl.serverinfo, key );
}

/*
=============
pfnGetClientMaxspeed

value that come from server
=============
*/
float pfnGetClientMaxspeed( void )
{
	return cl.frame.client.maxspeed;
}

/*
=============
pfnCheckParm

=============
*/
int pfnCheckParm( const char *parm, const char **ppnext )
{
	static char	str[64];

	if( Sys_GetParmFromCmdLine( parm, str ))
	{
		// get the pointer on cmdline param
		if( ppnext ) *ppnext = str;
		return 1;
	}
	return 0;
}

/*
=============
pfnGetMousePosition

=============
*/
void CL_GetMousePosition( int *mx, int *my )
{
	SDL_GetMouseState(mx, my);
}

/*
=============
pfnIsNoClipping

=============
*/
int pfnIsNoClipping( void )
{
	cl_entity_t *pl = CL_GetLocalPlayer();

	if( !pl ) return false;

	return pl->curstate.movetype == MOVETYPE_NOCLIP;
}

/*
====================
CL_GetLocalPlayer

Render callback for studio models
====================
*/
cl_entity_t* CL_GetLocalPlayer( void )
{
	cl_entity_t	*player;

	player = CL_EDICT_NUM( cl.playernum + 1 );
	//ASSERT( player != NULL );

	return player;
}

/*
=============
pfnGetViewModel

=============
*/
cl_entity_t* pfnGetViewModel( void )
{
	return &clgame.viewent;
}


/*
====================
CL_GetEntityByIndex

Render callback for studio models
====================
*/
cl_entity_t* CL_GetEntityByIndex( int index )
{
	if( !clgame.entities ) // not in game yet
		return NULL;

	if( index == 0 )
		return cl.world;

	if( index < 0 )
		return clgame.dllFuncs.pfnGetUserEntity( -index );

	if( index >= clgame.maxEntities )
		return NULL;

	return CL_EDICT_NUM( index );
}

/*
=============
pfnGetClientTime

=============
*/
float pfnGetClientTime( void )
{
	return cl.time;
}

/*
=============
pfnCalcShake

=============
*/
void pfnCalcShake( void )
{
	int	i;
	float	fraction, freq;
	float	localAmp;

	if( clgame.shake.time == 0 )
		return;

	if(( cl.time > clgame.shake.time ) || clgame.shake.amplitude <= 0 || clgame.shake.frequency <= 0 )
	{
		Q_memset( &clgame.shake, 0, sizeof( clgame.shake ));
		return;
	}

	if( cl.time > clgame.shake.next_shake )
	{
		// higher frequency means we recalc the extents more often and perturb the display again
		clgame.shake.next_shake = cl.time + ( 1.0f / clgame.shake.frequency );

		// compute random shake extents (the shake will settle down from this)
		for( i = 0; i < 3; i++ )
			clgame.shake.offset[i] = Com_RandomFloat( -clgame.shake.amplitude, clgame.shake.amplitude );
		clgame.shake.angle = Com_RandomFloat( -clgame.shake.amplitude * 0.25f, clgame.shake.amplitude * 0.25f );
	}

	// ramp down amplitude over duration (fraction goes from 1 to 0 linearly with slope 1/duration)
	fraction = ( clgame.shake.time - cl.time ) / clgame.shake.duration;

	// ramp up frequency over duration
	if( fraction )
	{
		freq = ( clgame.shake.frequency / fraction );
	}
	else
	{
		freq = 0;
	}

	// square fraction to approach zero more quickly
	fraction *= fraction;

	// Sine wave that slowly settles to zero
	fraction = fraction * sin( cl.time * freq );
	
	// add to view origin
	VectorScale( clgame.shake.offset, fraction, clgame.shake.applied_offset );

	// add to roll
	clgame.shake.applied_angle = clgame.shake.angle * fraction;

	// drop amplitude a bit, less for higher frequency shakes
	localAmp = clgame.shake.amplitude * ( host.frametime / ( clgame.shake.duration * clgame.shake.frequency ));
	clgame.shake.amplitude -= localAmp;
}

/*
=============
pfnApplyShake

=============
*/
void pfnApplyShake( float *origin, float *angles, float factor )
{
	if( origin ) VectorMA( origin, factor, clgame.shake.applied_offset, origin );
	if( angles ) angles[ROLL] += clgame.shake.applied_angle * factor;
}

/*
=============
pfnPointContents

=============
*/
int pfnPointContents( const float *p, int *truecontents )
{
	int	cont, truecont;

	truecont = cont = CL_TruePointContents( p );
	if( truecontents ) *truecontents = truecont;

	if( cont <= CONTENTS_CURRENT_0 && cont >= CONTENTS_CURRENT_DOWN )
		cont = CONTENTS_WATER;
	return cont;
}

/*
=============
pfnTraceLine

=============
*/
pmtrace_t* pfnTraceLine( float *start, float *end, int flags, int usehull, int ignore_pe )
{
	static pmtrace_t	tr;
	int		old_usehull;

	old_usehull = clgame.pmove->usehull;
	clgame.pmove->usehull = usehull;	

	switch( flags )
	{
	case PM_TRACELINE_PHYSENTSONLY:
		tr = PM_PlayerTraceExt( clgame.pmove, start, end, 0, clgame.pmove->numphysent, clgame.pmove->physents, ignore_pe, NULL );
		break;
	case PM_TRACELINE_ANYVISIBLE:
		tr = PM_PlayerTraceExt( clgame.pmove, start, end, 0, clgame.pmove->numvisent, clgame.pmove->visents, ignore_pe, NULL );
		break;
	}

	clgame.pmove->usehull = old_usehull;

	return &tr;
}

/*
=============
CL_LoadModel

=============
*/
model_t* CL_LoadModel( const char *modelname, int *index )
{
	int	idx;

	idx = CL_FindModelIndex( modelname );
	if( !idx ) return NULL;
	if( index ) *index = idx;
	
	return Mod_Handle( idx );
}

/*
=============
CL_AddEntity

=============
*/
int CL_AddEntity( int entityType, cl_entity_t *pEnt )
{
	if( !pEnt ) return false;

	// clear effects for all temp entities
	if( !pEnt->index ) pEnt->curstate.effects = 0;

	// let the render reject entity without model
	return CL_AddVisibleEntity( pEnt, entityType );
}

/*
=============
CL_GetSpritePointer

=============
*/
const model_t* CL_GetSpritePointer( HSPRITE hSprite )
{
	if( hSprite <= 0 || hSprite > ( MAX_IMAGES - 1 ))
		return NULL; // bad image
	return &clgame.sprites[hSprite];
}


/*
=============
pfnPlaySoundByNameAtLocation

=============
*/
void pfnPlaySoundByNameAtLocation( const char *szSound, float volume, float *origin )
{
	int hSound = S_RegisterSound( szSound );
	S_StartSound( origin, 0, CHAN_ITEM, hSound, volume, 1.0, PITCH_NORM, 0 );
}

/*
=============
pfnPrecacheEvent

=============
*/
static word pfnPrecacheEvent( int type, const char* psz )
{
	return CL_EventIndex( psz );
}

/*
=============
pfnHookEvent

=============
*/
void pfnHookEvent( const char *filename, pfnEventHook pfn )
{
	char		name[64];
	cl_user_event_t	*ev;
	int		i;

	// ignore blank names
	if( !filename || !*filename )
		return;	

	Q_strncpy( name, filename, sizeof( name ));
	COM_FixSlashes( name );

	// find an empty slot
	for( i = 0; i < MAX_EVENTS; i++ )
	{
		ev = clgame.events[i];		
		if( !ev ) break;

		if( !Q_stricmp( name, ev->name ) && ev->func != NULL )
		{
			MsgDev( D_WARN, "CL_HookEvent: %s already hooked!\n", name );
			return;
		}
	}

	CL_RegisterEvent( i, name, pfn );
}

/*
=============
pfnGetGameDirectory

=============
*/
const char* pfnGetGameDirectory( void )
{
	static char	szGetGameDir[MAX_SYSPATH];

	Q_sprintf( szGetGameDir, "%s", GI->gamefolder );
	return szGetGameDir;
}

/*
=============
Key_LookupBinding

=============
*/
const char* Key_LookupBinding( const char *pBinding )
{
	return Key_KeynumToString( Key_GetKey( pBinding ));
}

/*
=============
pfnGetLevelName

=============
*/
const char* pfnGetLevelName( void )
{
	static char	mapname[64];

	if( cls.state >= ca_connected )
		Q_snprintf( mapname, sizeof( mapname ), "maps/%s.bsp", clgame.mapname );
	else mapname[0] = '\0'; // not in game

	return mapname;
}

/*
=============
pfnGetScreenFade

=============
*/
void pfnGetScreenFade( struct screenfade_s *fade )
{
	if( fade ) *fade = clgame.fade;
}

/*
=============
pfnSetScreenFade

=============
*/
void pfnSetScreenFade( struct screenfade_s *fade )
{
	if( fade ) clgame.fade = *fade;
}

/*
=============
VGui_ViewportPaintBackground

=============
*/
void VGui_ViewportPaintBackground( int extents[4] )
{
	// WHAT?
}
	
/*
=============
pfnIsSpectateOnly

=============
*/
int pfnIsSpectateOnly( void )
{
	cl_entity_t *pPlayer = CL_GetLocalPlayer();
	return pPlayer ? (pPlayer->curstate.spectator != 0) : 0;
}

/*
=============
pfnLoadMapSprite

=============
*/
model_t* pfnLoadMapSprite( const char *filename )
{
	char	name[64];
	int	i;
	int texFlags = TF_NOPICMIP;


	if( cl_sprite_nearest->integer )
		texFlags |= TF_NEAREST;

	if( !filename || !*filename )
	{
		MsgDev( D_ERROR, "CL_LoadMapSprite: bad name!\n" );
		return NULL;
	}

	Q_strncpy( name, filename, sizeof( name ));
	COM_FixSlashes( name );

	// slot 0 isn't used
	for( i = 1; i < MAX_IMAGES; i++ )
	{
		if( !Q_stricmp( clgame.sprites[i].name, name ))
		{
			// prolonge registration
			clgame.sprites[i].needload = clgame.load_sequence;
			return &clgame.sprites[i];
		}
	}

	// find a free model slot spot
	for( i = 1; i < MAX_IMAGES; i++ )
	{
		if( !clgame.sprites[i].name[0] )
			break; // this is a valid spot
	}

	if( i == MAX_IMAGES ) 
	{
		MsgDev( D_ERROR, "LoadMapSprite: can't load %s, MAX_HSPRITES limit exceeded\n", filename );
		return NULL;
	}

	// load new map sprite
	if( CL_LoadHudSprite( name, &clgame.sprites[i], true, texFlags ))
	{
		clgame.sprites[i].needload = clgame.load_sequence;
		return &clgame.sprites[i];
	}
	return NULL;
}

/*
=============
PlayerInfo_ValueForKey

=============
*/
const char* PlayerInfo_ValueForKey( int playerNum, const char *key )
{
	// find the player
	if(( playerNum > cl.maxclients ) || ( playerNum < 1 ))
		return NULL;

	if( !cl.players[playerNum-1].name[0] )
		return NULL;

	return Info_ValueForKey( cl.players[playerNum-1].userinfo, key );
}

/*
=============
PlayerInfo_SetValueForKey

=============
*/
void PlayerInfo_SetValueForKey( const char *key, const char *value )
{
	cvar_t	*var;

	var = (cvar_t *)Cvar_FindVar( key );
	if( !var || !(var->flags & CVAR_USERINFO ))
		return;

	Cvar_DirectSet( var, value );
}

/*
=============
pfnGetPlayerUniqueID

=============
*/
qboolean pfnGetPlayerUniqueID( int iPlayer, char playerID[16] )
{
	// AVSARTODO: hashedcdkey

	playerID[0] = '\0';
	return false;
}

/*
=============
pfnGetTrackerIDForPlayer

=============
*/
int pfnGetTrackerIDForPlayer( int playerSlot )
{
	playerSlot -= 1;	// make into a client index

	if( !cl.players[playerSlot].userinfo[0] || !cl.players[playerSlot].name[0] )
			return 0;
	return Q_atoi( Info_ValueForKey( cl.players[playerSlot].userinfo, "*tracker" ));
}

/*
=============
pfnGetPlayerForTrackerID

=============
*/
int pfnGetPlayerForTrackerID( int trackerID )
{
	int	i;

	for( i = 0; i < MAX_CLIENTS; i++ )
	{
		if( !cl.players[i].userinfo[0] || !cl.players[i].name[0] )
			continue;

		if( Q_atoi( Info_ValueForKey( cl.players[i].userinfo, "*tracker" )) == trackerID )
		{
			// make into a player slot
			return (i+1);
		}
	}
	return 0;
}

/*
=============
pfnServerCmdUnreliable

=============
*/
int pfnServerCmdUnreliable( const char *szCmdString )
{
	if( !szCmdString || !szCmdString[0] )
		return 0;

	BF_WriteByte( &cls.datagram, clc_stringcmd );
	BF_WriteString( &cls.datagram, szCmdString );

	return 1;
}

/*
=============
pfnGetMousePos

=============
*/
void pfnGetMousePos( POINT *ppt )
{
	SDL_GetMouseState((int*)&ppt->x, (int*)&ppt->y);
}

/*
=============
pfnSetMousePos

=============
*/
void pfnSetMousePos( int mx, int my )
{
	SDL_WarpMouseInWindow( host.hWnd, mx, my );
}

/*
=============
pfnSetMouseEnable

=============
*/
void pfnSetMouseEnable( qboolean fEnable )
{
	if( fEnable ) IN_ActivateMouse( false );
	else IN_DeactivateMouse();
}

/*
=============
pfnGetServerTime

=============
*/
float pfnGetClientOldTime( void )
{
	return cl.oldtime;
}

/*
=============
pfnGetGravity

=============
*/
float pfnGetGravity( void )
{
	return clgame.movevars.gravity;
}

/*
=============
pfnEnableTexSort

TODO: implement
=============
*/
void pfnEnableTexSort( int enable )
{
}

/*
=============
pfnSetLightmapColor

TODO: implement
=============
*/
void pfnSetLightmapColor( float red, float green, float blue )
{
}

/*
=============
pfnSetLightmapScale

TODO: implement
=============
*/
void pfnSetLightmapScale( float scale )
{
}

/*
=============
pfnSPR_DrawGeneric

=============
*/
void pfnSPR_DrawGeneric( int frame, int x, int y, const wrect_t *prc, int blendsrc, int blenddst, int width, int height )
{
	pglEnable( GL_BLEND );
	pglBlendFunc( blendsrc, blenddst ); // g-cont. are params is valid?
	SPR_DrawGeneric( frame, x, y, width, height, prc );
}

/*
=============
pfnDrawString
AVSARTODO: replace with vgui2
=============
*/
int pfnDrawString( int x, int y, const char *str, int r, int g, int b )
{
	Con_UtfProcessChar(0);

	// draw the string until we hit the null character or a newline character
	for ( ; *str != 0 && *str != '\n'; str++ )
	{
		x += pfnVGUI2DrawCharacterAdditive( x, y, (unsigned char)*str, r, g, b, 0 );
	}

	return x;
}

/*
=============
pfnDrawStringReverse
AVSARTODO: replace with vgui2
=============
*/
int pfnDrawStringReverse( int x, int y, const char *str, int r, int g, int b )
{
	// find the end of the string
	char *szIt;
	for( szIt = (char*)str; *szIt != 0; szIt++ )
		x -= clgame.scrInfo.charWidths[ (unsigned char) *szIt ];
	pfnDrawString( x, y, str, r, g, b );
	return x;
}

/*
=============
LocalPlayerInfo_ValueForKey

=============
*/
const char* LocalPlayerInfo_ValueForKey( const char* key )
{
	return Info_ValueForKey( Cvar_Userinfo(), key );
}

/*
=============
pfnVGUI2DrawCharacter
AVSARTODO: replace with vgui2
=============
*/
int pfnVGUI2DrawCharacter( int x, int y, int number, unsigned int font )
{
	if( !cls.creditsFont.valid )
		return 0;

	number &= 255;

	number = Con_UtfProcessChar( number );

	if( number < 32 ) return 0;
	if( y < -clgame.scrInfo.iCharHeight )
		return 0;

	clgame.ds.adjust_size = true;
	menu.ds.gl_texturenum = cls.creditsFont.hFontTexture;
	pfnPIC_DrawAdditive( x, y, -1, -1, &cls.creditsFont.fontRc[number] );
	clgame.ds.adjust_size = false;

	return clgame.scrInfo.charWidths[number];
}

/*
=============
pfnVGUI2DrawCharacterAdditive
AVSARTODO: replace with vgui2
=============
*/
int pfnVGUI2DrawCharacterAdditive( int x, int y, int ch, int r, int g, int b, unsigned int font )
{
	if( !hud_utf8->integer )
		ch = Con_UtfProcessChar( ch );

	clgame.ds.adjust_size = true;
	int w = pfnDrawCharacter( x, y, ch, r, g, b );
	clgame.ds.adjust_size = false;
	return w;
}

/*
=============
GetCareerGameInterface
AVSARTODO: call CareerUI
=============
*/
void* GetCareerGameInterface( void )
{
	Con_Printf( "^1Career GameInterface called!\n" );
	return NULL;
}

/*
=============
pfnPlaySoundVoiceByName

=============
*/
void pfnPlaySoundVoiceByName( const char *filename, float volume, int pitch )
{
	int hSound = S_RegisterSound( filename );

	S_StartSound( NULL, cl.refdef.viewentity, CHAN_NETWORKVOICE_END + 1, hSound, volume, 1.0, pitch, SND_STOP_LOOPING );
}

/*
=============
pfnMP3_InitStream

=============
*/
void pfnMP3_InitStream( const char *filename, int looping )
{
	if( !filename )
	{
		S_StopBackgroundTrack();
		return;
	}

	if( looping )
	{
		S_StartBackgroundTrack( filename, filename, 0 );
	}
	else
	{
		S_StartBackgroundTrack( filename, NULL, 0 );
	}
}

/*
=============
pfnPlaySoundByNameAtPitch

=============
*/
void pfnPlaySoundByNameAtPitch( const char *filename, float volume, int pitch )
{
	int hSound = S_RegisterSound( filename );
	S_StartSound( NULL, cl.refdef.viewentity, CHAN_ITEM, hSound, volume, 1.0, pitch, SND_STOP_LOOPING );
}

/*
=============
pfnFillRGBABlend

=============
*/
void CL_FillRGBABlend( int x, int y, int width, int height, int r, int g, int b, int a )
{
	float x1 = x, y1 = y, w1 = width, h1 = height;
	r = bound( 0, r, 255 );
	g = bound( 0, g, 255 );
	b = bound( 0, b, 255 );
	a = bound( 0, a, 255 );
	pglColor4ub( r, g, b, a );

	SPR_AdjustSize( &x1, &y1, &w1, &h1 );

	GL_SetRenderMode( kRenderTransTexture );
	R_DrawStretchPic( x1, y1, w1, h1, 0, 0, 1, 1, cls.fillImage );
	pglColor4ub( 255, 255, 255, 255 );
}

/*
=============
pfnGetAppID
AVSARTODO:
=============
*/
int pfnGetAppID( void )
{
	return 70; // Half-Life AppID
}

/*
=============
pfnVguiWrap2_GetMouseDelta

AVSARTODO: implement
=============
*/
void pfnVguiWrap2_GetMouseDelta( int *x, int *y )
{
}


void* pfnVGui_GetPanel()
{
	return NULL;
}


// engine callbacks
cl_enginefunc_t cl_enginefuncs = 
{
	pfnSPR_Load,
	pfnSPR_Frames,
	pfnSPR_Height,
	pfnSPR_Width,
	pfnSPR_Set,
	pfnSPR_Draw,
	pfnSPR_DrawHoles,
	pfnSPR_DrawAdditive,
	SPR_EnableScissor,
	SPR_DisableScissor,
	pfnSPR_GetList,
	CL_FillRGBA,
	pfnGetScreenInfo,
	pfnSetCrosshair,
	pfnCvar_RegisterVariable,
	Cvar_VariableValue,
	Cvar_VariableString,
	pfnAddClientCommand,
	pfnHookUserMsg,
	pfnServerCmd,
	pfnClientCmd,
	pfnGetPlayerInfo,
	pfnPlaySoundByName,
	pfnPlaySoundByIndex,
	AngleVectors,
	CL_TextMessageGet,
	pfnDrawCharacter,
	pfnDrawConsoleString,
	pfnDrawSetTextColor,
	pfnDrawConsoleStringLen,
	pfnConsolePrint,
	pfnCenterPrint,
	pfnGetWindowCenterX,
	pfnGetWindowCenterY,
	pfnGetViewAngles,
	pfnSetViewAngles,
	CL_GetMaxClients,
	Cvar_SetFloat,
	Cmd_Argc,
	Cmd_Argv,
	Con_Printf,
	Con_DPrintf,
	Con_NPrintf,
	Con_NXPrintf,
	pfnPhysInfo_ValueForKey,
	pfnServerInfo_ValueForKey,
	pfnGetClientMaxspeed,
	pfnCheckParm,
	Key_Event,
	CL_GetMousePosition,
	pfnIsNoClipping,
	CL_GetLocalPlayer,
	pfnGetViewModel,
	CL_GetEntityByIndex,
	pfnGetClientTime,
	pfnCalcShake,
	pfnApplyShake,
	pfnPointContents,
	CL_WaterEntity,
	pfnTraceLine,
	CL_LoadModel,
	CL_AddEntity,
	CL_GetSpritePointer,
	pfnPlaySoundByNameAtLocation,
	pfnPrecacheEvent,
	CL_PlaybackEvent,
	CL_WeaponAnim,
	Com_RandomFloat,
	Com_RandomLong,
	pfnHookEvent,
	Con_Visible,
	pfnGetGameDirectory,
	pfnCVarGetPointer,
	Key_LookupBinding,
	pfnGetLevelName,
	pfnGetScreenFade,
	pfnSetScreenFade,
	pfnVGui_GetPanel,
	VGui_ViewportPaintBackground,
	COM_LoadFile,
	COM_ParseFile,
	COM_FreeFile,
	&CL_gTriApi,
	&CL_gEfxApi,
	&CL_gEventApi,
	&CL_gDemoApi,
	&CL_gNetApi,
	&CL_gVoiceApi,
	pfnIsSpectateOnly,
	pfnLoadMapSprite,
	COM_AddAppDirectoryToSearchPath,
	COM_ExpandFilename,
	PlayerInfo_ValueForKey,
	PlayerInfo_SetValueForKey,
	pfnGetPlayerUniqueID,
	pfnGetTrackerIDForPlayer,
	pfnGetPlayerForTrackerID,
	pfnServerCmdUnreliable,
	pfnGetMousePos,
	pfnSetMousePos,
	pfnSetMouseEnable,
	Cvar_GetList,
	(void* (*)(void))Cmd_GetFirstFunctionHandle,
	(void* (*)(void*))Cmd_GetNextFunctionHandle,
	(const char* (*)(void*))Cmd_GetName,
	pfnGetClientOldTime,
	pfnGetGravity,
	Mod_Handle,
	pfnEnableTexSort,
	pfnSetLightmapColor,
	pfnSetLightmapScale,
	pfnSequenceGet,
	pfnSPR_DrawGeneric,
	pfnSequencePickSentence,
	pfnDrawString,
	pfnDrawStringReverse,
	LocalPlayerInfo_ValueForKey,
	pfnVGUI2DrawCharacter,
	pfnVGUI2DrawCharacterAdditive,
	Sound_GetApproxWavePlayLen,
	GetCareerGameInterface,
	Cvar_Set,
	pfnIsCareerMatch,
	pfnPlaySoundVoiceByName,
	pfnMP3_InitStream,
	Sys_DoubleTime,
	pfnProcessTutorMessageDecayBuffer,
	pfnConstructTutorMessageDecayBuffer,
	pfnResetTutorMessageDecayData,
	pfnPlaySoundByNameAtPitch,
	CL_FillRGBABlend,
	pfnGetAppID,
	Cmd_AliasGetList,
	pfnVguiWrap2_GetMouseDelta,
};


#endif // XASH_DEDICATED