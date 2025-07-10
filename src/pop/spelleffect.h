#pragma once

enum class StaffEffectType
{
    Reduce,
    Set
};

class SpellEffect
{
public:
    virtual ~SpellEffect() = default;
    virtual StaffEffectType GetType() const = 0;
    virtual int GetEffectValue() const = 0;
};
class ReduceEffect : public SpellEffect
{
public:
    ReduceEffect(int value) : value_(value) {}
    StaffEffectType GetType() const override { return StaffEffectType::Reduce; }
    int GetEffectValue() const override { return value_; }

private:
    int value_;
};

class SetEffect : public SpellEffect
{
public:
    SetEffect(int value) : value_(value) {}
    StaffEffectType GetType() const override { return StaffEffectType::Set; }
    int GetEffectValue() const override { return value_; }

private:
    int value_;
};
