# Dark Ages AI Bot - Coding Standard

## File Organization

- `core/`: Core functionality and base classes
- `network/`: Packet handling and network operations
- `game/`: Game-specific data structures and logic
- `utils/`: Utility classes and helper functions

## Naming Conventions

### Files

- Use snake_case for file names: `packet_handler.h`, `game_state.cpp`
- Header files use `.h` extension, implementation files use `.cpp`
- Each class should be in its own file with matching names

### Classes, Structs, and Enums

- Use PascalCase: `class PacketHandler`, `struct PlayerData`, `enum class Direction`
- Abstract classes/interfaces should be prefixed with 'I': `class IPacketHandler`

### Functions and Methods

- Use camelCase: `void processPacket()`, `int getPlayerCount()`
- Boolean functions should be prefixed with 'is', 'has', or 'should': `bool isPlayerActive()`

### Variables

- Use camelCase for variables: `int playerCount`, `std::string playerName`
- Class member variables use camelCase with trailing underscore: `int playerCount_`
- Constants and static variables use ALL_CAPS with underscores: `const int MAX_PLAYERS`

### Namespaces

- Use snake_case for namespaces: `namespace game_utils`
- Prefer nested namespaces for organization: `namespace core::network`

## Code Formatting

- Indent with 4 spaces, not tabs
- Braces on new lines for functions, same line for control statements
- Max line length of 100 characters
- Use blank lines to separate logical sections of code

## SOLID Principles Implementation

1. **Single Responsibility Principle**
   - Each class should have a single responsibility
   - Break large classes into smaller, focused ones

2. **Open/Closed Principle**
   - Classes should be open for extension but closed for modification
   - Use inheritance and interfaces appropriately

3. **Liskov Substitution Principle**
   - Derived classes must be substitutable for their base classes
   - Avoid breaking inherited contracts

4. **Interface Segregation Principle**
   - Keep interfaces focused and minimal
   - Clients should not depend on methods they don't use

5. **Dependency Inversion Principle**
   - Depend on abstractions, not implementations
   - Use dependency injection where appropriate

## Best Practices

- Use smart pointers instead of raw pointers
- Prefer `const` for variables and parameters that shouldn't change
- Mark member functions as `const` when they don't modify object state
- Use `override` and `final` where appropriate
- Add `noexcept` to functions that won't throw exceptions
- Use `[[nodiscard]]` for functions whose return values shouldn't be ignored

## Documentation

- Use Doxygen-style comments for public interfaces
- Each file should have a brief description at the top
- Document preconditions, postconditions, and exceptions for functions
