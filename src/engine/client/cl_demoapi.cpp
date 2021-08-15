// cl_demoapi.cpp

#include "common.h"
#include "client.h"
#include "demo_api.h"
#include "cl_tent.h"

/*
=================
Demo_IsRecording

=================
*/
int Demo_IsRecording( void )
{
	return cls.demorecording;
}

/*
=================
Demo_IsPlayingback

=================
*/
int Demo_IsPlayingback( void )
{
	return cls.demoplayback;
}

/*
=================
Demo_IsTimeDemo

=================
*/
int Demo_IsTimeDemo( void )
{
	return cls.timedemo;
}

/*
=================
Demo_WriteBuffer

=================
*/
void Demo_WriteBuffer( int size, byte *buffer )
{
	CL_WriteDemoUserMessage( buffer, size );
}

demo_api_t CL_gDemoApi =
{
	Demo_IsRecording,
	Demo_IsPlayingback,
	Demo_IsTimeDemo,
	Demo_WriteBuffer,
};
