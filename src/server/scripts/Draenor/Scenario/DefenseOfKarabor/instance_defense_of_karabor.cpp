////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "defense_of_karabor.h"

using namespace DefenseOfKarabor;

class instance_defense_of_karabor : public InstanceMapScript
{
public:
    instance_defense_of_karabor() : InstanceMapScript("instance_defense_of_karabor", MAP_DEFENSE_OF_KARABOR) { }

    struct instance_defense_of_karabor_InstanceMapScript : public InstanceScript
    {
        instance_defense_of_karabor_InstanceMapScript(InstanceMap* map) : InstanceScript(map) { }

        ObjectGuid _akamaGUID, _yrelGUID, _karaGUID, _vorkaGUID, _arnokkGUID;
        GuidList _waveGUIDs;
        uint32 _checkTimer = IN_MILLISECONDS;
        uint32 _waveTimer = 0;
        uint32 _defenseTimer = 0;
        uint32 _exitTimer = 0;
        uint8 _wavesSent = 0;
        uint8 _lastStep = STEP_ARNOKK;
        bool _defenseStarted = false;
        bool _vorkaReleased = false;
        bool _completed = false;
        bool _sceneShown = false;
        bool _restoreChecked = false;

        void OnCreatureCreate(Creature* creature) override
        {
            InstanceScript::OnCreatureCreate(creature);

            switch (creature->GetEntry())
            {
                case NPC_AKAMA: _akamaGUID = creature->GetGUID(); break;
                case NPC_ARNOKK:
                    _arnokkGUID = creature->GetGUID();
                    creature->SetRespawnDelay(DAY);    // the courtyard stays free once he has fallen
                    break;
                case NPC_YREL:  _yrelGUID = creature->GetGUID(); break;
                case NPC_KARA:  _karaGUID = creature->GetGUID(); break;
                case NPC_VORKA:
                    _vorkaGUID = creature->GetGUID();
                    // circles the harbor out of reach until the last step
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
                    break;
                default:
                    break;
            }
        }

        void OnPlayerEnter(Player* player) override
        {
            InstanceScript::OnPlayerEnter(player);

            // logging back in after a server restart: the instance is new and nothing queued it, the scenario starts here
            if (!GetScenario() && player->GetQuestStatus(QUEST_DEFENSE_OF_KARABOR) == QUEST_STATUS_INCOMPLETE)
                if (Scenario* scenario = sScenarioMgr->AddScenario(instance, SCENARIO_DEFENSE))
                {
                    scenario->SetCompleteThroughBonusSteps(false);
                    scenario->SendStepUpdate(player, true);
                }

            if (Creature* akama = instance->GetCreature(_akamaGUID))
                if (Scenario* scenario = GetScenario())
                    if (scenario->GetCurrentStep() == STEP_ARNOKK)
                        akama->AI()->Talk(TALK_AKAMA_LAST_FORCES, player->GetGUID());
        }

        Scenario* GetScenario() const
        {
            Scenario* scenario = sScenarioMgr->GetScenario(instance->GetInstanceId());
            return scenario && scenario->GetMap() == instance ? scenario : nullptr;
        }

        Player* GetReferencePlayer() const
        {
            Map::PlayerList const& players = instance->GetPlayers();
            return players.isEmpty() ? nullptr : players.begin()->getSource();
        }

        void SendScenarioEvent(uint32 eventId)
        {
            Scenario* scenario = GetScenario();
            Player* reference = GetReferencePlayer();
            if (scenario && reference)
                scenario->UpdateAchievementCriteria(CRITERIA_TYPE_SCRIPT_EVENT_2, eventId, 0, 0, nullptr, reference);
        }

        void OnUnitDeath(Unit* unit) override
        {
            if (unit->GetEntry() == NPC_VORKA)
                SendScenarioEvent(EVENT_VORKA_DEFEATED);
        }

        // the step reached survives a server restart: a new instance resumes there
        void SaveStep(uint8 step)
        {
            instance->ApplyOnEveryPlayer([step](Player* player)
            {
                CharacterDatabase.PExecute("REPLACE INTO character_scenario_step (guid, scenario, step) VALUES (" UI64FMTD ", %u, %u)",
                    uint64(player->GetGUIDLow()), uint32(SCENARIO_DEFENSE), uint32(step));
            });
        }

        void RestoreStep(Scenario* scenario)
        {
            Player* player = GetReferencePlayer();
            if (!player)
                return;

            _restoreChecked = true;
            QueryResult result = CharacterDatabase.PQuery("SELECT step FROM character_scenario_step WHERE guid = " UI64FMTD " AND scenario = %u",
                uint64(player->GetGUIDLow()), uint32(SCENARIO_DEFENSE));
            if (!result)
                return;

            uint8 step = result->Fetch()[0].GetUInt8();
            if (step <= scenario->GetCurrentStep() || step > STEP_VORKA)
                return;

            scenario->SetCurrentStep(step);
            if (Creature* arnokk = instance->GetCreature(_arnokkGUID))
                if (arnokk->IsAlive())
                    arnokk->DespawnOrUnsummon(0, Seconds(DAY));
        }

        void OnStepChanged(uint8 step)
        {
            SaveStep(step);

            switch (step)
            {
                case STEP_HARBOR:
                    if (Creature* akama = instance->GetCreature(_akamaGUID))
                        akama->AI()->Talk(TALK_AKAMA_TO_THE_HARBOR);
                    break;
                case STEP_KARA:
                    StartDefense();
                    break;
                case STEP_VORKA:
                    ReleaseVorka();
                    break;
                default:
                    break;
            }
        }

        void StartDefense()
        {
            if (_defenseStarted)
                return;

            _defenseStarted = true;
            _wavesSent = 0;
            _waveTimer = 5 * IN_MILLISECONDS;
            _defenseTimer = DEFENSE_MAX_DURATION;

            if (Creature* yrel = instance->GetCreature(_yrelGUID))
                yrel->AI()->Talk(TALK_YREL_DEFEND_KARA);

            instance->ApplyOnEveryPlayer([](Player* player)
            {
                player->AddAura(SPELL_KARAS_BENEVOLENCE, player);
            });
        }

        void SendWave()
        {
            Creature* kara = instance->GetCreature(_karaGUID);
            if (!kara)
                return;

            Position const& from = WaveSpawns[_wavesSent % 2];
            for (uint8 i = 0; i < 3; ++i)
            {
                Position pos = from;
                pos.m_positionX += frand(-3.0f, 3.0f);
                pos.m_positionY += frand(-3.0f, 3.0f);
                // summoned by K'ara so the attackers share her phase
                if (TempSummon* unit = kara->SummonCreature(WaveUnits[_wavesSent][i], pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10 * IN_MILLISECONDS))
                {
                    _waveGUIDs.push_back(unit->GetGUID());
                    float x, y, z;
                    kara->GetNearPoint(unit, x, y, z, unit->GetObjectSize(), 6.0f, kara->GetAngle(unit));
                    unit->GetMotionMaster()->MovePoint(0, x, y, z);
                }
            }

            if (Creature* vorka = instance->GetCreature(_vorkaGUID))
                vorka->AI()->Talk(TALK_VORKA_WAVE);

            ++_wavesSent;
        }

        bool WavesAlive()
        {
            for (ObjectGuid const& guid : _waveGUIDs)
                if (Creature* unit = instance->GetCreature(guid))
                    if (unit->IsAlive())
                        return true;
            return false;
        }

        void FinishDefense()
        {
            _defenseStarted = false;
            SendScenarioEvent(EVENT_KARA_EMPOWERED);

            // the naaru's light sweeps the harbor once empowered
            for (ObjectGuid const& guid : _waveGUIDs)
                if (Creature* unit = instance->GetCreature(guid))
                    if (unit->IsAlive())
                        unit->DespawnOrUnsummon();
            _waveGUIDs.clear();

            if (Creature* yrel = instance->GetCreature(_yrelGUID))
                yrel->AI()->Talk(TALK_YREL_KARA_EMPOWERED);
        }

        void ReleaseVorka()
        {
            if (_vorkaReleased)
                return;

            Creature* vorka = instance->GetCreature(_vorkaGUID);
            Creature* kara = instance->GetCreature(_karaGUID);
            if (!vorka || !kara)
                return;

            _vorkaReleased = true;
            vorka->AI()->Talk(TALK_VORKA_LANDS);

            // lands between the naaru and the bay, then fights on foot
            float angle = kara->GetAngle(vorka);
            float x = kara->GetPositionX() + 25.0f * std::cos(angle);
            float y = kara->GetPositionY() + 25.0f * std::sin(angle);
            float z = instance->GetHeight(vorka->GetPhases(), x, y, vorka->GetPositionZ() + 10.0f);
            vorka->GetMotionMaster()->Clear();
            vorka->SetHomePosition(x, y, z, angle + float(M_PI));
            vorka->GetMotionMaster()->MoveLand(1, Position(x, y, z));
            vorka->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
        }

        void Complete()
        {
            _completed = true;

            instance->ApplyOnEveryPlayer([](Player* player)
            {
                CharacterDatabase.PExecute("DELETE FROM character_scenario_step WHERE guid = " UI64FMTD " AND scenario = %u",
                    uint64(player->GetGUIDLow()), uint32(SCENARIO_DEFENSE));
            });
            _exitTimer = EXIT_TELEPORT_DELAY;

            instance->ApplyOnEveryPlayer([](Player* player)
            {
                player->KilledMonsterCredit(NPC_SCENARIO_CREDIT);
                player->RemoveAurasDueToSpell(SPELL_KARAS_BENEVOLENCE);
            });

            if (Creature* yrel = instance->GetCreature(_yrelGUID))
            {
                yrel->AI()->Talk(TALK_YREL_THANKS);
                yrel->AI()->Talk(TALK_YREL_LIGHT_PREVAILS);
            }
        }

        void Update(uint32 diff) override
        {
            if (_completed)
            {
                if (_exitTimer <= diff)
                {
                    _exitTimer = 0;
                    // Raindash carries the hero back to Embaari Village
                    instance->ApplyOnEveryPlayer([](Player* player)
                    {
                        player->TeleportTo(1116, ExitPosition.GetPositionX(), ExitPosition.GetPositionY(), ExitPosition.GetPositionZ(), ExitPosition.GetOrientation());
                    });
                    return;
                }

                _exitTimer -= diff;
                if (!_sceneShown && _exitTimer <= EXIT_TELEPORT_DELAY - EXIT_SCENE_DELAY)
                {
                    _sceneShown = true;
                    instance->ApplyOnEveryPlayer([](Player* player)
                    {
                        player->CastSpell(player, SPELL_CELEBRATION_SCENE, true);
                    });
                }
                return;
            }

            if (_defenseStarted)
            {
                if (_wavesSent < WAVE_COUNT)
                {
                    if (_waveTimer <= diff)
                    {
                        SendWave();
                        _waveTimer = WAVE_INTERVAL;
                    }
                    else
                        _waveTimer -= diff;
                }

                _defenseTimer = _defenseTimer > diff ? _defenseTimer - diff : 0;
                if ((_wavesSent >= WAVE_COUNT && !WavesAlive()) || !_defenseTimer)
                    FinishDefense();
            }

            if (_checkTimer > diff)
            {
                _checkTimer -= diff;
                return;
            }
            _checkTimer = IN_MILLISECONDS;

            Scenario* scenario = GetScenario();
            if (!scenario)
                return;

            if (scenario->IsCompleted(false))
            {
                Complete();
                return;
            }

            if (!_restoreChecked)
                RestoreStep(scenario);

            uint8 step = scenario->GetCurrentStep();
            if (step != _lastStep)
            {
                _lastStep = step;
                OnStepChanged(step);
            }

            // reaching Yrel at the harbor
            if (step == STEP_HARBOR)
                if (Creature* yrel = instance->GetCreature(_yrelGUID))
                    if (yrel->SelectNearestPlayer(15.0f))
                        SendScenarioEvent(EVENT_MEET_YREL);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_defense_of_karabor_InstanceMapScript(map);
    }
};

void AddSC_instance_defense_of_karabor()
{
    new instance_defense_of_karabor();
}
