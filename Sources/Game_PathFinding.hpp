/*
******************************************
*                                        *
* Game_PathFinding.hpp                   *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <vector>
#include <list>
#include <queue>

#include "Game_GlobalDefinitions.hpp"
#include "Game_LevelHandling.hpp"

namespace Game_PathFinding
{


	std::int_fast32_t SetPathFindingPoint(std::int_fast32_t x, std::int_fast32_t y, std::int_fast32_t Width);
	void GenerateFlattenedMap(std::vector<float>& Map, std::int_fast32_t Width, std::int_fast32_t Height);
	bool CalculatePath(std::vector<float>& Map, std::int_fast32_t Width, std::int_fast32_t Height, std::int_fast32_t Start, std::int_fast32_t Target, bool Diagonal, std::list<lwmf::IntPointStruct>& WayPoints);

	//
	// Variables and constants
	//

	struct NodeStruct final
	{
		std::int_fast32_t Index{};
		float Cost{};

		NodeStruct(std::int_fast32_t i, float c) : Index(i), Cost(c) {}
	};

	inline std::vector<float> FlattenedMap;

	//
	// Functions
	//

	inline bool operator < (const NodeStruct& Node1, const NodeStruct& Node2)
	{
		return Node1.Cost > Node2.Cost;
	}

	inline std::int_fast32_t SetPathFindingPoint(const std::int_fast32_t x, const std::int_fast32_t y, const std::int_fast32_t Width)
	{
		return Width * y + x;
	}

	inline void GenerateFlattenedMap(std::vector<float>& Map, const std::int_fast32_t Width, const std::int_fast32_t Height)
	{
		Map.clear();
		Map.shrink_to_fit();
		Map.resize(static_cast<size_t>(Width) * static_cast<size_t>(Height), FLT_MAX);

		for (std::int_fast32_t y{}; y < Height; ++y)
		{
			const std::int_fast32_t TempY{ Width * y };

			for (std::int_fast32_t x{}; x < Width; ++x)
			{
				if (Game_LevelHandling::LevelMap[static_cast<std::int_fast32_t>(Game_LevelHandling::LevelMapLayers::Wall)][x][y] == 0)
				{
					Map[TempY + x] = 1.0F;
				}
			}
		}
	}

	//
	// A* pathfinding algorithm
	//
	// See explanation here:
	//
	// https://www.redblobgames.com/pathfinding/a-star/introduction.html
	// https://www.raywenderlich.com/3016-introduction-to-a-pathfinding
	//

	inline bool CalculatePath(std::vector<float>& Map, const std::int_fast32_t Width, const std::int_fast32_t Height, const std::int_fast32_t Start, const std::int_fast32_t Target, bool Diagonal, std::list<lwmf::IntPointStruct>& WayPoints)
	{
		NodeStruct StartNode(Start, 0.0F);
		NodeStruct TargetNode(Target, 0.0F);

		const std::int_fast32_t MapSize{ Width * Height };
		const std::int_fast32_t TargetCalc1{ Target / Width };
		const std::int_fast32_t TargetCalc2{ Target % Width };
		std::vector<std::int_fast32_t> Paths(MapSize);
		std::vector<std::int_fast32_t> Neighbours(8);
		std::vector<float> Costs(MapSize, FLT_MAX);
		std::priority_queue<NodeStruct> NodesToVisit{};
		bool PathFound{};

		Costs[Start] = 0.0F;
		NodesToVisit.push(StartNode);

		while (!NodesToVisit.empty())
		{
			const NodeStruct Current{ NodesToVisit.top() };

			if (Current.Index == TargetNode.Index)
			{
				PathFound = true;
				break;
			}

			NodesToVisit.pop();

			const std::int_fast32_t Row{ Current.Index / Width };
			const std::int_fast32_t Column{ Current.Index % Width };

			Neighbours[0] = (Diagonal && Row > 0 && Column > 0) ? Current.Index - Width - 1 : -1;
			Neighbours[1] = (Row > 0) ? Current.Index - Width : -1;
			Neighbours[2] = (Diagonal && Row > 0 && Column + 1 < Width) ? Current.Index - Width + 1 : -1;
			Neighbours[3] = (Column > 0) ? Current.Index - 1 : -1;
			Neighbours[4] = (Column + 1 < Width) ? Current.Index + 1 : -1;
			Neighbours[5] = (Diagonal && Row + 1 < Height && Column > 0) ? Current.Index + Width - 1 : -1;
			Neighbours[6] = (Row + 1 < Height) ? Current.Index + Width : -1;
			Neighbours[7] = (Diagonal && Row + 1 < Height && Column + 1 < Width) ? Current.Index + Width + 1 : -1;

			for (std::int_fast32_t i{}; i < 8; ++i)
			{
				if (Neighbours[i] >= 0)
				{
					if (const float NewCost{ Costs[Current.Index] + Map[Neighbours[i]] }; NewCost < Costs[Neighbours[i]])
					{
						const float HeuristicCost{ Diagonal ? lwmf::CalcChebyshevDistance<float>(Neighbours[i] / Width, Neighbours[i] % Width, TargetCalc1, TargetCalc2) :
							lwmf::CalcManhattanDistance<float>(Neighbours[i] / Width, Neighbours[i] % Width, TargetCalc1, TargetCalc2) };

						NodesToVisit.push(NodeStruct(Neighbours[i], NewCost + HeuristicCost));

						Costs[Neighbours[i]] = NewCost;
						Paths[Neighbours[i]] = Current.Index;
					}
				}
			}
		}

		std::int_fast32_t Index{ Target };

		while (Index != Start)
		{
			if (Paths[Index] != 0)
			{
				WayPoints.push_front({ Paths[Index] % Width, Paths[Index] / Width });
			}

			Index = Paths[Index];
		}

		return PathFound;
	}


} // namespace Game_PathFinding