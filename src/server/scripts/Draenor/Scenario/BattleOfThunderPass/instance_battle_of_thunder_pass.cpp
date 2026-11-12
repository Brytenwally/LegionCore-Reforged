////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "battle_of_thunder_pass.h"

using namespace BattleOfThunderPass;

class instance_battle_of_thunder_pass : public InstanceMapScript
{
public:
    instance_battle_of_thunder_pass() : InstanceMapScript("instance_battle_of_thunder_pass", MAP_THUNDER_PASS) { }

    struct instance_battle_of_thunder_pass_InstanceMapScript : public InstanceScript
    {
        instance_battle_of_thunder_pass_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_battle_of_thunder_pass_InstanceMapScript(map);
    }
};

void AddSC_instance_battle_of_thunder_pass()
{
    new instance_battle_of_thunder_pass();
}
