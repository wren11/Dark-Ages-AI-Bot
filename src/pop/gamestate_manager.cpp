#include "pch.h"
#include "gamestate_manager.h"
#include "hostile_players.h"
#include "spell.h"

game_state_manager game_state;

std::chrono::steady_clock::time_point lastUpdateTime = std::chrono::steady_clock::now();
double deltaTime = 0.0;

bool game_state_manager::initialize()
{

    hostile_players = loadPlayerNames("hostile.txt");
    std::cout << "Hostile Players Loaded [" << hostile_players.size() << "]" << std::endl;

    std::thread gameThread(&game_state_manager::update_game_states, this);
    gameThread.detach();

    return true;
}

void game_state_manager::update_game_states()
{
    while (true)
    {
        auto now = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUpdateTime).count();

        if (deltaTime >= 1.0)
        {
            update(deltaTime);
            lastUpdateTime = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void game_state_manager::update(double deltaTime)
{
    animations_manager.update(deltaTime);
}

void game_state_manager::refresh()
{
}
