//voice.cpp

#include "common.h"
#include "client.h"
#include "ivoicetweak.h"

/*
=================
Voice_StartVoiceTweakMode

=================
*/
static int Voice_StartVoiceTweakMode( void )
{
	// TODO: implement
	return 0;
}

/*
=================
Voice_EndVoiceTweakMode

=================
*/
static void Voice_EndVoiceTweakMode( void )
{
	// TODO: implement
}

/*
=================
Voice_SetControlFloat

=================
*/	
static void Voice_SetControlFloat( VoiceTweakControl iControl, float value )
{
	// TODO: implement
}

/*
=================
Voice_GetControlFloat

=================
*/
static float Voice_GetControlFloat( VoiceTweakControl iControl )
{
	// TODO: implement
	return 1.0f;
}

/*
=================
Voice_GetSpeakingVolume

=================
*/
static int Voice_GetSpeakingVolume( void )
{
	// TODO: implement
	return 255;
}

IVoiceTweak CL_gVoiceApi =
{
	Voice_StartVoiceTweakMode,
	Voice_EndVoiceTweakMode,
	Voice_SetControlFloat,
	Voice_GetControlFloat,
	Voice_GetSpeakingVolume,
};