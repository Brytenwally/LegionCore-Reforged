////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "iron_approach.h"

using namespace IronApproach;

class instance_iron_approach : public InstanceMapScript
{
public:
    instance_iron_approach() : InstanceMapScript("instance_iron_approach", MAP_GORGROND_FINALE) { }

    struct instance_iron_approach_InstanceMapScript : public InstanceScript
    {
        instance_iron_approach_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_iron_approach_InstanceMapScript(map);
    }
};

void AddSC_instance_iron_approach()
{
    new instance_iron_approach();
}
