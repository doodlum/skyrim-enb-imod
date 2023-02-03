#pragma once

#include "ENB/ENBSeriesAPI.h"
#include <shared_mutex>

class IMODSeparation
{
public:
	static IMODSeparation* GetSingleton()
	{
		static IMODSeparation singleton;
		return &singleton;
	}

	static void InstallHooks()
	{
		Hooks::Install();
	}

	std::shared_mutex fileLock;

	bool        bEnableIMODPassthrough;
	std::string sImagespaceFormat;

	bool enbEnabled = false;

	RE::ImageSpaceBaseData proxyData{};
	RE::ImageSpaceBaseData backupData{};

	void LoadINI();
	void SaveINI();

	void UpdateENBParams();

	void Update();
	void UpdateENB();

	// ENB UI

	void RefreshUI();

protected:
	struct Hooks
	{
		struct Main_Reset__ImagespaceManager_SetBaseData
		{
			static void thunk(struct ImagespaceParams* imagespaceParams, RE::ImageSpaceBaseData* baseData)
			{
				GetSingleton()->proxyData = *baseData;
				GetSingleton()->Update();
				func(imagespaceParams, &GetSingleton()->proxyData);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct Sky_Update__ImagespaceManager_SetBaseData
		{
			static void thunk(struct ImagespaceParams* imagespaceParams, RE::ImageSpaceBaseData* baseData)
			{
				GetSingleton()->proxyData = *baseData;
				GetSingleton()->Update();
				func(imagespaceParams, &GetSingleton()->proxyData);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		struct PlayerCharacter_Load__ImagespaceManager_SetBaseData
		{
			static void thunk(struct ImagespaceParams* imagespaceParams, RE::ImageSpaceBaseData* baseData)
			{
				GetSingleton()->proxyData = *baseData;
				GetSingleton()->Update();
				func(imagespaceParams, &GetSingleton()->proxyData);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		static void Install()
		{
			stl::write_thunk_call<Main_Reset__ImagespaceManager_SetBaseData>(REL::VariantID(18253, 18647, 0x260C30).address() + REL::Relocate(0x58, 0xC0, 0x58));
			stl::write_thunk_call<Sky_Update__ImagespaceManager_SetBaseData>(REL::RelocationID(25682, 26229).address() + REL::Relocate(0x25F, 0x312));
			stl::write_thunk_call<PlayerCharacter_Load__ImagespaceManager_SetBaseData>(REL::VariantID(39657, 40744, 0x6DA3D0).address() + REL::Relocate(0x70C, 0x70C, 0x79C));
		}
	};

private:
	IMODSeparation()
	{
		LoadINI();
	};

	IMODSeparation(const IMODSeparation&) = delete;
	IMODSeparation(IMODSeparation&&) = delete;

	~IMODSeparation() = default;

	IMODSeparation& operator=(const IMODSeparation&) = delete;
	IMODSeparation& operator=(IMODSeparation&&) = delete;
};
