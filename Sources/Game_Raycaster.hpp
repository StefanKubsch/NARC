/*
******************************************
*                                        *
* Game_Raycaster.hpp                     *
*                                        *
* (c) 2017 - 202 Stefan Kubsch           *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <cmath>
#include <algorithm>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "Game_DataStructures.hpp"
#include "Game_LevelHandling.hpp"
#include "Game_EntityHandling.hpp"

namespace Game_Raycaster
{


	enum class Renderpart : std::int_fast32_t
	{
		WallLeft,
		WalLRight,
		Ceiling,
		Floor
	};

	void Init();
	void RefreshSettings();
	void CastGraphics(Renderpart Part);

	//
	// Variables and constants
	//

	inline constexpr float VerticalLookLimitMin{ 0.0F };
	inline constexpr float VerticalLookLimitMax{ 0.4F };

	//
	// Functions
	//

	inline void Init()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Init raycaster config...");

		if (const std::string INIFile{ GameConfigFolder + "RaycasterConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			PlaneStartValue = { lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "PlaneXStartValue"), lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "PlaneYStartValue") };
			VerticalLookUpLimit = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "VerticalLookUpLimit");
			VerticalLookDownLimit = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "VerticalLookDownLimit");

			Tools_ErrorHandling::CheckAndClampRange(VerticalLookUpLimit, VerticalLookLimitMin, VerticalLookLimitMax, __FILENAME__, "VerticalLookUpLimit");
			Tools_ErrorHandling::CheckAndClampRange(VerticalLookDownLimit, VerticalLookLimitMin, VerticalLookLimitMax, __FILENAME__, "VerticalLookDownLimit");

			VerticalLookStep = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "VerticalLookStep");
			FogOfWarDistance = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "FogOfWarDistance");
		}
	}

	inline void RefreshSettings()
	{
		NARCLog.AddEntry(lwmf::LogLevel::Info, __FILENAME__, __LINE__, "Refresh raycaster settings...");

		Plane = PlaneStartValue;
		VerticalLook = 0;
		VerticalLookCamera = 0.0F;
	}

	inline void CastGraphics(const Renderpart Part)
	{
		std::int_fast32_t Start{};
		std::int_fast32_t End{ Canvas.Width };

		switch (Part)
		{
			case Renderpart::WallLeft:
			{
				End = Canvas.WidthMid;
				break;
			}
			case Renderpart::WalLRight:
			{
				Start = Canvas.WidthMid;
				break;
			}
			default: {}
		}

		const float FloorCeilingShading{ FogOfWarDistance + FogOfWarDistance * VerticalLookCamera };
		const std::int_fast32_t VerticalLookTemp{ Canvas.Height + VerticalLook };

		for (std::int_fast32_t x{ Start }; x < End; ++x)
		{
			const float Camera{ static_cast<float>(x + x) / static_cast<float>(Canvas.Width) - 1.0F };
			const lwmf::FloatPointStruct RayDir{ Player.Dir.X + Plane.X * Camera, Player.Dir.Y + Plane.Y * Camera };

			const lwmf::FloatPointStruct TempRayDir{ RayDir.X * RayDir.X, RayDir.Y * RayDir.Y };
			const lwmf::FloatPointStruct DeltaDist{ std::sqrtf(1.0F + TempRayDir.Y / TempRayDir.X), std::sqrtf(1.0F + TempRayDir.X / TempRayDir.Y) };

			lwmf::FloatPointStruct SideDist{};
			lwmf::FloatPointStruct Step{};
			lwmf::FloatPointStruct MapPos{ std::floorf(Player.Pos.X), std::floorf(Player.Pos.Y) };

			RayDir.X < 0.0F ? (Step.X = -1.0F, SideDist.X = (Player.Pos.X - MapPos.X) * DeltaDist.X) : (Step.X = 1.0F, SideDist.X = (MapPos.X + 1.0F - Player.Pos.X) * DeltaDist.X);
			RayDir.Y < 0.0F ? (Step.Y = -1.0F, SideDist.Y = (Player.Pos.Y - MapPos.Y) * DeltaDist.Y) : (Step.Y = 1.0F, SideDist.Y = (MapPos.Y + 1.0F - Player.Pos.Y) * DeltaDist.Y);

			bool WallHit{};
			bool WallSide{};
			std::int_fast32_t DoorNumber{ -1 };

			while (!WallHit)
			{
				SideDist.X < SideDist.Y ? (SideDist.X += DeltaDist.X, MapPos.X += Step.X, WallSide = false) : (SideDist.Y += DeltaDist.Y, MapPos.Y += Step.Y, WallSide = true);

				for (const auto& Door : Doors)
				{
					if (std::fabs(Door.Pos.X - MapPos.X) < FLT_EPSILON && std::fabs(Door.Pos.Y - MapPos.Y) < FLT_EPSILON)
					{
						lwmf::FloatPointStruct MapPos2{ MapPos };

						if (Player.Pos.X < MapPos2.X)
						{
							MapPos2.X -= 1.0F;
						}

						if (Player.Pos.Y > MapPos2.Y)
						{
							MapPos2.Y += 1.0F;
						}

						const float RayMulti{ WallSide ? (MapPos2.Y - Player.Pos.Y) / RayDir.Y : ((MapPos2.X - Player.Pos.X) + 1.0F) / RayDir.X };
						const lwmf::FloatPointStruct TempResult{ Player.Pos.X + RayDir.X * RayMulti, Player.Pos.Y + RayDir.Y * RayMulti };

						if (!WallSide)
						{
							const float StepY{ std::sqrtf(DeltaDist.X * DeltaDist.X - 1.0F) };

							if (std::fabs(std::floorf(TempResult.Y + (Step.Y * StepY) * 0.5F) - std::floorf(MapPos.Y)) < FLT_EPSILON && ((TempResult.Y + (Step.Y * StepY) * 0.5F) - MapPos.Y > Door.CurrentOpenPercent / 100.0F))
							{
								WallHit = true;
								DoorNumber = Door.Number;
							}
						}
						else
						{
							const float StepX{ std::sqrtf(DeltaDist.Y * DeltaDist.Y - 1.0F) };

							if (std::fabs(std::floorf(TempResult.X + (Step.X * StepX) * 0.5F) - std::floorf(MapPos.X)) < FLT_EPSILON && ((TempResult.X + (Step.X * StepX) * 0.5F) - MapPos.X > Door.CurrentOpenPercent / 100.0F))
							{
								WallHit = true;
								DoorNumber = Door.Number;
							}
						}
					}
				}

				if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(MapPos.X)][static_cast<std::int_fast32_t>(MapPos.Y)] > 0
					&& Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(MapPos.X)][static_cast<std::int_fast32_t>(MapPos.Y)] < INT_MAX)
				{
					WallHit = true;
				}
			}

			float WallDist{};

			if (!WallSide) //-V1051
			{
				if (DoorNumber > -1)
				{
					MapPos.X += Step.X * 0.5F;
				}

				WallDist = (MapPos.X - Player.Pos.X + (1.0F - Step.X) * 0.5F) / RayDir.X;
			}
			else
			{
				if (DoorNumber > -1)
				{
					MapPos.Y += Step.Y * 0.5F;
				}

				WallDist = (MapPos.Y - Player.Pos.Y + (1.0F - Step.Y) * 0.5F) / RayDir.Y;
			}

			const std::int_fast32_t LineHeight{ static_cast<std::int_fast32_t>(Canvas.Height / WallDist) };
			const std::int_fast32_t Temp{ VerticalLookTemp >> 1 };
			const std::int_fast32_t LineStart{ std::max(-(LineHeight >> 1) + Temp, 0) };
			std::int_fast32_t LineEnd{ std::min((LineHeight >> 1) + Temp, Canvas.Height) };
			float WallX{ WallSide ? Player.Pos.X + WallDist * RayDir.X : Player.Pos.Y + WallDist * RayDir.Y };
			WallX -= static_cast<std::int_fast32_t>(WallX);

			if (Part == Renderpart::WallLeft || Part == Renderpart::WalLRight)
			{
				std::int_fast32_t TextureX{ static_cast<std::int_fast32_t>(WallX * TextureSize) & (TextureSize - 1) };

				if (DoorNumber > -1)
				{
					if (Doors[DoorNumber].CurrentOpenPercent > DoorTypes[Doors[DoorNumber].DoorType].MinimumOpenPercent)
					{
						TextureX += 1;
					}

					TextureX -= static_cast<std::int_fast32_t>(Doors[DoorNumber].CurrentOpenPercent / DoorTypes[Doors[DoorNumber].DoorType].MaximumOpenPercent);
				}

				for (std::int_fast32_t y{ LineStart }; y < LineEnd; ++y)
				{
					float WallY{ static_cast<std::int_fast32_t>((y + y - VerticalLookTemp + LineHeight) / LineHeight) * 0.5F };
					WallY -= static_cast<std::int_fast32_t>(WallY);
					const std::int_fast32_t TextureY{ ((y + y - VerticalLookTemp + LineHeight) * TextureSize / LineHeight) >> 1 };
					const std::int_fast32_t WallTexel{ DoorNumber > -1 ? Doors[DoorNumber].AnimTexture.Pixels[TextureY * TextureSize + TextureX] :
						Game_LevelHandling::LevelTextures[Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][static_cast<std::int_fast32_t>(MapPos.X)][static_cast<std::int_fast32_t>(MapPos.Y)] - 1].Pixels[TextureY * TextureSize + TextureX] };

					if (Game_LevelHandling::LightingFlag)
					{
						std::int_fast32_t ShadedTexel{ lwmf::ShadeColor(WallTexel, WallDist, FogOfWarDistance) };

						for (auto&& Light : Game_LevelHandling::StaticLights)
						{
							if (Light.Location == static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall) || Light.Location == static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Door))
							{
								if (const float Intensity{ Light.GetIntensity(MapPos.X + WallX, MapPos.Y + WallY) }; Intensity > 0.0F)
								{
									ShadedTexel = lwmf::BlendColor(ShadedTexel, WallTexel, Intensity);
								}
							}
						}

						lwmf::SetPixel(Canvas, x, y, ShadedTexel);
					}
					else
					{
						lwmf::SetPixel(Canvas, x, y, WallTexel);
					}
				}
			}
			else
			{
				lwmf::FloatPointStruct FloorWall;

				if (!WallSide && RayDir.X > 0.0F)
				{
					FloorWall = { static_cast<float>(MapPos.X), MapPos.Y + WallX };
				}
				else if (!WallSide && RayDir.X < 0.0F)
				{
					FloorWall = { static_cast<float>(MapPos.X + 1), MapPos.Y + WallX };
				}
				else if (WallSide && RayDir.Y > 0.0F)
				{
					FloorWall = { MapPos.X + WallX, static_cast<float>(MapPos.Y) };
				}
				else
				{
					FloorWall = { MapPos.X + WallX, static_cast<float>(MapPos.Y + 1) };
				}

				// Store WallDist in 1D-ZBuffer for later calculation of entity distance
				// Needs only to be calculated once
				Game_EntityHandling::ZBuffer[x] = WallDist;

				LineEnd = std::clamp(LineEnd, 0, Canvas.Height);
				const std::int_fast32_t TotalHeight{ Canvas.Height + std::abs(VerticalLook) };
				const float WallDistTemp{ WallDist + WallDist * VerticalLookCamera };

				for (std::int_fast32_t y{ LineEnd + 1 }; y <= TotalHeight; ++y)
				{
					const float CurrentDist{ VerticalLookTemp / static_cast<float>(y + y - VerticalLookTemp) };
					const float FactorW{ CurrentDist / WallDistTemp };
					const lwmf::FloatPointStruct Floor{ FactorW * FloorWall.X + (1.0F - FactorW) * Player.Pos.X, FactorW * FloorWall.Y + (1.0F - FactorW) * Player.Pos.Y };

					switch (Part)
					{
						case Renderpart::Floor:
						{
							// Draw floor
							if (y < Canvas.Height)
							{
								const std::int_fast32_t FloorTexel{ Game_LevelHandling::LevelTextures[Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Floor)][static_cast<std::int_fast32_t>(Floor.X)][static_cast<std::int_fast32_t>(Floor.Y)] - 1].Pixels[(static_cast<std::int_fast32_t>(Floor.Y * TextureSize) & (TextureSize - 1)) * TextureSize + (static_cast<std::int_fast32_t>(Floor.X * TextureSize) & (TextureSize - 1))] };

								if (Game_LevelHandling::LightingFlag)
								{
									std::int_fast32_t ShadedTexel{ lwmf::ShadeColor(FloorTexel, CurrentDist, FloorCeilingShading) };

									for (auto&& Light : Game_LevelHandling::StaticLights)
									{
										if (Light.Location == static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Floor))
										{
											if (const float Intensity{ Light.GetIntensity(Floor.X, Floor.Y) }; Intensity > 0.0F)
											{
												ShadedTexel = lwmf::BlendColor(ShadedTexel, FloorTexel, Intensity);
											}
										}
									}

									lwmf::SetPixel(Canvas, x, y, ShadedTexel);
								}
								else
								{
									lwmf::SetPixel(Canvas, x, y, FloorTexel);
								}
							}

							break;
						}

						case Renderpart::Ceiling:
						{
							const std::int_fast32_t LevelCeilingMapPos{ Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Ceiling)][static_cast<std::int_fast32_t>(Floor.X)][static_cast<std::int_fast32_t>(Floor.Y)] - 1};
							const std::int_fast32_t TempY{ VerticalLookTemp - y };

							// Only render if ceiling is not transparent
							// Transparent ceiling tile is marked as "-1" in "Level_MapCeilingData.conf"
							if (LevelCeilingMapPos >= 0 && (TempY >= 0 && TempY <= LineStart))
							{
								const std::int_fast32_t CeilingTexel{ Game_LevelHandling::LevelTextures[LevelCeilingMapPos].Pixels[(static_cast<std::int_fast32_t>(Floor.Y * TextureSize) & (TextureSize - 1)) * TextureSize + (static_cast<std::int_fast32_t>(Floor.X * TextureSize) & (TextureSize - 1))] };

								if (Game_LevelHandling::LightingFlag)
								{
									std::int_fast32_t ShadedTexel{ lwmf::ShadeColor(CeilingTexel, CurrentDist, FloorCeilingShading) };

									for (auto&& Light : Game_LevelHandling::StaticLights)
									{
										if (Light.Location == static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Ceiling))
										{
											if (const float Intensity{ Light.GetIntensity(Floor.X, Floor.Y) }; Intensity > 0.0F)
											{
												ShadedTexel = lwmf::BlendColor(ShadedTexel, CeilingTexel, Intensity);
											}
										}
									}

									lwmf::SetPixel(Canvas, x, TempY, ShadedTexel);
								}
								else
								{
									lwmf::SetPixel(Canvas, x, TempY, CeilingTexel);
								}
							}

							break;
						}

						default: {}
					}
				}
			}
		}
	}


} // namespace Game_Raycaster