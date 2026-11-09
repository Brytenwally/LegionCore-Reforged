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
| 3.x | B per class, one class per step, **alphabetical** (owner, 2026-11-09): Death Knight, Demon Hunter, Druid, Hunter, Mage, Monk, Paladin, Priest, Rogue, Shaman, Warlock, Warrior. **3.1 Death Knight, 3.2 Demon Hunter, 3.3 Druid, 3.4 Hunter, 3.5 Mage, 3.6 Monk, 3.7 Paladin, 3.8 Priest, 3.9 Rogue, 3.10 Shaman, 3.11 Warlock done 2026-11-09** (see below); LegionCore-7.3.5's files that mix classes (boss speakers, combat stories, trash / owner voice, Mist Warder) were ported with the Monk (3.6), whose Fu Zan files build on them | 12 steps |
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

## Phase 3.4: Hunter (2026-11-09), with shared SmartAI and quest object support

LegionCore-7.3.5's developer recorded Titanstrike as confirmed in his client (acquisition and power upgrade); the other
two routes are "partial / client traversal pending" in its audits.

Shared engine support, needed by Hunter and by the classes after it:
- **SmartAI, all of LegionCore-7.3.5's changes** (`SmartAI.*`, `SmartScript.*`, `SmartScriptMgr.*`, three-way merged
  against `fed58f79`; 5 conflicts resolved by hand): progress and kill credit through pets and guardians go to their
  player (`GetScriptCreditPlayer`), `SMART_TARGET_ACTION_INVOKER_PLAYER` (130) and `SMART_TARGET_FARTHEST` (28), cast flags
  `SMARTCAST_INVOKER_PLAYER` (0x80), `SMARTCAST_RETRY_HEALTH_THRESHOLD` (0x100), `SMARTCAST_RETRY_LINK` (0x200, bounded
  retries of a one-shot linked self cast, also serviced on a corpse: `CreatureAI::UpdateCorpseAI`, called from
  `Creature::Update`), summon-object flags (`action_param3`: 0x2 retry on failure, 0x4 no creator), the gameobject flag
  actions (104-106) implemented, `StartPath` reports failure so a waypoint start retries, random emote / phase / action
  list off-by-one fixed, **`SmartGameObjectAI::GossipUse`: using a SmartAI gameobject fires its GOSSIP_HELLO event** (as on
  TrinityCore; LegionCore-7.3.5's SQL relies on it), `IsTextPending` / `HasRunEvent` for scripts. Checked against our
  data: no existing row uses the new flag values by accident (summon-object `action_param3` = 1 on 18 rows: no meaning).
- **Quest object scopes** (`Entities/Object/QuestObjectScope.h`): objects a script binds to a quest attempt
  (`Player::BindQuestObject`) and delayed actions tied to it (`AddQuestDelayedEvent`, `QuestStatusData::ScriptLifetime`)
  end when the quest is abandoned, failed back to none or restarted (`CancelQuestObjects` in `AddQuest`,
  `SetQuestStatus(NONE)`, `RemoveActiveQuest`); summons, summoned gameobjects and conversations inherit the scope; an
  expired object despawns / deletes itself and runs no SmartAI; spell target searches skip another attempt's objects
  (`WorldObjectSpellTargetCheck`). **Not ported yet**: the loot side (a scoped corpse / chest lootable only by its owner:
  `LootMgr`, `LootHandler`, `Player::SendLoot`; phase 6), and the New Blood / Sheilun bindings in `Map::SummonCreature`
  (with the warlock and monk). `Player::ScopedTeleportDelay`, `HasPendingTeleport`; gameobjects remember their summoner
  (`GetSummonerGUID`); `SummonGameObject` cancels and cleans up when the map refuses the object.
- **Missed in 3.1, added**: the Dark Riders camp objects (Ariden's camp, map 0: clue pages and journals usable only in
  their investigation stage, `GameObject::Use` / `ActivateToQuest` / update flags; LegionCore-7.3.5 `e108a1a9`), and the
  per-player refresh of quest objects when 40604 / 40835 / 40932 / 40606 (and Talonclaw's 40385) leave the log.
- **Choice panels**: `SendDisplayPlayerChoice` now applies the choice and response conditions for the warlock (245) and
  hunter (240) artifact panels and shows no empty hunter panel (before, only the answer was rechecked).
- Hunter pets keep their active-pet auras across taming, Call Pet and reloads (`Pet.cpp`, LegionCore-7.3.5 `1210a359`).

Hunter content: `titanstrike_runes.cpp`, `titanstrike_storms.cpp`, `titanstrike_workshop.cpp`, `titanstrike_rune_wave.h`
(new), LegionCore-7.3.5's hunter sections of `ClassHalls/artifact_acquisition.cpp` (Titanstrike's final Prustaga,
aftermath and combat conversations, world pickups retired by their summoner; Talonclaw's Dakarr gates, ambush,
presentation, return, departure and `talonclaw_boarded_departure.h`; Thas'dorah's Qormaladon, presentation, return and
claim, `go_marksmanship_pickup`), and 64 SQL files `2026_11_09_36..99_lc735_*` (LegionCore-7.3.5 2026_10_23_00 ..
2026_10_30_37: the hunter Dalaran invitation and intro, the three routes). 62 passed their guards verbatim on the
database copy; Talonclaw's linked-cast retry (only adds flag 0x200 to two rows of the expected shape) and boarded
departure (only `quest_objectives` differed: our English descriptions) are forced.

## Phase 3.5: Mage (2026-11-09), plus Death Knight Frost leftovers

All three routes are "client traversal pending" in LegionCore-7.3.5's audits. SQL `2026_11_10_00..14_lc735_*` (the
`2026_11_09_` numbers ran out: `_100` would sort before `_11`).
- **Arcane, Aluneth** (the Nexus Vault, scenario 1101 on map 1583, shared with the priest's Light's Wrath):
  `ClassHalls/lights_wrath_pickup.h` (both routes' pickup: native presentation, accepted return, retry), registered in
  `artifact_acquisition.cpp`; SQL: the shared Light's Wrath pickup admission and recovery (`_00`, `_01`), Aluneth's
  pickup recovery and its first chest retried without replaying the introduction (`_02`, `_03`), Nexus artifact energy
  to the killer's player when a pet or guardian kills (`_12`). The Light's Wrath **takeover** (`lights_wrath_takeover.h`,
  LegionCore-7.3.5 `2026_10_27_04`) waits for the Priest step. Reforged has no `Aura::GetAllDuration` (elapsed time):
  the header uses max minus remaining duration, the same for its never-refreshed 9 s aura.
- **Fire, Felo'melorn** (11997 "The Frozen Flame", scenario 957 on map 1480): chest 247494 linked to its loot (`_05`);
  scene 1125 equips the owned blade through 196020 (`_06`, `FireArtifactScene` in `artifact_acquisition.cpp`); Lyandra's
  80 % / 20 % attacks were stored as combat timers, now health events with the threshold retry (`_07`); Aethas' battle
  intro only in the boss step (`_08`); Aethas' return recovery (`fire_return_recovery.h`: `npc_fire_artifact_aethas`,
  `player_fire_artifact_return`, `_09`); the Iceborn conjurer's death story for pet owners (`_13`).
- **Frost, Ebonchill**: Meryl's sixth ambush choice gets its Legion Jailer wave (`_04`).
- **Death Knight Frost (missed in 3.1, same map 1480)**: Halahk's and the three Frost souls' presentation for pet owners
  (`_10`, `_11`) and Halahk's malformed combat link (`_14`).
- On the database copy: `_00`-`_05`, `_08`, `_09` passed verbatim (after their predecessors); forced: `_06`, `_10`, `_11`,
  `_12`, `_14` (only `creature_text` / `quest_objectives` / quest texts differed: broadcast ids and our English), `_07` and
  `_13` (one or two rows changed, of exactly the expected event / action / target shape).

## Phase 3.6: Monk (2026-11-09), plus the cross-class voice files

Audits: Sheilun "partial", Fu Zan and the Fists "client traversal pending". SQL `2026_11_10_15..30_lc735_*`.
- **Mistweaver, Sheilun** (41003, scenario 1007 on map 1541): `Sheylun.cpp` (CLEAN: taken as is) and its new headers
  (`sheilun_acquisition*.h`, `_companion_wounds`, `_plan`, `_return`, `_return_route`, `_tactics`, `_taran_intro`): manual
  loot of the staff, the boarded kite return to the temple, Taran's intro, companions protected through final damage and
  applied healing. Core: the completed scenario keeps its checkpoint while the staff's return is pending
  (`Player::HasPendingSheilunReturn`), a refused login into it returns a staff owner to the native Mistweaver exit
  (`ExitCompletedSheilunLogin`, `CharacterHandler`), the kites (102648) bind to the quest attempt when summoned
  (`Map::SummonCreature`). SQL `_24` (scene equip), `_25` (manual acquisition; its only failing check was the instance
  script name in `instance_template`, which Reforged registers in code).
- **Brewmaster, Fu Zan** (Temple of the Jade Serpent scenario): `instance_temple_of_the_jade_serpent.cpp` (CLEAN) and the
  new `fuzan_temple_intro.h`, `fuzan_temple_sanctuary_door.h`, `fuzan_belphiar.h`, `fuzan_summerpetal.h`; SQL `_26`-`_30`.
- **Windwalker / all monk routes**: the monk chests' real loot (`_19`) and scene equips (`MonkArtifactScene`, `_20`), the
  kite travel (`monk_kite_travel.h`, `_15`), Tak-Tak's temple transfer chain (`_16`).
- **Cross-class voice files** (`_17` boss speakers, `_18` Mist Warder, `_21` combat stories, `_22` trash voice, `_23` owner
  voice): talk rows retargeted so a line is said by its own NPC to the real player / pet owner (targets 1 and 130). Forced:
  only `creature_text` differed, or the talk rows they retarget have exactly the expected shape.
- New core support: `CreatureAI::UpdateLifecycle` (every creature update, also while charmed), `CanBeVisibleFor`,
  `OnHealApplied` (`Creature::NotifyHealApplied` from `Unit::DealHeal`), `PlayerScript::OnBeforeLogout` (before the
  logout save), `SpellScript::GetOriginalTargetGUID`, `Unit::GetDisabledAI`, `Player::GetScenarioLoginInstance`.
  `Player::IsMapObjectArrivalProtected` is a stub returning false: LegionCore-7.3.5's map-arrival object hold is engine
  work for phase 7, so nothing waits on it here.
- `port_lc735_sql.py` now also forces the second guard style (a temporary table with `CHECK(ok=1)`: the constraint is
  loosened and the `@..._safe` variable it was fed is set to 1 just before).

## Phase 3.7: Paladin (2026-11-09)

Audits: Silver Hand "partial", Truthguard and Ashbringer with their pickups repaired, all "client traversal pending".
- **Holy, The Silver Hand**: `SilverHand.cpp` and `instance_silver_hand.cpp` (CLEAN: taken as is) with the new
  `silver_hand_first_fight.h`, `_first_path.h`, `_first_room.h`, `_procession.h` (first-fight progression bound to the
  owned boss's death, companion guards and post-heal recovery); SQL scene equip, real loot, collapse exit, the initial
  motions of the companions and Mordoth (`2026_11_10_33..37`).
- **Protection, Truthguard** (Shield's Rest): `instance_shields_rest.cpp` (CLEAN), `shields_rest.cpp` merged three-way
  (Yrgrim's surrender through `DamageTakenAfterScaling`, companions protected after target-relative damage scaling; one
  conflict, the same `MoveJump` fix on both sides); SQL real loot and the travel click (`_31`, `_32`).
- **Retribution, Ashbringer** (map 1500, the class route ported in 3.2): `broken_shore.cpp` merged three-way (Balnazzar
  finished on a scaled lethal blow without killing the actor), `instance_broken_shore.cpp` (CLEAN); SQL spell loot (`_38`).
  The map 1500 instance also runs the warrior's scenario 933: it includes the warrior headers `Warrior/Intro/
  malgalor_finale.h`, `malgalor_scene.h`, `reinforcement_transition.h`, copied now; the rest of the warrior's content
  (its SQL, `wars_intro_scenario.cpp`) comes with 3.12, so the warrior part of this instance runs on our current data
  until then.
- All 8 SQL files passed their guards verbatim on the database copy.

## Phase 3.8: Priest (2026-11-09), plus a Death Knight Blood leftover

All three routes "client traversal pending" in LegionCore-7.3.5's audits. SQL `2026_11_10_39..58_lc735_*`.
- **Shadow, Xal'atath**: `artifact_acquisition.cpp` gains `XalatathFarthing`, `XalatathSoulAttack`,
  `XalatathPresentation`, `scene_xalatath_pickup`, `go_xalatath_pickup`; SQL `_40`-`_49` (pickup link, real loot, scene
  equip, progression / opening / pickup-cast admission, Farthing's finale, weapon summon / soul attack / finale actor
  retries), the priest's Dalaran messenger (`_39`).
- **Holy, T'uure**: `Tuure.cpp` merged three-way (3 conflicts, all where Reforged had made the same compile fixes:
  LegionCore-7.3.5's side taken), `instance_tuure.cpp` (CLEAN), new `Tuure.h`; core: Opening (3365) on T'uure's cage
  goober dispatches its gossip script after the lock check (`Spell::EffectOpenLock`), `WaypointMovementGenerator::
  GetPathId`; SQL scene equip, native pickup, contact credit (`_51`-`_53`); `_50` (the Devourer's native pickup) is the rogue's
  Fangs of the Devourer, shipped here by mistake of grouping: harmless, it is listed again under 3.9.
- **Discipline, Light's Wrath** (Nexus Vault, shared pickup ported with the mage): the takeover
  (`lights_wrath_takeover.h`, `npc_lights_wrath_takeover` replaces actor 104985's SmartAI) and its chest lifetime (`_54`,
  `_55`); Azuregos' Scion guards anchored on him and the priest healing phase after the real Scion clear (`_57`, `_58`).
  Ordering note: in LegionCore-7.3.5, `_54`/`_55` (its 2026_10_27_03/04) came before the shared pickup recovery we
  shipped with the mage (`2026_11_10_01`); both are forced, and `_55` deletes the rows `_54` flags, so the end state is
  LegionCore-7.3.5's.
- **Death Knight Blood leftover**: Sliver's departure callback (map 1489, scenario 940; `_56`).
- `port_lc735_sql.py` now also recognises `SET @x_guard := IF(` guards.

## Phase 3.9: Rogue (2026-11-09)

SQL `2026_11_10_59..68_lc735_*` (and `_50`, the Devourer pickup, shipped with 3.8).
- **Outlaw, Dreadblades** and the **Nexus** instance (map 1583, mage / priest / rogue): `Dreadblades.cpp`,
  `instance_class_art_scenarios.cpp` (CLEAN: taken), new `bilaal_death.h`, `bilaal_portal.h`: Nexus-Prince Bilaal's true
  death handed to the instance (`CreatureDies`), his portal arrival; SQL the Dreadblades pickup, Bilaal's three health
  thresholds with the retry flag (`_60`, forced: rows of the expected shape), his death rows moved to the instance (`_61`,
  forced: the two death rows it deletes exist as expected), the portal arrival (`_62`).
- **Assassination, Kingslayers**: `Kingslayers.cpp` merged three-way (2 conflicts, the same `MoveJump` fixes: LegionCore-
  7.3.5's side), new `kingslayers_return.h`; SQL native pickup and accepted return (`_63`, `_64`).
- **Subtlety, Fangs of the Devourer**: `Devourer.cpp` (CLEAN); the rogue campaign's Dalaran intel: Valzuun
  (`valzuun_intel.h`, `npc_valzuun_intel` in `artifact_acquisition.cpp`; password dispatch and full intel `_66`, `_68`),
  Desmond's native search (`_67`), Modera's facing messages (`_65`). Forced where only `creature_text` /
  `quest_objectives` differed.

## Phase 3.10: Shaman (2026-11-09)

- **Restoration, Sharas'dal** (map 1600): `Sharasdal.cpp`, `instance_sharasdal.cpp` (CLEAN: taken) and the new
  `sharasdal_acquisition.h`, `_erunak_entry.h`, `_erunak_upper.h`, `_gauntlet_arrival.h`, `_lower_ascent.h`,
  `_upper_wall.h` (Erunak, the ascent, the gauntlet, the acquisition, companions protected after damage scaling);
  SQL `2026_11_10_70..74` (all passed verbatim). **Not yet**: LegionCore-7.3.5 guards the acquisition jump's arrival
  (209683 -> 209684: cast only after a real landing on the same map, `MotionMaster::MoveJump` / `Spell::EffectJumpDest`);
  that mechanism also serves the warrior's Skyhold and Strom'kar and is ported with 3.12, Sharas'dal's case included.
- **Maelstrom** (scenario 1038): the wave producer is no longer called as a link from the interaction row
  (`2026_11_10_69`, forced: only `creature_text` differed).
- Doomhammer and the Fist of Ra-den have no LegionCore-7.3.5 changes beyond the class spells (phase 4).

## Phase 3.11: Warlock (2026-11-09)

SQL `2026_11_10_75..99` and `2026_11_11_00..06` (`_lc735_*`); the port tool now refuses an index above 99.
- **The New Blood** (the warlock class hall introduction, 40729, scenario 962 on map 1522; also maps 1220 / 1502):
  `new_blood.cpp` (new), `spell_gen_new_blood_imprisoned` (the jailer is no longer chained with his captive); core: the
  council actors, portal (101433, its Birth animation in the create packet) and ritual fire markers bound to the quest
  attempt (`Map::SummonCreature`), abandoning the quest resets the run (`Player::ResetNewBloodQuest`: tome, auras, the
  non-permanent bind, back to Dalaran), the return teleport (196340) cancels the attempt's objects, a login restores the
  scenario selector before the map is made (quest status loaded early for this case) and a fresh instance restarts at
  the prison, the smash spells' landing, Jagganoth's knockback stun after the movement, the rescue spellclick (195039)
  ignores the council's immunity, the ritual objective shown before the escape (`QuestDef`, presentation order only),
  no automatic scenario reward for 962 (and Stormheim missions 824 / 826) since the quest is turned in to Calydus; SQL
  `_76`-`_89`.
- **An Eye for a Scepter** (43153, scenario 1155 on map 1630): login restores the selector, a fresh instance starts at the
  entry spell's destination; SQL scenario sequence (`_91`).
- **Dark Whispers** (Demonology, Skull of the Man'ari): `ClassHalls/dark_whispers_artifact.cpp` (new); SQL `_94`-`_99`.
- **Ritual Ruination** and its reagents: `ritual_ruination.cpp` (new); SQL `_90`, `_92`, `2026_11_11_00`.
- **Dreadscar**: `ClassHalls/dreadscar_followup.cpp` (new); SQL claim and plans (`_77`, `_93`).
- **Ulthalesh** (Dark Riders, shared with the death knight, ported in 3.1): the login exit deferred from phase 2 —
  a refused login into the completed scenario returns the warlock to Revil outdoors (`ExitCompletedDarkRidersLogin`).
- The warlock's Dalaran offer, artifact choice (Calydus' panel conditions, the filter ported in 3.4), Affliction choice,
  artifact map and arsenal breadcrumb (`2026_11_11_01..05`); the Dalaran class introduction (`_75`, all classes);
  Sothozz's emergence conversation to the controlling player (`2026_11_11_06`, forced: text only).
- **Engine fixes found on the way**: gossip options requiring a secondary NPC flag (missions, work orders, recruiter,
  class hall upgrades) are checked against `UNIT_FIELD_NPC_FLAGS2` (both words in one 64-bit value; Reforged had worked
  around missions and work orders only), and `UNIT_NPC_FLAG2_CLASS_HALL_UPGRADE` is 0x200 (it had the recruiter's 0x100);
  artifact items cannot be destroyed by the player even if an item override drops their no-destroy flag. Reforged had
  already fixed the Enslave Demon crash and the vehicle join events itself.
- Not here: the warlock legendary refund and pet healing files and the pet regeneration change (class mechanics, phase 4).
- All 32 SQL files passed verbatim on the database copy except Sothozz (text only).

## Phase 3.12, part 1: Warrior's Warswords, Earth-Warder and Broken Shore (2026-11-09)

- **Fury, Warswords** (map 1500, scenario 933, the class route of 3.2): `Warswords.cpp` and `Intro/wars_intro_scenario.cpp`
  (CLEAN: taken), `instance_warswords.cpp` merged three-way (one conflict, a `MoveJump` compile fix on both sides), the
  Malgalor headers copied in 3.7; SQL weapon phase, real loot, scene equip, intro bubble, Malgalor's scene replacement
  (`2026_11_11_07..10`, `_19`). Core: the Broken Shore login exit deferred from phase 2 (`ExitCompletedBrokenShoreLogin`):
  a refused login after Malgalor's saved defeat goes to Skyhold, and after the Broken Isles intro's (map 1460) saved finale
  to the finale's destination, instead of the homebind.
- **Protection, Scale of the Earth-Warder**: `EarthWarder.cpp` merged three-way (3 conflicts: `MoveJump` compile fixes and
  one summon block, LegionCore-7.3.5's side); SQL real loot, scene equip, weapon beam, Hruthnir's and Magnar's
  conversations and intro scene (`_11..17`).
- Warrior creation actions (`_18`). All 13 SQL files passed verbatim.
- Part 2 (Arms, Strom'kar; the Skyhold jumps and the shared jump-arrival guard, also for Sharas'dal; Commander's Valor)
  follows.
