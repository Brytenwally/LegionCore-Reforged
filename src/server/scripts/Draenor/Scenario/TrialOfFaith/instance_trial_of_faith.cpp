////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "trial_of_faith.h"

using namespace TrialOfFaith;

class instance_trial_of_faith : public InstanceMapScript
{
public:
    instance_trial_of_faith() : InstanceMapScript("instance_trial_of_faith", MAP_TRIAL_OF_FAITH) { }

    struct instance_trial_of_faith_InstanceMapScript : public InstanceScript
    {
        instance_trial_of_faith_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_trial_of_faith_InstanceMapScript(map);
    }
};

void AddSC_instance_trial_of_faith()
{
    new instance_trial_of_faith();
}
