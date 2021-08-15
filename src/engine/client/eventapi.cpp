// eventapi.cpp

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
#include "cdll_exp.h"
#include "eventapi.h"
#include "cdll_exp_api.h"


/*
=============
pfnPlaySound

=============
*/
void pfnPlaySound(int ent, float* org, int chan, const char* samp, float vol, float attn, int flags, int pitch)
{
	S_StartSound(org, ent, chan, S_RegisterSound(samp), vol, attn, pitch, flags);
}

/*
=============
pfnIsLocal

=============
*/
int pfnIsLocal( int playernum )
{
	if( playernum == cl.playernum )
		return true;
	return false;
}

/*
=============
pfnLocalPlayerDucking

=============
*/
int pfnLocalPlayerDucking( void )
{
	return cl.predicted.usehull == 1;
}

/*
=============
pfnLocalPlayerViewheight

=============
*/
void pfnLocalPlayerViewheight( float *view_ofs )
{
	// predicted or smoothed
	if( !view_ofs ) return;

	if( CL_IsPredicted( ))
		VectorCopy( cl.predicted.viewofs, view_ofs );
	else VectorCopy( cl.frame.client.view_ofs, view_ofs );
}

/*
=============
pfnLocalPlayerBounds

=============
*/
void pfnLocalPlayerBounds( int hull, float *mins, float *maxs )
{
	if( hull >= 0 && hull < 4 )
	{
		if( mins ) VectorCopy( clgame.pmove->player_mins[hull], mins );
		if( maxs ) VectorCopy( clgame.pmove->player_maxs[hull], maxs );
	}
}

/*
=============
pfnIndexFromTrace

=============
*/
int pfnIndexFromTrace( struct pmtrace_s *pTrace )
{
	if( pTrace->ent >= 0 && pTrace->ent < clgame.pmove->numphysent )
	{
		// return cl.entities number
		return clgame.pmove->physents[pTrace->ent].info;
	}
	return -1;
}

/*
=============
pfnGetPhysent

=============
*/
physent_t * pfnGetPhysent( int idx )
{
	if( idx >= 0 && idx < clgame.pmove->numphysent )
	{
		// return physent
		return &clgame.pmove->physents[idx];
	}
	return NULL;
}

/*
=============
pfnTraceTexture

=============
*/
const char* pfnTraceTexture( int ground, float *vstart, float *vend )
{
	physent_t *pe;

	if( ground < 0 || ground >= clgame.pmove->numphysent )
		return NULL; // bad ground

	pe = &clgame.pmove->physents[ground];
	return PM_TraceTexture( pe, vstart, vend );
}

/*
=============
pfnStopAllSounds

=============
*/
void pfnStopAllSounds( int ent, int entchannel )
{
	S_StopSound( ent, entchannel, NULL );
}

/*
=============
pfnKillEvent

=============
*/
void pfnKillEvents( int entnum, const char *eventname )
{
	int		i;
	event_state_t	*es;
	event_info_t	*ei;
	int		eventIndex = CL_EventIndex( eventname );

	if( eventIndex < 0 || eventIndex >= MAX_EVENTS )
		return;

	if( entnum < 0 || entnum > clgame.maxEntities )
		return;

	es = &cl.events;

	// find all events with specified index and kill it
	for( i = 0; i < MAX_EVENT_QUEUE; i++ )
	{
		ei = &es->ei[i];

		if( ei->index == eventIndex && ei->entity_index == entnum )
		{
			CL_ResetEvent( ei );
			break;
		}
	}
}

/*
====================
CL_SoundFromIndex

return soundname from index
====================
*/
const char* CL_SoundFromIndex( int index )
{
	sfx_t	*sfx = NULL;
	int	hSound;

	// make sure that we're within bounds
	index = bound( 0, index, MAX_SOUNDS );
	hSound = cl.sound_index[index];

	if( !hSound )
	{
		MsgDev( D_ERROR, "CL_SoundFromIndex: invalid sound index %i\n", index );
		return NULL;
	}

	sfx = S_GetSfxByHandle( hSound );
	if( !sfx )
	{
		MsgDev( D_ERROR, "CL_SoundFromIndex: bad sfx for index %i\n", index );
		return NULL;
	}

	return sfx->name;
}

/*
=============
pfnTraceSurface

=============
*/
struct msurface_s * pfnTraceSurface( int ground, float *vstart, float *vend )
{
	physent_t *pe;

	if( ground < 0 || ground >= clgame.pmove->numphysent )
		return NULL; // bad ground

	pe = &clgame.pmove->physents[ground];
	return PM_TraceSurface( pe, vstart, vend );
}

event_api_t CL_gEventApi =
{
	EVENT_API_VERSION,
	pfnPlaySound,
	S_StopSound,
	CL_FindModelIndex,
	pfnIsLocal,
	pfnLocalPlayerDucking,
	pfnLocalPlayerViewheight,
	pfnLocalPlayerBounds,
	pfnIndexFromTrace,
	pfnGetPhysent,
	CL_SetUpPlayerPrediction,
	CL_PushPMStates,
	CL_PopPMStates,
	CL_SetSolidPlayers,
	CL_SetTraceHull,
	CL_PlayerTrace,
	CL_WeaponAnim,
	pfnPrecacheEvent,
	CL_PlaybackEvent,
	pfnTraceTexture,
	pfnStopAllSounds,
	pfnKillEvents,
	CL_EventIndex,
	CL_IndexEvent,
	CL_PlayerTraceExt,
	CL_SoundFromIndex,
	pfnTraceSurface
};
