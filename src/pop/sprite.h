#pragma once
#include <memory>
#include <string>
#include <cstdint>

enum class Direction : uint8_t;

enum class NpcType
{
    Mundane,
    Item
};

class Sprite
{
    uint16_t xCoord;
    uint16_t yCoord;
    uint32_t serial;
    uint16_t image;
    uint8_t color;
    Direction direction;
    NpcType type;
    std::string name;

public:
    Sprite(uint16_t x, uint16_t y, uint32_t id, uint16_t img)
        : xCoord(x), yCoord(y), serial(id), image(img), color(0), direction(), type(NpcType::Mundane) {}

    // Disable copy, enable move
    Sprite(const Sprite &) = delete;
    Sprite &operator=(const Sprite &) = delete;
    Sprite(Sprite &&) noexcept = default;
    Sprite &operator=(Sprite &&) noexcept = default;

    // Core accessors
    uint32_t GetSerial() const { return serial; }
    uint16_t GetXCoord() const { return xCoord; }
    uint16_t GetYCoord() const { return yCoord; }
    uint16_t GetImage() const { return image; }
    uint8_t GetColor() const { return color; }
    const std::string& GetName() const { return name; }
    
    // Core mutators
    void SetPosition(uint16_t x, uint16_t y) { xCoord = x; yCoord = y; }
    void SetImage(uint16_t img) { image = img; }
    void SetColor(uint8_t clr) { color = clr; }
    void SetName(const std::string& newName) { name = newName; }
    void SetType(NpcType newType) { type = newType; }
};