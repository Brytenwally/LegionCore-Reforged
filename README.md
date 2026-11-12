# DemonCore

**DemonCore** is an open source server emulator for **World of Warcraft: Legion 7.3.5 (build 26972)**, based on
[TrinityCore](https://github.com/TrinityCore/TrinityCore). Our goal is a complete, playable Legion: class halls and
artifact campaigns, Broken Isles questlines, dungeons, raids, battlegrounds and everything in between.

## Modules

DemonCore has a module system: drop a module into the `modules/` folder and it is compiled into the worldserver with its
own configuration file, without touching the core. See [`modules/README.md`](modules/README.md) to write your own, and
[DemonCore-Playerbots](https://github.com/DemonCore-Legion/DemonCore-Playerbots) for a full example: bot players that
group with you, run dungeons and battlegrounds, and fill queues.

Ready-to-run Windows binaries are built by [DemonCore-Build](https://github.com/DemonCore-Legion/DemonCore-Build).

## Bugs and missing features

Found a bug, a broken quest or something missing from Legion? Please open an
[issue](https://github.com/DemonCore-Legion/DemonCore/issues): every report helps. Say what you did, what you expected and
what happened, with the quest, NPC or spell name (or id) and any worldserver log lines.

## Installation

**Ready-made Windows build:** download the latest package from
[DemonCore-Build](https://github.com/DemonCore-Legion/DemonCore-Build) (Actions or Releases) and follow its README. It
holds the servers, the extractors, `mysql.exe` and the `sql` folder, so you can skip the compiling steps below.

**Building it yourself:** follow TrinityCore's guides, which apply to DemonCore:

1. [Requirements](https://www.trinitycore.info/en/install/requirements) for
   [Windows](https://www.trinitycore.info/en/install/requirements/windows),
   [Linux](https://www.trinitycore.info/en/install/requirements/linux) or
   [macOS](https://www.trinitycore.info/en/install/requirements/macos)
2. [Core installation](https://www.trinitycore.info/en/install/Core-Installation)
3. [Server setup](https://www.trinitycore.info/en/install/Server-Setup) (databases, configuration files)

DemonCore differs from current TrinityCore on these points:

- **Branch `main`** of this repository, not TrinityCore's `master` or `3.3.5`.
- **Boost 1.78 to 1.86.** The core still uses `io_service`, which Boost 1.87 removed, so newer Boost does not
  compile. On Windows, use Visual Studio 2022 with Boost 1.84, as the official build does; Boost 1.84 does not
  support Visual Studio 2026.
- **Database names:** the world and hotfixes databases must be named `world_legion` and `hotfixes_legion`, because
  the SQL updates refer to them by name. Set `WorldDatabaseInfo` and `HotfixDatabaseInfo` in `worldserver.conf`
  accordingly. `auth` and `characters` keep their usual names.
- **Database setup:** set `SourceDirectory` (the source folder, or `.` for the Windows package) and `MySQLExecutable`
  in `worldserver.conf` and `bnetserver.conf`. With `Updates.AutoSetup = 1`, the servers fill empty databases from
  `sql/base` and apply `sql/updates`. There is no separate database download.
- **Client:** Legion 7.3.5, build 26972. Connect it with the `connection_patcher` built alongside the servers.
- **Modules:** put a module in `modules/<name>` before running CMake, e.g.
  `git clone https://github.com/DemonCore-Legion/DemonCore-Playerbots.git modules/mod-playerbots`. Copy its
  `.conf.dist` to `.conf` next to `worldserver.conf`.

## Data Files
This core has been updated with tools to generate all required data files.

Run the tools in the following order, using client build 26972:

1. `mapextractor`
2. `vmap4extractor`
3. `vmap4assembler`
4. `mmaps_generator`

The generated folders `dbc`, `maps`, `vmaps`, `mmaps`, `cameras` and `gt` are all required.

## Thank you
- [TrinityCore Authors](https://github.com/TrinityCore/TrinityCore/blob/master/AUTHORS)
- [LegionCore Contributors](https://github.com/Titans-Project/LegionCore-Reforged/graphs/contributors)
- [ingussuveiks-dev](https://github.com/ingussuveiks-dev), for the work on LegionCore 7.3.5: the class halls, every
  class's artifact acquisition questlines and scenarios, the SmartAI and scenario fixes behind them, and the careful
  audits that documented each one. Much of that work has been ported into this core.
 
> **License: GPL 3.0** read [COPYING](https://github.com/DemonCore-Legion/DemonCore/blob/main/LICENSE).

*Any commercial use of this code or the operation of a paid public server is strictly prohibited by the terms of use of the game's original license and by the ethical standards of this preservation project.*
