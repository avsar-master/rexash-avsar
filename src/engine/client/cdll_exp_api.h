#ifndef ENGINE_CDLL_EXP_H
#define ENGINE_CDLL_EXP_H

#include "mathlib.h"
#include "cdll_int.h"
#include "cl_entity.h"
#include "com_model.h"
#include "mod_local.h"
#include "pm_defs.h"
#include "pm_movevars.h"
#include "render_api.h"
#include "cdll_exp.h"
#include "screenfade.h"
#include "protocol.h"
#include "netchan.h"
#include "net_api.h"
#include "world.h"
#include "qfont.h"
#include "wrect.h"

static struct crosshair_s
{
	// crosshair members
	const model_t	*pCrosshair;
	wrect_t		rcCrosshair;
	rgba_t		rgbaCrosshair;
} crosshair_state;

#define MAX_LINELENGTH		80
#define MAX_TEXTCHANNELS	8		// must be power of two (GoldSrc uses 4 channels)
#define TEXT_MSGNAME	"TextMessage%i"

void SPR_DrawGeneric( int frame, float x, float y, float width, float height, const wrect_t *prc );

HSPRITE pfnSPR_LoadExt( const char *szPicName, uint texFlags );

qboolean CL_LoadHudSprite( const char *szSpriteName, model_t *m_pSprite, qboolean mapSprite, uint texFlags );

extern rgba_t g_color_table[8];

HSPRITE pfnSPR_Load( const char *szPicName );

int pfnSPR_Frames( HSPRITE hPic );

int pfnSPR_Height( HSPRITE hPic, int frame );

int pfnSPR_Width( HSPRITE hPic, int frame );

void pfnSPR_Set( HSPRITE hPic, int r, int g, int b );

void pfnSPR_Draw( int frame, int x, int y, const wrect_t *prc );

void pfnSPR_DrawHoles( int frame, int x, int y, const wrect_t *prc );

void pfnSPR_DrawAdditive( int frame, int x, int y, const wrect_t *prc );

void SPR_EnableScissor( int x, int y, int width, int height );

void SPR_DisableScissor( void );

client_sprite_t* pfnSPR_GetList( const char *psz, int *piCount );

void CL_FillRGBA( int x, int y, int width, int height, int r, int g, int b, int a );

int pfnGetScreenInfo( SCREENINFO *pscrinfo );

void pfnSetCrosshair( HSPRITE hspr, wrect_t rc, int r, int g, int b );

int pfnAddClientCommand( const char *cmd_name, xcommand_t func );

int pfnHookUserMsg( const char *pszName, pfnUserMsgHook pfn );

int pfnServerCmd( const char *szCmdString );

int pfnClientCmd( const char *szCmdString );

void pfnGetPlayerInfo( int ent_num, hud_player_info_t *pinfo );

void pfnPlaySound( int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch );

void pfnPlaySoundByName( const char *szSound, float volume );

void pfnPlaySoundByIndex( int iSound, float volume );

client_textmessage_t* CL_TextMessageGet( const char *pName );

int pfnDrawCharacter( int x, int y, int number, int r, int g, int b );

void pfnConsolePrint( const char *string );

void pfnCenterPrint( const char *string );

int pfnGetWindowCenterX( void );

int pfnGetWindowCenterY( void );

void pfnGetViewAngles( float *angles );

void pfnSetViewAngles( float *angles );

const char* pfnPhysInfo_ValueForKey( const char *key );

const char* pfnServerInfo_ValueForKey( const char *key );

float pfnGetClientMaxspeed( void );

int pfnCheckParm( const char *parm, const char **ppnext );

void CL_GetMousePosition( int *mx, int *my );

int pfnIsNoClipping( void );

cl_entity_t* CL_GetLocalPlayer( void );

cl_entity_t* pfnGetViewModel( void );

cl_entity_t* CL_GetEntityByIndex( int index );

float pfnGetClientTime( void );

void pfnCalcShake( void );

void pfnApplyShake( float *origin, float *angles, float factor );

int pfnPointContents( const float *p, int *truecontents );

pmtrace_t* pfnTraceLine( float *start, float *end, int flags, int usehull, int ignore_pe );

model_t* CL_LoadModel( const char *modelname, int *index );

int CL_AddEntity( int entityType, cl_entity_t *pEnt );

const model_t* CL_GetSpritePointer( HSPRITE hSprite );

void pfnPlaySoundByNameAtLocation( const char *szSound, float volume, float *origin );

word pfnPrecacheEvent( int type, const char* psz );

typedef void (*pfnEventHook)(event_args_t* args);

void pfnHookEvent( const char *filename, pfnEventHook pfn );

const char* pfnGetGameDirectory( void );

const char* Key_LookupBinding( const char *pBinding );

const char* pfnGetLevelName( void );

void pfnGetScreenFade( struct screenfade_s *fade );

void pfnSetScreenFade( struct screenfade_s *fade );

void VGui_ViewportPaintBackground( int extents[4] );

int pfnIsSpectateOnly( void );

model_t* pfnLoadMapSprite( const char *filename );

const char* PlayerInfo_ValueForKey( int playerNum, const char *key );

void PlayerInfo_SetValueForKey( const char *key, const char *value );

qboolean pfnGetPlayerUniqueID( int iPlayer, char playerID[16] );

int pfnGetTrackerIDForPlayer( int playerSlot );

int pfnGetPlayerForTrackerID( int trackerID );

int pfnServerCmdUnreliable( const char *szCmdString );

void pfnGetMousePos( POINT *ppt );

void pfnSetMouseEnable( qboolean fEnable );

float pfnGetClientOldTime( void );

float pfnGetGravity( void );

void pfnEnableTexSort( int enable );

void pfnSetLightmapColor( float red, float green, float blue );

void pfnSetLightmapScale( float scale );

void pfnSPR_DrawGeneric( int frame, int x, int y, const wrect_t *prc, int blendsrc, int blenddst, int width, int height );

int pfnDrawString( int x, int y, const char *str, int r, int g, int b );

int pfnDrawStringReverse( int x, int y, const char *str, int r, int g, int b );

const char* LocalPlayerInfo_ValueForKey( const char* key );

int pfnVGUI2DrawCharacter( int x, int y, int number, unsigned int font );

int pfnVGUI2DrawCharacterAdditive( int x, int y, int ch, int r, int g, int b, unsigned int font );

void* GetCareerGameInterface( void );

void pfnPlaySoundVoiceByName( const char *filename, float volume, int pitch );

void pfnMP3_InitStream( const char *filename, int looping );

void pfnPlaySoundByNameAtPitch( const char *filename, float volume, int pitch );

void CL_FillRGBABlend( int x, int y, int width, int height, int r, int g, int b, int a );

int pfnGetAppID( void );

void pfnVguiWrap2_GetMouseDelta( int *x, int *y );

extern cl_enginefunc_t cl_enginefuncs;

#endif // ENGINE_CDLL_EXP_H