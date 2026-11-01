/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <fstream>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include "Config.h"

namespace bpt = boost::property_tree;

namespace
{
    std::string _filename;
    std::vector<std::string> _args;
    boost::property_tree::ptree _config;
    std::vector<std::string> _moduleConfigs;
    std::mutex _configLock;

    // Reads one ini file and copies its keys over the loaded config.
    bool MergeConfigFile(std::string const& file, std::string& error)
    {
        try
        {
            bpt::ptree fullTree;
            read_ini(file, fullTree);
            for (auto const& section : fullTree)
                for (auto const& entry : section.second)
                    _config.put(bpt::ptree::path_type(entry.first, '/'), entry.second.data());
        }
        catch (bpt::ini_parser::ini_parser_error const& e)
        {
            if (e.line() == 0)
                error = e.message() + " (" + e.filename() + ")";
            else
                error = e.message() + " (" + e.filename() + ":" + std::to_string(e.line()) + ")";
            return false;
        }
        return true;
    }

    // Merges <dir of the main config>/<name>.dist, then <name>, when they exist.
    bool MergeModuleConfig(std::string const& name, std::string& error)
    {
        std::string::size_type const slash = _filename.find_last_of("/\\");
        std::string const path = (slash != std::string::npos ? _filename.substr(0, slash + 1) : std::string()) + name;

        bool found = false;
        for (std::string const& file : { path + ".dist", path })
        {
            std::ifstream stream(file);
            if (!stream.is_open())
                continue;
            stream.close();

            found = true;
            if (!MergeConfigFile(file, error))
                return false;
        }

        if (!found)
            error = "neither " + path + ".dist nor " + path + " exists";
        return found;
    }
}

bool ConfigMgr::LoadInitial(std::string file, std::vector<std::string> args, std::string& error)
{
    std::lock_guard<std::mutex> lock(_configLock);

    _filename = std::move(file);
    _args = std::move(args);

    try
    {
        bpt::ptree fullTree;
        read_ini(_filename, fullTree);

        if (fullTree.empty())
        {
            error = "empty file (" + _filename + ")";
            return false;
        }

        // Since we're using only one section per config file, we skip the section and have direct property access
        _config = fullTree.begin()->second;
    }
    catch (bpt::ini_parser::ini_parser_error const& e)
    {
        if (e.line() == 0)
            error = e.message() + " (" + e.filename() + ")";
        else
            error = e.message() + " (" + e.filename() + ":" + std::to_string(e.line()) + ")";
        return false;
    }

    return true;
}

ConfigMgr* ConfigMgr::instance()
{
    static ConfigMgr instance;
    return &instance;
}

bool ConfigMgr::Reload(std::string& error)
{
    if (!LoadInitial(_filename, std::move(_args), error))
        return false;

    std::lock_guard<std::mutex> lock(_configLock);
    for (std::string const& name : _moduleConfigs)
        if (!MergeModuleConfig(name, error))
            return false;

    return true;
}

bool ConfigMgr::LoadModuleConfig(std::string const& name, std::string& error)
{
    std::lock_guard<std::mutex> lock(_configLock);

    if (std::find(_moduleConfigs.begin(), _moduleConfigs.end(), name) == _moduleConfigs.end())
        _moduleConfigs.push_back(name);

    return MergeModuleConfig(name, error);
}

std::string ConfigMgr::GetStringDefault(std::string const& name, const std::string& def)
{
    auto value = _config.get<std::string>(bpt::ptree::path_type(name, '/'), def);
    value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
    return value;
}

bool ConfigMgr::GetBoolDefault(std::string const& name, bool def)
{
    try
    {
        auto val = _config.get<std::string>(bpt::ptree::path_type(name, '/'));
        val.erase(std::remove(val.begin(), val.end(), '"'), val.end());
        return (val == "true" || val == "TRUE" || val == "yes" || val == "YES" || val == "1");
    }
    catch (std::exception const& /*ex*/)
    {
        return def;
    }
}

int ConfigMgr::GetIntDefault(std::string const& name, int def)
{
    return _config.get<int>(bpt::ptree::path_type(name, '/'), def);
}

float ConfigMgr::GetFloatDefault(std::string const& name, float def)
{
    return _config.get<float>(bpt::ptree::path_type(name, '/'), def);
}

std::string const& ConfigMgr::GetFilename()
{
    std::lock_guard<std::mutex> lock(_configLock);
    return _filename;
}

std::vector<std::string> const& ConfigMgr::GetArguments() const
{
    return _args;
}

std::vector<std::string> ConfigMgr::GetKeysByString(std::string const& name)
{
    std::lock_guard<std::mutex> lock(_configLock);

    std::vector<std::string> keys;
    for (const auto& child : _config)
        if (child.first.compare(0, name.length(), name) == 0)
            keys.push_back(child.first);

    return keys;
}

