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
#include <list>
#include <map>

#include "Game_PlayerClass.hpp"

//
// Structure for entities
// Tried tp "pad" the elements by their size..
//

struct EntityStruct final
{
	std::list<lwmf::IntPointStruct> PathFindingWayPoints;
	std::map<std::string, std::int_fast32_t> ContainedItem;
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
	bool WillBeDead{};
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
	std::vector<lwmf::Wav> WeaponSounds;
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
	GLuint WeaponTexture{};
	GLuint MuzzleFlashTexture{};
	float Weight{};
	float PaceFactor{};
};

//
// Structure for entity asset data (textures, sounds etc.)
//

struct EntityAssetStruct final
{
	std::vector<std::vector<lwmf::TextureStruct>> WalkingTextures;
	std::vector<lwmf::TextureStruct> AttackTextures;
	std::vector<lwmf::TextureStruct> KillTextures;
	std::vector<std::string> Sounds;
	std::string Name;
	std::int_fast32_t Number{};
};

//
// Structure for doors
//

struct DoorStruct final
{
	lwmf::TextureStruct AnimTexture;
	std::string OpenCloseSound;
	lwmf::IntPointStruct Pos{};
	std::int_fast32_t Number{};
	std::int_fast32_t OriginalTexture{};
	std::int_fast32_t OpenCloseWidth{};
	std::int_fast32_t OpenCloseSpeed{};
	std::int_fast32_t OpenCloseCounter{};
	std::int_fast32_t StayOpenTime{};
	std::int_fast32_t StayOpenCounter{};
	bool IsOpen{};
	bool IsOpenTriggered{};
	bool IsCloseTriggered{};
	bool OpenCloseAudioFlag{};
};

//
// Init all needed objects
//

inline std::vector<DoorStruct> Doors;
inline std::vector<EntityStruct> Entities;
inline std::vector<WeaponStruct> Weapons;
inline std::vector<EntityAssetStruct> EntityAssets;
inline Game_PlayerClass Player;