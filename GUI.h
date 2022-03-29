#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "imgui/kiero/kiero.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;

bool bIsReady = false;
bool bShowWindow = true;
bool init = false;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
 
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

static void InitStyle()
{
	ImGuiStyle* style = &ImGui::GetStyle();
	auto io = ImGui::GetIO();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	//io.Fonts->AddFontFromFileTTF("../data/Fonts/Ruda-Bold.ttf", 15.0f, &config);
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().GrabRounding = 4.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (bShowWindow) {
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		InitStyle();

		ImGui::SetNextWindowSize(ImVec2(535, 320), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowBgAlpha(0.935);

		const auto Flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		{
			ImGui::Begin("Uranium Menu", reinterpret_cast<bool*>(true), Flags);

			if (ImGui::BeginTabBar("")) {
				if (!bIsReady) { // we only show this when your in the lobby
					if (ImGui::BeginTabItem("Lobby")) {

						static char InputMap[512] = "";

						ImGui::InputText("##Map", InputMap, IM_ARRAYSIZE(InputMap));

						ImGui::SameLine();

						if (ImGui::Button("LoadMap")) {
							std::string MapAsString = (const char*)InputMap;

							if (MapAsString.empty()) {
								MapAsString = "Artemis_Terrain";
							}

							std::wstring MapAsWString = std::wstring(MapAsString.begin(), MapAsString.end());

							MapAsWString += L"?Game=/Game/Athena/Athena_GameMode.Athena_GameMode_C";

							playerControllerFunctions->SwitchLevel(MapAsWString.c_str());
							bIsReady = true;
						}

						//ImGui::NewLine();

						ImGui::Text("To load into a match, enter the name of the map\nyou want, then press \"F1\" after your\nloading bar is full!");

						ImGui::NewLine();

						ImGui::Text("List of maps:");
						ImGui::Text("Artemis_Terrain - Main battle royale island");
						ImGui::Text("Apollo_Papaya - Party Royale island");

						ImGui::EndTabItem();
					}
				}
				else { //Only show ingame
					if (ImGui::BeginTabItem("Game")) {
						if (ImGui::Button("Exit Game")) {
							exit(0);
						}

						static char BPToLoad[2048] = "";

						ImGui::InputText("##BP", BPToLoad, IM_ARRAYSIZE(BPToLoad));

						if (ImGui::Button("LoadBP"))
						{
							std::string BPToLoadAsString = (const char*)BPToLoad;

							StaticLoadObject(FindObject(crypt("Class /Script/Engine.BlueprintGeneratedClass")), nullptr, (std::wstring(BPToLoadAsString.begin(), BPToLoadAsString.end()).c_str()));
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Player")) {
						int Height = 60000;

						ImGui::InputInt("##Height", &Height);

						ImGui::SameLine();

						if (ImGui::Button("TeleportToSkydive")) {
							pawnFunctions->TeleportToSkydive(Height);
						}

						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("Weapons")) {

						{
							if (ImGui::CollapsingHeader("Inventory")) {
								static char InputWIDInv[512] = "";

								ImGui::InputText("##WIDInv", InputWIDInv, IM_ARRAYSIZE(InputWIDInv));

								ImGui::SameLine();

								if (ImGui::Button("EquipWeapon")) {
									std::string WIDInvAsString = (const char*)InputWIDInv;

									if (!WIDInvAsString.empty()) {
										auto Weapon = FindObject(WIDInvAsString + "." + WIDInvAsString);

										if (Weapon) {
											inventoryFunctions->AddItemToInventory(Weapon, 1);
										}
									}
								}
							}
						}

						{
							if (ImGui::CollapsingHeader("Pickups")) {
								static char InputWIDPickup[512] = "";

								ImGui::InputText("##WIDPickup", InputWIDPickup, IM_ARRAYSIZE(InputWIDPickup));

								ImGui::SameLine();

								if (ImGui::Button("SpawnPickup")) {
									std::string WIDPickupAsString = (const char*)InputWIDPickup;

									if (!WIDPickupAsString.empty()) {
										auto Weapon = FindObject(WIDPickupAsString + "." + WIDPickupAsString);

										if (Weapon) {
											pickupFunctions->SpawnPickup(Weapon, 1, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::TossedByPlayer);
										}
									}
								}
							}
						}

					}
				}

				if (ImGui::BeginTabItem("About")) {
					ImGui::Text("This project was created by Jacobb626 and M1.\nWith help from others such as Kemo, Dani, Ender and Fevers.\nIt is currently maintained by Jacobb626 and Fevers!");
				}

				ImGui::EndTabBar();
			}

			ImGui::End();

			ImGui::Render();

			pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}

	return oPresent(pSwapChain, SyncInterval, Flags);
}

void SetupGUI()
{
	auto renderingApi = Functions::GetRenderingApi();

	bool init_hook = false;

	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);

	return;
}