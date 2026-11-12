////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "grommashar.h"

using namespace Grommashar;

class instance_grommashar : public InstanceMapScript
{
public:
    instance_grommashar() : InstanceMapScript("instance_grommashar", MAP_GROMMASHAR) { }

    struct instance_grommashar_InstanceMapScript : public InstanceScript
    {
        instance_grommashar_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_grommashar_InstanceMapScript(map);
    }
};

void AddSC_instance_grommashar()
{
    new instance_grommashar();
}
