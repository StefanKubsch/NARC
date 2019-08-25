/*
*******************************************************
*                                                     *
* lwmf_gamepad - lightweight media framework          *
*                                                     *
* (C) 2019 - present by Stefan Kubsch                 *
*                                                     *
*******************************************************
*/

#pragma once

#include <Windows.h>
#include <Xinput.h>
#include <algorithm>
#include <string>
#include <map>
#include <cmath>

#pragma comment(lib, "xinput")

namespace lwmf
{


	class Gamepad
	{
	public:
		enum AnalogButtons
		{
			LeftStickLeft,
			LeftStickRight,
			LeftStickUp,
			LeftStickDown,
			RightStickLeft,
			RightStickRight,
			RightStickUp,
			RightStickDown,
			LeftTrigger,
			RightTrigger,
			EndOfButtons
		};

		struct AnalogMapping
		{
			float threshold;
			int key;
		};

	private:
		int cId;
		XINPUT_STATE state;
		HWND targetWindow;
		float deadzoneX;
		float deadzoneY;
		std::map<WORD, int> keyMap;
		std::map<AnalogButtons, AnalogMapping> analogMap;
		std::map<WORD, unsigned int> repeatMs;
		std::map<AnalogButtons, unsigned int> analogRepeatMs;
		std::map<WORD, DWORD> lastPress;
		std::map<AnalogButtons, DWORD> analogLastPress;


		XINPUT_STATE previous;

		float prevLeftStickX;
		float prevLeftStickY;
		float prevRightStickX;
		float prevRightStickY;
		float prevLeftTrigger;
		float prevRightTrigger;

		void sendKeysOnThreshold(AnalogButtons, float, float, float, int);

	public:
		Gamepad() : deadzoneX(0.05f), deadzoneY(0.02f), targetWindow(MainWindow) { SetRepeatIntervalMsAll(0); }
		Gamepad(float dzX, float dzY) : deadzoneX(dzX), deadzoneY(dzY), targetWindow(MainWindow) { SetRepeatIntervalMsAll(0); }
		std::map<WORD, std::string> Buttons;
		void SetDeadzone(float, float);
		void SetWindow(HWND);
		void AddKeyMapping(WORD, int);
		void RemoveKeyMappingByButton(WORD);
		void RemoveKeyMapping(int);
		void AddAnalogKeyMapping(AnalogButtons, float, int);
		void RemoveAnalogKeyMapping(AnalogButtons);
		void ClearMappings();
		void SetRepeatIntervalMsAll(unsigned int);
		void SetRepeatIntervalMs(WORD, unsigned int);
		void SetAnalogRepeatIntervalMs(AnalogButtons, unsigned int);

		float leftStickX;
		float leftStickY;
		float rightStickX;
		float rightStickY;
		float leftTrigger;
		float rightTrigger;
		int  GetPort();
		XINPUT_GAMEPAD* GetState();
		bool CheckConnection();
		bool Refresh();
		bool IsPressed(WORD);

		float RotationXLimit{ 0.01F };
		float Sensitivity{ 0.3F };
		lwmf::IntPointStruct RightStickPos;
	};

	int Gamepad::GetPort()
	{
		return cId + 1;
	}

	XINPUT_GAMEPAD* Gamepad::GetState()
	{
		return &state.Gamepad;
	}

	bool Gamepad::CheckConnection()
	{
		int controllerId = -1;

		// Check each port for a connection until one is found
		for (DWORD i = 0; i < XUSER_MAX_COUNT && controllerId == -1; i++)
		{
			XINPUT_STATE State;
			ZeroMemory(&State, sizeof(XINPUT_STATE));

			if (XInputGetState(i, &State) == ERROR_SUCCESS)
				controllerId = i;
		}

		cId = controllerId;

		return controllerId != -1;
	}

	// Returns false if the controller has been disconnected
	bool Gamepad::Refresh()
	{
		// Try to establish a connection with the controller if none was connected last time
		if (cId == -1)
			CheckConnection();

		// If the controller is connected...
		if (cId != -1)
		{
			// Store previous state
			previous = state;

			prevLeftStickX = leftStickX;
			prevLeftStickY = leftStickY;
			prevRightStickX = rightStickX;
			prevRightStickY = rightStickY;
			prevLeftTrigger = leftTrigger;
			prevRightTrigger = rightTrigger;

			// Check state and check for disconnection
			ZeroMemory(&state, sizeof(XINPUT_STATE));
			if (XInputGetState(cId, &state) != ERROR_SUCCESS)
			{
				cId = -1;
				return false;
			}

			// Calculate deadzone-normalized percentages of movement for the
			// analog sticks and triggers

			float normLX = max(-1, (float)state.Gamepad.sThumbLX / 32767);
			float normLY = max(-1, (float)state.Gamepad.sThumbLY / 32767);

			leftStickX = (abs(normLX) < deadzoneX ? 0 : (fabsf(normLX) - deadzoneX) * (normLX / fabsf(normLX)));
			leftStickY = (abs(normLY) < deadzoneY ? 0 : (fabsf(normLY) - deadzoneY) * (normLY / fabsf(normLY)));

			if (deadzoneX > 0) leftStickX *= 1 / (1 - deadzoneX);
			if (deadzoneY > 0) leftStickY *= 1 / (1 - deadzoneY);

			float normRX = max(-1, (float)state.Gamepad.sThumbRX / 32767);
			float normRY = max(-1, (float)state.Gamepad.sThumbRY / 32767);

			rightStickX = (abs(normRX) < deadzoneX ? 0 : (fabsf(normRX) - deadzoneX) * (normRX / fabsf(normRX)));
			rightStickY = (abs(normRY) < deadzoneY ? 0 : (fabsf(normRY) - deadzoneY) * (normRY / fabsf(normRY)));

			if (deadzoneX > 0) rightStickX *= 1 / (1 - deadzoneX);
			if (deadzoneY > 0) rightStickY *= 1 / (1 - deadzoneY);

			leftTrigger = (float)state.Gamepad.bLeftTrigger / 255;
			rightTrigger = (float)state.Gamepad.bRightTrigger / 255;

			// Dispatch keyboard events if desired
			if (targetWindow != NULL)
			{
				for (auto button : Buttons)
				{
					// If button is pushed
					if ((state.Gamepad.wButtons & button.first) != 0)
					{
						// Get key mapping or use XINPUT_GAMEPAD_* value if no mapping exists
						WORD mapping = (keyMap.find(button.first) != keyMap.end() ?
							static_cast<WORD>(keyMap.find(button.first)->second) : static_cast<WORD>(button.first));

						// Get current time and last WM_KEYDOWN message for repeat interval check
						DWORD now = GetTickCount();
						DWORD last = (lastPress.find(button.first) != lastPress.end() ?
							lastPress.find(button.first)->second : 0);

						// Find desired repeat interval for this button
						unsigned int ms = repeatMs.find(button.first)->second;

						// If first press, or repeat interval passed (and repeat interval != 0)
						if ((now - last >= ms && ms > 0)
							|| last == 0
							|| (ms == 0 && (previous.Gamepad.wButtons & button.first) == 0))
						{
							// Update last press time
							lastPress.erase(button.first);
							lastPress.insert(std::map<WORD, DWORD>::value_type(button.first, now));

							// Send keyboard event
							SendMessage(targetWindow, WM_KEYDOWN, mapping,
								((previous.Gamepad.wButtons & button.first) == 0 ? 0 << 30 : 1 << 30));
						}
					}

					// Checking for button release events, will cause the state
					// packet number to be incremented
					if (previous.dwPacketNumber < state.dwPacketNumber)
					{
						// If the button was pressed but is no longer pressed
						if ((state.Gamepad.wButtons & button.first) == 0
							&& (previous.Gamepad.wButtons & button.first) != 0)
						{
							// Get key mapping if one exists
							WORD mapping = (keyMap.find(button.first) != keyMap.end() ?
								static_cast<WORD>(keyMap.find(button.first)->second) : static_cast<WORD>(button.first));

							// Remove last press time
							lastPress.erase(button.first);

							// Send keyboard event
							SendMessage(targetWindow, WM_KEYUP, mapping, 0);
						}
					}
				}

				// Do keyboard event dispatch processing for analog items
				// (unmapped items won't have events generated for them)
				for (auto item : analogMap)
				{
					WORD mapping = static_cast<WORD>(item.second.key);

					switch (item.first) {
					case AnalogButtons::LeftStickLeft:
						sendKeysOnThreshold(AnalogButtons::LeftStickLeft, leftStickX, prevLeftStickX, -item.second.threshold, mapping);
						break;

					case AnalogButtons::LeftStickRight:
						sendKeysOnThreshold(AnalogButtons::LeftStickRight, leftStickX, prevLeftStickX, item.second.threshold, mapping);
						break;

					case AnalogButtons::LeftStickUp:
						sendKeysOnThreshold(AnalogButtons::LeftStickUp, leftStickY, prevLeftStickY, item.second.threshold, mapping);
						break;

					case AnalogButtons::LeftStickDown:
						sendKeysOnThreshold(AnalogButtons::LeftStickDown, leftStickY, prevLeftStickY, -item.second.threshold, mapping);
						break;

					case AnalogButtons::RightStickLeft:
						sendKeysOnThreshold(AnalogButtons::RightStickLeft, rightStickX, prevRightStickX, -item.second.threshold, mapping);
						break;

					case AnalogButtons::RightStickRight:
						sendKeysOnThreshold(AnalogButtons::RightStickRight, rightStickX, prevRightStickX, item.second.threshold, mapping);
						break;

					case AnalogButtons::RightStickUp:
						sendKeysOnThreshold(AnalogButtons::RightStickUp, rightStickY, prevRightStickY, item.second.threshold, mapping);
						break;

					case AnalogButtons::RightStickDown:
						sendKeysOnThreshold(AnalogButtons::RightStickDown, rightStickY, prevRightStickY, -item.second.threshold, mapping);
						break;

					case AnalogButtons::LeftTrigger:
						sendKeysOnThreshold(AnalogButtons::LeftTrigger, leftTrigger, prevLeftTrigger, item.second.threshold, mapping);
						break;

					case AnalogButtons::RightTrigger:
						sendKeysOnThreshold(AnalogButtons::RightTrigger, rightTrigger, prevRightTrigger, item.second.threshold, mapping);
						break;
					}
				}
			}

			return true;
		}
		return false;
	}

	// Processing of analog item key event dispatch
	void Gamepad::sendKeysOnThreshold(AnalogButtons button, float now, float before, float threshold, int key)
	{
		// Check whether the item is and was passed the threshold or not
		bool isPressed = (now >= threshold && threshold > 0) || (now <= threshold && threshold < 0);
		bool wasPressed = (before >= threshold && threshold > 0) || (before <= threshold && threshold < 0);
		// If currently pressed
		if (isPressed)
		{
			// Repeat interval calculation
			DWORD Now = GetTickCount();
			DWORD last = (analogLastPress.find(button) != analogLastPress.end() ?
				analogLastPress.find(button)->second : 0);

			unsigned int ms = analogRepeatMs.find(button)->second;

			// Send message (uses same logic as digital buttons)
			if ((Now - last >= ms && ms > 0) || last == 0 || (ms == 0 && !wasPressed))
			{
				analogLastPress.erase(button);
				analogLastPress.insert(std::map<AnalogButtons, DWORD>::value_type(button, Now));

				SendMessage(targetWindow, WM_KEYDOWN, key, (wasPressed ? 1 << 30 : 0 << 30));
			}
		}

		// Same logic as digital buttons
		if (previous.dwPacketNumber < state.dwPacketNumber)
			if (!isPressed && wasPressed)
			{
				analogLastPress.erase(button);

				SendMessage(targetWindow, WM_KEYUP, key, 0);
			}
	}

	bool Gamepad::IsPressed(WORD button)
	{
		return (state.Gamepad.wButtons & button) != 0;
	}

	void Gamepad::SetDeadzone(float x, float y)
	{
		deadzoneX = x;
		deadzoneY = y;
	}

	void Gamepad::SetWindow(HWND hwnd)
	{
		targetWindow = hwnd;
	}

	void Gamepad::AddKeyMapping(WORD button, int key)
	{
		keyMap.erase(button);
		keyMap.insert(std::map<WORD, int>::value_type(button, key));
	}

	void Gamepad::RemoveKeyMapping(int key)
	{
		for (auto it = keyMap.begin(); it != keyMap.end(); ++it)
			if (it->second == key)
			{
				keyMap.erase(it);
				break;
			}
	}

	void Gamepad::RemoveKeyMappingByButton(WORD button)
	{
		keyMap.erase(button);
	}

	void Gamepad::AddAnalogKeyMapping(AnalogButtons button, float threshold, int key)
	{
		AnalogMapping a = { threshold, key };

		analogMap.erase(button);
		analogMap.insert(std::make_pair(button, a));
	}

	void Gamepad::RemoveAnalogKeyMapping(AnalogButtons button)
	{
		analogMap.erase(button);
	}

	void Gamepad::ClearMappings()
	{
		keyMap.clear();
		analogMap.clear();
	}

	void Gamepad::SetRepeatIntervalMsAll(unsigned int ms)
	{
		repeatMs.clear();

		for (auto button : Buttons)
			repeatMs.insert(std::map<WORD, unsigned int>::value_type(button.first, ms));

		analogRepeatMs.clear();

		for (int i = 0; i < AnalogButtons::EndOfButtons; i++)
			analogRepeatMs.insert(std::map<AnalogButtons, unsigned int>::value_type((AnalogButtons)i, ms));
	}

	void Gamepad::SetRepeatIntervalMs(WORD button, unsigned int ms)
	{
		repeatMs.erase(button);
		repeatMs.insert(std::map<WORD, unsigned int>::value_type(button, ms));
	}

	void Gamepad::SetAnalogRepeatIntervalMs(AnalogButtons button, unsigned int ms)
	{
		analogRepeatMs.erase(button);
		analogRepeatMs.insert(std::map<AnalogButtons, unsigned int>::value_type(button, ms));
	}


} // namespace lwmf