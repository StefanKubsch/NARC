/*
******************************************
*                                        *
* Game_MenuClass.hpp                     *
*                                        *
* (c) 2017, 2018, 2019 Stefan Kubsch     *
******************************************
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include "Game_GlobalDefinitions.hpp"
#include "Tools_ErrorHandling.hpp"
#include "GFX_TextClass.hpp"

class Game_MenuClass final
{
public:
	void Init();
	void Show();
	void ExecuteChoice();
	void ItemDown();
	void ItemUp();
	void ItemSelect();
	void LevelUp();

private:
	void DrawItem(std::int_fast32_t ItemNumber, std::int_fast32_t TempPosY);

	struct MenuItemStruct final
	{
		std::vector<std::int_fast32_t> ChildItems;
		std::string Entry;
		std::int_fast32_t Level{};
		std::int_fast32_t ParentItem{};
	};

	std::vector<MenuItemStruct> MenuItems;

	GFX_TextClass Text{};
	GFX_TextClass TextHighlight{};

	lwmf::IntPointStruct Pos{};
	std::int_fast32_t HighLightedItem{};
	std::int_fast32_t SelectedLevel{};
	std::int_fast32_t SelectedItem{};
};

inline void Game_MenuClass::Init()
{
	if (const std::string INIFile{ "./DATA/GameConfig/MenuConfig.ini" }; Tools_ErrorHandling::CheckFileExistence(INIFile, StopOnError))
	{
		Pos = { lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MENU", "MenuPosX"), lwmf::ReadINIValue<std::int_fast32_t>(INIFile, "MENU", "MenuPosY") };
		Text.InitFont("./DATA/GameConfig/MenuConfig.ini", "MENUFONT");
		TextHighlight.InitFont("./DATA/GameConfig/MenuConfig.ini", "MENUFONTHIGHLIGHT");
	}

	if (const std::string MenuDefinitionFileName{ "./DATA/GameConfig/MenuDefinition.txt" }; Tools_ErrorHandling::CheckFileExistence(MenuDefinitionFileName, StopOnError))
	{
		std::ifstream MenuDefinitionFile(MenuDefinitionFileName, std::ios::in);
		std::string Line;

		while (std::getline(MenuDefinitionFile, Line))
		{
			MenuItems.emplace_back();

			std::istringstream InputStream(Line);
			std::string TempString;

			std::getline(InputStream, TempString, ',');
			const std::int_fast32_t ItemNumber{ std::stoi(TempString) };

			std::getline(InputStream, MenuItems[ItemNumber].Entry, ',');

			std::getline(InputStream, TempString, ',');
			MenuItems[ItemNumber].ParentItem = std::stoi(TempString);

			std::getline(InputStream, TempString, ',');
			MenuItems[ItemNumber].Level = std::stoi(TempString);

			if (MenuItems[ItemNumber].Level > 0)
			{
				MenuItems[MenuItems[ItemNumber].ParentItem].ChildItems.emplace_back(ItemNumber);
			}
		}
	}
}

inline void Game_MenuClass::DrawItem(const std::int_fast32_t ItemNumber, const std::int_fast32_t TempPosY)
{
	ItemNumber == HighLightedItem ? TextHighlight.RenderText(MenuItems[ItemNumber].Entry, Pos.X, TempPosY) :
		Text.RenderText(MenuItems[ItemNumber].Entry, Pos.X, TempPosY);
}

inline void Game_MenuClass::Show()
{
	const std::int_fast32_t Length{ static_cast<std::int_fast32_t>(MenuItems.size()) };

	for (std::int_fast32_t TempMenuPosY{ Pos.Y }, ItemNumber{}; ItemNumber < Length; ++ItemNumber)
	{
		if (MenuItems[ItemNumber].Level == SelectedLevel)
		{
			if (MenuItems[ItemNumber].Level == 0)
			{
				DrawItem(ItemNumber, TempMenuPosY);
				TempMenuPosY += Text.GetFontHeight();
			}
			else
			{
				for (const auto& ChildItemNumber : MenuItems[SelectedItem].ChildItems)
				{
					if (ItemNumber == ChildItemNumber)
					{
						DrawItem(ItemNumber, TempMenuPosY);
						TempMenuPosY += Text.GetFontHeight();
						break;
					}
				}
			}
		}
	}
}

inline void Game_MenuClass::ExecuteChoice()
{
	if (MenuItems[HighLightedItem].Entry == "Exit Game")
	{
		QuitGameFlag = true;
		PostQuitMessage(0);
	}
	else if (MenuItems[HighLightedItem].Entry == "Toogle Mouse / GameController")
	{
		GameControllerFlag = !GameControllerFlag;

		while (GamePausedFlag)
		{
			LevelUp();
		}
	}
}

inline void Game_MenuClass::ItemDown()
{
	if (MenuItems[HighLightedItem].Level == 0)
	{
		if (MenuItems[++HighLightedItem].Level != SelectedLevel)
		{
			--HighLightedItem;
		}
	}
	else if (HighLightedItem < MenuItems[SelectedItem].ChildItems.back())
	{
		++HighLightedItem;
	}
}

inline void Game_MenuClass::ItemUp()
{
	if ((MenuItems[HighLightedItem].Level == 0 && HighLightedItem > 0) || HighLightedItem > MenuItems[SelectedItem].ChildItems.front())
	{
		--HighLightedItem;
	}
}

inline void Game_MenuClass::ItemSelect()
{
	if (!MenuItems[HighLightedItem].ChildItems.empty())
	{
		SelectedItem = HighLightedItem;
		HighLightedItem = MenuItems[HighLightedItem].ChildItems.front();
		++SelectedLevel;
	}
	else
	{
		ExecuteChoice();
	}
}

inline void Game_MenuClass::LevelUp()
{
	if (SelectedLevel == 0)
	{
		GamePausedFlag = !GamePausedFlag;
	}
	else if (SelectedLevel == 1)
	{
		SelectedLevel = 0;
		HighLightedItem = SelectedItem;
	}
	else
	{
		SelectedItem = MenuItems[MenuItems[HighLightedItem].ParentItem].ParentItem;
		HighLightedItem = MenuItems[SelectedItem].ChildItems.front();
		--SelectedLevel;
	}
}