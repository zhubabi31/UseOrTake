// SPDX-License-Identifier: GPL-3.0-or-later
// SkyPrompt fork addition, 2026-11-12.
#include "ObjectPrompts.h"

#include "Manager.h"

bool ObjectPrompts::Initialize()
{
	if (!REX::W32::GetModuleHandleW(L"SkyPrompt.dll")) {
		REX::WARN("SkyPrompt is not installed; using legacy modifier-key actions");
		return false;
	}

	clientID = SkyPromptAPI::RequestClientID();
	if (!clientID) {
		REX::ERROR("SkyPrompt API 2.0 registration failed; using legacy modifier-key actions");
		return false;
	}

	REX::INFO("SkyPrompt object actions enabled; vanilla activation is unchanged");
	return true;
}

RE::NiPointer<RE::TESObjectREFR> ObjectPrompts::GetTarget()
{
	const auto ui = RE::UI::GetSingleton();
	const auto controls = RE::ControlMap::GetSingleton();
	const auto playerControls = RE::PlayerControls::GetSingleton();
	// Input handler fields are stable across the 1.6.1170 / 1.7.104 player layout change.
	const auto activateHandler = playerControls ? playerControls->GetActivateHandler() : nullptr;
	const auto player = RE::PlayerCharacter::GetSingleton();
	if (!ui || !ui->IsMenuOpen(RE::HUDMenu::MENU_NAME) || !ui->IsShowingMenus() ||
		ui->GameIsPaused() || ui->IsApplicationMenuOpen() || ui->IsItemMenuOpen() ||
		ui->IsModalMenuOpen() || ui->IsMenuOpen(RE::Console::MENU_NAME) ||
		!controls || !controls->IsActivateControlsEnabled() || controls->textEntryCount > 0 ||
		!playerControls || playerControls->blockPlayerInput ||
		!activateHandler || activateHandler->disabled || activateHandler->heldStateActive ||
		!player || player->IsDead()) {
		return nullptr;
	}

	const auto crosshairs = RE::CrosshairPickData::GetSingleton();
	const auto ref = crosshairs ? crosshairs->target.get() : nullptr;
	if (!ref || ref->IsDeleted() || ref->IsDisabled() || !ref->Is3DLoaded() || ref->IsActivationBlocked()) {
		return nullptr;
	}

	const auto base = ref->GetBaseObject();
	const auto action = base ? Manager::GetSingleton()->GetActionForType(base->GetFormType()) : nullptr;
	if (!action || !action->IsEnabled()) {
		return nullptr;
	}
	if (const auto light = base->As<RE::TESObjectLIGH>(); light && !light->CanBeCarried()) {
		return nullptr;
	}

	return ref;
}

void ObjectPrompts::Update()
{
	const auto ref = GetTarget();
	if (!ref) {
		Clear();
		return;
	}

	const auto isCrime = ref->IsCrimeToActivate();
	if (target == ref->GetHandle() && crime == isCrime) {
		return;
	}

	Clear();
	target = ref->GetHandle();
	crime = isCrime;
	const auto base = ref->GetBaseObject();
	label = Manager::GetSingleton()->GetActionForType(base->GetFormType())->GetUseLabel(base);
	if (label.empty()) {
		REX::WARN("Empty alternate action label for {:08X}; no SkyPrompt submitted", base->GetFormID());
		return;
	}

	++eventID;
	prompts.emplace_back(label, eventID, 0, SkyPromptAPI::kSinglePress, ref->GetFormID(),
		std::span<const std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>>{},
		crime ? 0xFF0000FF : 0xFFFFFFFF);
	if (!SkyPromptAPI::SendPrompt(this, clientID)) {
		REX::ERROR("Failed to submit SkyPrompt for {:08X}", ref->GetFormID());
	}
}

void ObjectPrompts::Clear()
{
	if (!prompts.empty()) {
		SkyPromptAPI::RemovePrompt(this, clientID);
		prompts.clear();
	}
	target.reset();
	label.clear();
}

std::span<const SkyPromptAPI::Prompt> ObjectPrompts::GetPrompts() const
{
	return prompts;
}

void ObjectPrompts::ProcessEvent(SkyPromptAPI::PromptEvent a_event) const
{
	if (a_event.type != SkyPromptAPI::kAccepted || a_event.prompt.actionID != 0) {
		return;
	}

	// SkyPrompt callbacks may run off-thread; revalidate the target on the game thread.
	SKSE::GetTaskInterface()->AddTask([refID = a_event.prompt.refid, id = a_event.prompt.eventID]() {
		GetSingleton()->Accept(refID, id);
	});
}

void ObjectPrompts::Accept(RE::FormID a_refID, SkyPromptAPI::EventID a_eventID)
{
	const auto ref = GetTarget();
	if (prompts.empty() || a_eventID != eventID || !ref ||
		ref->GetFormID() != a_refID || target != ref->GetHandle()) {
		return;
	}

	const auto player = RE::PlayerCharacter::GetSingleton();
	const auto base = ref->GetBaseObject();
	const auto countBefore = player->GetItemCount(base);
	const auto count = ref->extraList.GetCount();
	Clear();

	// Use normal activation for scripts, ownership/crime, and pickup restrictions.
	if (!ref->ActivateRef(player, 0, nullptr, count, false) || player->GetItemCount(base) <= countBefore) {
		REX::WARN("Alternate action skipped: activation did not add {:08X} to the inventory", base->GetFormID());
		return;
	}

	Action::Perform(player, base, base->Is(RE::FormType::Scroll));
}
