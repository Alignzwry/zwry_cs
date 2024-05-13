#include "memory/memory.h"
#include <cstdlib>
#include "a2x/client.dll.hpp"
#include "a2x/offsets.hpp"
#include "a2x/buttons.hpp"
#include "util/util.h"
#include <d2d1.h>
#include <dwrite.h>
#include "driver\Kernelinterface.hpp"
#include <filesystem>
#pragma once;
using namespace memory;
class cs2;

class cs2 {
public:
	DWORD pID;
	LocalPl LocalPlayer;
	KernelInterface Driver = KernelInterface("\\\\.\\RWDriver");;
	DWORD64 base_address, EntityListEntry = 0, EntityPawnAddress = 0, EntityPawnListEntry = 0, GameSceneNode = 0, BoneArrayAddress = 0;
	DWORD pawn = 0;
	float Matrix[4][4]{};
	HWND counter_strike_hwnd;
	Vec2 screen;
	std::vector<Entity> BonePosList;

	void Init() {
		if (Driver.hDriver == INVALID_HANDLE_VALUE)
		{
			Driver.~KernelInterface();
			std::filesystem::path Path = std::filesystem::current_path() / "start.exe";
			std::string pathStr = Path.generic_string();
			if (!std::filesystem::exists(Path)) {
				cout << "could not find exe!" << endl;
				Sleep(10000);
				exit(-1);
				return;
			}
			SHELLEXECUTEINFO ShExecInfo = { 0 };
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.fMask = SEE_MASK_DEFAULT;
			ShExecInfo.hwnd = NULL;
			ShExecInfo.lpVerb = "runas";
			ShExecInfo.lpFile = pathStr.c_str();
			ShExecInfo.lpParameters = "";
			ShExecInfo.lpDirectory = NULL;
			ShExecInfo.nShow = SW_NORMAL;
			ShExecInfo.hInstApp = NULL;

			BOOL result = ShellExecuteEx(&ShExecInfo);
			if (!result) {
				std::cout << "Error launching the process." << std::endl;
				exit(-1);
			}
			exit(1);
		}
		pID = getPId("cs2.exe");
		base_address = GetModule(pID, "client.dll");
		counter_strike_hwnd = FindWindow(TEXT("SDL_app"), TEXT("Counter-Strike 2"));
		if (!counter_strike_hwnd)
			exit(1);
		ReadMemory<DWORD64>(base_address + cs2_dumper::offsets::client_dll::dwEntityList, EntityListEntry);
		ReadMemory<DWORD64>(EntityListEntry + 0x10, EntityListEntry);
	}

	~cs2() {
		Driver.~KernelInterface();
	}

	void UpdateLocalPlayer() {
		if (!ReadMemory<DWORD64>((DWORD64)(base_address + cs2_dumper::offsets::client_dll::dwLocalPlayerController), LocalPlayer.ControllerAddress))
			return;
		if (!ReadMemory(base_address + cs2_dumper::offsets::client_dll::dwViewMatrix, Matrix, 64))
			return;
		if (!ReadMemory<int>((DWORD64)(LocalPlayer.ControllerAddress + cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive), LocalPlayer.alive))
			return;
		if (!ReadMemory<DWORD>((DWORD64)(LocalPlayer.ControllerAddress + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn), LocalPlayer.pawn))
			return;
		if (!ReadMemory<DWORD64>(base_address + cs2_dumper::offsets::client_dll::dwEntityList, LocalPlayer.EntityPawnListEntry))
			return;
		if (!ReadMemory<DWORD64>(LocalPlayer.EntityPawnListEntry + 0x10 + 8 * ((LocalPlayer.pawn & 0x7FFF) >> 9), LocalPlayer.EntityPawnListEntry))
			return;
		if (!ReadMemory<DWORD64>(LocalPlayer.EntityPawnListEntry + 0x78 * (LocalPlayer.pawn & 0x1FF), LocalPlayer.PawnAddress))
			return;
		ReadMemory<int>(LocalPlayer.PawnAddress + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum, LocalPlayer.teamID);
		ReadMemory<Vec2>(base_address + cs2_dumper::offsets::client_dll::dwViewAngles, LocalPlayer.ViewAngle);
	}

	void update() {
		if (!ReadMemory<DWORD64>(LocalPlayer.PawnAddress + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode, GameSceneNode))
			return;
		if (!ReadMemory<DWORD64>(GameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecOrigin, BoneArrayAddress))
			return;
		BoneJointData BoneArray;
		if (!ReadMemory(BoneArrayAddress + sizeof(BoneJointData) * 6, BoneArray, sizeof(BoneJointData)))
			return;
		BonePosList.clear();
		for (int i = 0; i < 64; i++) {
			ReadMemory<DWORD64>(base_address + cs2_dumper::offsets::client_dll::dwEntityList, EntityListEntry);
			ReadMemory<DWORD64>(EntityListEntry + 0x10, EntityListEntry);
			DWORD64 EntityAddress = 0;
			DWORD64 PawnAddress = 0;
			ReadMemory<DWORD64>(EntityListEntry + (i + 1) * 0x78, EntityAddress);

			if (EntityAddress == LocalPlayer.ControllerAddress)
				continue;
			Entity e;
			if (EntityAddress == 0)
				continue;
			if (!ReadMemory<int>((DWORD64)(EntityAddress + cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive), e.alive))
				continue;
			if (!ReadMemory((DWORD64)(EntityAddress + cs2_dumper::schemas::client_dll::CBasePlayerController::m_iszPlayerName), e.name, MAX_PATH))
				continue;
			if (!e.alive)
				continue;
			EntityPawnListEntry = 0;
			EntityPawnAddress = 0;
			pawn = 0;
			if (!ReadMemory<DWORD>((DWORD64)(EntityAddress + cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn), pawn))
				continue;
			if (!ReadMemory<DWORD64>(base_address + cs2_dumper::offsets::client_dll::dwEntityList, EntityPawnListEntry))
				continue;
			if (!ReadMemory<DWORD64>(EntityPawnListEntry + 0x10 + 8 * ((pawn & 0x7FFF) >> 9), EntityPawnListEntry))
				continue;
			if (!ReadMemory<DWORD64>(EntityPawnListEntry + 0x78 * (pawn & 0x1FF), EntityPawnAddress))
				continue;
			ReadMemory<int>(EntityPawnAddress + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum, e.teamID);
			ReadMemory<int>(EntityPawnAddress + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth, e.health);
			ReadMemory<int>(EntityPawnAddress + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iMaxHealth, e.maxHealth);

			if (!ReadMemory<DWORD64>(EntityPawnAddress + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode, GameSceneNode))
				continue;
			if (!ReadMemory<DWORD64>(GameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecOrigin, BoneArrayAddress))
				continue;

			constexpr size_t NUM_BONES = 30;
			BoneJointData BoneArray[NUM_BONES]{};
			if (!ReadMemory(BoneArrayAddress, BoneArray, NUM_BONES * sizeof(BoneJointData)))
				continue;

			for (const auto& bone : BoneArray) {
				Vec2 ScreenPos;
				bool IsVisible = false;
				if (WorldToScreen(bone.Pos, ScreenPos, Matrix, screen)) {
					IsVisible = true;
				}
				int health = 0;

				e.bones.push_back({ ScreenPos, IsVisible, health });

			}
			BoneArray[28].Pos.z -= 0.5;
			WorldToScreen(BoneArray[28].Pos, e.namePos, Matrix, screen);
			BonePosList.push_back(e);

		}
	}

	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		ReadType Buffer = {};
		DWORD Bytes;

		KERNEL_READ_REQUEST ReadReq;
		ReadReq.ProcessId = pID;
		ReadReq.Address = Address;
		ReadReq.pBuff = &Value;
		ReadReq.Size = Size;

		if (DeviceIoControl(Driver.hDriver, IO_READ_REQUEST, &ReadReq, sizeof(ReadReq), &ReadReq, sizeof(ReadReq), &Bytes, nullptr))
		{
			return true;
		}
		std::cerr << "ReadVirtualMemory failed: " << GetLastError() << std::endl;
		return false;
	}

	template <typename ReadType>
	bool ReadMemory(DWORD64 Address, ReadType& Value)
	{
		ReadType Buffer = {};
		KERNEL_READ_REQUEST ReadReq;
		ReadReq.ProcessId = pID;
		ReadReq.Address = Address;
		ReadReq.pBuff = &Value;
		ReadReq.Size = sizeof(ReadType);

		DWORD Bytes;
		if (DeviceIoControl(Driver.hDriver, IO_READ_REQUEST, &ReadReq, sizeof(ReadReq), &ReadReq, sizeof(ReadReq), &Bytes, nullptr))
		{
			return true;
		}
		std::cerr << "ReadVirtualMemory failed: " << GetLastError() << std::endl;
		return false;
	}

	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value, int Size)
	{
		DWORD Bytes;
		KERNEL_WRITE_REQUEST WriteReq;
		WriteReq.ProcessId = pID;
		WriteReq.Address = Address;
		WriteReq.pBuff = &Value;
		WriteReq.Size = Size;

		if (DeviceIoControl(Driver.hDriver, IO_WRITE_REQUEST, &WriteReq, sizeof(WriteReq), nullptr, 0, &Bytes, nullptr))
		{
			return true;
		}
		std::cerr << "WriteVirtualMemory failed: " << GetLastError() << std::endl;
		return false;
	}

	template <typename ReadType>
	bool WriteMemory(DWORD64 Address, ReadType& Value)
	{
		DWORD Bytes;
		KERNEL_WRITE_REQUEST WriteReq;
		WriteReq.ProcessId = pID;
		WriteReq.Address = Address;
		WriteReq.pBuff = &Value;
		WriteReq.Size = sizeof(ReadType);

		if (DeviceIoControl(Driver.hDriver, IO_WRITE_REQUEST, &WriteReq, sizeof(WriteReq), nullptr, 0, &Bytes, nullptr))
		{
			return true;
		}
		std::cerr << "WriteVirtualMemory failed: " << GetLastError() << std::endl;
		return false;
	}
};