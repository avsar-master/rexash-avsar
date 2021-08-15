/*
imgui_lcsm_warning.cpp
Copyright (C) 2020 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "imgui_lcsm_warning.h"
#include "imgui.h"
#include "imgui_utils.h"

static bool lcsm_enabled = true;

void ImGui_LCSM_OnGUI(void)
{
	if (!lcsm_enabled)
		return;

	ImGuiUtils::CenterNextWindow(ImGuiCond_Always);
	ImGui::OpenPopup("Top Text");
	if (ImGui::BeginPopupModal("Top Text", &lcsm_enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("");
		ImColor warning_color(1.0f, 0.2f, 0.2f, 1.0f);
		ImGui::TextColored(warning_color, "-----------------------------------特别声明---------------------------------");
		ImGui::TextColored(warning_color, "1.联盟戦亡選機稿 [L9 LIE DETECTOR TECHNOLOGY ] PRESENTS.... NEW RATEARL LYING SCRIPT 100% UNDETECTED");
		ImGui::TextColored(warning_color, "----------------------------------------------------------------------------");

		ImGui::NewLine();
		ImGui::SameLine(ImGui::GetWindowSize().x * 1 / 4 - 80);
		if (ImGui::Button("Bottom Text", ImGuiUtils::GetScaledSize({ 160, 36 }))) {
			lcsm_enabled = false;
		}
		ImGui::SameLine(ImGui::GetWindowSize().x / 2 + 80);
		if (ImGui::Button("Bottom Text", ImGuiUtils::GetScaledSize({ 160, 36 }))) {
			exit(0);
		}
		ImGui::EndPopup();
	}
}