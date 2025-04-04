# Dark Ages AI Bot Migration Plan

This document outlines the refactoring of the network code and packet handling system to improve structure, readability, and maintainability.

## Deprecated Files

The following files are being phased out and should not be used for new development:

| Old File | Replacement | Status |
|----------|-------------|--------|
| packet_structures.h | network/packet.h | Replace |
| packet_reader.h | network/packet_reader.h | Replace |
| packet_writer.h | network/packet_writer.h | Replace |
| packet_registry.h | network/packet_handler_registry.h | Replace |
| packet_handler.h | network/packet_handler.h | Replace |
| packet_processor.h | network/packet_handler_registry.h | Replace |
| recv_handlers.cpp | network/packet_handlers/* | Migrate |
| send_handlers.cpp | network/packet_handlers/* | Migrate |
| x33_player_handler.cpp | network/packet_handlers/player_info_handler.cpp | Migrate |

## Migration Strategy

1. **Phase 1: Parallel Systems** (Current)
   - Keep both old and new systems functioning
   - Add forwarding functions in legacy handlers to call new implementations
   - Update intercept_manager.cpp to use new packet handling system

2. **Phase 2: Code Migration** (In Progress)
   - Move game structures to game/ namespace
   - Move network structures to network/ namespace
   - Implement configuration system for game constants and addresses
   - Add memory scanning capabilities for automatic address detection

3. **Phase 3: Clean Up** (Planned)
   - Remove deprecated files
   - Update project files to reflect new structure
   - Document new architecture

## New Components

### Memory Scanning System

The new memory scanning system provides:
- Pattern-based memory scanning for function addresses
- Configuration-driven address management
- Runtime function detection and hooking

### Configuration System

The configuration system enables:
- Loading/saving settings from INI files
- Default values for game addresses
- Runtime configuration of bot behavior

## Usage Notes

When working with this codebase:
1. Use classes from the `network` namespace for packet handling
2. Use classes from the `game` namespace for game data structures
3. Use `utils::ConfigManager` for accessing configuration values
4. Use `utils::MemoryScanner` for locating memory addresses

### Example: Sending a Packet

Old approach:
```cpp
packet pkt;
// Fill packet data
send_packet(pkt);
```

New approach:
```cpp
network::PacketWriter writer(0x10); // Packet type
writer.writeByte(42);
// Fill rest of packet
writer.sendToServer();
```

### Example: Reading a Packet

Old approach:
```cpp
void handle_packet(const packet& pkt) {
    PacketReader reader(pkt);
    uint8_t value = reader.readByte();
    // Process packet
}
```

New approach:
```cpp
void processPacket(const network::Packet& packet) {
    network::PacketReader reader(packet);
    uint8_t value = reader.readByte();
    // Process packet with improved error handling
}
```
