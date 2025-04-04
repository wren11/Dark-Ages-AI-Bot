#pragma once
#include "pch.h"
#include "structures.h"

struct Player
{
	unsigned int Serial;
	Location Position;
	USHORT Head, Form, Body, Arms, Boots, Armor, Shield, Weapon;
	USHORT HeadColor, BootColor, Acc1Color, Acc2Color, OvercoatColor, SkinColor;
	USHORT Acc1, Acc2, Acc3, Overcoat;
	BYTE RestCloak, HideBool, FaceShape, Unknown, Unknown2;
	std::string Name, GroupName;
	BYTE NameTagStyle;
	bool Hostile;
	__time64_t KelbLastSeen;
	__time64_t LastSealSeen;

	Player() : Serial(0), Head(0), Form(0), Body(0), Arms(0), Boots(0), Armor(0), Shield(0), Weapon(0),
			   HeadColor(0), BootColor(0), Acc1Color(0), Acc2Color(0), OvercoatColor(0), SkinColor(0),
			   Acc1(0), Acc2(0), Acc3(0), Overcoat(0), RestCloak(0), HideBool(0), FaceShape(0), Unknown(0), Unknown2(0),
			   NameTagStyle(0), Hostile(false)
	{
	}

	unsigned int GetSerial() const { return Serial; }
	USHORT GetLocationX() const { return Position.X; }
	USHORT GetLocationY() const { return Position.Y; }
	Location GetLocation() const { return Position; }
	Direction GetDirection() const { return Position.FacingDirection; }
	USHORT GetHead() const { return Head; }
	USHORT GetForm() const { return Form; }
	USHORT GetBody() const { return Body; }
	USHORT GetArms() const { return Arms; }
	USHORT GetBoots() const { return Boots; }
	USHORT GetArmor() const { return Armor; }
	USHORT GetShield() const { return Shield; }
	USHORT GetWeapon() const { return Weapon; }
	USHORT GetHeadColor() const { return HeadColor; }
	USHORT GetBootColor() const { return BootColor; }
	USHORT GetAcc1Color() const { return Acc1Color; }
	USHORT GetAcc2Color() const { return Acc2Color; }
	USHORT GetOvercoatColor() const { return OvercoatColor; }
	USHORT GetSkinColor() const { return SkinColor; }
	USHORT GetAcc1() const { return Acc1; }
	USHORT GetAcc2() const { return Acc2; }
	USHORT GetAcc3() const { return Acc3; }
	USHORT GetOvercoat() const { return Overcoat; }
	BYTE GetRestCloak() const { return RestCloak; }
	BYTE GetHideBool() const { return HideBool; }
	BYTE GetFaceShape() const { return FaceShape; }
	BYTE GetUnknown() const { return Unknown; }
	BYTE GetUnknown2() const { return Unknown2; }
	std::string GetName() const { return Name; }
	std::string GetGroupName() const { return GroupName; }
	BYTE GetNameTagStyle() const { return NameTagStyle; }

	// Setters
	void SetSerial(unsigned int serial) { Serial = serial; }

	void SetLocation(const Location &newLocation)
	{
		Position = newLocation;
	}

	void SetDirection(const Direction &direction)
	{
		Position.FacingDirection = direction;
	}

	void SetHead(USHORT head) { Head = head; }
	void SetForm(USHORT form) { Form = form; }
	void SetBody(USHORT body) { Body = body; }
	void SetArms(USHORT arms) { Arms = arms; }
	void SetBoots(USHORT boots) { Boots = boots; }
	void SetArmor(USHORT armor) { Armor = armor; }
	void SetShield(USHORT shield) { Shield = shield; }
	void SetWeapon(USHORT weapon) { Weapon = weapon; }
	void SetHeadColor(USHORT headColor) { HeadColor = headColor; }
	void SetBootColor(USHORT bootColor) { BootColor = bootColor; }
	void SetAcc1Color(USHORT acc1Color) { Acc1Color = acc1Color; }
	void SetAcc2Color(USHORT acc2Color) { Acc2Color = acc2Color; }
	void SetOvercoatColor(USHORT overcoatColor) { OvercoatColor = overcoatColor; }
	void SetSkinColor(USHORT skinColor) { SkinColor = skinColor; }
	void SetAcc1(USHORT acc1) { Acc1 = acc1; }
	void SetAcc2(USHORT acc2) { Acc2 = acc2; }
	void SetAcc3(USHORT acc3) { Acc3 = acc3; }
	void SetOvercoat(USHORT overcoat) { Overcoat = overcoat; }
	void SetRestCloak(BYTE restCloak) { RestCloak = restCloak; }
	void SetHideBool(BYTE hideBool) { HideBool = hideBool; }
	void SetFaceShape(BYTE faceShape) { FaceShape = faceShape; }
	void SetUnknown(BYTE unknown) { Unknown = unknown; }
	void SetUnknown2(BYTE unknown2) { Unknown2 = unknown2; }
	void SetName(const std::string &name) { Name = name; }
	void SetGroupName(const std::string &groupName) { GroupName = groupName; }
	void SetNameTagStyle(BYTE nameTagStyle) { NameTagStyle = nameTagStyle; }

	void Refresh()
	{
	}

	bool HasKelbReady()
	{
		auto now = std::chrono::system_clock::now();
		auto kelbTimePoint = std::chrono::system_clock::from_time_t(KelbLastSeen);
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - kelbTimePoint).count();

		return duration >= 30;
	}

	bool HasSeal() const
	{
		auto now = std::chrono::system_clock::now();
		auto SealTimePoint = std::chrono::system_clock::from_time_t(LastSealSeen);
		auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - SealTimePoint).count();

		return duration <= 2.5;
	}

	void MergeUpdates(const Player &updatedPlayer)
	{
		this->Position = updatedPlayer.Position;
		this->Head = updatedPlayer.Head;
		this->Form = updatedPlayer.Form;
		this->Body = updatedPlayer.Body;
		this->Arms = updatedPlayer.Arms;
		this->Boots = updatedPlayer.Boots;
		this->Armor = updatedPlayer.Armor;
		this->Shield = updatedPlayer.Shield;
		this->Weapon = updatedPlayer.Weapon;
		this->HeadColor = updatedPlayer.HeadColor;
		this->BootColor = updatedPlayer.BootColor;
		this->Acc1Color = updatedPlayer.Acc1Color;
		this->Acc2Color = updatedPlayer.Acc2Color;
		this->OvercoatColor = updatedPlayer.OvercoatColor;
		this->SkinColor = updatedPlayer.SkinColor;
		this->Acc1 = updatedPlayer.Acc1;
		this->Acc2 = updatedPlayer.Acc2;
		this->Acc3 = updatedPlayer.Acc3;
		this->Overcoat = updatedPlayer.Overcoat;
		this->RestCloak = updatedPlayer.RestCloak;
		this->HideBool = updatedPlayer.HideBool;
		this->FaceShape = updatedPlayer.FaceShape;
		this->Name = updatedPlayer.Name;
		this->GroupName = updatedPlayer.GroupName;
		this->NameTagStyle = updatedPlayer.NameTagStyle;

		this->LastSealSeen = updatedPlayer.LastSealSeen;
		this->KelbLastSeen = updatedPlayer.KelbLastSeen;
	}

	bool IsHostile(const std::vector<std::string> &hostileList) const
	{
		return std::find(hostileList.begin(), hostileList.end(), this->Name) != hostileList.end();
	}

	void PrintData() const
	{
		constexpr int width = 15;
		std::cout << std::left << std::setw(width) << "Field" << std::setw(width) << "Value" << '\n';
		std::cout << std::string(30, '-') << '\n';
		std::cout << std::setw(width) << "Serial" << std::setw(width) << Serial << '\n';
		std::cout << std::setw(width) << "Location"
				  << "X: " << Position.X << ", Y: " << Position.Y << ", Direction: "
				  << static_cast<int>(Position.FacingDirection) << '\n';
		std::cout << std::setw(width) << "Head" << std::setw(width) << Head << '\n';
		std::cout << std::setw(width) << "Form" << std::setw(width) << Form << '\n';
		std::cout << std::setw(width) << "Body" << std::setw(width) << Body << '\n';
		std::cout << std::setw(width) << "Arms" << std::setw(width) << Arms << '\n';
		std::cout << std::setw(width) << "Boots" << std::setw(width) << Boots << '\n';
		std::cout << std::setw(width) << "Armor" << std::setw(width) << Armor << '\n';
		std::cout << std::setw(width) << "Shield" << std::setw(width) << Shield << '\n';
		std::cout << std::setw(width) << "Weapon" << std::setw(width) << Weapon << '\n';
		std::cout << std::setw(width) << "HeadColor" << std::setw(width) << HeadColor << '\n';
		std::cout << std::setw(width) << "BootColor" << std::setw(width) << BootColor << '\n';
		std::cout << std::setw(width) << "Acc1Color" << std::setw(width) << Acc1Color << '\n';
		std::cout << std::setw(width) << "Acc2Color" << std::setw(width) << Acc2Color << '\n';
		std::cout << std::setw(width) << "OvercoatColor" << std::setw(width) << OvercoatColor << '\n';
		std::cout << std::setw(width) << "SkinColor" << std::setw(width) << SkinColor << '\n';
		std::cout << std::setw(width) << "Acc1" << std::setw(width) << Acc1 << '\n';
		std::cout << std::setw(width) << "Acc2" << std::setw(width) << Acc2 << '\n';
		std::cout << std::setw(width) << "Acc3" << std::setw(width) << Acc3 << '\n';
		std::cout << std::setw(width) << "Overcoat" << std::setw(width) << Overcoat << '\n';
		std::cout << std::setw(width) << "RestCloak" << std::setw(width) << static_cast<int>(RestCloak) << '\n';
		std::cout << std::setw(width) << "HideBool" << std::setw(width) << static_cast<int>(HideBool) << '\n';
		std::cout << std::setw(width) << "FaceShape" << std::setw(width) << static_cast<int>(FaceShape) << '\n';
		std::cout << std::setw(width) << "Unknown" << std::setw(width) << static_cast<int>(Unknown) << '\n';
		std::cout << std::setw(width) << "Unknown2" << std::setw(width) << static_cast<int>(Unknown2) << '\n';
		std::cout << std::setw(width) << "Name" << std::setw(width) << Name << '\n';
		std::cout << std::setw(width) << "GroupName" << std::setw(width) << GroupName << '\n';
		std::cout << std::setw(width) << "NameTagStyle" << std::setw(width) << static_cast<int>(NameTagStyle) << '\n';
		std::cout << std::string(30, '-') << '\n';
	}
};
