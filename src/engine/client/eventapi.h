#ifndef ENGINE_EVENTAPI_H
#define ENGINE_EVENTAPI_H

#include "common.h"
#include "const.h"
#include "triangleapi.h"
#include "r_efx.h"
#include "demo_api.h"
#include "pm_local.h"
#include "cl_tent.h"
#include "sprite.h"
#include "gl_local.h"
#include "cdll_exp.h"

void pfnPlaySound(int ent, float* org, int chan, const char* samp, float vol, float attn, int flags, int pitch);

int pfnIsLocal( int playernum );

int pfnLocalPlayerDucking( void );

void pfnLocalPlayerViewheight( float *view_ofs );

void pfnLocalPlayerBounds( int hull, float *mins, float *maxs );

int pfnIndexFromTrace( struct pmtrace_s *pTrace );

physent_t * pfnGetPhysent( int idx );

const char* pfnTraceTexture( int ground, float *vstart, float *vend );

void pfnStopAllSounds( int ent, int entchannel );

void pfnKillEvents( int entnum, const char *eventname );

const char* CL_SoundFromIndex( int index );

struct msurface_s * pfnTraceSurface( int ground, float *vstart, float *vend );

extern event_api_t CL_gEventApi; 

#endif // ENGINE_EVENTAPI_H