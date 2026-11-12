// Modified for the SkyPrompt fork, 2026-11-12.
#include "Action.h"

Action::Action(std::string_view a_section, std::string a_label) :
	enabled(a_section, "Enabled"sv, "bEnabled"sv, true),
	action(a_section, "Default action"sv, "iDefaultAction"sv, kTake),
	primaryActionLabel(a_section, "Alternate action label"sv, "sAlternateActionLabel"sv, std::move(a_label))
{}

std::string Action::get_take_label(RE::TESObjectREFR* a_activator)
{
	constexpr auto get_take_or_steal = [](RE::TESObjectREFR* b_activator) {
		return b_activator->IsCrimeToActivate() ? *"sSteal"_gs :
		                                          *"sTake"_gs;
	};

	if (const auto count = a_activator->extraList.GetCount(); count > 1) {
		return std::format("{}\n{} ({})", get_take_or_steal(a_activator), a_activator->GetDisplayFullName(), count);
	} else {
		return std::format("{}\n{}", get_take_or_steal(a_activator), a_activator->GetDisplayFullName());
	}
}

std::string Action::get_alt_action_label(RE::TESObjectREFR* a_activator, std::string_view a_label)
{
	const auto count = a_activator->extraList.GetCount();
	const auto name = a_activator->GetDisplayFullName();

	if (a_activator->IsCrimeToActivate()) {
		return count > 1 ? std::format("<font color='#FF0000'>{}</font>\n{} ({})", a_label, name, count) :
		                   std::format("<font color='#FF0000'>{}</font>\n{}", a_label, name);
	}
	return count > 1 ? std::format("{}\n{} ({})", a_label, name, count) :
	                   std::format("{}\n{}", a_label, name);
}

std::string Action::GetActionLabel(RE::TESObjectREFR* a_activator, RE::TESBoundObject*, bool a_keyPressed, bool) const
{
	if (GetDefaultAction() == kTake ? a_keyPressed : !a_keyPressed) {
		return get_alt_action_label(a_activator, GetPrimaryLabel());
	}
	return get_take_label(a_activator);
}

const std::string& Action::GetUseLabel(RE::TESBoundObject*) const
{
	return GetPrimaryLabel();
}

void Action::Perform(RE::Actor* a_actor, RE::TESBoundObject* a_base, bool a_secondary)
{
	if (!a_secondary) {
		RE::ActorEquipManager::GetSingleton()->EquipObject(a_actor, a_base);
		return;
	}

	switch (a_base->GetFormType()) {
	case RE::FormType::Scroll:
		if (const auto scroll = a_base->As<RE::ScrollItem>()) {
			if (const auto caster = a_actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)) {
				caster->CastSpellImmediate(scroll, false, a_actor, 1.0f, false, 0.0f, nullptr);
				a_actor->RemoveItem(scroll, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
			} else {
				REX::WARN("Cannot cast scroll {:08X}: actor has no instant spell caster", scroll->GetFormID());
			}
		}
		break;
	case RE::FormType::Weapon:
		RE::ActorEquipManager::GetSingleton()->EquipObject(a_actor, a_base);
		if (const RE::ActorState* actorState = a_actor->As<RE::ActorState>(); actorState && !actorState->IsWeaponDrawn()) {
			a_actor->DrawWeaponMagicHands(true);
		}
		break;
	default:
		break;
	}
}

SecondaryAction::SecondaryAction(std::string_view a_section, std::string a_label, std::string a_secondaryLabel) :
	Action(a_section, std::move(a_label)),
	secondaryActionLabel(a_section, "Alternate secondary action label"sv, "sAlternateSecondaryActionLabel"sv, std::move(a_secondaryLabel))
{}

std::string SecondaryAction::GetActionLabel(RE::TESObjectREFR* a_activator, RE::TESBoundObject*, bool a_keyPressed, bool a_keyHeld) const
{
	if (a_keyHeld) {
		return get_alt_action_label(a_activator, static_cast<const std::string&>(secondaryActionLabel));
	}
	if (GetDefaultAction() == kTake ? a_keyPressed : !a_keyPressed) {
		return get_alt_action_label(a_activator, GetPrimaryLabel());
	}
	return get_take_label(a_activator);
}

const std::string& SecondaryAction::GetUseLabel(RE::TESBoundObject* a_base) const
{
	return a_base->Is(RE::FormType::Scroll) ? static_cast<const std::string&>(secondaryActionLabel) : GetPrimaryLabel();
}

AlchemyAction::AlchemyAction(std::string_view a_section, std::string a_label, std::string a_foodLabel, std::string a_poisonLabel) :
	Action(a_section, std::move(a_label)),
	foodActionLabel(a_section, "Alternate action label (Food)"sv, "sAlternateActionLabelFood"sv, std::move(a_foodLabel)),
	poisonActionLabel(a_section, "Alternate action label (Poison)"sv, "sAlternateActionLabelPoison"sv, std::move(a_poisonLabel))
{}

std::string AlchemyAction::GetActionLabel(RE::TESObjectREFR* a_activator, RE::TESBoundObject* a_base, bool a_keyPressed, bool) const
{
	if (GetDefaultAction() == kTake ? a_keyPressed : !a_keyPressed) {
		return get_alt_action_label(a_activator, GetLabelForAlchType(a_base));
	}
	return get_take_label(a_activator);
}

const std::string& AlchemyAction::GetUseLabel(RE::TESBoundObject* a_base) const
{
	return GetLabelForAlchType(a_base);
}

const std::string& AlchemyAction::GetLabelForAlchType(RE::TESBoundObject* a_base) const
{
	if (const auto alch = a_base ? a_base->As<RE::AlchemyItem>() : nullptr) {
		if (alch->IsFood()) {
			if (const auto useSound = alch->data.consumptionSound; useSound && useSound->GetFormID() == 0x000B6435) {  // ITMPotionUse
				return GetPrimaryLabel();                                                                              // Drink
			}
			return static_cast<const std::string&>(foodActionLabel);  // Eat
		}
		if (alch->IsPoison()) {
			return static_cast<const std::string&>(poisonActionLabel);  // Apply
		}
	}

	return GetPrimaryLabel();
}
