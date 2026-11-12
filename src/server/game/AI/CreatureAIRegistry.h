////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///   Copyright 2005-2009 MaNGOS         ///
///   <http://getmangos.com/>            ///
///                                      ///
////////////////////////////////////////////

#ifndef TRINITY_CREATUREAIREGISTRY_H
#define TRINITY_CREATUREAIREGISTRY_H

namespace AIRegistry
{
    void Initialize();

    // Movement generators are only registered by Initialize(), which runs late in the world
    // setup. Anything that spawns a creature before that hits an assert in the factory lookup.
    bool IsInitialized();
}
#endif

