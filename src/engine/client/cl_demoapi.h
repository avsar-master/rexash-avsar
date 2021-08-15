#ifndef ENGINE_CL_DEMOAPI_H
#define ENGINE_CL_DEMOAPI_H

#include "common.h"
#include "demo_api.h"
#include "cl_tent.h"

int Demo_IsRecording( void );

int Demo_IsPlayingback( void );

int Demo_IsTimeDemo( void );

void Demo_WriteBuffer( int size, byte *buffer );

extern demo_api_t CL_gDemoApi;

#endif // ENGINE_CL_DEMOAPI_H
