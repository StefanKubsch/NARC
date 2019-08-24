/*
******************************************
*                                        *
* Game_Raycaster.hpp                     *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>

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
	// Functions
	//

	inline void Init()
	{
		if (const std::string INIFile{ "./DATA/GameConfig/RaycasterConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
		{
			PlaneStartValue.X = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "PlaneXStartValue");
			PlaneStartValue.Y = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "PlaneYStartValue");
			VerticalLookUpLimit = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "VerticalLookUpLimit");
			VerticalLookDownLimit = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "VerticalLookDownLimit");
			VerticalLookStep = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "VerticalLookStep");
			FogOfWarDistance = lwmf::ReadINIValue<float>(INIFile, "RAYCASTER", "FogOfWarDistance");
		}
	}

	inline void RefreshSettings()
	{
		Plane.X = PlaneStartValue.X;
		Plane.Y = PlaneStartValue.Y;
		VerticalLook = 0;
		VerticalLookCamera = 0.0F;
	}

	inline void CastGraphics(const Renderpart Part)
	{
		std::int_fast32_t Start{};
		std::int_fast32_t End{ ScreenTexture.Width };

		switch (Part)
		{
			case Renderpart::WallLeft:
			{
				End = ScreenTexture.WidthMid;
				break;
			}
			case Renderpart::WalLRight:
			{
				Start = ScreenTexture.WidthMid;
				break;
			}
			default: {}
		}

		const float FloorCeilingShading{ FogOfWarDistance + FogOfWarDistance * VerticalLookCamera };
		const std::int_fast32_t VerticalLookTemp{ ScreenTexture.Height + VerticalLook };

		for (std::int_fast32_t x{ Start }; x < End; ++x)
		{
			const float Camera{ (x + x) / static_cast<float>(ScreenTexture.Width) - 1.0F };
			const lwmf::FloatPointStruct RayDir{ Player.Dir.X + Plane.X * Camera, Player.Dir.Y + Plane.Y * Camera };
			const lwmf::FloatPointStruct DeltaDist{ std::abs(1.0F / RayDir.X), std::abs(1.0F / RayDir.Y) };

			lwmf::FloatPointStruct SideDist;
			lwmf::IntPointStruct Step;
			lwmf::IntPointStruct MapPos{ static_cast<std::int_fast32_t>(Player.Pos.X), static_cast<std::int_fast32_t>(Player.Pos.Y) };

			RayDir.X < 0.0F ? (Step.X = -1, SideDist.X = (Player.Pos.X - MapPos.X) * DeltaDist.X) : (Step.X = 1, SideDist.X = (MapPos.X + 1 - Player.Pos.X) * DeltaDist.X);
			RayDir.Y < 0.0F ? (Step.Y = -1, SideDist.Y = (Player.Pos.Y - MapPos.Y) * DeltaDist.Y) : (Step.Y = 1, SideDist.Y = (MapPos.Y + 1 - Player.Pos.Y) * DeltaDist.Y);

			bool WallHit{};
			bool WallSide{};
			std::int_fast32_t DoorNumber{ -1 };

			while (!WallHit)
			{
				SideDist.X < SideDist.Y ? (SideDist.X += DeltaDist.X, MapPos.X += Step.X, WallSide = false) : (SideDist.Y += DeltaDist.Y, MapPos.Y += Step.Y, WallSide = true);

				for (const auto& Door : Doors)
				{
					if (Door.Pos.X == MapPos.X && Door.Pos.Y == MapPos.Y)
					{
						Door.IsOpen ? Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][Door.Pos.X][Door.Pos.Y] = 0 :
							static_cast<std::int_fast32_t>((Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][Door.Pos.X][Door.Pos.Y] = 1, DoorNumber = Door.Number, WallHit = true));

						break;
					}
				}

				if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][MapPos.X][MapPos.Y] > 0)
				{
					WallHit = true;
				}
			}

			const float WallDist{ WallSide ? (MapPos.Y - Player.Pos.Y + (1 - Step.Y) * 0.5F) / RayDir.Y : (MapPos.X - Player.Pos.X + (1 - Step.X) * 0.5F) / RayDir.X };
			const std::int_fast32_t LineHeight{ static_cast<std::int_fast32_t>(ScreenTexture.Height / WallDist) };
			const std::int_fast32_t Temp{ VerticalLookTemp >> 1 };
			const std::int_fast32_t LineStart{ (std::max)(-(LineHeight >> 1) + Temp, 0) };
			std::int_fast32_t LineEnd{ (std::min)((LineHeight >> 1) + Temp, ScreenTexture.Height) };
			float WallX{ WallSide ? Player.Pos.X + WallDist * RayDir.X : Player.Pos.Y + WallDist * RayDir.Y };
			WallX -= static_cast<std::int_fast32_t>(WallX);

			if (Part == Renderpart::WallLeft || Part == Renderpart::WalLRight)
			{
				const std::int_fast32_t TextureX{ static_cast<std::int_fast32_t>(WallX * TextureSize) & TextureSizeBitwiseAnd };

				for (std::int_fast32_t y{ LineStart }; y < LineEnd; ++y)
				{
					float WallY{ static_cast<std::int_fast32_t>((y + y - VerticalLookTemp + LineHeight) / LineHeight) * 0.5F };
					WallY -= static_cast<std::int_fast32_t>(WallY);
					const std::int_fast32_t TextureY{ ((y + y - VerticalLookTemp + LineHeight) * TextureSize / LineHeight) >> 1 };
					const std::int_fast32_t WallTexel = DoorNumber > -1 ? Doors[DoorNumber].AnimTexture.Pixels[TextureY * TextureSize + TextureX] :
						Game_LevelHandling::LevelTextures[Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][MapPos.X][MapPos.Y] - 1].Pixels[TextureY * TextureSize + TextureX];

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

						lwmf::SetPixel(ScreenTexture, x, y, ShadedTexel);
					}
					else
					{
						lwmf::SetPixel(ScreenTexture, x, y, WallTexel);
					}
				}
			}
			else
			{
				lwmf::FloatPointStruct FloorWall;

				if (!WallSide && RayDir.X > 0.0F)
				{
					FloorWall.X = static_cast<float>(MapPos.X);
					FloorWall.Y = MapPos.Y + WallX; //-V537
				}
				else if (!WallSide && RayDir.X < 0.0F)
				{
					FloorWall.X = static_cast<float>(MapPos.X + 1);
					FloorWall.Y = MapPos.Y + WallX; //-V537
				}
				else if (WallSide && RayDir.Y > 0.0F)
				{
					FloorWall.X = MapPos.X + WallX;
					FloorWall.Y = static_cast<float>(MapPos.Y);
				}
				else
				{
					FloorWall.X = MapPos.X + WallX;
					FloorWall.Y = static_cast<float>(MapPos.Y + 1);
				}

				// Store WallDist in 1D-ZBuffer for later calculation of entity distance
				// Needs only to be calculated once
				Game_EntityHandling::ZBuffer[x] = WallDist;

				if (LineEnd < 0)
				{
					LineEnd = ScreenTexture.Height;
				}

				const std::int_fast32_t TotalHeight{ ScreenTexture.Height + std::abs(VerticalLook) };
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
							if (y < ScreenTexture.Height)
							{
								const std::int_fast32_t FloorTexel{ Game_LevelHandling::LevelTextures[Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Floor)][static_cast<std::int_fast32_t>(Floor.X)][static_cast<std::int_fast32_t>(Floor.Y)] - 1].Pixels[(static_cast<std::int_fast32_t>(Floor.Y * TextureSize) & TextureSizeBitwiseAnd) * TextureSize + (static_cast<std::int_fast32_t>(Floor.X * TextureSize) & TextureSizeBitwiseAnd)] };

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

									lwmf::SetPixel(ScreenTexture, x, y, ShadedTexel);
								}
								else
								{
									lwmf::SetPixel(ScreenTexture, x, y, FloorTexel);
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
								const std::int_fast32_t CeilingTexel{ Game_LevelHandling::LevelTextures[LevelCeilingMapPos].Pixels[(static_cast<std::int_fast32_t>(Floor.Y * TextureSize) & TextureSizeBitwiseAnd) * TextureSize + (static_cast<std::int_fast32_t>(Floor.X * TextureSize) & TextureSizeBitwiseAnd)] };

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

									lwmf::SetPixel(ScreenTexture, x, TempY, ShadedTexel);
								}
								else
								{
									lwmf::SetPixel(ScreenTexture, x, TempY, CeilingTexel);
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