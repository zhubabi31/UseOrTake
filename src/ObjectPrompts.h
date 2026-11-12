// SPDX-License-Identifier: GPL-3.0-or-later
// SkyPrompt fork addition, 2026-11-12.
#pragma once

#include <SkyPrompt/API.hpp>
#undef ERROR

class ObjectPrompts :
	public REX::TSingleton<ObjectPrompts>,
	public SkyPromptAPI::PromptSink
{
public:
	[[nodiscard]] bool Initialize();
	[[nodiscard]] bool IsActive() const { return clientID != 0; }

	void Update();
	void Clear();

	[[nodiscard]] std::span<const SkyPromptAPI::Prompt> GetPrompts() const override;
	void ProcessEvent(SkyPromptAPI::PromptEvent a_event) const override;

private:
	[[nodiscard]] static RE::NiPointer<RE::TESObjectREFR> GetTarget();
	void HandleEvent(RE::FormID a_refID, SkyPromptAPI::EventID a_eventID, SkyPromptAPI::ActionID a_actionID,
		SkyPromptAPI::PromptEventType a_type);

	SkyPromptAPI::ClientID clientID{ 0 };
	SkyPromptAPI::EventID eventID{ 0 };
	RE::ObjectRefHandle target;
	std::array<std::string, 2> labels;
	std::vector<SkyPromptAPI::Prompt> prompts;
	bool crime{ false };
};
