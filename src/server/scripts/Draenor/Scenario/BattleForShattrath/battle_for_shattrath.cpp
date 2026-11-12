////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

// The Battle for Shattrath (quests 34099 and 33731): a private scenario on map 1207, a copy of Shattrath
// taken by the Iron Horde. The player walks through the Arch of Sha'tar; Thaelin's Iron Star rolls down to
// the harbor, Khadgar clears the docks, the hero sinks the battleship's crew and faces Blackhand on his flagship.

#include "battle_for_shattrath.h"

using namespace BattleForShattrath;

// Khadgar, Yrel, Durotan, Draka, Maraad, Orgrim: the leaders of the assault cannot fall before the story says so
class npc_shattrath_hero : public CreatureScript
{
public:
    npc_shattrath_hero() : CreatureScript("npc_shattrath_hero") { }

    struct npc_shattrath_heroAI : public ScriptedAI
    {
        npc_shattrath_heroAI(Creature* creature) : ScriptedAI(creature) { }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            // Blackhand's own blow ends Orgrim, everything else only wounds them
            if (me->GetEntry() == NPC_ORGRIM && attacker && attacker->GetEntry() == NPC_BLACKHAND)
                return;
            if (damage >= me->GetHealth())
                damage = me->GetHealth() > 1 ? me->GetHealth() - 1 : 0;
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;
            if (id == 1)
                me->SetFacingTo(me->GetHomePosition().GetOrientation());
            else if (id == POINT_STAIRS_FOOT)
                me->GetMotionMaster()->MovePoint(2, me->GetHomePosition());
        }

        uint32 _seekTimer = IN_MILLISECONDS;

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
            {
                // Khadgar keeps to his spell and Orgrim to Blackhand; the others take on the Iron Horde at hand
                if (me->GetEntry() == NPC_KHADGAR || me->GetEntry() == NPC_ORGRIM)
                    return;
                if (_seekTimer > diff)
                {
                    _seekTimer -= diff;
                    return;
                }
                _seekTimer = 1500;
                if (Creature* foe = FindFoe(me, 15.0f))
                    AttackStart(foe);
                return;
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_heroAI(creature);
    }
};

// 77195 - Archmage Khadgar, in the scenario: begins his spell at the harbor, then sends the hero to the battleship and to the flagship
class npc_shattrath_khadgar : public CreatureScript
{
public:
    npc_shattrath_khadgar() : CreatureScript("npc_shattrath_khadgar") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        uint8 step = GetStep(creature);
        InstanceScript* instance = creature->GetInstanceScript();
        uint32 menu = 0, text = 0;

        if (step == STEP_BATTLE_IS_JOINED)
            menu = GOSSIP_MENU_BEGIN_SPELL, text = TEXT_KHADGAR_HARBOR;
        else if (step == STEP_THE_WEAPON && instance && instance->GetData(DATA_CONFRONTED))
            menu = GOSSIP_MENU_BATTLESHIP, text = TEXT_KHADGAR_BATTLESHIP;
        else if (step == STEP_BLACKHAND)
            menu = GOSSIP_MENU_FLAGSHIP, text = TEXT_KHADGAR_FLAGSHIP;

        // between two moments of the battle he has nothing to offer: not his default menu either
        if (!menu)
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        player->ADD_GOSSIP_ITEM_DB(menu, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SCENARIO);
        player->SEND_GOSSIP_MENU(text, creature->GetGUID());
        return true;
    }

    // Khadgar's spell takes the hero away in a flash of arcane light
    static void Teleport(Player* player, Creature* khadgar, Position const& dest)
    {
        khadgar->SetFacingToObject(player);
        khadgar->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_OMNI);
        player->CastSpell(player, SPELL_TELEPORT_VISUAL, true);
        ObjectGuid const guid = player->GetGUID();
        player->AddDelayedEvent(1200, [player, guid, dest]() -> void
        {
            if (player->GetGUID() == guid && player->IsInWorld() && player->GetMapId() == MAP_SHATTRATH_SCENARIO)
            {
                player->NearTeleportTo(dest);
                player->CastSpell(player, SPELL_TELEPORT_VISUAL, true);
            }
        });
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->CLOSE_GOSSIP_MENU();
        InstanceScript* instance = creature->GetInstanceScript();
        if (action != GOSSIP_ACTION_SCENARIO || !instance)
            return true;

        switch (GetStep(creature))
        {
            case STEP_BATTLE_IS_JOINED:
                instance->SetData(DATA_TALKED_TO_KHADGAR, 0);
                break;
            case STEP_THE_WEAPON:
                Teleport(player, creature, PlayerBattleship);
                instance->SetData(DATA_TELEPORT_BATTLESHIP, 0);
                break;
            case STEP_BLACKHAND:
                Teleport(player, creature, PlayerFlagship);
                instance->SetData(DATA_TELEPORT_FLAGSHIP, 0);
                break;
            default:
                break;
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_hero::npc_shattrath_heroAI(creature);
    }
};

// 77192 - Thaelin Darkanvil, beside the Iron Star he and Gazlowe repaired
class npc_shattrath_thaelin : public CreatureScript
{
public:
    npc_shattrath_thaelin() : CreatureScript("npc_shattrath_thaelin") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (GetStep(creature) != STEP_PREPARATIONS)
            return false;

        player->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_THAELIN, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SCENARIO);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->CLOSE_GOSSIP_MENU();
        if (action == GOSSIP_ACTION_SCENARIO)
            if (InstanceScript* instance = creature->GetInstanceScript())
                instance->SetData(DATA_TALKED_TO_THAELIN, 0);
        return true;
    }
};

// 86671 - Iron Star: the hero climbs in, it rolls down the avenue through the Iron Horde and drops them at the harbor
class npc_shattrath_iron_star : public CreatureScript
{
public:
    npc_shattrath_iron_star() : CreatureScript("npc_shattrath_iron_star") { }

    // once Thaelin has sent the hero on, a click makes them the star; the ride is driven by the instance script
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->CLOSE_GOSSIP_MENU();
        if (GetStep(creature) == STEP_IRON_STAR)
            if (InstanceScript* instance = creature->GetInstanceScript())
                instance->SetGuidData(DATA_STAR_RIDER, player->GetGUID());
        return true;
    }

    struct npc_shattrath_iron_starAI : public ScriptedAI
    {
        npc_shattrath_iron_starAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 /*diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_iron_starAI(creature);
    }
};

// 77256 - Blackhand, on his flagship: he cannot be beaten, the hero has to outlast him behind the Iron Stars
class npc_shattrath_blackhand : public CreatureScript
{
public:
    npc_shattrath_blackhand() : CreatureScript("npc_shattrath_blackhand") { }

    enum
    {
        EVENT_MARK          = 1,
        EVENT_MARK_STRIKE,
        EVENT_ONSLAUGHT_END,
        FIGHT_DURATION      = 75 * IN_MILLISECONDS,
    };

    struct npc_shattrath_blackhandAI : public ScriptedAI
    {
        npc_shattrath_blackhandAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap _events;
        ObjectGuid _markedGUID;
        bool _done = false;

        void Reset() override
        {
            _events.Reset();
            _markedGUID.Clear();
            if (GetStep(me) < STEP_BLACKHAND || !me->GetInstanceScript())
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }

        bool _started = false;

        // the heroes only hold him: his onslaught begins when the champion joins the fight
        void StartOnslaught()
        {
            // the scene is led by the instance script: he only fights
            _started = true;
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType /*damageType*/) override
        {
            if (!victim)
                return;
            if (victim->IsControlledByPlayer())
                damage = std::min<uint32>(damage, uint32(victim->CountPctFromMaxHealth(1)));
            else
                damage = std::min<uint32>(damage, HERO_SCRATCH);
        }

        // he fights the heroes: a hero who strikes him does not draw him away from them
        uint32 _heroCheck = 1000;
        void KeepOnHeroes(uint32 diff)
        {
            if (_heroCheck > diff)
            {
                _heroCheck -= diff;
                return;
            }
            _heroCheck = 1000;
            Unit* victim = me->getVictim();
            if (victim && !victim->IsControlledByPlayer())
                return;
            std::list<Creature*> heroes;
            for (uint32 entry : { uint32(NPC_YREL), uint32(NPC_DUROTAN), uint32(NPC_MARAAD) })
                me->GetCreatureListWithEntryInGridAppend(heroes, entry, 20.0f);
            for (Creature* hero : heroes)
                if (hero->IsAlive() && hero->isInCombat())
                {
                    if (victim)
                        me->getThreatManager().modifyThreatPercent(victim, -100);
                    AttackStart(hero);
                    return;
                }
        }

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (attacker && attacker->IsControlledByPlayer())
                StartOnslaught();
            else
                damage = std::min<uint32>(damage, HERO_SCRATCH);
            if (damage >= me->GetHealth())
                damage = me->GetHealth() > 1 ? me->GetHealth() - 1 : 0;
        }

        // an Iron Star within a few yards of the line between Blackhand and the target shields it
        bool IsSheltered(Unit* target)
        {
            std::list<Creature*> stars;
            me->GetCreatureListWithEntryInGrid(stars, NPC_IRON_STAR_SHELTER, 40.0f);
            float ax = me->GetPositionX(), ay = me->GetPositionY();
            float bx = target->GetPositionX(), by = target->GetPositionY();
            float len2 = (bx - ax) * (bx - ax) + (by - ay) * (by - ay);
            for (Creature* star : stars)
            {
                float t = len2 > 0.0f ? ((star->GetPositionX() - ax) * (bx - ax) + (star->GetPositionY() - ay) * (by - ay)) / len2 : 0.0f;
                if (t <= 0.0f || t >= 1.0f)
                    continue;
                float px = ax + t * (bx - ax), py = ay + t * (by - ay);
                if (star->GetExactDist2d(px, py) < 3.0f)
                    return true;
            }
            return false;
        }

        void EndOnslaught()
        {
            _done = true;
            _events.Reset();
            // a last swing throws everyone back across the deck
            me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 40.0f);
            for (Player* player : players)
                player->KnockbackFrom(me->GetPositionX(), me->GetPositionY(), 15.0f, 7.0f);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            me->CombatStop(true);
            me->SetReactState(REACT_PASSIVE);
            if (InstanceScript* instance = me->GetInstanceScript())
                instance->SetData(DATA_BLACKHAND_DONE, 0);
        }

        void UpdateAI(uint32 diff) override
        {
            if (_done || !UpdateVictim())
                return;

            KeepOnHeroes(diff);

            _events.Update(diff);
            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MARK:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                        {
                            _markedGUID = target->GetGUID();
                            Talk(EMOTE_BLACKHAND_MARKED, target->GetGUID());
                        }
                        _events.ScheduleEvent(EVENT_MARK_STRIKE, 5 * IN_MILLISECONDS);
                        _events.ScheduleEvent(EVENT_MARK, 18 * IN_MILLISECONDS);
                        break;
                    case EVENT_MARK_STRIKE:
                        if (Player* target = ObjectAccessor::GetPlayer(*me, _markedGUID))
                            if (target->IsAlive() && !IsSheltered(target))
                                me->DealDamage(target, uint32(target->CountPctFromMaxHealth(60)));
                        _markedGUID.Clear();
                        break;
                    case EVENT_ONSLAUGHT_END:
                        EndOnslaught();
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
        return new npc_shattrath_blackhandAI(creature);
    }
};

// 80962 - Machinist B'randt, at the battleship's cannon: waits behind his six guards, scatters bombs when cornered
class npc_shattrath_machinist_brandt : public CreatureScript
{
public:
    npc_shattrath_machinist_brandt() : CreatureScript("npc_shattrath_machinist_brandt") { }

    struct npc_shattrath_machinist_brandtAI : public ScriptedAI
    {
        npc_shattrath_machinist_brandtAI(Creature* creature) : ScriptedAI(creature) { }

        enum
        {
            SPELL_NAPALM_THROWER    = 168127,   // sprays napalm ahead of him; its own burn (168126) does next to nothing at this level
            SPELL_NAPALM_BURN       = 168126,
            SPELL_BOMB_FRENZY       = 168367,
            EVENT_NAPALM            = 1,
            EVENT_NAPALM_TICK,
            EVENT_THROW_BOMB,
        };

        bool _bombs = false;
        uint8 _napalmTicks = 0;
        EventMap _events;

        void Reset() override
        {
            _bombs = false;
            _events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _events.ScheduleEvent(EVENT_NAPALM, 6 * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_THROW_BOMB, 10 * IN_MILLISECONDS);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (_bombs || !me->HealthBelowPctDamaged(20, damage))
                return;

            _bombs = true;
            Talk(SAY_BRANDT_TAKE_YOU_OUT);
            DoCast(me, SPELL_BOMB_FRENZY, true);
            for (uint8 i = 0; i < 6; ++i)
            {
                float angle = frand(0.0f, 2.0f * float(M_PI));
                float dist = frand(3.0f, 9.0f);
                Position pos(me->GetPositionX() + dist * std::cos(angle), me->GetPositionY() + dist * std::sin(angle), me->GetPositionZ());
                me->SummonCreature(NPC_BLACKROCK_BOMB, pos, TEMPSUMMON_TIMED_DESPAWN, 6 * IN_MILLISECONDS);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            _events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                Unit* victim = me->getVictim();
                switch (eventId)
                {
                    case EVENT_NAPALM:
                        DoCast(me, SPELL_NAPALM_THROWER, true);
                        _napalmTicks = 8;
                        _events.ScheduleEvent(EVENT_NAPALM_TICK, 750);
                        _events.ScheduleEvent(EVENT_NAPALM, 15 * IN_MILLISECONDS);
                        break;
                    case EVENT_NAPALM_TICK:
                    {
                        // whoever stands in the spray ahead of him burns
                        std::list<Player*> players;
                        me->GetPlayerListInGrid(players, 13.0f);
                        for (Player* player : players)
                            if (player->IsAlive() && me->isInFront(player, float(M_PI) * 70.0f / 180.0f))
                            {
                                me->DealDamage(player, uint32(player->CountPctFromMaxHealth(4)));
                                if (!player->HasAura(SPELL_NAPALM_BURN))
                                    me->AddAura(SPELL_NAPALM_BURN, player);
                            }
                        if (--_napalmTicks)
                            _events.ScheduleEvent(EVENT_NAPALM_TICK, 750);
                        break;
                    }
                    case EVENT_THROW_BOMB:
                        // a fused Blackrock bomb at the feet of whoever he fights
                        me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_THROWN);
                        me->SummonCreature(NPC_BLACKROCK_BOMB, victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 8 * IN_MILLISECONDS);
                        _events.ScheduleEvent(EVENT_THROW_BOMB, urand(10, 12) * IN_MILLISECONDS);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_machinist_brandtAI(creature);
    }
};

// 84206 - Blackrock Bomb, scattered by B'randt: goes off after a short fuse
class npc_shattrath_blackrock_bomb : public CreatureScript
{
public:
    npc_shattrath_blackrock_bomb() : CreatureScript("npc_shattrath_blackrock_bomb") { }

    struct npc_shattrath_blackrock_bombAI : public ScriptedAI
    {
        npc_shattrath_blackrock_bombAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 _fuse = 4 * IN_MILLISECONDS;

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!_fuse)
                return;
            if (_fuse > diff)
            {
                _fuse -= diff;
                return;
            }

            _fuse = 0;
            me->CastSpell(me, SPELL_FIRE_EXPLOSION, true);
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 5.0f);
            for (Player* player : players)
                if (player->IsAlive())
                {
                    me->DealDamage(player, uint32(player->CountPctFromMaxHealth(20)));
                    player->KnockbackFrom(me->GetPositionX(), me->GetPositionY(), 8.0f, 6.0f);
                }
            me->DespawnOrUnsummon(500);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_blackrock_bombAI(creature);
    }
};

// Shattrath defenders and the Iron Horde in the streets and on the docks: they seek each other out and fight on,
// wounds between them stay light so the battle lasts until a hero or the Iron Star settles it
class npc_shattrath_street_fighter : public CreatureScript
{
public:
    npc_shattrath_street_fighter() : CreatureScript("npc_shattrath_street_fighter") { }

    struct npc_shattrath_street_fighterAI : public ScriptedAI
    {
        npc_shattrath_street_fighterAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 _seekTimer = IN_MILLISECONDS;
        uint32 _spellTimer = 4 * IN_MILLISECONDS;

        void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*dmgType*/) override
        {
            if (attacker && !attacker->IsControlledByPlayer() && me->HealthBelowPctDamaged(30, damage))
                damage = 0;
        }

        // the defenders' withdrawal once the Iron Horde hangs frozen: up the middle of the stairs, then gone
        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;
            if (id == POINT_WITHDRAW_FOOT)
                me->GetMotionMaster()->MovePoint(POINT_WITHDRAW_TOP, StairsTop.GetPositionX(), me->GetPositionY(), StairsTop.GetPositionZ());
            else if (id == POINT_WITHDRAW_TOP)
                me->DespawnOrUnsummon(500, Seconds(DAY));
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (KeepApart(me, who))
                return;
            ScriptedAI::MoveInLineOfSight(who);
        }

        void AttackStart(Unit* who) override
        {
            if (KeepApart(me, who))
                return;
            ScriptedAI::AttackStart(who);
        }

        // each kind of soldier has its one ability, cast on whoever it fights
        uint32 Ability(bool& onSelf) const
        {
            onSelf = false;
            switch (me->GetEntry())
            {
                case NPC_GROMKAR_VANGUARD: case NPC_DOCKS_VANGUARD: case NPC_WAVE_VANGUARD:            return SPELL_SHOULDER_CHARGE;
                case NPC_GROMKAR_GRIMSHOT: case NPC_DOCKS_GRIMSHOT: case NPC_WAVE_GRIMSHOT:            return SPELL_HIDE_PIERCING_SHOT;
                case NPC_GROMKAR_SHADOWBLADE: case NPC_DOCKS_SHADOWBLADE: case NPC_WAVE_SHADOWBLADE:   return SPELL_STORMSTRIKE;
                case NPC_GROMKAR_DEADEYE:                                                              return SPELL_RIFLE_SHOT;
                case NPC_FIREBLADE_INVOKER:                                                            return SPELL_LAVA_BURST;
                case NPC_BURNING_BLADEMASTER: case NPC_DOCKS_BLADEMASTER: case NPC_WAVE_BLADEMASTER:
                    onSelf = true;
                    return SPELL_WHIRLWIND;
                default:
                    return 0;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (IsFrozen(me))
                return;

            if (!UpdateVictim())
            {
                // the line at the docks waits for the heroes to come down
                if (me->GetReactState() == REACT_PASSIVE)
                    return;
                if (_seekTimer > diff)
                {
                    _seekTimer -= diff;
                    return;
                }
                _seekTimer = 1500;
                // the waves come for the heroes; the line of the docks waits for them
                if (Creature* foe = FindFoe(me, IsWave(me->GetEntry()) && !me->GetDBTableGUIDLow() ? 40.0f : 12.0f))
                    AttackStart(foe);
                return;
            }

            if (Leashed(me))
            {
                EnterEvadeMode();
                return;
            }

            // caught up with the wrong foe all the same: let go of it and go for the heroes
            if (KeepApart(me, me->getVictim()))
            {
                me->getThreatManager().modifyThreatPercent(me->getVictim(), -100);
                me->AttackStop();
                if (Creature* foe = FindFoe(me, IsWave(me->GetEntry()) ? 40.0f : 12.0f))
                    AttackStart(foe);
                return;
            }

            if (_spellTimer <= diff)
            {
                _spellTimer = urand(8, 14) * IN_MILLISECONDS;
                bool onSelf = false;
                if (uint32 spell = Ability(onSelf))
                    if (!me->HasUnitState(UNIT_STATE_CASTING))
                        DoCast(onSelf ? me : me->getVictim(), spell);
            }
            else
                _spellTimer -= diff;

            if (!me->HasUnitState(UNIT_STATE_CASTING))
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_street_fighterAI(creature);
    }
};

// 84630 - Grom'kar Grenadier, on the battleship: stands waiting for the hero, then lobs mortar shells and leaves fused bombs at their feet
class npc_shattrath_grenadier : public CreatureScript
{
public:
    npc_shattrath_grenadier() : CreatureScript("npc_shattrath_grenadier") { }

    enum
    {
        EVENT_MORTAR_SHOT       = 1,
        EVENT_DROP_BOMB,
    };

    struct npc_shattrath_grenadierAI : public ScriptedAI
    {
        npc_shattrath_grenadierAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap _events;

        void Reset() override
        {
            _events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            _events.ScheduleEvent(EVENT_MORTAR_SHOT, urand(2, 4) * IN_MILLISECONDS);
            _events.ScheduleEvent(EVENT_DROP_BOMB, urand(8, 12) * IN_MILLISECONDS);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            if (Leashed(me))
            {
                EnterEvadeMode();
                return;
            }

            _events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = _events.ExecuteEvent())
            {
                Unit* victim = me->getVictim();
                switch (eventId)
                {
                    case EVENT_MORTAR_SHOT:
                        me->CastSpell(victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), SPELL_MORTAR_SHOT, false);
                        _events.ScheduleEvent(EVENT_MORTAR_SHOT, urand(6, 9) * IN_MILLISECONDS);
                        break;
                    case EVENT_DROP_BOMB:
                        me->SummonCreature(NPC_BLACKROCK_BOMB, victim->GetPositionX(), victim->GetPositionY(), victim->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 8 * IN_MILLISECONDS);
                        _events.ScheduleEvent(EVENT_DROP_BOMB, urand(12, 16) * IN_MILLISECONDS);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_grenadierAI(creature);
    }
};

// 84092 - Blackrock Bomb, lying on the battleship's deck: a click hands the hero a bomb to throw where they aim
class npc_shattrath_bomb_pickup : public CreatureScript
{
public:
    npc_shattrath_bomb_pickup() : CreatureScript("npc_shattrath_bomb_pickup") { }

    struct npc_shattrath_bomb_pickupAI : public ScriptedAI
    {
        npc_shattrath_bomb_pickupAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void OnSpellClick(Unit* /*clicker*/) override
        {
            me->DespawnOrUnsummon(0, Seconds(20));
        }

        void UpdateAI(uint32 /*diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_bomb_pickupAI(creature);
    }
};

// 84229 - the battleship's great cannon, where B'randt waits
class npc_shattrath_big_cannon : public CreatureScript
{
public:
    npc_shattrath_big_cannon() : CreatureScript("npc_shattrath_big_cannon") { }

    struct npc_shattrath_big_cannonAI : public ScriptedAI
    {
        npc_shattrath_big_cannonAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(uint32 /*diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_shattrath_big_cannonAI(creature);
    }
};

// 231620 - Portal to Talador: home to Fort Wrynn or Vol'jin's Pride once the flagship is lost
class go_shattrath_portal_to_talador : public GameObjectScript
{
public:
    go_shattrath_portal_to_talador() : GameObjectScript("go_shattrath_portal_to_talador") { }

    bool OnGossipHello(Player* player, GameObject* /*go*/) override
    {
        Position const& exit = player->GetTeam() == HORDE ? HordeExit : AllianceExit;
        player->TeleportTo(MAP_TALADOR, exit.GetPositionX(), exit.GetPositionY(), exit.GetPositionZ(), exit.GetOrientation());
        return true;
    }
};

// 75805 - Archmage Khadgar, before the Arch of Sha'tar: the battle begins with him
class npc_talador_khadgar_shattrath : public CreatureScript
{
public:
    npc_talador_khadgar_shattrath() : CreatureScript("npc_talador_khadgar_shattrath") { }

    struct npc_talador_khadgar_shattrathAI : public ScriptedAI
    {
        npc_talador_khadgar_shattrathAI(Creature* creature) : ScriptedAI(creature) { }

        void sQuestAccept(Player* player, Quest const* quest) override
        {
            if (quest->GetQuestId() == QUEST_BATTLE_ALLIANCE || quest->GetQuestId() == QUEST_BATTLE_HORDE)
                Talk(SAY_KHADGAR_TALADOR_ACCEPT, player->GetGUID());
        }
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!HasBattleQuest(player))
            return false;

        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());
        player->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_ENTER, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_SCENARIO);
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* /*creature*/, uint32 /*sender*/, uint32 action) override
    {
        player->CLOSE_GOSSIP_MENU();
        if (action == GOSSIP_ACTION_SCENARIO)
            EnterScenario(player);
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_talador_khadgar_shattrathAI(creature);
    }
};

// walking through the Arch of Sha'tar with the quest leads into the battle; abandoning the quest starts it over
class playerscript_battle_for_shattrath : public PlayerScript
{
public:
    playerscript_battle_for_shattrath() : PlayerScript("playerscript_battle_for_shattrath") { }

    void OnUpdate(Player* player, uint32 /*diff*/) override
    {
        if (player->GetMapId() != MAP_TALADOR || player->IsBeingTeleported() || player->GetExactDist2dSq(ArchOfShatar.GetPositionX(), ArchOfShatar.GetPositionY()) > 12.0f * 12.0f)
            return;
        if (std::fabs(player->GetPositionZ() - ArchOfShatar.GetPositionZ()) < 8.0f && player->IsAlive() && HasBattleQuest(player))
            EnterScenario(player);
    }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (spell->GetSpellInfo()->Id != SPELL_BLACKROCK_GRENADE)
            return;
        ObjectGuid const guid = player->GetGUID();
        player->AddDelayedEvent(500, [player, guid]() -> void
        {
            if (player->GetGUID() == guid)
                player->RemoveAurasDueToSpell(SPELL_BLACKROCK_GRENADES);
        });
    }

    void OnQuestAbandon(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_BATTLE_ALLIANCE || quest->GetQuestId() == QUEST_BATTLE_HORDE)
            CharacterDatabase.PExecute("DELETE FROM character_scenario_step WHERE guid = " UI64FMTD " AND scenario = %u",
                uint64(player->GetGUIDLow()), uint32(SCENARIO_SHATTRATH));
    }
};

void AddSC_battle_for_shattrath()
{
    new npc_shattrath_hero();
    new npc_shattrath_khadgar();
    new npc_shattrath_thaelin();
    new npc_shattrath_iron_star();
    new npc_shattrath_blackhand();
    new npc_shattrath_machinist_brandt();
    new npc_shattrath_blackrock_bomb();
    new npc_shattrath_street_fighter();
    new npc_shattrath_grenadier();
    new npc_shattrath_bomb_pickup();
    new npc_shattrath_big_cannon();
    new go_shattrath_portal_to_talador();
    new npc_talador_khadgar_shattrath();
    new playerscript_battle_for_shattrath();
}
