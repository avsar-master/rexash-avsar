#include <vector>
#include <string.h>

#include "vgui_int.h"
#include "cdll_int.h"
#include "menu_int.h"

#include "entity_state.h"
#include "usercmd.h"
#include "ref_params.h"
#include "cl_entity.h"
#include "cdll_exp.h"

#include "BaseUI/IBaseUI.h"
#include "BaseUISurface.h"

#include "public/FileSystem.h"
#include "APIProxy.h"

cldll_func_t	gClDllFuncs;
extern cl_enginefuncs_s cl_enginefuncs;
extern BaseUISurface* staticSurface;


int UI_VidInit(void);
void VGui_Startup(int width, int height);


namespace ClientDLL_Hooks {
	int VidInit()
	{
		SCREENINFO screeninfo;
		screeninfo.iSize = sizeof(SCREENINFO);
		cl_enginefuncs.pfnGetScreenInfo(&screeninfo);
		staticSurface->SetScreenInfo(&screeninfo);

		return gClDllFuncs.pfnVidInit();
	}

	int Initialize(cl_enginefunc_t* pEnginefuncs, int iVersion) {

		SCREENINFO screeninfo;
		screeninfo.iSize = sizeof(SCREENINFO);
		cl_enginefuncs.pfnGetScreenInfo(&screeninfo);
		VGui_Startup(screeninfo.iWidth, screeninfo.iHeight);
		return 1;
	}
	void HUD_ChatInputPosition(int* x, int* y) {
		if (gClDllFuncs.pfnChatInputPosition)
			gClDllFuncs.pfnChatInputPosition(x, y);
	}

	int HUD_GetPlayerTeam(int iplayer) {
		if (gClDllFuncs.pfnGetPlayerTeam) {
			return gClDllFuncs.pfnGetPlayerTeam(iplayer);
		}

		return 0;
	}

	void* GetClientFactory() {
		if (gClDllFuncs.pfnGetClientFactory) {
			return gClDllFuncs.pfnGetClientFactory();
		}

		return 0;
	}

} // namespace ClientDLL_Hooks

void LoadFFuncs(void* pv) {
	*reinterpret_cast<cldll_func_t*>(pv) = {
		ClientDLL_Hooks::Initialize,
		[] { gClDllFuncs.pfnInit(); },
		ClientDLL_Hooks::VidInit,
		[](float flTime, int intermission) { return gClDllFuncs.pfnRedraw(flTime, intermission); },
		[](client_data_t* cdata, float flTime) { return gClDllFuncs.pfnUpdateClientData(cdata, flTime); },
		[] { gClDllFuncs.pfnReset(); },
		[](struct playermove_s* ppmove, int server) { gClDllFuncs.pfnPlayerMove(ppmove, server); },
		[](struct playermove_s* ppmove) { gClDllFuncs.pfnPlayerMoveInit(ppmove); },
		[](char* name) { return gClDllFuncs.pfnPlayerMoveTexture(name); },
		[] { gClDllFuncs.IN_ActivateMouse(); },
		[] { gClDllFuncs.IN_DeactivateMouse(); },
		[](int mstate) { gClDllFuncs.IN_MouseEvent(mstate); },
		[] { gClDllFuncs.IN_ClearStates(); },
		[] { gClDllFuncs.IN_Accumulate(); },
		[](float frametime, struct usercmd_s* cmd, int active) { gClDllFuncs.CL_CreateMove(frametime, cmd, active); },
		[] { return gClDllFuncs.CL_IsThirdPerson(); },
		[](float* ofs) { return gClDllFuncs.CL_CameraOffset(ofs); },
		[](const char* name) { return gClDllFuncs.KB_Find(name); },
		[] { return gClDllFuncs.CAM_Think(); },
		[](ref_params_t* pparams) { gClDllFuncs.pfnCalcRefdef(pparams); },
		[](int type, cl_entity_t* ent, const char* modelname) { return gClDllFuncs.pfnAddEntity(type, ent, modelname); },
		[] { gClDllFuncs.pfnCreateEntities(); },
		[] { gClDllFuncs.pfnDrawNormalTriangles(); },
		[] { gClDllFuncs.pfnDrawTransparentTriangles(); },
		[](const struct mstudioevent_s* event, const cl_entity_t* entity) { gClDllFuncs.pfnStudioEvent(event, entity); },
		[](struct local_state_s* from, struct local_state_s* to, usercmd_t* cmd, int runfuncs, double time, unsigned int random_seed) { gClDllFuncs.pfnPostRunCmd(from, to, cmd, runfuncs, time, random_seed); },
		[] { gClDllFuncs.pfnShutdown(); },
		[](entity_state_t* state, const clientdata_t* client) { gClDllFuncs.pfnTxferLocalOverrides(state, client); },
		[](entity_state_t* dst, const entity_state_t* src) { gClDllFuncs.pfnProcessPlayerState(dst, src); },
		[](entity_state_t* ps, const entity_state_t* pps, clientdata_t* pcd, const clientdata_t* ppcd, weapon_data_t* wd, const weapon_data_t* pwd) { gClDllFuncs.pfnTxferPredictionData(ps, pps, pcd, ppcd, wd, pwd); },
		[](int size, byte* buffer) { gClDllFuncs.pfnDemo_ReadBuffer(size, buffer); },
		[](const struct netadr_s* net_from, const char* args, char* buffer, int* size) { return gClDllFuncs.pfnConnectionlessPacket(net_from, args, buffer, size); },
		[](int hullnumber, float* mins, float* maxs) { return gClDllFuncs.pfnGetHullBounds(hullnumber, mins, maxs); },
		[](double time) { gClDllFuncs.pfnFrame(time); },
		[](int eventcode, int keynum, const char* pszCurrentBinding) { return gClDllFuncs.pfnKey_Event(eventcode, keynum, pszCurrentBinding); },
		[](double frametime, double client_time, double cl_gravity, struct tempent_s** ppTempEntFree, struct tempent_s** ppTempEntActive, int(*Callback_AddVisibleEntity)(cl_entity_t* pEntity), void(*Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp)) { gClDllFuncs.pfnTempEntUpdate(frametime, client_time, cl_gravity, ppTempEntFree, ppTempEntActive, Callback_AddVisibleEntity, Callback_TempEntPlaySound); },
		[](int index) { return gClDllFuncs.pfnGetUserEntity(index); },
		[](int entindex, qboolean bTalking) { gClDllFuncs.pfnVoiceStatus(entindex, bTalking); },
		[](int iSize, void* pbuf) { gClDllFuncs.pfnDirectorMessage(iSize, pbuf); },
		[](int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio) { return gClDllFuncs.pfnGetStudioModelInterface(version, ppinterface, pstudio); },
		ClientDLL_Hooks::HUD_ChatInputPosition,
		ClientDLL_Hooks::HUD_GetPlayerTeam,
		ClientDLL_Hooks::GetClientFactory,
	};
}