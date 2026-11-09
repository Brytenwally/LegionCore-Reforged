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
| 2 | B shared support. **Done 2026-11-09**: artifact force-equip (`Spell::EffectForceEquipItem`), post-scaling damage hooks (`InstanceScript::OnCreatureDamageTaken`, `UnitAI::DamageTakenAfterScaling`), SmartAI scenario-step conditions on the base actor (371 conditions), the artifact chests (`go_artifact_acquisition`, real loot only, shop copies replay the event). **Left**: scenario resume after logout (`PlayerScenario.cpp`, `ScenarioResume.h`), scene instance retirement (`Player`), artifact empowerment / Netherlight Crucible validation (`ArtifactHandler`, 7.3 content: check against `Game.Patch` first) | medium (core merges) |
| 3.x | B per class, one class per step, **alphabetical** (owner, 2026-11-09): Death Knight, Demon Hunter, Druid, Hunter, Mage, Monk, Paladin, Priest, Rogue, Shaman, Warlock, Warrior. **3.1 Death Knight done 2026-11-09** (see below) | 12 steps |
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
