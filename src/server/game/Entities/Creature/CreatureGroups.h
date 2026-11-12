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

#ifndef _FORMATIONS_H
#define _FORMATIONS_H

#include <mutex>
#include <unordered_map>
#include <vector>

class Creature;
class CreatureGroup;

struct FormationInfo
{
    ObjectGuid::LowType leaderGUID;
    float follow_dist;
    float follow_angle;
    uint8 groupAI;

    // scripts delete the shared info of a custom formation themselves in places: the manager has to hear of it
    static TC_GAME_API void operator delete(void* info);
};

typedef std::unordered_map<ObjectGuid::LowType/*memberDBGUID*/, FormationInfo*>   CreatureGroupInfoType;

class TC_GAME_API FormationMgr
{
        FormationMgr() { }
        ~FormationMgr();

    public:
        static FormationMgr* instance();

        void AddCreatureToGroup(ObjectGuid::LowType const& group_id, Creature* creature);
        void RemoveCreatureFromGroup(CreatureGroup* group, Creature* creature);
        void LoadCreatureFormations();
        CreatureGroupInfoType CreatureGroupMap;
        // the info belongs to the leader's group and is freed with it
        FormationInfo* CreateCustomFormation(Creature* c);

        bool IsCustomFormationOwnedBy(FormationInfo const* info, CreatureGroup const* group);
        void ForgetCustomFormation(void const* info);

    private:
        std::mutex _customLock;                             // protects _customFormations (maps run in their own threads)
        std::unordered_map<void const*, CreatureGroup const*> _customFormations;  // custom infos not deleted yet, with their group
};

class TC_GAME_API CreatureGroup
{
        Creature* m_leader; //Important do not forget sometimes to work with pointers instead synonims :D:D
        typedef std::map<Creature*, FormationInfo*>  CreatureGroupMemberType;
        CreatureGroupMemberType m_members;

        std::vector<FormationInfo*> m_ownedInfos;           // custom infos created for this group

        ObjectGuid::LowType m_groupID;
        bool m_Formed;

    public:
        //Group cannot be created empty
        explicit CreatureGroup(ObjectGuid::LowType const& id);
        ~CreatureGroup();

        void AddOwnedInfo(FormationInfo* info) { m_ownedInfos.push_back(info); }

        Creature* getLeader() const { return m_leader; }
        ObjectGuid::LowType GetId() const { return m_groupID; }
        bool isEmpty() const { return m_members.empty(); }
        bool isFormed() const { return m_Formed; }

        void AddMember(Creature* member, FormationInfo* f = nullptr);
        void RemoveMember(Creature* member);
        void FormationReset(bool dismiss);

        void LeaderMoveTo(float x, float y, float z, float speed = 0.0f);
        void MemberAttackStart(Creature* member, Unit* target);

        CreatureGroupMemberType& GetMembers() { return m_members; }
};

#define sFormationMgr FormationMgr::instance()

#endif
