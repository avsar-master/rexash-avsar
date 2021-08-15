// r_efx.cpp

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

efx_api_t CL_gEfxApi =
{
	CL_AllocParticle,
	CL_BlobExplosion,
	CL_Blood,
	CL_BloodSprite,
	CL_BloodStream,
	CL_BreakModel,
	CL_Bubbles,
	CL_BubbleTrail,
	CL_BulletImpactParticles,
	CL_EntityParticles,
	CL_Explosion,
	CL_FizzEffect,
	CL_FireField,
	CL_FlickerParticles,
	CL_FunnelSprite,
	CL_Implosion,
	CL_Large_Funnel,
	CL_LavaSplash,
	CL_MultiGunshot,
	CL_MuzzleFlash,
	CL_ParticleBox,
	CL_ParticleBurst,
	CL_ParticleExplosion,
	CL_ParticleExplosion2,
	CL_ParticleLine,
	CL_PlayerSprites,
	CL_Projectile,
	CL_RicochetSound,
	CL_RicochetSprite,
	CL_RocketFlare,
	CL_RocketTrail,
	CL_RunParticleEffect,
	CL_ShowLine,
	CL_SparkEffect,
	CL_SparkShower,
	CL_SparkStreaks,
	CL_Spray,
	CL_Sprite_Explode,
	CL_Sprite_Smoke,
	CL_Sprite_Spray,
	CL_Sprite_Trail,
	CL_Sprite_WallPuff,
	CL_StreakSplash,
	CL_TracerEffect,
	CL_UserTracerParticle,
	CL_TracerParticles,
	CL_TeleportSplash,
	CL_TempSphereModel,
	CL_TempModel,
	CL_DefaultSprite,
	CL_TempSprite,
	CL_DecalIndex,
	CL_DecalIndexFromName,
	CL_DecalShoot,
	CL_AttachTentToPlayer,
	CL_KillAttachedTents,
	CL_BeamCirclePoints,
	CL_BeamEntPoint,
	CL_BeamEnts,
	CL_BeamFollow,
	CL_BeamKill,
	CL_BeamLightning,
	CL_BeamPoints,
	CL_BeamRing,
	CL_AllocDlight,
	CL_AllocElight,
	CL_TempEntAlloc,
	CL_TempEntAllocNoModel,
	CL_TempEntAllocHigh,
	CL_TempEntAllocCustom,
	CL_GetPackedColor,
	CL_LookupColor,
	CL_DecalRemoveAll,
	CL_FireCustomDecal,
};
