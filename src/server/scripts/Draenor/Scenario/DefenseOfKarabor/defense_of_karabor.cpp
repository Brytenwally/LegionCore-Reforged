////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

// The Defense of Karabor (quest 33256): a private scenario on map 1277, a copy of Karabor.
// Exarch Akama sends the player in; the four steps are Arnokk the Burner, the way to the harbor,
// K'ara's empowerment under waves of the Iron Horde, then Commander Vorka.

#include "defense_of_karabor.h"

using namespace DefenseOfKarabor;

// 75358 - Arnokk the Burner: sweeps the northern courtyard with his flamethrower, spins it around himself in a fight
class npc_arnokk_the_burner : public CreatureScript
{
public:
    npc_arnokk_the_burner() : CreatureScript("npc_arnokk_the_burner") { }

    enum
    {
        SPELL_BURN              = 149378,   // cone in front of him
        SPELL_FIERY_BLAST       = 151732,   // torrent of flame, then the flame line below
        SPELL_FIERY_BLAST_LINE  = 151731,   // 6 s line of fire attached to him (areatrigger 1400)
        SPELL_SCORCHED_EARTH    = 151766,

        TALK_AGGRO              = 1,
        TALK_DEATH              = 2,
        TALK_ABILITY            = 4,

        EVENT_WALK              = 1,
        EVENT_SWEEP_START,
        EVENT_SWEEP_STEP,
        EVENT_BURN,
        EVENT_FIERY_BLAST,
        EVENT_SPIN_START,
        EVENT_SPIN_STEP,
        EVENT_SCORCHED_EARTH,

        POINT_STATION           = 1,
        FLAME_STEPS             = 24,       // over the 6 s of the flame line
    };

    // where he was seen working the courtyard
    static Position const* Stations()
    {
        static Position const stations[4] =
        {
            { 858.6f, -2858.1f, 100.6f, 0.86f },
            { 830.5f, -2875.5f, 100.5f, 3.03f },
            { 818.9f, -2904.1f, 100.6f, 4.71f },
            { 819.0f, -2874.9f, 101.0f, 3.54f },
        };
        return stations;
    }

    struct npc_arnokk_the_burnerAI : public ScriptedAI
    {
        npc_arnokk_the_burnerAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap _events;
        uint8 _station = 0;
        uint8 _step = 0;
        float _facing = 0.0f;
        bool _spinning = false;

        void Reset() override
        {
            _events.Reset();
            _spinning = false;
            me->SetReactState(REACT_AGGRESSIVE);
            _events.ScheduleEvent(EVENT_WALK, urand(2, 5) * IN_MILLISECONDS);
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _events.Reset();
            Talk(TALK_AGGRO);
            _events.ScheduleEvent(EVENT_BURN, urand(6, 9) * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_FIERY_BLAST, urand(12, 15) * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_SCORCHED_EARTH, urand(16, 20) * IN_MILLISECONDS);
        }

        void JustDied(Unit* /*killer*/) override
        {
            Talk(TALK_DEATH);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE && id == POINT_STATION && !me->isInCombat())
                _events.ScheduleEvent(EVENT_SWEEP_START, 500);
        }

        void UpdateAI(uint32 diff) override
        {
            _events.Update(diff);

            if (!me->isInCombat())
            {
                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WALK:
                        {
                            _station = (_station + 1 + urand(0, 2)) % 4;
                            Position const& pos = Stations()[_station];
                            me->GetMotionMaster()->MovePoint(POINT_STATION, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                            break;
                        }
                        case EVENT_SWEEP_START:
                            // a slow sweep, from one side to the other
                            _facing = Stations()[_station].GetOrientation() - float(M_PI) / 3.0f + frand(-0.5f, 0.5f);
                            me->SetFacingTo(_facing);
                            me->CastSpell(me, SPELL_FIERY_BLAST_LINE, true);
                            _step = 0;
                            _events.ScheduleEvent(EVENT_SWEEP_STEP, 250);
                            break;
                        case EVENT_SWEEP_STEP:
                            _facing += (2.0f * float(M_PI) / 3.0f) / FLAME_STEPS;
                            me->SetFacingTo(_facing);
                            if (++_step < FLAME_STEPS)
                                _events.ScheduleEvent(EVENT_SWEEP_STEP, 250);
                            else
                                _events.ScheduleEvent(EVENT_WALK, urand(3, 6) * IN_MILLISECONDS);
                            break;
                        default:
                            break;
                    }
                }
                return;
            }

            // the flame line turns with him, faster than one can run
            if (_spinning)
            {
                while (uint32 eventId = _events.ExecuteEvent())
                {
                    if (eventId != EVENT_SPIN_STEP)
                    {
                        _events.ScheduleEvent(eventId, 1000);
                        continue;
                    }

                    _facing += 2.0f * float(M_PI) / FLAME_STEPS;
                    me->SetFacingTo(_facing);
                    if (++_step < FLAME_STEPS)
                        _events.ScheduleEvent(EVENT_SPIN_STEP, 250);
                    else
                    {
                        _spinning = false;
                        me->SetReactState(REACT_AGGRESSIVE);
                        if (Unit* victim = me->SelectVictim())
                            AttackStart(victim);
                    }
                }
                return;
            }

            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BURN:
                        if (urand(0, 1))
                            Talk(TALK_ABILITY);
                        DoCastVictim(SPELL_BURN);
                        _events.ScheduleEvent(EVENT_BURN, urand(10, 14) * IN_MILLISECONDS);
                        return;
                    case EVENT_FIERY_BLAST:
                        Talk(TALK_ABILITY);
                        DoCastVictim(SPELL_FIERY_BLAST);
                        _events.ScheduleEvent(EVENT_SPIN_START, 2600);  // once the 2.5 s cast has lit the flame line
                        _events.ScheduleEvent(EVENT_FIERY_BLAST, urand(18, 22) * IN_MILLISECONDS);
                        return;
                    case EVENT_SPIN_START:
                        if (!me->HasAura(SPELL_FIERY_BLAST_LINE))   // the cast was interrupted
                            break;
                        _spinning = true;
                        _step = 0;
                        _facing = me->GetOrientation();
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveIdle();
                        _events.ScheduleEvent(EVENT_SPIN_STEP, 250);
                        return;
                    case EVENT_SCORCHED_EARTH:
                        DoCast(me, SPELL_SCORCHED_EARTH);
                        _events.ScheduleEvent(EVENT_SCORCHED_EARTH, urand(15, 20) * IN_MILLISECONDS);
                        return;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_arnokk_the_burnerAI(creature);
    }
};

// 148425 - Launch Defense of Karabor
class spell_defense_of_karabor_launch : public SpellScriptLoader
{
public:
    spell_defense_of_karabor_launch() : SpellScriptLoader("spell_defense_of_karabor_launch") { }

    class spell_defense_of_karabor_launch_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_defense_of_karabor_launch_SpellScript);

        void HandleAfterCast()
        {
            if (Unit* caster = GetCaster())
                EnterScenario(caster->ToPlayer());
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_defense_of_karabor_launch_SpellScript::HandleAfterCast);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_defense_of_karabor_launch_SpellScript();
    }
};

// 72413 - Exarch Akama, at the temple gates: offers the scenario while the quest is in progress, again after a death or a hearthstone
class npc_karabor_exarch_akama : public CreatureScript
{
public:
    npc_karabor_exarch_akama() : CreatureScript("npc_karabor_exarch_akama") { }

    enum
    {
        GOSSIP_MENU_ENTER   = 65010,      // gossip_menu_option: "I am ready to defend Karabor."
        GOSSIP_ACTION_ENTER = GOSSIP_ACTION_INFO_DEF + 1,
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->GetMapId() == MAP_DEFENSE_OF_KARABOR || player->GetQuestStatus(QUEST_DEFENSE_OF_KARABOR) != QUEST_STATUS_INCOMPLETE)
            return false;

        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());
        player->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_ENTER, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_ENTER);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        player->CLOSE_GOSSIP_MENU();
        // the scenario is offered like a dungeon finder proposal, which the player accepts to enter
        if (action == GOSSIP_ACTION_ENTER)
        {
            lfg::LfgDungeonSet dungeons;
            dungeons.insert(LFG_DUNGEON_DEFENSE);
            sLFGMgr->JoinLfg(player, player->GetSpecializationRoleMaskForGroup(), dungeons);
        }
        return true;
    }
};

// abandoning the quest starts the scenario over
class playerscript_defense_of_karabor : public PlayerScript
{
public:
    playerscript_defense_of_karabor() : PlayerScript("playerscript_defense_of_karabor") { }

    void OnQuestAbandon(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_DEFENSE_OF_KARABOR)
            CharacterDatabase.PExecute("DELETE FROM character_scenario_step WHERE guid = " UI64FMTD " AND scenario = %u",
                uint64(player->GetGUIDLow()), uint32(SCENARIO_DEFENSE));
    }
};

void AddSC_defense_of_karabor()
{
    new npc_arnokk_the_burner();
    new spell_defense_of_karabor_launch();
    new npc_karabor_exarch_akama();
    new playerscript_defense_of_karabor();
}
