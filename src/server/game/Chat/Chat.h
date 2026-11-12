////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#ifndef CHAT_H
#define CHAT_H

#include "Player.h"
#include <boost/any.hpp>
#include "StringFormat.h"

class ChatHandler;
class WorldSession;
class WorldObject;
class Creature;
class Group;
class Player;
class Unit;
struct GameTele;

class ChatCommand
{
    typedef bool(*pHandler)(ChatHandler*, char const*);

    public:
        ChatCommand(char const* name, uint32 securityLevel, bool allowConsole, pHandler handler, std::string help, std::vector<ChatCommand> childCommands = std::vector<ChatCommand>()) :
            Name(name), SecurityLevel(securityLevel), AllowConsole(allowConsole), Handler(handler), Help(std::move(help)), ChildCommands(std::move(childCommands)) { }

        char const* Name;
        uint32 SecurityLevel;
        bool AllowConsole;
        pHandler Handler;
        std::string Help;
        std::vector<ChatCommand> ChildCommands;
};

class TC_GAME_API ChatHandler
{
    public:
        WorldSession* GetSession() { return m_session; }
        explicit ChatHandler(WorldSession* session) : m_session(session), sentErrorMessage(false) {}
        explicit ChatHandler(Player* player) : m_session(player->GetSession()), sentErrorMessage(false) {}
        virtual ~ChatHandler() {}

        static char* LineFromMessage(char*& pos) { char* start = strtok(pos, "\n"); pos = nullptr; return start; }

        // function with different implementation for chat/console
        virtual const char *GetTrinityString(int32 entry) const;
        virtual void SendSysMessage(const char *str);

        void SendSysMessage(int32 entry);

        template<typename... Args>
        void PSendSysMessage(const char* fmt, Args&&... args)
        {
            SendSysMessage(Trinity::StringFormat(fmt, std::forward<Args>(args)...).c_str());
        }

        template<typename... Args>
        void PSendSysMessage(uint32 entry, Args&&... args)
        {
            SendSysMessage(PGetParseString(entry, std::forward<Args>(args)...).c_str());
        }

        template<typename... Args>
        std::string PGetParseString(int32 entry, Args&&... args) const
        {
            return fmt::sprintf(GetTrinityString(entry), std::forward<Args>(args)...);
        }

        int ParseCommands(const char* text);

        bool PlayerExtraCommand(const char* text);

        static std::vector<ChatCommand> const& getCommandTable();
        static void invalidateCommandTable();

        bool isValidChatMessage(const char* msg);
        void SendGlobalSysMessage(const char *str);

        bool hasStringAbbr(const char* name, const char* part);

        // function with different implementation for chat/console
        virtual bool isAvailable(ChatCommand const& cmd) const;
        virtual std::string GetNameLink() const { return GetNameLink(m_session->GetPlayer()); }
        virtual bool needReportToTarget(Player* chr) const;
        virtual LocaleConstant GetSessionDbcLocale() const;
        virtual int GetSessionDbLocaleIndex() const;

        bool HasLowerSecurity(Player* target, ObjectGuid guid, bool strong = false);
        bool HasLowerSecurityAccount(WorldSession* target, uint32 account, bool strong = false);

        void SendGlobalGMSysMessage(const char *str);
        Player*   getSelectedPlayer();
        Creature* getSelectedCreature();
        Unit*     getSelectedUnit();
        WorldObject* getSelectedObject();

        char*     extractKeyFromLink(char* text, char const* linkType, char** something1 = nullptr);
        char*     extractKeyFromLink(char* text, char const* const* linkTypes, int* found_idx, char** something1 = nullptr);

        // if args have single value then it return in arg2 and arg1 == NULL
        void      extractOptFirstArg(char* args, char** arg1, char** arg2);
        char*     extractQuotedArg(char* args);

        uint32    extractSpellIdFromLink(char* text);
        ObjectGuid    extractGuidFromLink(char* text);
        GameTele const* extractGameTeleFromLink(char* text);
        bool GetPlayerGroupAndGUIDByName(const char* cname, Player* &player, Group* &group, ObjectGuid &guid, bool offline = false);
        std::string extractPlayerNameFromLink(char* text);
        // select by arg (name/link) or in-game selection online/offline player.
        // *player is set only for a player standing on the caller's own map (never from the console): another map's player belongs
        // to another thread. The guid and name are still filled for an online player of another map, see IsOnlineElsewhere.
        bool extractPlayerTarget(char* args, Player** player, ObjectGuid* player_guid = nullptr, std::string* player_name = nullptr);
        // true for a target that is logged in but has no pointer from extractPlayerTarget: read it with ObjectAccessor::WithPlayer,
        // change it with ObjectAccessor::PostToPlayer, and never take the offline database path for him
        static bool IsOnlineElsewhere(Player const* target, ObjectGuid guid);
        // extractPlayerTarget for a command that needs a logged-in player: an offline name or guid is refused
        bool extractOnlinePlayerTarget(char* args, Player** player, ObjectGuid* player_guid, std::string* player_name = nullptr);
        // needReportToTarget for a logged-in player of another map
        bool NeedReportToElsewhere(ObjectGuid guid) const;
        // group of a target (null without group): read under the accessor lock when he is on another map
        static Group* FindTargetGroup(Player* target, ObjectGuid guid);
        // runs action in the own thread of a logged-in target of another map; false (and "player not found") if he just left.
        // With announceName the caller is told at once that the action was queued.
        bool PostToTarget(ObjectGuid guid, std::function<void(Player*)>&& action, char const* announceName = nullptr);
        // same selection, but only the guid and name are returned: a player found by name may stand on another map, so no pointer
        bool extractPlayerGuidTarget(char* args, ObjectGuid* player_guid, std::string* player_name = nullptr);

        std::string playerLink(std::string const& name) const { return m_session ? "|cffffffff|Hplayer:"+name+"|h["+name+"]|h|r" : name; }
        std::string GetNameLink(Player* chr) const { return playerLink(chr->GetName()); }

        GameObject* GetNearbyGameObject();
        GameObject* GetObjectGlobalyWithGuidOrNearWithDbGuid(ObjectGuid::LowType lowguid, uint32 entry);
        bool HasSentErrorMessage() const { return sentErrorMessage; }
        void SetSentErrorMessage(bool val){ sentErrorMessage = val; }

        bool ShowHelpForCommand(std::vector<ChatCommand> const& table, const char* cmd);
    protected:
        explicit ChatHandler() : m_session(nullptr), sentErrorMessage(false) {}      // for CLI subclass
        static bool SetDataForCommandInTable(std::vector<ChatCommand>& table, const char* text, uint32 permission, std::string const& help, std::string const& fullcommand);
        bool ExecuteCommandInTable(std::vector<ChatCommand> const& table, const char* text, std::string const& fullcmd);
        bool ShowHelpForSubCommands(std::vector<ChatCommand> const& table, char const* cmd, char const* subcmd);

    private:
        WorldSession* m_session;                           // != NULL for chat command call and NULL for CLI command

        // common global flag
        bool sentErrorMessage;
};

class CliHandler : public ChatHandler
{
    public:
        typedef void Print(void*, char const*);
        explicit CliHandler(void* callbackArg, Print* zprint);

        // overwrite functions
        const char *GetTrinityString(int32 entry) const override;
        bool isAvailable(ChatCommand const& cmd) const override;
        void SendSysMessage(const char *str) override;
        std::string GetNameLink() const override;
        bool needReportToTarget(Player* chr) const override;
        LocaleConstant GetSessionDbcLocale() const override;
        int GetSessionDbLocaleIndex() const override;

    private:
        void* m_callbackArg;
        Print* m_print;
};

class TC_GAME_API CommandArgs
{
public:
    enum CommandArgsType
    {
        ARG_INT,
        ARG_UINT,
        ARG_FLOAT,
        ARG_STRING,
        ARG_QUOTE_ENCLOSED_STRING,
    };

    CommandArgs(ChatHandler* handler, char const* args);
    CommandArgs(ChatHandler* handler, char const* args, std::initializer_list<CommandArgsType> argsType);

    bool ValidArgs() const;
    void Initialize(std::initializer_list<CommandArgsType> argsType);

    uint32 GetArgInt(uint32 index);
    uint32 GetArgUInt(uint32 index);
    float GetArgFloat(uint32 index);
    std::string GetArgString(uint32 index);

    template<typename T>
    T GetArg(uint32 index)
    {
        ASSERT(index < _args.size());
        return boost::any_cast<T>(_args[index]);
    }

private:
    bool _validArgs;
    ChatHandler* _handler;
    char const* _charArgs;
    std::initializer_list<CommandArgsType> _argsType;
    std::vector<boost::any> _args;
};

#endif
