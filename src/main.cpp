// Modified for the SkyPrompt fork, 2026-11-12.
#include "Hooks.h"
#include "Manager.h"
#include "ObjectPrompts.h"

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SKSE::MessagingInterface::kPostLoad:
		{
			Manager::GetSingleton()->LoadSettings();
		}
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		if (!Manager::GetSingleton()->UseSkyPrompt() || !ObjectPrompts::GetSingleton()->Initialize()) {
			Hooks::Install();
		}
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		Manager::Register();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		ObjectPrompts::GetSingleton()->Clear();
		break;
	default:
		break;
	}
}

#ifdef SKYRIM_SUPPORT_AE
SKSE_PLUGIN_VERSION = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(REL::Version{ Version::MAJOR, Version::MINOR, Version::PATCH });
	v.PluginName("Use Or Take");
	v.AuthorName("powerofthree");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_1_6_1170, SKSE::RUNTIME_SSE_1_6_1179, SKSE::RUNTIME_SSE_1_7_104 });
	v.MinimumRequiredXSEVersion(REL::Version{ 2, 2, 6 });

	return v;
}();
#else
SKSE_PLUGIN_QUERY(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "Use Or Take";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		REX::CRITICAL("Loaded in editor, marking as incompatible");
		return false;
	}

	if (const auto ver = a_skse->RuntimeVersion(); ver < SKSE::RUNTIME_SSE_1_5_39) {
		REX::CRITICAL("Unsupported runtime version {}", ver);
		return false;
	}

	return true;
}
#endif

SKSE_PLUGIN_LOAD(const SKSE::LoadInterface* a_skse)
{
	SKSE::Init(a_skse, { .log = true,
						   .logName = Version::PROJECT.data() });

	auto runtimeVersion = a_skse->RuntimeVersion();

	REX::INFO("Game version : {}", runtimeVersion);

#ifdef SKYRIM_SUPPORT_AE
	if (runtimeVersion != SKSE::RUNTIME_SSE_1_6_1170 &&
		runtimeVersion != SKSE::RUNTIME_SSE_1_6_1179 &&
		runtimeVersion != SKSE::RUNTIME_SSE_1_7_104) {
		REX::FAIL(
			"Unsupported Skyrim runtime for {0}: {1}\n"
			"Supported: 1.6.1170 (Steam), 1.6.1179 (GOG), 1.7.104 (Steam).\n"
			"Install SKSE64 and Address Library matching your game version.",
			Version::PROJECT, runtimeVersion);
	}
#endif

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageHandler);

	return true;
}
