# CLAUDE.md

Notes for Claude sessions working on LegionCore-Reforged (Brytenwally's fork). **Keep this file
up to date** with anything future sessions need, in the same commit as the change.

## What this is

A WoW Legion 7.3.5 (build 26972) server, from the 2020 UWOW leak by way of upstream LegionCore,
forked from Apheleos' LegionCore-Reforged. The owner chose it as the base for a Legion server
with **modules** (like SkyFire / AzerothCore) and, step by step, a **playerbots** module.
Full features take months; the owner wants baby steps, one working milestone at a time.

## Owner's decisions so far

- **Reforged is the base.** LegionCore-7.3.5's work is ported into it phase by phase (rewritten / merged here, no
  cherry-picks of its history), per the next point.
- **Started 2026-11-09** (owner: "map all the features they implemented that our core hasn't and implement as
  many as possible", in steps): bringing over LegionCore-7.3.5's work since the leak. The map, the phases and the
  method are in `docs/legioncore-735-port.md`; `docs/legioncore-735-files.tsv` lists every changed C++ file with its
  status against Reforged (CLEAN: take as is, NEW: copy + register, BOTH_CHANGED: merge by hand). Phase 1 (artifact
  choice onboarding) is done; see "LegionCore-7.3.5 port" below.
- **Random bots' open-world life is the absolute last priority** (owner, 2026-11-02): random
  bots questing on their own, roaming level-appropriate zones, grinding and the like come after
  everything else in the plan. Bots the player adds and leads (group, dungeons/LFG, raids,
  BG/arena) come first.
- **Basic combat comes from LegionBotAI** (owner, 2026-11-03): LegionCore-7.3.5's
  `src/server/scripts/Custom/LegionBotAI.cpp` has Legion 7.3.5 spell ids and rotations; its combat
  logic is the reference for step 4 (see "Combat plan"), rewritten into the module's per-bot
  threading. Everything else still comes from SkyFire-playerbots.
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
- **Do not start Windows builds (LegionCore-Build workflow) on your own** (owner, 2026-11-05): push
  the change, say it is ready, and run the workflow only when the owner asks for a build.
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
| `Brytenwally/AshamaneCore-Legion735` | Read-only reference (public; clone with plain git, not attached). TrinityCore-based 7.3.5 core with a bot system in `src/server/scripts/Custom/Bots/` (see "AshamaneCore bots" below). TrinityCore API (`GetSpellHistory`, `GetPrimarySpecialization`, `TRIGGERED_NONE`), so its code is translated, not copied. |

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

### Patch progression: removed 2026-11-09
Reforged had `Game.Patch` (1..5 = 7.0 .. 7.3) gating content tiers: startup SQL scripts in `sql/optional` that closed or
opened each tier's maps / quests / spawns (state = `disables` rows whose comment starts with "<tier> content - "),
per-tier defaults for legendary / obliterum / max item level, Artifact Knowledge start / cap and Mythic+ caps,
currency-to-gold conversion, world quest / game event / Mythic+ map / contribution / Dauntless loot gates, and a
login relocation out of closed content. **Owner, 2026-11-09: "not a feature I'm interested in ... get rid of it".** Removed
in the core commit "Remove Game.Patch content tiers": the server always runs the full 7.3.5 game; the tunable settings
keep their keys with the 7.3 defaults (`Item.LegendaryLevel` 1000, `Max.ItemLevel` 985, `Artifact.Knowledge.Cap` 55 /
`Start` 40, `ItemLevel.Dungeon.Mythic` 845, `ItemLevel.MythicPlus.Cap` 940...); `Custom.ItemLevel.Catchup.Enable` and
`Custom.ItemLevel.DungeonTier` (item level options of their own) stay; the login check keeps only maps disabled as a
whole. **The base world dump ships with 7.3 closed** (its `disables` holds the 7.3 rows): the server used to open it at
every startup, so `sql/updates/world/2026_11_09_24_open_all_content_tiers.sql` runs each former enable script once, only
for a tier still marked closed. Raid opening (`Raid.<Raid>.*`), PvP seasons and world events keep their own switches.

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
- Dungeon finder (2026-11-05): LFG state on this core is kept **per queue id** (`LFGMgr::PlayersStore[guid]
  [queueId]`, queue id = the dungeon's id, a player may be in several queues: `PlayerDungeons[guid]`), all
  behind `m_lock` and private. Core commit "Add LFG accessors for modules" adds read-only `GetQueueIds`,
  `GetRoleCheckRoles(gguid, guid, roles)` (role check in state INITIALITING) and `GetPendingProposal(guid)`
  (proposal whose answer for him is `LFG_ANSWER_PENDING`). A module answers as the client would by calling
  what the handlers call, on the player's own thread: `UpdateRoleCheck`, `UpdateProposal` (its `Ticket.Time`
  feeds wait times: send the player's `GetTicket`), `JoinLfg(player, roles, dungeons)`.
- Dungeon finder replacement (2026-11-06, owner's build #28 report): when a member left a dungeon finder group, the
  core offered "find another player" (`SendLfgOfferContinue`, `LFGGroupScript::OnRemoveMember`) only to the LFG
  leader, and not at all when the leader was the one leaving (the hook runs before `Group::RemoveMember` picks the new
  leader); with a bot as leader nobody saw it. Also `HandleSetPartyLeader` refused any lead change while a member was
  in an instance, so a player could not take the lead inside the dungeon. Core commit "Fix dungeon finder replacement
  offer and lead change": the offer goes to the next leader (first other online member) when the leader leaves, and
  dungeon finder groups may change leader inside their instance (their instance is bound to the group, the exploit
  check is for hand-made groups). The module makes bot leaders pass the lead to a player.
- **Still no replacement queue on build #30** (owner, 2026-11-06: the offer now reaches him, but neither it nor a
  manual join requeues the group). Cause: `LFGMgr::JoinLfg` refused every member with the random dungeon cooldown
  (71328, cast on each member 10 ms after a random dungeon's group is made, `LFGMgr` ~1488) with
  `LFG_JOIN_RANDOM_COOLDOWN` / `PARTY_RANDOM_COOLDOWN`, even for a continue request (a dungeon finder group asking for a
  replacement in its own dungeon); TrinityCore skips that check when `isContinue`. Reforged's threaded rewrite of
  `JoinLfg` computes the player and member checks before it knows whether the request continues, and applied them
  always. Core commit "Let dungeon finder groups requeue for a replacement": the cooldown no longer stops a continue
  request (deserter and battleground checks still do). The log showed no join at all, only `RemoveFromGroupQueue`
  lines (`TC_LOG_ERROR` in `LFGMgr::RemoveFromGroupQueue`: printed by every leave, not an error).
- Vote kicks (2026-11-06): `LFGMgr::BootsStore` is private; core commit "Add pending boot vote accessor for modules"
  adds `GetPendingBoot(gguid, guid, victim, agreed)` (a kick vote waiting for his answer, its victim, who voted yes;
  the kicker is not stored, he is among the yes votes). A module votes with `UpdateBoot(gguid, guid, accept)` on the
  player's thread, as `HandleLfgBootPlayerVote`.
- Group loot (2026-11-05): `Group::RollId` (the rolls) is private; core commit "Add pending loot rolls accessor
  for modules" adds `Group::GetPendingRolls(member)` (rolls of live loot still waiting for his vote). A module
  votes with `CountRollVote(player, aoeSlot, choice)` on the player's own thread, like `HandleLootRoll`.
- Battlegrounds (2026-11-06, for the playerbots module; no core change): every client packet is handled on the session's
  own thread on this core whatever its `PROCESS_*` (`ProcessingPlace` is never read), so a module may call
  `JoinBattlegroundQueue` / `HandleBattleFieldPort` on the player's thread. `Battleground::BgObjects` / `BgCreatures` are
  public; a capture point's owner is its `GAMEOBJECT_FIELD_SPELL_VISUAL_ID` (`UpdateCapturePoint`); flag captures are
  server-side area trigger checks (`_CheckPositions`); `GetTypeID(true)` is the real map's type, `GetTypeID()` the queued
  one (random). Details in LegionCore-Playerbots' `CLAUDE.md`, "Battlegrounds".
- Upstream merge `8434fb9` (Titans-Project PR #4, 2026-11-06: Mardum demon hunter intro, quest package rewards,
  login talent and level-achievement fixes, removal of the original server's custom events, license headers):
  checked against the module the same day: every module hook survives, no API it uses changed, all 14 module files
  compile. Behaviour notes: `RewardQuest` now gives a package quest's only selectable item when no reward is chosen
  (bots choose one anyway); `Quest::GetClientPackageID` hides packages the server lists as choices.
- **Class hall messengers after "In the Blink of an Eye"** (owner, 2026-11-07: players never got their artifact
  quests after 44663). Cause: every class's first artifact / class hall quest has its giver in the database with a
  SmartAI made for being summoned (`SMART_EVENT_JUST_SUMMONED` -> follow invoker, personal visibility, talk, despawn
  after 30 s or on quest accept), but nothing summons them and none is spawned (the retail summon spells are client
  data, not in the repo; only the hotfix rows of `spell_effect` are). The mage (41035) and death knight (40714) quests
  have no giver at all (retail popups). The world DB had **no English titles for most Legion quests** (`quest_template.LogTitle`
  NULL, only frFR in `quest_template_locale`; filled from TDB 735 on 2026-11-08, see "English only"): quests were
  identified by class, level 98 and French titles. New
  script `scripts/Legion/legion_class_hall_messengers.cpp` (PlayerScript): 5 s - 2 min after 44663 / 44184 is
  rewarded, or at login / map change for a character who has it done, on the Broken Isles (map 1220), while his class's
  first quest is untaken and takeable, the messenger is summoned 3 yd behind him for him alone (`SummonCreature` with
  `viewerGuid`, 2 min timed despawn, followed by its SmartAI or a `MoveFollow` when it has none); retried every 3 min
  (15 s while in combat / flying / in a vehicle). DK and mage: popup offer (`SetPopupQuestId` +
  `SendQuestGiverQuestDetails(quest, player guid)`: `HandleQuestGiverAcceptQuest` accepts a self-given quest only as
  the popup quest), every 5 min. Quests / messengers: Warrior 42814 / 108961 (Alliance), 41052 / 93775 (Horde),
  Paladin 38710 / 92909, Hunter 40384 / 100786, Rogue 40832 / 102018, Priest 40705 / 101344, Shaman 39746 / 91731
  (also spawned in Dalaran), Warlock 40716 / 103506, Monk 12103 / 98519 (its SmartAI teleports with 194004), Druid
  40643 / 101061, Demon Hunter 39047 / 99343. Bot sessions are skipped. Compiled in the container; untested.
- **English only** (owner, 2026-11-08: "get rid of all the French text and switch it to English", his client is enUS).
  Findings: the server picks text by the session locale, taken from the client at login (`account.locale`, bnetserver
  `LOGIN_UPD_ACCOUNT_LOGIN_INFO`), and every quest / creature text / hotfix path uses the base (English) column unless
  the session is not enUS (`ObjectMgr::GetLocaleString`, `DB2StorageBase::WriteRecord`); no code falls back to
  another locale. Upstream Reforged is maintained in French: its updates rightly put French into the `_locale` rows
  and `content_loc2` / `...Localized` columns, but **~9,800 quests (mostly Legion) had no English at all** (NULL
  `quest_template.LogTitle` and texts, only `quest_template_locale` frFR), a few dozen rows held French in English
  columns, and some C++ messages were French. Changes:
  - `sql/updates/world/2026_11_08_00_quest_texts_english.sql`: English from TrinityCore's **TDB 735.00** (2018-02-19,
    same build; release asset `TDB_full_735.00_2018_02_19.7z`) into quest fields still empty here (`IF(col IS NULL OR
    col = '', ...)`, existing English never overwritten): 9,295 titles, ~3,900 descriptions, 5,043 objective
    descriptions, reward / progress texts; plus the reused ids whose English was a placeholder ("REUSE",
    "DEPRECATED": 8237, 11997, **12103 = the monk's "Before the Storm"**, 24661) get all of TDB's texts. 525 titles stay
    empty (not in TDB either).
  - `.../2026_11_08_01_french_texts_english.sql`: shop entries (`battlepay_display_info`, unofficial translations of
    the custom shop's names), `battlepay_tokens` "Credits", custom quest 316999, `creature_text` 56056 / 57106 (lines
    without a broadcast text; lines with one show the broadcast text, so the garbled Russian / German duplicates in
    `creature_text` never reach the client), gossip 44202, `script_texts` -1850507, calendar `...Localized` = English.
  - `.../2026_11_08_02_remove_french_locale.sql` and `hotfixes/2026_11_08_01_remove_french_locale.sql`: every frFR row
    of the 12 world and 200 hotfix `_locale(s)` tables deleted, `*_loc2` columns emptied. A later upstream update may
    add French rows again: harmless, only a frFR client gets them.
  - `hotfixes/2026_11_08_00_character_service_info_english.sql`: the level 90 / 100 boost texts.
  - C++: Recruit-a-Friend messages (`ReferAFriendHandler.cpp`, `.account raf` in `cs_account.cpp`), the rename log
    command (`cs_character.cpp`), Icecrown jousting trainers' gossip (`zone_icecrown.cpp`, garbled French defines).
  Checked: all five SQL files run (twice) on MariaDB against the affected base tables; the C++ compiles. Untested in game.
  **Build #35 (owner, 2026-11-09): class messengers work; "plenty of quests" still blank.** They are not Russian:
  no Russian is left in any quest text column (`[ÐÑ]` mojibake / Cyrillic: 0 quest rows), the fields are simply
  empty. Follow-up `2026_11_09_00_quest_texts_english_tdb12.sql`: (1) fields TDB 735 could not fill, from
  **TDB 1210.26091** (12.1, asset `TDB_full_1210.26091_2026_09_09.7z`; tag names from `git ls-remote --tags`, the
  releases pages are blocked here but asset downloads work): 229 quests, 71 reward, 55 progress, 98 objective texts;
  (2) **upstream's frFR rows often held the English text** (no translation made), so the frFR clean-up had dropped
  English that existed nowhere else: those rows' English goes into fields still empty (525 quest fields, 281
  objectives; mangled breaks "...nnWhen" -> `$B$B`; 5 real French lines translated by hand). Blank titles: 9,820 ->
  90. ~8,900 quests keep no description: hidden trackers, bonus objectives, turn-in-only quests (Lunar Festival
  elders, Candy Buckets, champions) whose only text is the reward text; no source has more.
  **The client caches quest texts** (`Cache/WDB/enUS`): quests it saw blank stay blank until that folder is deleted.
  Tools for next time: the base dumps are one `INSERT` per table per line; a small parser of those lines (values in
  parentheses, MySQL escapes) answers most data questions without a database.
- **LegionCore-7.3.5 port** (2026-11-09, plan in `docs/legioncore-735-port.md`). LegionCore-7.3.5 is cloned read-only at
  `/home/user/brytenwally/legioncore-7.3.5` in a session (`add_repo` says public reads go through the git proxy; clone with
  `GIT_LFS_SKIP_SMUDGE=1 git clone --depth 1 ...`, then `git fetch --depth=3000 origin main` to reach `fed58f79`).
  Its developer's `docs/audits` explain each fix. **Phase 1, artifact choice onboarding**: core hook
  `PlayerScript::OnPlayerChoiceResponse(player, choiceId, responseId)` (`ScriptMgr`), called at the end of
  `WorldSession::HandleChoiceResponse` after the reward spell, plus LegionCore-7.3.5's recheck of the choice / response
  conditions for the warlock (245) and hunter (240) panels at execution (a stale panel could restart a finished route);
  script `scripts/Legion/ClassHalls/class_hall_artifact_onboarding.cpp` (copied unchanged): for the 36 artifact
  responses (choices 235-266; same ids and responses in our database, checked) it credits the class hall quest's
  selection objective and starts that weapon's acquisition quest, and at login it starts the quest for a character
  whose hidden "<spec> Chosen" quest is rewarded. It complements the class hall messengers (they bring the first class
  hall quest; the choice comes later in that campaign). Compiled; untested in game.
  **Phase 2 (shared artifact support) and 3.1 (Death Knight), 2026-11-09**, details in the plan: core
  `Spell::EffectForceEquipItem` works (the leak returned before equipping; LegionCore-7.3.5 `c592d121`), new hooks
  `InstanceScript::OnCreatureDamageTaken` and `UnitAI::DamageTakenAfterScaling` in `Unit::DealDamage` after level
  scaling (finales kept alive through a scaled killing blow), scripts `Legion/ClassHalls/artifact_acquisition.cpp`
  (the parts ported so far: artifact chests, Blood / Frost scene equips, Dark Riders completion) and
  `dark_riders_artifact.cpp` (whole), SQL `2026_11_09_01..23_lc735_*`. **A full copy of this realm's world database
  can be rebuilt in the container in ~3 minutes** (method in the plan): LegionCore-7.3.5's guarded SQL is run on it
  first, then shipped with the guard forced to pass. Compiled; untested in game.
- **Game.Patch removed** (2026-11-09, see "Patch progression: removed" above).
- **Phase 2 finished** (2026-11-09; details in `docs/legioncore-735-port.md`, "Phase 2 completion"): scenario
  checkpoints across logouts (`PlayerScenario.cpp`, `characters.scenarioResume`; the scenario login check now needs one),
  scene instance retirement, Netherlight Crucible checks and empowerment after a power purchase. **Rule: a login inside a
  scenario is admitted only with its checkpoint** (live instance of this boot, or a fresh run from the recorded entrance).
  Compiled; untested in game.
- **Phase 3.2 Demon Hunter** (2026-11-09, details in the plan): map 1500 class route (warrior 933 / paladin 775 /
  demon hunter 961 were one map with classless `scenario_data` rows: a warrior got the demon hunter's scenario), the
  Twinblades chest's loot link, Varedis' lines, a Vengeance intro flag, and the Dalaran class invitations for every class
  (`spell_area` end status 74 -> 1: the inviter now shows before the quest is taken). Compiled; untested in game.
- **Phase 3.3 Druid** (2026-11-09, SQL only, details in the plan): Guardian, Feral and Restoration chests linked to
  their loot, Feral's scene equip, Malithar's transform and the Guardian trial waves. Untested in game.
- **Phase 3.4 Hunter** (2026-11-09, details in the plan): **all of LegionCore-7.3.5's SmartAI changes merged** (credit
  through pets, invoker-player target 130, farthest target 28, cast flags 0x80 / 0x100 / 0x200, summon-object flags,
  gameobject flag actions, `GossipUse` firing GOSSIP_HELLO on use) and **quest object scopes** (`QuestObjectScope.h`,
  `Player::BindQuestObject` / `AddQuestDelayedEvent`: script objects and delayed actions end with the quest attempt; loot
  side not ported yet). Rule: LegionCore-7.3.5 SQL using SmartAI flags needs this merged SmartAI; check new flag values
  against existing rows when merging (`action_param3` of summon-object rows is now flags). Titanstrike / Talonclaw /
  Thas'dorah scripts and 64 SQL files; also the Dark Riders camp objects missed in 3.1. Compiled; untested in game.
- **Phase 3.5 Mage** (2026-11-09, details in the plan): Aluneth with the shared Nexus pickup (`lights_wrath_pickup.h`,
  also the priest's), Felo'melorn's loot, scene equip, Lyandra's health events and Aethas' return, Meryl's sixth wave,
  and the Death Knight Frost leftovers (Halahk, Frost souls). SQL `2026_11_10_00..14`: **`2026_11_09_` stopped at 99,
  new port files use the next day's prefix** (the updater sorts by name). Compiled; untested in game.
- **Phase 3.6 Monk** (2026-11-09, details in the plan): Sheilun (new headers, login exit and checkpoint exception), Fu Zan's
  temple, monk chests / scene equips / kite travel, and the cross-class voice files; core lifecycle / heal / before-logout
  hooks. `IsMapObjectArrivalProtected` is a stub (false) until LegionCore-7.3.5's map-arrival hold is ported (phase 7).
  Compiled; untested in game.
- **Phase 3.7 Paladin** (2026-11-09, details in the plan): Silver Hand, Truthguard, Ashbringer scenario scripts
  (CLEAN files taken, two merged) and 8 SQL files (all passed verbatim). The shared map 1500 instance pulls three warrior
  headers in early. Compiled; untested in game.
- **Phase 3.8 Priest** (2026-11-09, details in the plan): Xal'atath, T'uure (merged; cage opening hook), Light's Wrath
  takeover and Azuregos, Sliver (DK Blood leftover). Compiled; untested in game.
- **Phase 3.9 Rogue** (2026-11-09, details in the plan): Dreadblades and the Nexus instance (Bilaal), Kingslayers
  (merged), Fangs of the Devourer, Valzuun / Desmond / Modera intel. Compiled; untested in game.
- **Phase 3.10 Shaman** (2026-11-09, details in the plan): Sharas'dal's scripts and SQL, the Maelstrom wave; the guarded
  jump arrival comes with the warrior (3.12). Compiled; untested in game.
- **Phase 3.11 Warlock** (2026-11-09, details in the plan): The New Blood (scripts, quest-attempt binds, abandon reset,
  login restore), An Eye for a Scepter, Dark Whispers, Ritual Ruination, Dreadscar, the Dark Riders login exit; gossip
  secondary NPC flags checked against the second flag word and the class hall upgrade flag fixed (0x200). SQL numbering
  rolled to `2026_11_11_`; the port tool refuses an index above 99. Compiled; untested in game.
- **Phase 3.12 Warrior** (2026-11-09, details in the plan): Warswords, Earth-Warder, the Broken Shore login exit, Strom'kar,
  Commander's Valor, and the guarded jump arrivals (Skyhold, Strom'kar, Sharas'dal: arrival spells as self casts, same
  map, alive, real movement). **Phase 3 (all twelve classes) is complete.** Left out: Strom'kar's loot auto-equip with full
  bags, its blade's custom scale and crack decal (cosmetic). Compiled; untested in game.
- **Build #36 stopped at `2026_11_09_17_lc735_laith_dialogue_dispatch.sql`** (owner's log, 2026-11-09): `ERROR 1232 Incorrect
  argument type to variable 'group_concat_max_len'`. Cause: the ported SQL had only been tested on MariaDB; the owner runs
  **MySQL 8.0.44**, which differs. Found by running every pending world / hotfixes update on a real MySQL 8.0.46 in the
  container, and fixed (all files from `_17` on, none applied on the owner's realm yet): `GREATEST(@@SESSION.
  group_concat_max_len,1048576)` (unsigned with signed gives a non-integer result: both cast to UNSIGNED, 54 files);
  a temporary table read twice in one query (ERROR 1137, Feral scene equip); a `DELETE` whose subquery reads its own
  table (ERROR 1093, Titanstrike workshop: derived table); `--comment` without a space (syntax error); `groups` as an
  alias (reserved word in MySQL 8); guards not forced (MySQL cannot `PREPARE` a `SIGNAL`, ERROR 1295: 14 files forced).
  The end state matches the MariaDB copy (smart_scripts, conditions, script names, scenario rows). `port_lc735_sql.py`
  applies all of these. **Rule: test SQL on MySQL 8 before shipping.** Method: `apt-get download mysql-server-core-8.0
  mysql-client-core-8.0 libprotobuf-lite32t64 libaio1t64`, `dpkg -x` into the scratchpad, `LD_LIBRARY_PATH` to its
  `usr/lib/x86_64-linux-gnu`, `mysqld --no-defaults --initialize-insecure`, start with `--secure-file-priv=` and a /tmp
  socket, `SET GLOBAL sql_mode='NO_ENGINE_SUBSTITUTION'` (the owner's server is not strict: it applied a file strict mode
  refuses), import `sql/base/world` into `world_legion` and `sql/base/hotfixes` into `hotfixes_legion` (~1.5 min), then
  apply the files of `sql/old/<db>` and `sql/updates/<db>` in name order, skipping those in `updates`, as the updater
  does. Upstream `2026_10_28_01_russian_texts_leftovers.sql` fails on a fresh realm (it updates a table that a later
  upstream file drops); realms that already have it applied are not affected.
- **Phase 4, class spell mechanics** (2026-11-09, details in the plan): LegionCore-7.3.5's per-spec spell audits, Tier 21
  bonuses, racials and warlock legendary / pet fixes: five spell files taken whole, ten merged, the core hunks applied
  (Soul Leech, Master Harvester, Cosmic Ripple, `PlayerScript::OnDeath` merged by hand), SQL `2026_11_11_45..99` and
  `2026_11_12_00..25` (checked on MySQL 8). Compiled; untested in game.
- **Phase 5.1, Stormheim** (2026-11-10, details in the plan): LegionCore-7.3.5's Stormheim questlines (42 new scripts,
  Vrekt world quest), their core quest-credit / opening-spell / visibility pieces (`Creature::IsNeverVisible` now calls
  `CreatureAI::CanBeVisibleFor`), SQL world `2026_11_12_26..99` + `2026_11_13_00..01`, characters
  `2026_11_10_00_lc735_stormheim_mission.sql` (checked on MySQL 8). Compiled; untested in game.
- **Phase 5.2, Highmountain** (2026-11-10, details in the plan): 23 new Highmountain scripts, Rockaway Coast, the Hag Feathers
  stack hotfix and Addie's hunting credit, SQL world `2026_11_13_02..41` (checked on MySQL 8). Compiled; untested in game.
  Owner, 2026-11-10: finish all of phase 5 (Val'sharah, Azsuna) before the next build and test.
- **Phase 5.3, Val'sharah** (2026-11-10, details in the plan): 4 new Val'sharah scripts, the Darkheart Thicket escape, SQL world
  `2026_11_13_42..45` and characters `2026_11_10_01..02` (objective slot migration; checked on MySQL 8). Compiled; untested
  in game. The container's MySQL 8 stops between turns: `<scratchpad>/my8start.sh` restarts it.
- **Phase 5.4, Azsuna** (2026-11-10, details in the plan): Scythe of Souls, Faronaar, Azurewing Repose, Save Yourself,
  Nar'thalas Academy, Grey Shoals, the Eye of Azshara quests and Dalaran pads; `GameObject::IsNeverVisible` asks
  `GameObjectAI::CanBeVisibleFor`; SQL world `2026_11_13_46..63`, characters `2026_11_10_03..04` (checked on MySQL 8).
  Compiled; untested in game. **Phase 5 complete**: next is the owner's build and test.
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
- `DungeonFinder.DebugJoin` (worldserver.conf.dist) shipped as 1, i.e. one player alone forms a dungeon
  group at once; set to 0 (its documented default) on the owner's request, 2026-11-05, so the dungeon finder
  works normally and the playerbots fill gets its time.
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

## AshamaneCore bots (read 2026-11-05)

`Brytenwally/AshamaneCore-Legion735`, `src/server/scripts/Custom/Bots/` (~6.7k lines: `BotMgr.cpp`
4.2k, `BotMgr.h`, `bot_commandscript.cpp` = `.bottest` GM commands, `bot_dungeonclear_control.cpp`).
Its README lists only GM-command-driven actions as verified (login/logout, move, attack, death/
graveyard, gear from a pool, group join, follow); "no autonomous state machine".
- **Worth taking:**
  - `g_BotSpecRotations` (`BotMgr.cpp` ~2612-3180): a priority list for **all 36 specs** (3-6 steps
    each), with role, interrupt and dispel spell per spec. Steps name spells by **English name**,
    resolved once against `Spell.db2` (`ResolveSpellIdByName`: exact name, class spell family first,
    highest id among duplicates, one-match fallback across classes; `.bottest diagspells` reports
    unresolved names). Conditions: Always, TargetHealthPctBelow, SelfHealthPctBelow,
    ResourceAtLeast, Aura Missing/Present On Self, AuraMissingOnTarget; target override (Disc
    Atonement). Each spec carries the author's confidence note (Demonology marked low). Simple
    lists, not optimal rotations; not live-verified per spec.
  - Ground-effect dodge (`FindHarmfulGroundEffectUnderBot` / `ProcessBotMechanicReactions`): harmful
    `DynamicObject` whose radius covers the bot -> `MovePoint` radially out (radius + 5 yd), before
    the rotation; then interrupt, then dispel.
  - Artifact weapon per spec (`BotArtifactTable`, ~2045: 36 spec -> item entry, from Artifact.db2 +
    ItemSparse) for step 5; its trait budget (`SkillBotArtifact`) is a rough rule (1 rank per 2
    levels above 98).
- **Not usable as is:** login by faking CharEnum + `HandlePlayerLoginOpcode` (we have bot
  sessions); everything runs from `BotMgr::Tick` on the world thread and walks group members'
  `Player*` (fine on TrinityCore, where maps do not update during world scripts; **not** on this
  core, whose maps run on their own threads); group join by `Group::AddMember` directly (we queue
  the client packet); gear from `world.bot_equipment_pool` (58k rows, kept private, not in the
  repo); dungeon clear mode (autonomous boss navigation: step 6/7 territory); Ollama chat.

## Combat plan (step 4, from LegionBotAI; mapped 2026-11-03)

### What LegionBotAI has (LegionCore-7.3.5, `src/server/scripts/Custom/LegionBotAI.cpp`, 2,567 lines)
| Piece | Where | Port? |
| --- | --- | --- |
| `BotCast(bot, target, spellId, minCooldown)`: checks spell exists, level, `HasSpell`, cooldown, GCD (`GetGlobalCooldownMgr`), not already casting, an own per-spell throttle (max of 1.5 s, RecoveryTime, CategoryRecoveryTime); faces the target, `CastSpell(target, spellInfo, false)`, checks `SpellCastResult` | ~736 | yes, as the one casting primitive |
| Rotations `CastClassAbilities`: Blood DK (Vampiric Blood / Icebound / Dancing Rune Weapon on low HP, Death Strike < 70%, Marrowrend for Bone Shield < 5 stacks), Frost DK (Frost Strike at RP > 70, Obliterate), Ret/any Paladin (Judgment, Crusader Strike), Rogue (Eviscerate at 4 CP, Backstab), Fury Warrior (Enraged Regeneration < 40%, Battle Cry, Execute < 20%, Rampage at 85 rage, Bloodthirst, Raging Blow), Priest (Holy Fire, Smite) | ~1008 | yes, as per-spec priority lists |
| Healing `CastHealAbility`: Holy Priest (Guardian Spirit < 25%, Prayer of Healing if 3 hurt in 15 yd, Serenity < 75%, Flash Heal < 55%, Renew, Heal), Holy Paladin (Lay on Hands < 25%, Light of Dawn, Holy Shock, Flash of Light, Bestow Faith, Holy Light); `FindLowestHpAlly` (< 85%, tank first once < 75%); triage: < 40% heal before dispel | ~931, ~1100 | yes |
| `TryDispel`: removable dispel types read from the dispel spell's effects (`GetSimilarEffectsMiscValueMask(SPELL_EFFECT_DISPEL)`), `GetDispellableAuraList`, harmful auras only (Cleanse 4987, Purify 527) | ~853 | yes |
| `TryInterrupt`: current generic/channelled spell, `IsInterruptable`; Mind Freeze 47528, Pummel 6552, Hammer of Justice 853, Holy Word: Chastise 88625 | ~889 | yes, list per spec |
| `MaintainBotBuff`: Beacon of Light 53563 / Beacon of Faith 156910, Prayer of Mending 33076 (aura 41635) on the tank | ~972 | yes |
| Targeting rules: **never pull**, only units attacking a team member (`IsPartyAggressor`); 1.2 s engage delay so the owner swings first; 30 yd max; `Attack` + `MoveChase`; drop a target that attacks nobody; self-defence always | ~2350-2500 | yes, the core of it |
| Tank: taunt anything on a teammate (Dark Command 56222 / Death Grip 49576, Hand of Reckoning 62124, Taunt 355); `player tank` mode (owner holds aggro) | ~2440 | yes |
| Assist modes `full` / `defend` (only what hits the owner or the bot) / `chill` (only ordered targets); `.lbot attack` | ~2350 | yes, as orders |
| Formation by role (tank 2.5 yd, dps 4 yd spread, healer 9 yd behind the owner) | ~177 | later (formation) |
| Potions from bags (76097 / 76098, 30 s own cooldown) | ~1062 | yes, bag items only |
| Specs with spells: Blood DK 250, Frost DK 251 (partial), Holy Paladin 65, Prot Paladin 66 (taunt only), Fury Warrior 72, Holy Priest 257, Sub/Assa Rogue (2 spells). **No** Warlock, Mage, Hunter, Shaman, Druid, Monk, DH | | others written per spec |

### What LegionBotAI does that is NOT ported as is
- Threading: it runs every bot from the **owner's** `OnUpdate` and reaches bots with
  `ObjectAccessor::FindPlayer` (only safe on the bot's own map thread) and moves them between maps
  by hand (`RemovePlayerFromMap` / `AddPlayerToMap`). Here each bot runs its own combat on its own
  thread (`PlayerScript::OnUpdate`); teammates are read with `ObjectAccessor::GetPlayer(*bot, guid)`
  (same map only); map changes go through the core's teleports.
- Changes to the character: `GiveLevel` level sync, `LearnBotSpells` / `LearnBotTalents`, MoP
  crafted gear sets (`GetBotGear`), so alts would be changed without an order: owner's alt rule.
- Cheats: `target->AddThreat(bot, 120)` every tick for tanks, revive with full health when combat
  ends, `FixFallenCreatures` (repack vmap workaround). Need the owner's decision before porting.
- Its own bot accounts/characters SQL, companion creatures (`LegionBotMgr.cpp`), trace logs.
- Gearing (~424-735), read 2026-11-03, **not a reference for step 5**: `EquipBotGear` (level sync
  every 2 s and `.lbot autogear`) **destroys every equipped item** and equips new ones (alt rule).
  `PickItemForSlot` scans `sItemSparseStore` for the highest item level, then quality, under caps
  by level (`IlvlCapForLevel`: level x2+6 to 60, x3 to 80, x5 to 90, x8 to 100; quality white/
  green/blue/epic by level), no heirlooms, scaling or unique items. No stat weights (its primary
  stat argument is unused), no bonus IDs, no `Game.Patch` tiers. Armor/weapon per class is
  hard-coded: DK/Warrior plate STR two-hander, Paladin plate INT mace + shield (any spec), Rogue
  leather dagger, **every other class cloth + staff**. At max level it gives MoP crafted sets
  (ilvl 384, `GetBotGear`). Step 5 ports SkyFire's autogear (stat weights, slot-by-slot upgrades,
  old piece kept) instead, with Legion item levels from bonus IDs.

### Steps (each one a build and a test on the owner's server)
| # | Result | Test |
| --- | --- | --- |
| 4.1 | **Done 2026-11-05 with 4.2** (works on the owner's server, build #17) (see LegionCore-Playerbots' `CLAUDE.md`, "Combat"). Combat skeleton, no spells: role from the spec (`ChrSpecializationEntry` role), targeting rules above (never pull, 1.2 s delay, 30 yd), `Attack` + chase (melee) or stay at range (casters), back to follow after combat; `attack` order (owner's target), assist modes by order (`assist full|defend|passive`) | bot melees what attacks the group, never pulls |
| 4.2 | **Done 2026-11-05 (build #17): AshamaneCore's 36-spec table instead of writing specs one by one.** `BotCast` + per-spec rotation table (priority list of conditions) + log of each cast/refusal (like the quest logs); first specs: LegionBotAI's DPS (Fury Warrior, Frost DK, Ret/Prot Paladin, rogue) **plus Warlock** (the owner's test bot; not in LegionBotAI) | bot casts its rotation; log shows why a spell is skipped |
| 4.3 | **Built 2026-11-05, untested** (LegionCore-Playerbots' `CLAUDE.md`, "Combat"). Healers: lowest-HP ally among group members on the bot's map, triage, Holy Priest / Holy Paladin from LegionBotAI, dispel, beacons / Prayer of Mending | healer keeps the group up |
| 4.4 | **Built 2026-11-05, untested** (LegionCore-Playerbots' `CLAUDE.md`, "Combat"). Tanks: Blood DK rotation, taunt mobs off teammates, `player tank` order; interrupts per spec | tank holds mobs |
| 4.5 | **Built 2026-11-05, untested** (LegionCore-Playerbots' `CLAUDE.md`, "Survival"). Defensives and potions from bags; regen out of combat (SkyFire eat/drink) | low HP handled |
| 4.6 | Remaining specs one class at a time (all 36 + DH), in the order the owner plays them | per spec |

Owner's decisions (2026-11-03):
- **No tank threat bonus** for now (LegionBotAI's `AddThreat(bot, 120)` stays out).
- **Revive only by order**: a dead bot is revived when its owner says `summon` in party/raid chat
  (not by whisper), then teleports to him; `Playerbots.Summon.Revive` 1 = only while the owner is
  out of combat (default), 2 = also in combat, 0 = never. No revive when combat ends.
- **Spells and talents are learned automatically, alts included**; on any bot (alts too) only
  empty talent rows are filled, a row the player already chose keeps its talent
  (`BotTalents::LearnTalents` does exactly that). Wired in with the specs.
- **Specs**: chosen later; step 4.1 (no spells) does not need them.

## Plan (agreed outline)

| Step | Result | Size |
| --- | --- | --- |
| 0 | **Done 2026-11-01** (green Windows build). Windows build workflow in `LegionCore-Build` (pattern: SkyFire-build; match Reforged's Boost/OpenSSL/MySQL requirements) | ~1 session |
| 1 | **Done 2026-11-01** (see above; verified on the owner's server 2026-11-02: both modules log at startup). Module system: CMake also lists `modules/<name>/src` as script modules (`Add<name>Scripts`), module `.conf.dist` copied and loaded, module SQL (create tables from code at startup like mod-destiny-weaver, or hook the DB updater later), a hello-world module, `modules/README.md` | 1-2 sessions |
| 2 | **Done 2026-11-02** (see "Bot sessions"; verified on the owner's server with build #5, core `b1befd3`). Bot session core hooks: ticking socketless sessions, `WorldSession::IsBot()`, logout/save, normal teleports | 2-4 sessions |
| 3 | **Group/orders/quests part done** (verified on the owner's server 2026-11-03, builds #10-#14: group invite, follow, stay/follow by whisper and party chat, summon across maps, leave, quest mirroring incl. class and previous-quest copies, manual quest orders). Strategy engine still to port. Port the SkyFire-playerbots skeleton into `LegionCore-Playerbots`: `.playerbots add/remove/list`, group, follow/formation, whisper orders, `co`/`nc` strategy engine. Translate uint64 guids to `ObjectGuid` (128-bit), Legion API names; drop glyphs and hit/expertise caps | 1-2 weeks of sessions |
| 4 | Combat, from LegionBotAI (see "Combat plan"): targeting rules, `BotCast`, per-spec rotations, healers, tanks, interrupts | ~1-2 weeks |
| 5 | LFG, loot rolls, gear init by item level/bonus IDs within the `Game.Patch` tier, artifact weapon + traits. **Started 2026-11-05** (owner's request, for testing): `.playerbots create <class> [spec]` (bot accounts, character creation, init to the creator's level), stat-weight gear (Ask Mr. Robot scales from Pawn) by `autogear`, `talents` order; see LegionCore-Playerbots' `CLAUDE.md`. **Dungeon finder built 2026-11-05, untested**: party bots answer role checks and proposals, idle pool bots fill missing roles of a queued player after 30 s (core accessors `LFGMgr::GetQueueIds` / `GetRoleCheckRoles` / `GetPendingProposal`, topic commit "Add LFG accessors for modules"); also `.playerbots self` and formations. Loot rolls, bonus IDs, tiers and artifacts still to do | ~1-2 weeks |
| 6 | More specs (36 + Demon Hunter), random bot pool (accounts/characters to fill LFG, BG/arena queues), BG/arena | months |
| 7 | **Last, by the owner's priority:** random bots' open-world activity (questing on their own, level-appropriate zones, grinding, wandering) | after everything else |

Legion differences to plan for: artifacts (weapon, traits, relics, Knowledge tiers), 7 talent
tiers plus PvP talents, class resources (Maelstrom, Insanity, Fury, Pain, Astral Power...),
secondary stats crit/haste/mastery/versatility only, item level from bonus IDs, Legion phasing.

