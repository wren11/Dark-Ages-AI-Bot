#pragma once
#include "pch.h"
#include "script_manager.h"
#include "gamestate_manager.h"
#include "network_functions.h"
#include "ui_manager.h"

ScriptManager script_manager;

ScriptManager::ScriptManager() {
    L = luaL_newstate();
    luaL_openlibs(L);
    RegisterFunctions();
}

ScriptManager::~ScriptManager() {
    lua_close(L);
}

void ScriptManager::LoadScript(const std::string& scriptPath) {
    if (luaL_dofile(L, scriptPath.c_str()) != LUA_OK) {
        const char* error = lua_tostring(L, -1);
        std::cerr << "Error loading script: " << scriptPath << " with error: " << error << std::endl;
    }
}

void ScriptManager::StartScript() {
    TriggerEvent("OnGameStart");
}

void ScriptManager::StopScript() {
    TriggerEvent("OnGameEnd");
}

void ScriptManager::SubscribeToEvent(const std::string& eventName, lua_State* L, int index) {
    if (!lua_isfunction(L, index)) return;
    lua_pushvalue(L, index);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    eventCallbacks[eventName].push_back(ref);
}

void ScriptManager::TriggerEvent(const std::string& eventName) {
    std::cout << "Triggering event: " << eventName << std::endl;
    auto& callbacks = eventCallbacks[eventName];

    for (int ref : callbacks) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            const char* error = lua_tostring(L, -1);
            std::cerr << "Error triggering event '" << eventName << "': " << error << std::endl;
        }
    }
}
void ScriptManager::RegisterFunctions() {
    lua_register(L, "BotFunction", Lua_BotFunction);
    lua_register(L, "SubscribeToEvent", Lua_SubscribeToEvent); 
}

int ScriptManager::Lua_SubscribeToEvent(lua_State* L) {
    std::cout << "Subscribing to event from Lua" << std::endl; 
    if (lua_gettop(L) != 2) {
        lua_pushstring(L, "Incorrect number of arguments to SubscribeToEvent");
        lua_error(L);
        return 0;
    }

    if (!lua_isstring(L, 1)) {
        lua_pushstring(L, "First argument must be a string (event name)");
        lua_error(L);
        return 0;
    }
    const std::string eventName = lua_tostring(L, 1);

    if (!lua_isfunction(L, 2)) {
        lua_pushstring(L, "Second argument must be a function (event callback)");
        lua_error(L);
        return 0;
    }

    lua_pushvalue(L, 2);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);

    script_manager.SubscribeToEvent(eventName, L, ref);
    return 0;
}

int ScriptManager::Lua_BotFunction(lua_State* L) {
    return 0;
}