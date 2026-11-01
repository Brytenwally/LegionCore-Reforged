# CLAUDE.md

Notes for Claude sessions working on LegionCore-Reforged (Brytenwally's fork). **Keep this file
up to date** with anything future sessions need, in the same commit as the change.

## What this is

A WoW Legion 7.3.5 (build 26972) server, from the 2020 UWOW leak by way of upstream LegionCore,
forked from Apheleos' LegionCore-Reforged. The owner chose it as the base for a Legion server
with **modules** (like SkyFire / AzerothCore) and, step by step, a **playerbots** module.
Full features take months; the owner wants baby steps, one working milestone at a time.

## Owner's decisions so far

- **Reforged is the base.** No commits from LegionCore-7.3.5 are brought in for now.
- Later (not started): bring over from LegionCore-7.3.5 only its Legion content (class halls,
  artifact acquisition, artifact scenarios, Legion dungeon/raid fixes, likely the zone
  questlines). See "LegionCore-7.3.5" below before touching that.
- Playerbots are ported from **SkyFire-playerbots** (5.4.8), not written from scratch and not
  from LegionBotAI.

## Rules (shared with the owner's other repos)

- Commit and push straight to `main`. No feature branches or PRs unless the owner asks.
- Commit messages: imperative subject, body explaining what and why. No model names in
  commits.
- Core changes: **only hooks the modules cannot do without, or fixes to bugs the owner hits**.
  Logic lives in modules. Keep each core change a small topic commit, so upstream Reforged
  stays easy to merge.
- Keep each file's line endings (some TrinityCore-era files are CRLF): edit with tools that
  preserve them (Python `newline=''`), check with `grep -c $'\r' file`.
- Bots: "nothing changes an alt unless the player orders it" (an alt = a player's own
  character logged in as a bot); new automatic behaviour stays off for alts. No server changes
  just to serve an addon.
- No server, database or client in the cloud container: runtime behaviour is untested there;
  say so when reporting.

## Related repositories

| Repo | Role |
| --- | --- |
| `Brytenwally/SkyFire-playerbots` | The 5.4.8 playerbots module to port from (~25k lines). Its `CLAUDE.md` documents every system (strategy engine `co`/`nc`, chat orders, gear/autogear with stat weights, maintenance, enchants/gems, loot rolls, auto-equip, alts, quests, LFG). `upstream-pr` branch = clean history. |
| `Brytenwally/SkyFire_548` | 5.4.8 core: reference for the module system (`modules/<name>/src`, `Add<name>Scripts`) and the bot hooks (`WorldSession::IsBot`, socketless bot sessions). |
| `Brytenwally/SkyFire-build` | GitHub Actions Windows build of SkyFire: template for a Reforged build workflow. |
| `Brytenwally/LegionCore-7.3.5` | Sibling fork of the same base (read for reference). Has LegionBotAI (`src/server/scripts/Custom/LegionBotAI.cpp`, `LegionBotMgr.cpp`): useful only to see how a character is loaded into the world on this core. |
| `Brytenwally/mod-playerbots` (branch `coa`) | AzerothCore 3.3.5 playerbots: read-only reference for behaviour. Never build or change it. |

## Findings about this codebase (2026-11-01 survey)

### Fork history
- Reforged and LegionCore-7.3.5 share upstream LegionCore commit `fed58f79` (2025-04-19).
  Since then Reforged: ~560 commits (Apheleos), broad core work: challenge modes/Mythic+,
  quests, spells, handlers, PvP and seasons, loot, LFG, pet battles, garrison/Draenor and
  Pandaria content, collections, datastores, extractors (map/vmap/mmap tools fixed for build
  26972), connection patcher, Argon2id account hashes.
- LegionCore-7.3.5: ~1,570 commits, Legion story content (class halls, artifact acquisition,
  ~135 artifact-scenario files, ~90 Stormheim/Highmountain/Val'sharah/Azsuna quest files,
  ~30 raid/dungeon fixes), BattlePay, Solocraft, AH bot, LegionBotAI, ~2,600 test/audit files.
  It changed 152 core files, **121 of which Reforged also changed**; 79 of its 135 scenario
  edits are in files Reforged moved to `src/server/scripts/Legion/Scenario`. Any future
  port of its content needs per-file / per-scenario merging, not a plain cherry-pick.
- Upstream already scripts every Legion dungeon and raid (175 files in
  `src/server/scripts/Legion`).

### Patch progression (Reforged's own system)
`worldserver.conf`: `Game.Patch` = 1..5 (7.0, 7.1, 7.1.5, 7.2, 7.3; default 5) drives content
tiers: `sql/optional/enable|disable-patch-7.x-content.sql` run at startup on a tier change
(`Game.Patch.ContentScripts`; they close the tier's maps/dungeons, opening quests and
emissaries, not every addition), original per-tier item levels (`Custom.ItemLevel.*`,
`ItemLevel.*`, catch-up optional), Mythic+ caps per tier (`Challenge.*`), the restored 7.0
Artifact Knowledge system (`Artifact.Knowledge.*`: no weekly auto increase, work orders and
Knowledge books, cap 25/40/55 by tier). Raids open per difficulty/LFR wing in `RAID OPENING`
(`Raid.<Raid>.*`), independent of `Game.Patch`; PvP season and world events have their own
switches. Not found: per-patch class/spell tuning, 7.2 Concordance or 7.3 Netherlight
Crucible gating. Bots must follow these tiers (gear item levels, artifact knowledge).

### Script build (basis for modules)
- TrinityCore's script-module build: every folder under `src/server/scripts` is a module
  (`cmake/macros/ConfigureScripts.cmake`: `GetScriptsBasePath`, `GetScriptModuleList` globs
  the folders, `GetPathToScriptModule`); `src/server/scripts/CMakeLists.txt` generates a
  loader per module (`ConfigureScriptLoader`, `ScriptLoader.cpp.in.cmake`) calling
  `Add<Folder>Scripts()`; static (linked into worldserver) or dynamic (shared library).
- Custom scripts live in `src/server/scripts/Custom` (`custom_script_loader.cpp`).

### Hooks already present (`src/server/game/Scripting/ScriptMgr.h`)
`WorldScript`, `CommandScript`, `GroupScript`, and `PlayerScript` with OnUpdate, OnChat
(whisper/party/guild/channel), OnLogin, OnLogout, OnCreate, OnDelete, OnMapChanged,
OnUpdateZone, OnQuestAccept, OnQuestComplete, OnQuestAbandon, OnQuestReward, OnQuestRewardItem,
OnLootItem, OnCreateItem, OnLevelChanged, OnGiveXP, OnEnterCombat, OnSpellCast, OnSpellLearned,
OnMovementInform, OnCreatureKill, OnPVPKill and more: most of what SkyFire needed added by hand.

### Sessions
- `WorldSession(id, name, std::shared_ptr<WorldSocket> sock, ...)`; `SendPacket` returns
  quietly when there is no socket, so a socketless session is a usable base.
- LegionBotAI (in the 7.3.5 fork) created bot sessions with a null socket and kept them **out
  of the world session update loop**, so they never ticked: delayed teleports never ran and
  it moved bots between maps by hand. Making bot sessions update properly (registered and
  ticked, `IsBot()`, no timeout/kick for the missing socket, safe save/logout) is the first
  real core change for playerbots.
- Maps update on several threads: cross-map bot actions (summon, teleport, invites, logout)
  must be queued to the right thread or done from a world-level update.

## Plan (agreed outline; nothing implemented yet)

| Step | Result | Size |
| --- | --- | --- |
| 0 | Windows build workflow for Reforged (pattern: SkyFire-build; match Reforged's Boost/OpenSSL/MySQL requirements) | ~1 session |
| 1 | Module system: CMake also lists `modules/<name>/src` as script modules (`Add<name>Scripts`), module `.conf.dist` copied and loaded, module SQL (create tables from code at startup like mod-destiny-weaver, or hook the DB updater later), a hello-world module, `modules/README.md` | 1-2 sessions |
| 2 | Bot session core hooks: ticking socketless sessions, `WorldSession::IsBot()`, logout/save, normal teleports | 2-4 sessions |
| 3 | Port the SkyFire-playerbots skeleton as a module: `.playerbots add/remove/list`, group, follow/formation, whisper orders, `co`/`nc` strategy engine. Translate uint64 guids to `ObjectGuid` (128-bit), Legion API names; drop glyphs and hit/expertise caps | 1-2 weeks of sessions |
| 4 | Combat: rotation framework + one tank, one healer, one DPS spec | ~1-2 weeks |
| 5 | LFG, loot rolls, gear init by item level/bonus IDs within the `Game.Patch` tier, artifact weapon + traits | ~1-2 weeks |
| 6 | More specs (36 + Demon Hunter), random bots, BG/arena, questing | months |

Legion differences to plan for: artifacts (weapon, traits, relics, Knowledge tiers), 7 talent
tiers plus PvP talents, class resources (Maelstrom, Insanity, Fury, Pain, Astral Power...),
secondary stats crit/haste/mastery/versatility only, item level from bonus IDs, Legion phasing.

## Checking

No build has been done in the cloud container yet. TrinityCore-era code may build on Linux
with distro packages (check Reforged's requirements first); otherwise verify through the build
workflow (step 0).
