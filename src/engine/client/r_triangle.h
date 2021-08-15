#ifndef ENGINE_R_TRIANGLE_H
#define ENGINE_R_TRIANGLE_H

#include "triangleapi.h"
#include "r_efx.h"
#include "cl_tent.h"
#include "shake.h"
#include "sprite.h"
#include "gl_local.h"

void TriRenderMode( int mode );

void TriBegin( int mode );

void TriEnd( void );

void TriColor4f( float r, float g, float b, float a );

void TriColor4ub( byte r, byte g, byte b, byte a );

void TriTexCoord2f( float u, float v );

void TriVertex3fv( const float *v );

void TriVertex3f( float x, float y, float z );

void TriBrightness( float brightness );

void TriCullFace( TRICULLSTYLE mode );

int TriSpriteTexture( model_t *pSpriteModel, int frame );

int TriWorldToScreen( float *world, float *screen );

void TriFog( float flFogColor[3], float flStart, float flEnd, int bOn );

void TriGetMatrix( const int pname, float *matrix );

int TriBoxInPVS( float *mins, float *maxs );

void TriLightAtPoint( float *pos, float *value );

void TriColor4fRendermode( float r, float g, float b, float a, int rendermode );

void TriFogParams( float flDensity, int iFogSkybox );

extern triangleapi_t CL_gTriApi;

#endif // ENGINE_R_TRIANGLE_H