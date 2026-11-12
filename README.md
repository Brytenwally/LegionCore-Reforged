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
