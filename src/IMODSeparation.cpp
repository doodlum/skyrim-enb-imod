#include "IMODSeparation.h"

#include <SimpleIni.h>

extern ENB_API::ENBSDKALT1002* g_ENB;

#define GetSettingFloat(a_section, a_setting) a_setting = (float)ini.GetDoubleValue(a_section, #a_setting, a_setting);
#define SetSettingFloat(a_section, a_setting) ini.SetDoubleValue(a_section, #a_setting, a_setting);

#define GetSettingBool(a_section, a_setting) a_setting = ini.GetBoolValue(a_section, #a_setting, a_setting);
#define SetSettingBool(a_section, a_setting) ini.SetBoolValue(a_section, #a_setting, a_setting);

void IMODSeparation::LoadINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();
	ini.LoadFile(L"enbseries\\enbimodseparator.ini");

	bEnableIMODSeparation = ini.GetBoolValue("Settings", "bEnableIMODSeparation", false);
	sImagespaceFormat = ini.GetValue("Settings", "sImagespaceFormat", "Imagespace - {}");
}

void IMODSeparation::SaveINI()
{
	std::lock_guard<std::shared_mutex> lk(fileLock);
	CSimpleIniA                        ini;
	ini.SetUnicode();

	ini.SetBoolValue("Settings", "bEnableIMODSeparation", bEnableIMODSeparation);
	ini.SetValue("Settings", "sImagespaceFormat", sImagespaceFormat.c_str());

	ini.SaveFile(L"enbseries\\enbimodseparator.ini");
}

template <typename... Args>
std::string dyna_print(std::string_view rt_fmt_str, Args&&... args)
{
	return std::vformat(rt_fmt_str, std::make_format_args(args...));
}

void IMODSeparation::UpdateENBParams()
{
	ENB_SDK::ENBParameter param;

	param.Type = ENB_SDK::ENBParameterType::ENBParam_FLOAT;
	param.Size = ENBParameterTypeToSize(param.Type);

	memcpy(param.Data, &backupData.hdr.bloomScale, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Bloom Scale").c_str(), &param);

	memcpy(param.Data, &backupData.hdr.bloomThreshold, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Bloom Threshold").c_str(), &param);

	memcpy(param.Data, &backupData.hdr.white, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "White").c_str(), &param);

	memcpy(param.Data, &backupData.cinematic.brightness, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Brightness").c_str(), &param);

	memcpy(param.Data, &backupData.cinematic.contrast, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Contrast").c_str(), &param);

	memcpy(param.Data, &backupData.cinematic.saturation, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Saturation").c_str(), &param);

	memcpy(param.Data, &backupData.tint.amount, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Tint Amount").c_str(), &param);

	param.Type = ENB_SDK::ENBParameterType::ENBParam_COLOR3;
	param.Size = ENBParameterTypeToSize(param.Type);

	RE::NiColor color { backupData.tint.color.red, backupData.tint.color.green, backupData.tint.color.blue };
	memcpy(param.Data, &color, param.Size);
	g_ENB->SetParameter(NULL, "ENBEFFECT.FX", dyna_print(sImagespaceFormat, "Tint Color").c_str(), &param);
}


void IMODSeparation::Update()
{
	//static RE::ImageSpaceBaseData& baseData = (*(RE::ImageSpaceBaseData*)RELOCATION_ID(514970, 514970).address());
	backupData = proxyData;

	if (enbEnabled && bEnableIMODSeparation) {
		proxyData.hdr.bloomScale = 1;
		proxyData.hdr.bloomThreshold = 1;
		proxyData.hdr.white = 1;

		proxyData.cinematic.brightness = 1;
		proxyData.cinematic.contrast = 1;
		proxyData.cinematic.saturation = 1;

		proxyData.tint.amount = 0;
		proxyData.tint.color.blue = 1;
		proxyData.tint.color.green = 1;
		proxyData.tint.color.red = 1;
	}
}

void IMODSeparation::UpdateENB()
{
	if (g_ENB) {
		BOOL         res;
		ENBParameter param;
		res = g_ENB->GetParameter("enbseries.ini", "GLOBAL", "UseEffect", &param);
		if ((res == TRUE) && (param.Type == ENB_SDK::ENBParameterType::ENBParam_BOOL)) {
			BOOL bvalue = FALSE;
			memcpy(&bvalue, param.Data, 4);
			enbEnabled = bvalue;
			if (enbEnabled) {
				UpdateENBParams();
			}
			return;
		}
	}
	enbEnabled = false;
}

#define TWDEF "group='MOD:IMOD Separator'"

void IMODSeparation::RefreshUI()
{
	auto bar = g_ENB->TwGetBarByEnum(!REL::Module::IsVR() ? ENB_API::ENBWindowType::EditorBarEffects : ENB_API::ENBWindowType::EditorBarObjects);  // ENB misnames its own bar, whoops!
	g_ENB->TwAddVarRW(bar, "EnableIMODSeparation", ETwType::TW_TYPE_BOOLCPP, &bEnableIMODSeparation, TWDEF);
	g_ENB->TwAddVarRW(bar, "ImagespaceFormat", ETwType::TW_TYPE_STDSTRING, &sImagespaceFormat, TWDEF " readonly = true");
	g_ENB->TwDefine("EditorBarEffects/'MOD:IMOD Separator' opened=false");
}
