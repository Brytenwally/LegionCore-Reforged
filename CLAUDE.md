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
- **Random bots' open-world life is the absolute last priority** (owner, 2026-11-02): random
  bots questing on their own, roaming level-appropriate zones, grinding and the like come after
  everything else in the plan. Bots the player adds and leads (group, dungeons/LFG, raids,
  BG/arena) come first.
- Playerbots are ported from **SkyFire-playerbots** (5.4.8), not written from scratch and not
  from LegionBotAI. The SkyFire repos are reference manuals only: their code is read and
  rewritten for Legion here, never changed there.

## Rules (shared with the owner's other repos)

- **Update the CLAUDE.md files as you go** (owner, 2026-11-03): every time a problem is resolved
  or something new is learned about how the core, the database or the module works, write it
  into the relevant CLAUDE.md (this one for the core/project, LegionCore-Playerbots' for the
  module, LegionCore-Build's for the build) **in the same commit as the fix**, without being
  asked: the cause, the evidence that found it, and the rule that follows. Also record each
  runtime test result the owner reports (what works, on which build).

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

## This project's repositories

These are the ones this project writes to (commit and push to `main`).

| Repo | Role |
| --- | --- |
| `Brytenwally/LegionCore-Reforged` (this repo) | The core. Gets the module system and the hooks modules cannot do without, as small topic commits. |
| `Brytenwally/LegionCore-Playerbots` | The playerbots module, ported from SkyFire-playerbots. Lives in the core at `modules/mod-playerbots` (the folder name decides the loader name, `Addmod_playerbotsScripts`). Has its own `CLAUDE.md`. As of 2026-11-02: bots log in/out, teleport and follow summons (`.playerbots add/remove/list`, verified on the owner's server), no AI yet. |
| `Brytenwally/LegionCore-Build` | GitHub Actions workflow building Windows x64 binaries of the core with the modules checked out into `modules/` (pattern: SkyFire-build). Has its own `CLAUDE.md`. |

## Related repositories

**All of them are read-only references for this project.** Read them to see how something was
done, then write it in this project's repositories above. Never commit, push, open PRs
or run builds in the SkyFire repos, LegionCore-7.3.5 or mod-playerbots from a Reforged session.

| Repo | Role |
| --- | --- |
| `Brytenwally/SkyFire-playerbots` | Read-only reference. The 5.4.8 playerbots module to port from (~25k lines). Its `CLAUDE.md` documents every system (strategy engine `co`/`nc`, chat orders, gear/autogear with stat weights, maintenance, enchants/gems, loot rolls, auto-equip, alts, quests, LFG). `upstream-pr` branch = clean history. |
| `Brytenwally/SkyFire_548` | Read-only reference. 5.4.8 core: shows the module system (`modules/<name>/src`, `Add<name>Scripts`) and the bot hooks (`WorldSession::IsBot`, socketless bot sessions). |
| `Brytenwally/SkyFire-build` | Read-only reference. GitHub Actions Windows build of SkyFire: the pattern for a Reforged build workflow (written in this project's own repo, not there). |
| `Brytenwally/LegionCore-7.3.5` | Read-only reference. Sibling fork of the same base. Has LegionBotAI (`src/server/scripts/Custom/LegionBotAI.cpp`, `LegionBotMgr.cpp`): useful only to see how a character is loaded into the world on this core. |
| `Brytenwally/mod-playerbots` (branch `coa`) | Read-only reference. AzerothCore 3.3.5 playerbots: how features behave. Never build it. |

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

## Module system (step 1, done 2026-11-01)

- `modules/<folder>` with a `src/` directory is a **script module** (`cmake/macros/
  ConfigureScripts.cmake`: `GetModuleFolderList`, `ModuleFolderToScriptModule`,
  `GetModuleFolderOfScriptModule`; `GetScriptModuleList` / `GetPathToScriptModule` include
  them). Name = folder with non-alphanumerics as `_`; loader `Add<name>Scripts()` is called by
  the generated script loader (`Addmod_playerbotsScripts` for `mod-playerbots`). Linkage per
  module: `SCRIPTS_MOD_<NAME>` = default/static/dynamic/disabled; `-DMODULES=0` drops all.
- `src/server/scripts/CMakeLists.txt`: module `src/` subfolders are include dirs;
  `conf/*.conf.dist` staged next to the worldserver (POST_BUILD on `scripts`) and installed.
- Config: `sConfigMgr->LoadModuleConfig("<name>.conf", error)` (core hook in
  `src/common/Configuration/Config.cpp`) merges `<name>.conf.dist` then `<name>.conf` from the
  main config's folder; `Reload()` re-merges them. Call it in the loader; read values in
  `WorldScript::OnConfigLoad` (runs after the loaders, and on reload). Files need a section
  header (`[worldserver]`).
- Module SQL: not hooked into the updater; modules create their tables from code at startup.
- `modules/mod-example`: minimal module (config, startup log, `.example` GM command).
- Checked in the cloud container: CMake configure (static, dynamic, MODULES=0) and compiling
  `Config.cpp` and `mod_example.cpp` with the scripts PCH, on Ubuntu 24.04 apt packages
  (cmake 3.28, g++ 13, Boost 1.83); the Windows build compiles and links both modules and
  packages their `.conf.dist`. **Runtime verified 2026-11-02** on the owner's Windows server:
  `[mod-example] Module loaded: Hello from mod-example!` and `[mod-playerbots] Module loaded
  (disabled)` at startup, so the loader, `LoadModuleConfig` and `OnConfigLoad` work.

## Bot sessions (step 2, done 2026-11-02)

**Runtime verified 2026-11-02** (build #5, core `b1befd3` + playerbots `c77aa64`) with an alt
bot on the GM's own account: login, the GM's own session keeps working beside it, `.summon`
(near teleport), `.tele name <bot> darnassus` (far teleport, map 0 -> 1), `.playerbots list`, and
the bot logging out with its account's player.

Core commit "Add bot sessions" (`WorldSession.h/.cpp`, `CharacterHandler.cpp`,
`MovementHandler.cpp`, `Player.cpp`):
- How this core moves sessions: `World::m_sessions` and every `Map::m_sessions` are keyed by
  **account id** and hold `WorldSessionPtr` (shared_ptr). `World::UpdateSessions` updates only
  sessions without a map; at login (`HandlePlayerLogin`) and in teleports (three paths in
  `Player.cpp`) the session is handed to the destination map by `sWorld->FindSession(accountId)`.
  The map then updates it on its own thread (`Update(diff, map)`, packets only when
  `map == m_map`). LegionBotAI broke on exactly this (null session -> never on a map).
- `WorldSession::CreateBotSession(...)`: socketless, `IsBot()`, keeps a weak pointer to itself;
  **not** in `World::m_sessions` (an alt bot shares its account with the player's session, and
  `World::AddSession` kicks a second session of an account). `GetSessionPtr()` replaces
  `sWorld->FindSession(GetAccountId())` at those four sites (same lookup for clients).
- Owner of a bot session (the module) calls `Update(diff)` while `GetMap()` is null; `Update`
  returns false when the bot has ended (no player, not loading); maps then drop it too.
- `UpdateBot()` (in `Update`, where packets are handled) plays the client: far teleport ack
  (`HandleWorldPortAck`, when `IsBeingTeleportedFar && !IsInWorld && !IsChangeMap`), near ack
  (`FinishBotTeleportNear`, copy of `HandleMoveTeleportAck`), end of loading screen (as
  `HandleLoadScreenOpcode`: `SendInitialPacketsAfterAddToMap` = zone/world states,
  `CastPendingCreateSpells(true)`, clears `m_playerLoading`) after login and non-seamless far
  teleports. Login itself: `LoginBot(guid)` = set `m_playerLoading`, `HandleContinuePlayerLogin`
  (async holder; player added to the map by the 100 ms delayed event in `HandlePlayerLogin`).
- `LogoutBot()` (atomic flag; `KickPlayer` calls it for bots) -> in `UpdateBot` on the
  session's thread: `SetRemoveFromMap(true)` (instant logout: the map removes the player, sets
  `canLogout`, the session's next update runs `LogoutPlayer(true)`), or `LogoutPlayer` directly
  between maps. Bots: `LogoutPlayer` sets only their character offline, `~WorldSession` sets the
  account offline only if no client session of the account remains.
- `Map::m_sessions` is keyed by `GetMapSessionKey()`: the account id for clients, `0x80000000 |
  botId` for bots. Keyed by account id (the original code), an alt bot replaced its player's
  session in the map when it entered: the map stopped updating the player's session, so his
  commands, chat and combat went unprocessed (owner hit it 2026-11-02, fixed the same day).
  Any other per-account table a bot session enters needs the same care.
- Bot sessions process queued packets (`QueuePacket`) although they have no socket: a module
  drives client actions by queueing the client packet (e.g. `CMSG_PARTY_INVITE_RESPONSE`), so the
  core's own handler, status checks and threading apply (many handlers, like `AcceptPartyInvite`,
  are file-static and do careful cross-thread work). Build the packet exactly as the matching
  `WorldPackets::...::Read()` parses it.
- Whispers: `PlayerScript::OnChat(..., Player* receiver)` gets the receiver only when he stands
  on the sender's map (`Player::Whisper` passes `ObjectAccessor::GetPlayer(*this, guid)`). Core
  hook `PlayerScript::OnWhisper(sender, receiverGuid, lang, msg)` (added 2026-11-02, called in
  `Player::Whisper` before delivery) gives the receiver's guid wherever he is; it runs in the
  sender's thread, so act on the receiver with `PostToPlayer`.
- Cross-thread player access on this core: `ObjectAccessor::PostToPlayer(guid, fn)` (runs in the
  player's own update), `WithPlayer` (short reads under the accessor lock). `FindPlayer` pointers
  are only safe on the player's own map thread.
- Lessons from the first bot features (details in LegionCore-Playerbots' `CLAUDE.md`):
  - An invite from a player outside any group makes a **pending group with an empty guid**
    until someone accepts (`HandlePartyInvite` -> `AddLeaderInvite`; the leader guid is set).
  - The database has **class/race copies of quests** (same title and `QuestSortID`, one per
    class through `quest_template_addon.AllowableClasses`, each chaining to its own class's
    next quest), e.g. Northshire's "Beating Them Back!" = 9 quests. Anything that mirrors or
    compares quests between characters of different classes must map to the other's copy.
    Some class versions are **not** copies (different title and objectives, e.g. Northshire's
    "Encrypted Letter"): no automatic mapping exists, so bots take those by manual order.
    Copies may also differ **only by `PrevQuestID`** (no class restriction; Northshire's "They
    Sent Assassins" = 9 quests, each after one class's "Join the Battle!"): pick the copy the
    character can take now (`CanTakeQuest`), never just the first one its class allows.
- Not handled yet: a client logging into a character that is currently a bot (the module logs
  an account's bots out when its player logs out, which covers switching characters).

## Building

- Windows: `Brytenwally/LegionCore-Build` workflow (step 0). First green build 2026-11-01
  (run #3: core `56ca66e` + playerbots `5680c13`, both modules compiled in, ~25 min). Runs on `windows-2022`: Boost must
  stay 1.84 (the core uses `io_service` / `io_context::strand`, removed in Boost 1.87) and
  Boost 1.84 does not know VS 2026. See that repo's `CLAUDE.md`.
- The fork's history is one squashed snapshot (`3b20490`). That snapshot did not compile:
  `instance_auchindoun.cpp` called a missing `Spell::GetRemainingCastTime()` (added to
  `Spell.h`, as in TrinityCore). Expect more such gaps; fix them as small topic commits.
- **Database names: `world_legion` and `hotfixes_legion`** (auth and characters keep their
  names). Apheleos' recent SQL updates name them across databases (`hotfixes_legion`.
  `broadcast_text` in a world update, `world_legion`.`version` in ~12 hotfixes updates), so a
  realm with `world` / `hotfixes` fails "Unknown database 'hotfixes_legion'" during the world
  updates (owner hit it 2026-11-01). The conf.dist defaults still say `world` / `hotfixes`;
  the owner's confs use the `_legion` names. Do not rewrite the update files (they are
  upstream's and the updater re-applies files whose hash changes).
- Owner's local setup (2026-11-02): `D:\Legion Repack` holds the binaries, confs, a portable
  `mysql-8.0.44-winx64` started by a `start_mysql.bat` (root/admin on 127.0.0.1:3306, my.ini
  written by the bat), the source in `LegionCore-Reforged-main` (`SourceDirectory`), and
  `ClientData` from an `extract_data.bat` run in the client folder. Logging appenders use flag
  1 (timestamps).
- Linux quick check: `apt-get install cmake g++ libboost-all-dev libssl-dev libmysqlclient-dev
  libreadline-dev libbz2-dev zlib1g-dev`, `cmake -S . -B <scratch>/build -DTOOLS=0`, then
  `make <path>.o` inside the target's build folder (e.g. `src/server/scripts`) to compile single
  files with the PCH. After changing a header, delete the build's `*.gch` files first: make
  does not rebuild the PCH for a single-object target. A full build was not tried (hours on 4
  cores).

## Plan (agreed outline)

| Step | Result | Size |
| --- | --- | --- |
| 0 | **Done 2026-11-01** (green Windows build). Windows build workflow in `LegionCore-Build` (pattern: SkyFire-build; match Reforged's Boost/OpenSSL/MySQL requirements) | ~1 session |
| 1 | **Done 2026-11-01** (see above; verified on the owner's server 2026-11-02: both modules log at startup). Module system: CMake also lists `modules/<name>/src` as script modules (`Add<name>Scripts`), module `.conf.dist` copied and loaded, module SQL (create tables from code at startup like mod-destiny-weaver, or hook the DB updater later), a hello-world module, `modules/README.md` | 1-2 sessions |
| 2 | **Done 2026-11-02** (see "Bot sessions"; verified on the owner's server with build #5, core `b1befd3`). Bot session core hooks: ticking socketless sessions, `WorldSession::IsBot()`, logout/save, normal teleports | 2-4 sessions |
| 3 | **In progress** (verified on the owner's server 2026-11-03, build #10: group invite, follow, stay/follow by whisper and party chat, summon across maps, leave; quest mirroring with class copies verified on build #12). Port the SkyFire-playerbots skeleton into `LegionCore-Playerbots`: `.playerbots add/remove/list`, group, follow/formation, whisper orders, `co`/`nc` strategy engine. Translate uint64 guids to `ObjectGuid` (128-bit), Legion API names; drop glyphs and hit/expertise caps | 1-2 weeks of sessions |
| 4 | Combat: rotation framework + one tank, one healer, one DPS spec | ~1-2 weeks |
| 5 | LFG, loot rolls, gear init by item level/bonus IDs within the `Game.Patch` tier, artifact weapon + traits | ~1-2 weeks |
| 6 | More specs (36 + Demon Hunter), random bot pool (accounts/characters to fill LFG, BG/arena queues), BG/arena | months |
| 7 | **Last, by the owner's priority:** random bots' open-world activity (questing on their own, level-appropriate zones, grinding, wandering) | after everything else |

Legion differences to plan for: artifacts (weapon, traits, relics, Knowledge tiers), 7 talent
tiers plus PvP talents, class resources (Maelstrom, Insanity, Fury, Pain, Astral Power...),
secondary stats crit/haste/mastery/versatility only, item level from bonus IDs, Legion phasing.

