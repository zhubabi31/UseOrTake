# Use Or Take - SkyPrompt fork

An unofficial modification of [powerofthree's Use Or Take](https://github.com/powerof3/UseOrTake).
Keep Skyrim's familiar **Take/Steal** activation prompt and button, with a separate
object-attached [SkyPrompt](https://www.nexusmods.com/skyrimspecialedition/mods/148703)
for eating, drinking, equipping, applying poison, or reading a scroll.

Modification notice (2026-11-12): this fork adds optional SkyPrompt integration, direct
alternate-action input, object/menu/save lifecycle handling, and shared Skyrim
1.6.1170 / 1.7.104 loader support. It preserves the original configurable action labels and legacy mode.
It is not an official release by powerofthree or the SkyPrompt author.

## Build requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/) or newer
	* Desktop development with C++

## User Requirements
* SKSE64 matching your game version; see the runtime table below.
* [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
	* Needed for SSE/AE
* [SkyPrompt](https://www.nexusmods.com/skyrimspecialedition/mods/148703) with API 2.0
	* Needed for the new object-attached alternate action prompts.

### Runtime compatibility

The AE build is a **single DLL targeting both Steam runtimes**:

| Skyrim runtime | Matching SKSE64 | Address Library |
| --- | --- | --- |
| 1.6.1170 (Steam) | 2.2.8 from the supplied archive (minimum 2.2.6) | For 1.6.1170 |
| 1.7.104 (Steam) | 2.3.1 from the supplied archive | For 1.7.104, v5 encoding |
| 1.6.1179 (GOG) | GOG SKSE64 matching that runtime | For 1.6.1179 |

Use the **same plugin DLL**, but do not interchange SKSE64, Address Library, or SkyPrompt
runtime builds between game versions. SkyPrompt itself must support your game version.
The supplied SKSE 2.3.1 archive contains `skse64_1_7_104.dll`, not a 1.6.1170 extender.
The plugin declares the newer Address Library v5 capability while retaining the older
SKSE minimum; its pinned CommonLib dependency can load both Address Library encodings.
Other AE runtime versions are rejected explicitly.

The SDKs have different `PlayerCharacter` layouts. The integration does not read that
class's runtime-dependent member fields: player operations go through relocated engine
functions or the unchanged Actor base, and activation/drag suppression uses the input
handler. Build and loader-metadata checks do **not** substitute for running Skyrim:
both Steam runtimes and GOG still require in-game confirmation before a public release.

## Installation

1. Install SKSE64, Address Library, and SkyPrompt versions compatible with your Skyrim runtime.
2. Install this fork's `Data` directory using a mod manager, or place its DLL and INI in
   `Data\SKSE\Plugins`.
3. Replace the original Use Or Take DLL; do not install two copies under different names.
   Keep a backup of your existing INI and check `bUseSkyPrompt = true`.

The compiled plugin is `buildae\Release\po3_UseOrTake.dll`. The default configuration is
`Skyrim\Data\SKSE\Plugins\po3_UseOrTake.ini`. A source checkout alone is not an installable
mod: build the DLL or obtain a release package. This fork does not bundle SkyPrompt,
SKSE64, Address Library, or their runtime assets.

## Object-attached actions

With SkyPrompt installed, the vanilla activation prompt and its mapped button are left unchanged.
For example, a cheese wheel retains vanilla **Take** (`E` with default keyboard bindings),
while a separate **Eat** prompt is attached to the cheese wheel. Press the button shown by
SkyPrompt directly; modifier + Activate is not required. SkyPrompt manages its keyboard,
mouse, and gamepad bindings through its own settings.

Armor, ammo, and carryable torches show **Equip**; ingredients and food show **Eat**;
drinks show **Drink**; poisons show **Apply**. Weapons show both **Equip** and
**Equip and Draw**; scrolls show both **Equip** and **Read** (cast), as separate
SkyPrompt action rows. **Take/Steal stays exclusively vanilla**, never in SkyPrompt.
SkyPrompt assigns its row bindings to these actions; keep those bindings distinct
from vanilla Activate and from each other. Its theme controls the row layout.
Existing per-category enable settings and alternate action labels still apply. Stolen
items have red alternate prompts, and use still goes through normal pickup/activation.
Prompts are removed when the target is no longer valid or gameplay is blocked by a menu.

`bUseSkyPrompt = true` is the default in `Data\SKSE\Plugins\po3_UseOrTake.ini`.
In this mode, `iDefaultAction` and the legacy modifier/hold settings are ignored:
normal activation always uses the game's original behavior. Set `bUseSkyPrompt = false`
to restore legacy behavior. If SkyPrompt is absent or its API is incompatible, the plugin
logs a warning/error and uses legacy behavior instead.

## Translations and custom labels

Edit `Data\SKSE\Plugins\po3_UseOrTake.ini` and save it as **UTF-8**, then restart Skyrim.
Labels are displayed as written; no source changes or rebuild are needed. Translate
the values on the right, but leave section names and setting keys unchanged:

```ini
[Armors]
sAlternateActionLabel = Equip

[Weapons]
sAlternateActionLabel = Equip
sAlternateSecondaryActionLabel = Equip and Draw

[Potions]
sAlternateActionLabel = Drink
sAlternateActionLabelFood = Eat
sAlternateActionLabelPoison = Apply

[Ingredients]
sAlternateActionLabel = Eat

[Scrolls]
sAlternateActionLabel = Equip
sAlternateSecondaryActionLabel = Read

[Torches]
sAlternateActionLabel = Equip

[Ammo]
sAlternateActionLabel = Equip
```

For example, `sAlternateActionLabelFood = Manger` changes the food prompt to French.
Non-Latin text requires a SkyPrompt font that contains the language's glyphs.
SkyPrompt also supports `$LocalizationKeys` for authors supplying its translation files.
The game's vanilla Take/Steal text and activation button retain the game's own
localization and control mapping. Both original weapon and scroll alternate labels
are retained, including the **secondary** label for Equip and Draw / Read.

### In-game check (repeat on each supported runtime)

With the matching SKSE64, Address Library, and SkyPrompt installed, look at a cheese wheel:
vanilla **Take** with the mapped activation button and an attached **Eat** prompt should
both be visible. Activate takes without eating; SkyPrompt's button picks up and eats.
Also check both alternate weapon/scroll actions, stolen food, gamepad bindings, switching targets, opening menus,
and loading a save. These UI/input checks require running Skyrim.

## Building

Clone this fork's development branch (or check out the exact release tag/source revision):

```
git clone --branch zhubabi31-skyprompt-alternate-actions https://github.com/zhubabi31/UseOrTake.git
cd UseOrTake
git submodule update --init --recursive
```

### SSE (1.5.97)
```
cmake --preset vs2022-se
cmake --build --preset vs2022-se
```
### AE (1.6.1170 / 1.6.1179 / 1.7.104)
```
cmake --preset vs2022-ae
cmake --build --preset vs2022-ae
```

Replace `vs2022` with `vs2026` to build with Visual Studio 2026.
SkyPrompt API is fetched through the pinned vcpkg overlay in
`cmake\ports\skyprompt-api`; no SkyPrompt runtime DLL is linked or redistributed.

After building AE, run `.\tests\CheckPluginMetadata.ps1` in 64-bit PowerShell to
check the actual DLL exports, runtime declarations, extender minimum, and Address
Library flags against the two supplied SKSE versions. This does not start Skyrim.

To check both alternate labels and UTF-8 INI round trips without starting Skyrim:

```
cmake --preset vs2022-ae -DBUILD_TESTING=ON
cmake --build --preset vs2022-ae
ctest --test-dir buildae -C Release --output-on-failure
```

## Credits and licensing

* **Use Or Take:** powerofthree / powerof3. Original project and retained code are
  covered by the [GNU GPL v3](LICENSE); see also the original project's licensing notices.
  This fork is distributed under the project's GPL-3.0-or-later terms.
* **SkyPrompt API:** A. Ege Yilmaz (@Quantumyilmaz), QTR-Modding; MIT licensed.
  Its copyright notice and full license are retained in [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt).
  The API header is fetched unmodified using the upstream pinned port.
* **CommonLibSSE:** the [powerof3/CommonLibSSE](https://github.com/powerof3/CommonLibSSE)
  contributors, used under its own license. Other build dependencies retain their
  respective licenses; vcpkg installs their notices under each package's `share` directory.
* **SKSE64, Address Library, and SkyPrompt runtime:** independently distributed
  requirements, not included in this repository or licensed by this fork.

When distributing a compiled release, include `LICENSE`, `THIRD_PARTY_NOTICES.txt`,
and the applicable dependency copyright/license files from vcpkg, including those for
CommonLib, fmt, spdlog, SimpleIni, and Xbyak. Provide the complete corresponding source
for that exact binary, including build scripts and required dependency sources, using
a GPL-compliant distribution method. Keep existing copyright notices, identify this as
a modified fork, and do not impose an "all rights reserved" or no-redistribution rule on
the GPL-covered code.

### Publishing on Nexus Mods

Use a distinct title such as **Use Or Take - SkyPrompt Integration (Unofficial)**.
Credit and link both original projects, describe the changes, and list their runtimes
as requirements rather than re-uploading their DLLs, assets, screenshots, or branding.
Publish the matching source and license notices alongside the binary. Check the original
Nexus pages' current permissions and Nexus's upload rules; contacting the authors before
publishing is the safest way to resolve any mismatch between page permissions and the
repository license. A license-compliant fork does not guarantee that Nexus will accept
an upload, and this guidance is not legal advice.
