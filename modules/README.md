# Modules

Optional, self-contained features compiled into the worldserver without editing the core
(the playerbots port lives here as `modules/mod-playerbots`).

## How it works

- Every folder `modules/<folder>` that has a `src/` directory is a **script module**, built
  exactly like the folders of `src/server/scripts` (`cmake/macros/ConfigureScripts.cmake`).
  Its name is the folder name with every character that is not a letter or digit replaced by
  `_`: `mod-playerbots` → `mod_playerbots`.
- The module must define one loader function, which the generated script loader calls at
  startup:

  ```cpp
  void Addmod_playerbotsScripts();   // folder "mod-playerbots"
  ```

  Register scripts in it like core scripts do (`new MyPlayerScript();`, `PlayerScript`,
  `WorldScript`, `CommandScript`... from `ScriptMgr.h`).
- Linkage follows the `SCRIPTS` option (`static` by default: linked into the worldserver) and
  can be set per module like any script module: `-DSCRIPTS_MOD_PLAYERBOTS=static|dynamic|disabled`.
- Every subfolder of `src/` is an include directory, so headers are included by name.
- `-DMODULES=0` leaves every module out.

## Layout

```
modules/
  mod-yourmodule/
    src/                       compiled into the module (any subfolders)
      mod_yourmodule.cpp       defines Addmod_yourmoduleScripts()
    conf/                      optional
      yourmodule.conf.dist     staged next to worldserver.exe, installed beside worldserver.conf.dist
    sql/                       optional, see below
    README.md
```

## Config files

Load the module's config from its loader function:

```cpp
std::string error;
if (!sConfigMgr->LoadModuleConfig("yourmodule.conf", error))
    TC_LOG_ERROR("server.loading", "[mod-yourmodule] %s", error.c_str());
```

`LoadModuleConfig` reads `yourmodule.conf.dist` (the defaults) and then `yourmodule.conf` (the
server's own copy) from the folder of `worldserver.conf`, merges their keys into the main
config, and reads them again on `.reload config`. Read values with the usual
`sConfigMgr->GetIntDefault(...)`, typically in `WorldScript::OnConfigLoad`, which runs after
the loader and again on every reload. The file needs a section header (`[worldserver]`), like
`worldserver.conf`.

## Database tables

The core's SQL updater only knows the core's `sql/` folder. A module creates the tables it
needs from code at startup (`CREATE TABLE IF NOT EXISTS ...` in `OnStartup` or its loader),
and ships the same SQL in its `sql/` folder for reference.

## Example

`mod-example` is the smallest working module: a startup log line, a config file and a `.example`
GM command. Set `-DSCRIPTS_MOD_EXAMPLE=disabled` or delete the folder to leave it out.
