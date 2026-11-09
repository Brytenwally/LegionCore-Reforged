# Porting LegionCore-7.3.5's work into Reforged

Owner's request (2026-11-09): look at how `Brytenwally/LegionCore-7.3.5` does class halls and artifacts, map every
improvement its developer made after both forks left the UWoW leak, and bring over as much as possible, in steps.

LegionCore-7.3.5 stays a **read-only reference**: its code is read there and rewritten / merged here, one phase at a
time, each phase built and tested in game by the owner before the next.

## Where the forks split

Both forks share upstream LegionCore commit `fed58f79` (2025-04-19). Since then LegionCore-7.3.5 has 1,566 commits:
5,081 files, ~670,000 added lines, but most of that is its developer's test scaffolding (`tools/tests`,
`tools/research`: 2,605 files) and audit notes (`docs/audits`: 1,034 files). The real work is:

| Part | Files | Added lines |
| --- | --- | --- |
| C++ (`src/`) | 649 | ~119,000 |
| World SQL updates | 739 | |
| Characters / hotfixes / auth SQL updates | 16 | |

Reforged's history is one squashed snapshot, so each changed file was compared by content with its `fed58f79`
version (`legioncore-735-files.tsv`, next to this file; Reforged moved some folders, files are matched by name):

| Status | Files | Added lines | Meaning |
| --- | --- | --- | --- |
| `CLEAN` | 227 | 41,946 | Reforged still has the leak's version: LegionCore-7.3.5's file can be taken as is |
| `NEW` | 224 | 56,908 | new in LegionCore-7.3.5: copy it and register it in its script loader |
| `BOTH_CHANGED` | 191 | 15,815 | both forks changed it: merge by hand |
| `NEW_BUT_EXISTS` | 5 | 4,554 | both forks wrote their own (Grimrail Depot): compare, keep the better |
| `MISSING_IN_REFORGED` | 2 | 73 | removed in Reforged |

## Feature map (what LegionCore-7.3.5 has and Reforged does not)

| # | Area | What it is | LegionCore-7.3.5 pieces | Fit with Reforged |
| --- | --- | --- | --- | --- |
| A | **Artifact choice onboarding** | The class hall "choose your artifact" panel (PlayerChoice 235-266, all 36 weapons) records the hidden "<spec> Chosen" quest, then credits the class hall quest's objective and starts that weapon's acquisition quest; a stale panel cannot restart a finished route | `ClassHalls/class_hall_artifact_onboarding.cpp`, core hook `PlayerScript::OnPlayerChoiceResponse`, a condition recheck in `HandleChoiceResponse` | Reforged's handler only casts the reward spell; the choice ids and responses are the same in both databases (checked) |
| B | **Artifact acquisition questlines and scenarios** | The weapon chains themselves, end to end: Strom'kar, Earth-Warder, Warswords, Silver Hand, Truthguard, Ashbringer, Titanstrike, Talonclaw, Thas'dorah, Kingslayers, Dreadblades, Fangs, T'uure, Xal'atath, Light's Wrath, Maw / Blades / Apocalypse, Doomhammer, Fist of Ra-den, Sharas'dal, the mage, warlock, monk, druid and demon hunter weapons: scene and pickup scripts, chests with real loot, artifact equip, return flights to the class hall, scenario checkpoints that survive a logout | `ClassHalls/*.cpp` (6,438 lines, new), `Scenario/Artifacts/*` (4,704 clean, 5,493 new, 2,253 merged), ~200 SQL updates; core: scene completion, artifact force-equip (`SpellEffects`), artifact purchase validation (`ArtifactHandler`), scenario resume (`PlayerScenario`, `ScenarioResume.h`), SmartAI scenario-step conditions without a player invoker | the largest and most valuable part; Reforged moved these scenarios to `Legion/Scenario/Artifacts` |
| C | **Class spell mechanics** | Legion spell fixes for every class (Affliction, Frost / Unholy DK, Resto Druid, warlock pets, Lightforged racials, Burning Rush, interrupts...), removal of pre-Legion leftovers | `Spells/spell_*.cpp`: hunter, shaman, warrior, mage, DK clean (4,434 lines); priest, rogue, monk, warlock merged (3,411); `SpellEffects` / `SpellAuraEffects` / `Spell.cpp` / `Unit.cpp` | helps players and the playerbots' rotations |
| D | **Legion zone questlines** | Stormheim (fleet, Helheim, Valor, Haustvald), Highmountain (Underking, Skyhorn, Blood), Val'sharah rituals, Azsuna, Dalaran / Broken Shore routing, The New Blood and Dreadscar (warlock), Dark Riders (DK) | `Legion/stormheim_fleet.cpp`, `azurewing_repose.cpp`, `rockaway_coast.h`, `new_blood.cpp`..., ~110 SQL updates | mostly new files |
| E | **Raid and dungeon fixes** | Trial of Valor (Helya), Tomb of Sargeras (Mariner, Mythic Grimelord), Darkheart Thicket, The Stockade, raid re-entry and ghost / corpse handling | `Legion/TrialOfValor`, `DarkHeartThicket`, `EasternKingdoms/TheStockade`, `Map.cpp` | some merged with Reforged's own raid work |
| F | **Loot** | Raid chests (earned rewards kept on close, personal cooldowns), LFR personal loot, recipients eligibility, quest-object loot ownership | `LootMgr`, `Player`, `Unit`, `LootHandler` | Reforged also reworked loot: merge carefully |
| G | **Engine robustness** | Teleports and map arrivals (pending destinations, rejected transfers, Dalaran floor streaming), transports (Heroic Leap / Charge on moving ships), instance departures, logout while loading, time sync, socket diagnostics | `Player`, `Map`, `MapManager`, `MovementHandler`, `WorldSocket`, `Transport*` headers | Reforged rewrote much of this: only targeted fixes, one at a time |
| H | **Older scenarios** | Pandaria scenarios (Proving Grounds, Dagger in the Dark, Brewmoon, Ragefire, High Seas, Blood in the Snow, ...: 25,000 clean lines), new Isle of Thunder, Fall of Theramore, Noodle Time, Cataclysm / Classic scenarios | `Scenario/*` | easy for the clean ones, low priority on a Legion server |
| I | **Shop and character boost** | BattlePay boost wire format, store icons, boost tutorial scenario (`BoostExperience`), boost action bars | `BattlePay/*`, `Scenario/BoostExperience` | Reforged has its own shop and removed the custom shop items (2026-11-04): owner's decision |
| J | **Legionfall contributions** | Broken Shore buildings (Mage Tower, Command Center, Nether Disruptor) contribution states | `Contribution/ContributionMgr` (+349) | small, self-contained |
| K | Not ported | LegionBotAI and its SQL | `Custom/LegionBot*` | replaced by the playerbots module |

## Phases

Each phase: port the code (clean files taken, new files copied and registered, merged files merged by hand), review
its SQL updates against Reforged's database (ids, conditions, rows Reforged changed since), write them as new
`sql/updates` files that name their LegionCore-7.3.5 source, compile, then a build and the owner's test in game.

| Phase | Content | Size |
| --- | --- | --- |
| 1 | **A: artifact choice onboarding** (hook, recheck, script) | small; done 2026-11-09 |
| 2 | B shared support. **Done 2026-11-09**: artifact force-equip (`Spell::EffectForceEquipItem`), post-scaling damage hooks (`InstanceScript::OnCreatureDamageTaken`, `UnitAI::DamageTakenAfterScaling`), SmartAI scenario-step conditions on the base actor (371 conditions), the artifact chests (`go_artifact_acquisition`, real loot only, shop copies replay the event), scenario checkpoints across logouts (see below), scene instance retirement (`Player::SendSpellScene` / `SceneCompleted` / `TrigerScene`), Netherlight Crucible validation and empowerment right after a power purchase (`ArtifactHandler`) | medium (core merges) |
| 3.x | B per class, one class per step, **alphabetical** (owner, 2026-11-09): Death Knight, Demon Hunter, Druid, Hunter, Mage, Monk, Paladin, Priest, Rogue, Shaman, Warlock, Warrior. **3.1 Death Knight, 3.2 Demon Hunter, 3.3 Druid done 2026-11-09** (see below); LegionCore-7.3.5's files that mix classes (`2026_10_29_24_artifact_boss_speakers`, `2026_10_30_20_artifact_combat_stories`, `_23_artifact_trash_voice`, `_24_artifact_owner_voice`...) are ported in one sweep after the last class | 12 steps |
| 4 | C: class spells, clean files first, then the merged ones | medium |
| 5 | D: zone questlines, Stormheim first (76 SQL updates), then Highmountain, Val'sharah, Azsuna | large |
| 6 | E + F: raid / dungeon and loot fixes | medium |
| 7 | J (contributions), then G item by item as the owner hits problems | small, ongoing |
| 8 | H and I if the owner wants them | optional |

## Notes for whoever continues

- The manifest's status was computed against Reforged on 2026-11-09; recompute it before a phase (a file Reforged
  changed since then becomes `BOTH_CHANGED`). Script: compare `git show fed58f79:<path>` (in a LegionCore-7.3.5 clone
  deep enough to hold `fed58f79`, `git fetch --depth=3000`) with Reforged's file, line endings ignored.
- LegionCore-7.3.5's `docs/audits/*.md|json` explain most of its fixes (cause, evidence, verification): read the
  matching audit before porting a fix.
- Its SQL updates often build on each other (restore / archive / re-apply files): port the final state of the rows,
  not every intermediate file.

## Method that works (2026-11-09)

- **A copy of this realm's world database** in the container: MariaDB (`apt-get install mariadb-server`, started
  with a short `--socket`, `--pid-file` in /tmp and `--skip-grant-tables`), `sql/base/world/*.sql` imported (MySQL 8
  collations `utf8mb4_0900_*` replaced), the hotfixes base as `hotfixes_legion` (cross-database updates), then every
  `sql/old` / `sql/updates` world file the base's `updates` table does not list, by file name (~1 min + ~2 min). The
  only errors are MariaDB charset conversions in upstream's Russian-text clean-ups.
- **Run LegionCore-7.3.5's SQL files verbatim on it**: their newer files hash the rows they expect and `SIGNAL` on a
  difference, so a pass proves our rows match. For a refusal, run only the hash `SET`s and compare each with its
  expected value to see which table differs, then judge whether the difference matters.
- **Ship with the guard forced to pass** (`docs/tools/port_lc735_sql.py`; its logic: insert
  `SET @x_safe=1, @x_old=1;` before `SET @x_guard=IF(...)`, add a provenance header, name the file
  `<date>_<nn>_lc735_<original name>.sql`): on the owner's server a failing guard would stop the updater at startup.

## Phase 3.1: Death Knight (2026-11-09)

LegionCore-7.3.5's own audit marks all three as "client traversal pending": its developer had not played them
through either.
- Maw of the Damned (Blood, quest 40740, scenario 940 step 9, map 1489): the pickup object 247566 gets its real loot
  (`Data1`, conditions), and scene 1168's completion casts equip spell 200134 (`scene_blood_artifact_equipment`
  refuses outside the scenario step); Fleshripper (101778) speaks his own lines.
- Blades of the Fallen Prince (Frost, 38990, scenario 901 step 6, map 1480): pickup 247386 real loot, scene 1071 ->
  equip spell 197092 (`scene_frost_artifact_equipment`); the scene start spells 190435 / 200152 recheck at cast.
- Apocalypse (Unholy, 40930-40935 / 40986-40987, the Dark Riders scenario 988 on map 1533, shared with the warlock's
  Ulthalesh): `dark_riders_artifact.cpp` (instance script: stage actors, Ariden's finale kept alive through a scaled
  killing blow, ambient spirits), chest 246699 through `go_artifact_acquisition`, Laith Sha'ol's dialogue, the Dark
  Riders SmartAI fixes (stage lifecycle, player arrival, Revil handoffs, scythe jump), and the Death Knight scenario
  entrances (`lfg_entrances`).
- SQL: `sql/updates/world/2026_11_09_01..23_lc735_*.sql` (LegionCore-7.3.5 files 2026_09_19_147, 2026_10_15_25..27,
  2026_10_20_01..12, 2026_10_29_11, 2026_10_30_07..11, 2026_10_30_14). 19 applied verbatim on the database copy;
  Laith, Fleshripper, Frost scene equip and scene start refused only because of unrelated columns (broadcast text ids
  upstream added to `creature_text`, the English objective descriptions, and the Frost file the scene start depends on).

## Phase 2 completion (2026-11-09)

- **Scenario checkpoints** (LegionCore-7.3.5 `ebe32f73` and follow-ups; `Entities/Player/PlayerScenario.cpp`,
  `Scenarios/ScenarioResume.h`, column `characters.scenarioResume` by `sql/updates/characters/2026_11_09_00_...`):
  entering a quest scenario records its entrance (map, scenario, instance id, server boot time, position;
  `Map::AddPlayerToMap` -> `RecordScenarioEntrance`), saved with the character; a completed run or leaving the scenario
  clears it. At login (`LoadFromDB` -> `PrepareScenarioLogin`) the player rejoins the same live instance (same boot,
  same instance id: `FindMap`) where he stood, else a fresh run of that scenario (`SetScenarioId`) at the recorded
  entrance. The login check (`MapManager::CanPlayerEnter`, login) now admits a scenario only with that checkpoint (or
  the Broken Shore intro, map 1460, while its quest is unfinished). Reforged's own check let any login stay in a
  scenario whose instance existed, even a fresh step-zero one at a later coordinate. Characters logged out inside a
  scenario before this update have no checkpoint: they leave it once (go-back trigger, dungeon finder entry point or
  home), as before when the instance was gone. Not ported yet, with their class phases: the per-quest exits (Dark
  Riders for the warlock, Broken Shore warrior finale, Sheilun), the New Blood / Scepter warlock login restores, and
  the map 1500 class route (ported in 3.2).
- **Scene instances** (`c49051e2`): cancelling a scene retires every instance of it, not only the last; a completion or
  trigger of an instance replaced by a newer one of the same scene does nothing; a completed scene is not completed twice
  and its status no longer drops back to "triggered".
- **Artifacts** (`3d9c710e`, `77fb6f0f`): Reforged already had most relic checks (slots, talent indices, socketed
  relic, one pick per tier). Added: the Crucible is usable only at 110 after "An Offering of Light" and "An Offering of
  Shadow" (48559, 48560) or achievement 12184; the first relic talent before the others; the third tier follows the
  second's branch (3 after 1, 5 after 2, 4 after either); `UnLockThirdSocketIfNeed` right after a power purchase, so
  empowerment comes at once instead of at the next login. LegionCore-7.3.5's `AddOrRemoveSocketTalent` returning a
  result is not ported (ours returns nothing).

## Phase 3.2: Demon Hunter (2026-11-09)

LegionCore-7.3.5's audits mark both routes "client traversal pending" (its developer had not played them through).
- **Map 1500 class route** (`Scenarios/BrokenShoreClassRoute.h`, `InstanceMap::AddPlayerToMap`, `PlayerScenario.cpp`;
  SQL `2026_11_09_26_lc735_broken_shore_class_routes.sql`): map 1500 holds warrior 933 (dungeon finder entry 1176, whose
  ScenarioID is 0), paladin 775 (906) and Vengeance 961 (1166), difficulty 12. Our `scenario_data` rows had no class,
  difficulty or dungeon, so `GetScenarioOnMap` returned the last row (961) for anyone and the first dungeon of the map
  (906) was taken: a warrior got the demon hunter's scenario (LegionCore-7.3.5's report). Now each of the three classes
  gets its own scenario and dungeon, an instance already running another class's scenario refuses the player, a stale
  scenario selector of another class is dropped, and a checkpoint resumes only the class's own scenario.
- **Havoc, Twinblades of the Deceiver** (quests 39247 / 41119 "The Hunt", scenario 900 on map 1498):
  `2026_11_09_27_lc735_twinblades_real_loot.sql`: chest 251878 (summoned by Varedis' death) had its loot row but
  `Data1` 0, so it gave nothing; `Data1` = 251878, with LegionCore-7.3.5's 140 conditions (demon hunter, map 1498,
  scenario 900 step 4/5, either "The Hunt", no Twinblades yet). Scene 1063 then equips them (201012 -> 201010, effect 249:
  phase 2's force-equip). Varedis (94836) and Allaris (110556) speak their own combat lines instead of a pet victim
  (`2026_11_09_28_lc735_acquisition_combat_speakers.sql`, written by hand: LegionCore-7.3.5's guard also hashes
  warlock rows not ported yet).
- **Vengeance, Aldrachi Warblades** (41863 / 40249, scenario 961): Caria Felsoul's second intro line tested the first
  flag (`DH_vengeance_art_scenario.cpp`); the pickup script (`go_bs_the_aldrachi_warblades`) is unchanged in both forks.
- **Dalaran class invitations, every class** (`2026_11_09_25_lc735_dalaran_class_invitations.sql`): the 22 Dalaran /
  class campaign invitation auras (`spell_area`, areas 7502 / 7581) applied only once their quest was taken (end status
  mask 74); now while it is not taken (1), so the inviting NPC shows before the quest. Quests 40714 (DK) and 41035
  (mage) limited to their class; Eitrigg's (93775) accept event listened for the warlock's quest 40716 instead of 41052,
  so the summoned Horde warrior messenger never left when the quest was accepted. Complements the class hall messengers.
- All four SQL files applied (and re-applied) on the database copy; the three LegionCore-7.3.5 guards passed verbatim
  (route, Twinblades) or were replaced (speakers). Compiled; untested in game.

## Phase 3.3: Druid (2026-11-09)

SQL only (LegionCore-7.3.5 changed no druid C++); its audits mark the routes "partial / client traversal pending".
- **Balance, Scythe of Elune**: shares the Dark Riders scenario (988, map 1533) ported in 3.1; its chest 246437 is in
  `go_artifact_acquisition` already.
- **Guardian, Claws of Ursoc** (40647, scenario 990): chest 248853 linked to its loot, final-stage conditions
  (`2026_11_09_29`); Malithar waits for his real Bear Form before his dialogue and the claw clean-up (`_33`, guard forced:
  only `creature_text` differed, the broadcast text ids again); the trials' waves: each death credit is its own root
  (links broken), a wave ends only when no Shadowstalker of it is left, and the "nobody left" checks look around the
  dying creature, not its killer, with a unit killer required (`_34`, `_35`; guards forced: the hashed rows differed
  from both of LegionCore-7.3.5's states while the rows they change have the expected events, links and conditions).
- **Feral, Fangs of Ashamane** (42430, scenario 1108): chest 250386 linked (`_30`); scene 1282's completion casts the
  equip spell 214715 (`spell_scene_event`, conditions: scenario 1108 step 6/7, quest, the Fangs in the bags) (`_31`).
- **Restoration, G'Hanir** (41689, scenario 1061): chest 248499 linked, final-stage conditions (`_32`).
- Applied on the database copy: the four loot / scene files verbatim (guards passed), the other three forced.
