/*
******************************************
*                                        *
* Game_DataStructures.hpp                *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include "Game_PlayerClass.hpp"

// Tried tp "pad" the elements by their size..

//
// Structure for entity asset data (textures, sounds etc.)
//

struct EntityAssetStruct final
{
	std::vector<std::vector<lwmf::TextureStruct>> WalkingTextures{};
	std::vector<lwmf::TextureStruct> AttackTextures{};
	std::vector<lwmf::TextureStruct> KillTextures{};
	std::vector<lwmf::MP3> Sounds{};
	std::string Name;
	std::int_fast32_t Number{};
};

//
// Structure for entities
//

struct EntityStruct final
{
	std::list<lwmf::IntPointStruct> PathFindingWayPoints{};
	std::map<std::string, std::int_fast32_t> ContainedItem{};
	lwmf::FloatPointStruct Pos{};
	lwmf::FloatPointStruct Dir{};
	std::string TypeName;
	std::int_fast32_t Number{};
	std::int_fast32_t Type{};
	std::int_fast32_t TypeNumber{};
	std::int_fast32_t WaitTimer{};
	std::int_fast32_t Hitpoints{};
	std::int_fast32_t WalkAnimCounter{};
	std::int_fast32_t WalkAnimStep{};
	std::int_fast32_t WalkAnimStepWidth{};
	std::int_fast32_t AttackMode{};
	std::int_fast32_t AttackAnimCounter{};
	std::int_fast32_t AttackAnimStep{};
	std::int_fast32_t AttackAnimStepWidth{};
	std::int_fast32_t HitAnimDuration{};
	std::int_fast32_t HitAnimCounter{};
	std::int_fast32_t KillAnimStep{};
	std::int_fast32_t KillAnimStepWidth{};
	std::int_fast32_t KillAnimCounter{};
	std::int_fast32_t MovementBehaviour{};
	std::int_fast32_t RotationFactor{};
	std::int_fast32_t DamagePoints{};
	std::int_fast32_t DamageHitrate{};
	std::int_fast32_t DamageHitrateCounter{};
	std::int_fast32_t PathFindingStart{};
	std::int_fast32_t PathFindingTarget{};
	float MoveSpeed{};
	float MoveV{};
	char Direction{ '\0' };
	bool AttackFinished{};
	bool AttackAnimEnabled{};
	bool KillAnimEnabled{};
	bool IsPickedUp{};
	bool IsDead{};
	bool IsHit{};
	bool ValidPathFound{};
};

//
// Structure for weapons
//

struct WeaponStruct final
{
	lwmf::ShaderClass WeaponShader{};
	lwmf::ShaderClass MuzzleFlashShader{};
	std::vector<lwmf::MP3> Sounds{};
	lwmf::IntRectStruct WeaponRect{};
	lwmf::IntRectStruct MuzzleFlashRect{};
	std::string HUDAmmoInfo;
	std::string HUDCarriedAmmoInfo;
	std::string Name;
	std::int_fast32_t Type{};
	std::int_fast32_t ReloadDuration{};
	std::int_fast32_t ReloadCounter{};
	std::int_fast32_t FadeInOutSpeed{};
	std::int_fast32_t CarriedAmmo{};
	std::int_fast32_t Number{};
	std::int_fast32_t MuzzleFlashDuration{};
	std::int_fast32_t MuzzleFlashCounter{};
	std::int_fast32_t Capacity{};
	std::int_fast32_t LoadedRounds{};
	std::int_fast32_t Damage{};
	std::int_fast32_t Cadence{};
	std::int_fast32_t CadenceCounter{};
	float Weight{};
	float PaceFactor{};
};

//
// Structures for doors
//

struct DoorTypeStruct final
{
	lwmf::TextureStruct OriginalTexture{};
	std::vector<lwmf::MP3> Sounds{};
	std::int_fast32_t OpenCloseSpeed{};
	std::int_fast32_t StayOpenTime{};
	float MaximumOpenPercent{};
	float MinimumOpenPercent{};
};

struct DoorStruct final
{
	enum class States : std::int_fast32_t
	{
		Closed,
		Triggered,
		Open
	};

	lwmf::TextureStruct AnimTexture{};
	lwmf::FloatPointStruct Pos{};
	States State{};
	std::int_fast32_t DoorType{};
	std::int_fast32_t Number{};
	std::int_fast32_t StayOpenCounter{};
	float CurrentOpenPercent{};
	bool CloseAudioFlag{};
};

//
// Init all needed objects
//

inline std::vector<EntityAssetStruct> EntityAssets{};
inline std::vector<EntityStruct> Entities{};
inline std::vector<WeaponStruct> Weapons{};
inline std::vector<DoorTypeStruct> DoorTypes{};
inline std::vector<DoorStruct> Doors{};
inline Game_PlayerClass Player;