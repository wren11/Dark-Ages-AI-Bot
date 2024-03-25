#pragma once
#include "pch.h"
#include "lua.hpp"
class ScriptManager {
public:
    ScriptManager();
    ~ScriptManager();

    void LoadScript(const std::string& scriptPath);
    void StartScript();
    void StopScript();
    void RegisterFunctions();
    void SubscribeToEvent(const std::string& eventName, lua_State* L, int index);
    void TriggerEvent(const std::string& eventName);

    static int Lua_SubscribeToEvent(lua_State* L);
    static int Lua_BotFunction(lua_State* L);
private:
    lua_State* L;
    std::map<std::string, std::vector<int>> eventCallbacks;
};

extern ScriptManager script_manager;