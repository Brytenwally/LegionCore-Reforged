# DemonCore

## ❤️ Project Philosophy

This project is first and foremost a **preservation effort**. Legion 7.3.5 has disappeared from official servers, replaced by newer interpretations. 
Our ambition is straightforward : **bring this expansion back to life, keep it playable as it was on day one, and fix the design flaws left behind.**

We aren't trying to reinvent Legion — we are refining it.

## 🔧 Key Differences & Enhancements

Rather than a static copy baked with late-patch shortcuts, we implemented a **progressive gameplay experience**:

* **Progressive Progression (7.0 ➔ 7.3) :** Removal of late-game catch-up mechanisms *(exponential Artifact Power scaling, ilvl inflation, skipped content, etc.)* that ruined the sense of progression and shortened the adventure.
* **Complete System Scripting & Immersion :** Scripting missing or non-functional mechanics, polishing small visual and world details to enhance immersion, and ensuring all content is playable as close to retail accuracy as possible.
* **Rebalancing & Fixes :** Fine-tuning classes, and frustrating progression systems that Blizzard didn't have the time to address back then.
* **Quality of Life (QoL) :** Subtle ergonomic improvements and minor enrichments designed strictly within the original spirit of Legion.

---

#### Table of Contents
* [Introduction](#introduction)
* [Requirements](#requirements)
* [Install](#install)
* [Client](#client)
* [Data Files](#data-files)
* [Thank you](#thank-you)

#### Introduction
DemonCore is a **MMORPG** framework for WOW Legion *(Build 26972)*. It is a fork of LegionCore - Reforged (Titans-Project), which is based off of the UWOW core leak of 2020. Which was derived from an old version of [TrinityCore](https://github.com/TrinityCore/TrinityCore).

# Requirements
 
[Windows specific](https://www.trinitycore.info/en/install/requirements/windows)
  
[Linux specific](https://www.trinitycore.info/en/install/requirements/linux)

[Mac specific](https://www.trinitycore.info/en/install/requirements/macos)

# Install
Most of the install steps are the same as the TrinityCore ones [here](https://www.trinitycore.info/en/install/Core-Installation).

# Client
[Legion 7.3.5 (Build 26972)](https://www.dragonwar.fr/download/Legion7.3.5.zip.torrent)

# Data Files
This core has been updated with tools to generate all required data files.

Run the tools in the following order, using client build 26972:

1. `mapextractor`
2. `vmap4extractor`
3. `vmap4assembler`
4. `mmaps_generator`

The generated folders `dbc`, `maps`, `vmaps`, `mmaps`, `cameras` and `gt` are all required.

### Thank you
- [TrinityCore Authors](https://github.com/TrinityCore/TrinityCore/blob/master/AUTHORS)
- [LegionCore Contributors](https://github.com/Titans-Project/LegionCore-Reforged/graphs/contributors)
- [ingussuveiks-dev](https://github.com/ingussuveiks-dev), for the work on LegionCore 7.3.5: the class halls, every
  class's artifact acquisition questlines and scenarios, the SmartAI and scenario fixes behind them, and the careful
  audits that documented each one. Much of that work has been ported into this core.
 
> **License: GPL 3.0** read [COPYING](https://github.com/DemonCore/DemonCore/blob/main/LICENSE).

*Any commercial use of this code or the operation of a paid public server is strictly prohibited by the terms of use of the game's original license and by the ethical standards of this preservation project.*
