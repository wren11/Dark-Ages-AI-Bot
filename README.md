# Dark Ages AI: pop

## Overview
**Dark Ages AI: pop** — This here's the nuttiest, dumbest thing I ever spat out: crammin' AI into some ancient MMORPG just for the heck of it. Dreamt it up outta pure spite and a deep, burning loathing for them no-good arena botting fools and  desi programmers in Dark Ages.

## Features
- **Packet Pranks:** Messing with internet thingies for the LOLs.
- **Overlay Overkill:** Smashing the screen with too much stuff because why not?
- **Scripting Shenanigans:** Sneakily using code spells to do sneaky things.
- **AI:** Making smart bots question their life choices or just annoy their creators.
- **Self-Injection:** Sneaks into the game like a ninja through DLL proxying, no separate injector needed!

## Disclaimer
This ain't just a bad idea—it's THE worst idea, mate.

## How to (Not) Play Along
1. **Clone the Repository (But Seriously, Why Would Ya?)**
   ```bash
   git clone https://github.com/wren11/dark-ages-ai-pop.git
   cd dark-ages-ai-pop
   ```

2. **Build This Monstrosity (New CMake System)**
   ```bash
   # Just run the build script, ya crazy dingbat
   build.bat
   
   # Or if you're a masochist wanting manual control:
   mkdir build && cd build
   cmake ..
   cmake --build . --config Release
   ```

3. **Install Your Doom**
   * The build script will ask where your Dark Ages installation is
   * It'll copy the dinput8.dll proxy and our actual bot DLL
   * Once copied, just start the game normally (but maybe say sorry first)

## CMake Build System
We've done gone and modernized this disaster with a proper build system:
- Uses CMake because we're fancy like that
- Builds as a proxy DLL (default: dinput8.dll) for automatic loading
- Creates both the proxy and the actual bot DLL
- Automatically backs up the original DLL

## The Self-Injection Magic Trick
This bad boy uses DLL proxying to auto-inject itself:
1. Our fake dinput8.dll gets loaded by the game
2. It loads the original dinput8.dll (renamed to original_dinput8.dll)
3. It then loads our bot DLL and initializes it
4. All DirectInput calls get forwarded to the original DLL

## Configuration
- Config files go in `config/` directory
- Game data in `data/` directory 
- Logs in `logs/` directory

## Memory Scanning
The bot includes a memory scanner that can find important game functions automatically. Access this through the in-game menu by pressing F8 (default hotkey).

## For The Extra Paranoid
If you wanna be sure this thing ain't doing anything TOO evil, check:
- `src/pop/proxy/proxy_dllmain.cpp` for the DLL proxy code
- `src/pop/initialization.h` for the bot's initialization process

Remember: with great power comes great irresponsibility!
