// SPDX-License-Identifier: GPL-3.0-or-later
// SkyPrompt fork addition, 2026-11-12.
#include "Action.h"

#include <chrono>
#include <iostream>

namespace
{
	void CheckLabels(const Action& a_action, std::string_view a_primary, std::string_view a_secondary = {})
	{
		if (a_action.GetUseLabel(nullptr) != a_primary ||
			a_action.HasSecondaryAction() != !a_secondary.empty() ||
			(!a_secondary.empty() && a_action.GetUseLabel(nullptr, true) != a_secondary)) {
			throw std::runtime_error("Unexpected alternate action labels");
		}
	}
}

int main()
{
	const auto path = std::filesystem::temp_directory_path() /
		std::format("UseOrTake-ActionLabels-{}.ini", std::chrono::steady_clock::now().time_since_epoch().count());
	struct Cleanup
	{
		std::filesystem::path path;
		~Cleanup()
		{
			std::error_code error;
			std::filesystem::remove(path, error);
			if (error) {
				std::cerr << "Unable to remove test INI: " << error.message() << '\n';
			}
		}
	} cleanup{ path };

	try {
		Action ingredients("Ingredients", "Eat");
		SecondaryAction weapons("Weapons", "Equip", "Equip and Draw");
		SecondaryAction scrolls("Scrolls", "Equip", "Read");
		CheckLabels(ingredients, "Eat");
		CheckLabels(weapons, "Equip", "Equip and Draw");
		CheckLabels(scrolls, "Equip", "Read");

		const std::string eat = "\xE5\x90\x83";
		{
			std::ofstream ini(path, std::ios::binary);
			ini << "[Ingredients]\nsAlternateActionLabel = " << eat
				<< "\n[Weapons]\niDefaultAction = 2\nsAlternateActionLabel = Equiper"
				<< "\nsAlternateSecondaryActionLabel = Equiper et degainer"
				<< "\n[Scrolls]\nsAlternateActionLabel = Ausruesten"
				<< "\nsAlternateSecondaryActionLabel = Lesen\n";
			if (!ini) {
				throw std::runtime_error("Unable to write test INI");
			}
		}
		const auto filename = path.string();
		const auto store = REX::FIniSettingStore::GetSingleton();
		store->Init(filename.c_str(), "");
		store->Load();
		CheckLabels(ingredients, eat);
		CheckLabels(weapons, "Equiper", "Equiper et degainer");
		CheckLabels(scrolls, "Ausruesten", "Lesen");
		store->Save();
		store->Load();
		CheckLabels(ingredients, eat);
		CheckLabels(weapons, "Equiper", "Equiper et degainer");
		CheckLabels(scrolls, "Ausruesten", "Lesen");
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		return 1;
	}
}
