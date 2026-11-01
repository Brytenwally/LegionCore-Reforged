/*
 * mod-example: the smallest working module, to check the module system.
 *
 * - its config file (conf/example.conf.dist) is loaded through sConfigMgr->LoadModuleConfig
 *   and read again on ".reload config";
 * - a WorldScript logs at startup that the module is loaded;
 * - a GM command ".example" answers in chat.
 */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Config.h"
#include "Log.h"

namespace
{
    bool ExampleEnabled = false;
    std::string ExampleGreeting;

    void LoadExampleConfig()
    {
        ExampleEnabled = sConfigMgr->GetBoolDefault("Example.Enable", true);
        ExampleGreeting = sConfigMgr->GetStringDefault("Example.Greeting", "Hello from mod-example!");
    }
}

class mod_example_world : public WorldScript
{
public:
    mod_example_world() : WorldScript("mod_example_world") { }

    void OnConfigLoad(bool /*reload*/) override
    {
        LoadExampleConfig();
    }

    void OnStartup() override
    {
        if (ExampleEnabled)
            TC_LOG_INFO("server.loading", "[mod-example] Module loaded: %s", ExampleGreeting.c_str());
    }
};

class mod_example_commandscript : public CommandScript
{
public:
    mod_example_commandscript() : CommandScript("mod_example_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commandTable =
        {
            { "example", SEC_GAMEMASTER, true, &HandleExampleCommand, "" }
        };
        return commandTable;
    }

    static bool HandleExampleCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (!ExampleEnabled)
        {
            handler->SendSysMessage("mod-example is disabled (Example.Enable = 0).");
            return true;
        }

        handler->PSendSysMessage("%s", ExampleGreeting.c_str());
        return true;
    }
};

// Loader: the folder name "mod-example" becomes Addmod_exampleScripts().
void Addmod_exampleScripts()
{
    std::string error;
    if (!sConfigMgr->LoadModuleConfig("example.conf", error))
        TC_LOG_ERROR("server.loading", "[mod-example] Config not loaded, using defaults: %s", error.c_str());

    new mod_example_world();
    new mod_example_commandscript();
}
