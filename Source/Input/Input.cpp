#include <Input/Input.h>
#include <imgui/imgui.h>
#include <string>
#include <windows.h>
#include <Core/Device.h>

using namespace Osm;
using namespace std;

void joystick_callback(int joy, int event)
{
	if (event == GLFW_CONNECTED)
	{
		Game.Input().AddJoystick(joy);		
	}
	else if (event == GLFW_DISCONNECTED)
	{
		Game.Input().RemoveJoystick(joy);
	}
}

InputManager::InputManager(CGame& engine)
	: Component(engine)
	, _joyState(0)
{
	Init();
	glfwSetJoystickCallback(joystick_callback);
}

void InputManager::Init()
{
	for (int i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_LAST; i++)
	{
		if(glfwJoystickPresent(i))
			AddJoystick(i);			
	}
}

unordered_map<JoystickButtons, int> joyMappingOne =
{
	{ JOYSTICK_BUTTON_A,			GLFW_KEY_G },
	{ JOYSTICK_BUTTON_B,			GLFW_KEY_R },
	{ JOYSTICK_BUTTON_X,			GLFW_KEY_F },
	{ JOYSTICK_BUTTON_Y,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_LB,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_RB,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_BACK,			GLFW_KEY_Z },
	{ JOYSTICK_BUTTON_START,		GLFW_KEY_X },
	{ JOYSTICK_BUTTON_L3,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_R3,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_DPAD_UP,		GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_DPAD_RIGHT,	GLFW_KEY_E },
	{ JOYSTICK_BUTTON_DPAD_DOWN,	GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_DPAD_LEFT,	GLFW_KEY_Q },
};

unordered_map<JoystickButtons, int> joyMappingTwo =
{
	{ JOYSTICK_BUTTON_A,			GLFW_KEY_M },
	{ JOYSTICK_BUTTON_B,			GLFW_KEY_H },
	{ JOYSTICK_BUTTON_X,			GLFW_KEY_N },
	{ JOYSTICK_BUTTON_Y,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_LB,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_RB,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_BACK,			GLFW_KEY_Z	},
	{ JOYSTICK_BUTTON_START,		GLFW_KEY_X },
	{ JOYSTICK_BUTTON_L3,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_R3,			GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_DPAD_UP,		GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_DPAD_RIGHT,	GLFW_KEY_PERIOD },
	{ JOYSTICK_BUTTON_DPAD_DOWN,	GLFW_KEY_0 },
	{ JOYSTICK_BUTTON_DPAD_LEFT,	GLFW_KEY_COMMA },
};

void InputManager::Update()
{	
	if (_joyState.size() == 0)
		return;

	for (auto& i : _joyState)
	{	
		int joy = i.first;
		JoystickState& state = i.second;

		// Virtual
		if (joy >= 256)
		{
			const float f = 0.1f;

			for (int j = 0; j < JOYSTICK_AXIS_COUNT; j++)
				state.LastAxes[j] = state.Axes[j];			

			if (joy % 2 == 0)
			{
				state.Axes[JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL] = (float)(GetKey(GLFW_KEY_RIGHT) - GetKey(GLFW_KEY_LEFT));
				state.Axes[JOYSTICK_AXIS_LEFT_THUMB_VERTICAL] = (float)(GetKey(GLFW_KEY_UP) - GetKey(GLFW_KEY_DOWN));
				state.Axes[JOYSTICK_AXIS_RIGHT_TRIGGER] = (float)(GetKey(GLFW_KEY_J) * 2.0f - 1.0f);
			}
			else
			{
				state.Axes[JOYSTICK_AXIS_LEFT_THUMB_HORIZONTAL] = (float)(GetKey(GLFW_KEY_D) - GetKey(GLFW_KEY_A));
				state.Axes[JOYSTICK_AXIS_LEFT_THUMB_VERTICAL] = (float)(GetKey(GLFW_KEY_W) - GetKey(GLFW_KEY_S));
				state.Axes[JOYSTICK_AXIS_RIGHT_TRIGGER] = (float)(GetKey(GLFW_KEY_T) * 2.0f - 1.0f);
			}

			auto& mapping = joy % 2 != 0 ? joyMappingOne : joyMappingTwo;
			for (int j = 0; j < JOYSTICK_BUTTON_COUNT; j++)
			{
				state.LastButtons[j] = state.Buttons[j];
				state.Buttons[j] = GetKey(mapping[(JoystickButtons)j]);
			}
		}
		else
		{
			int count;
			const float* axes = glfwGetJoystickAxes(joy, &count);

			if (count != state.Axes.size())
				state.Axes.resize(count);
			for (int j = 0; j < count && j < JOYSTICK_AXIS_COUNT; j++)
			{
				state.LastAxes[j] = state.Axes[j];
				state.Axes[j] = axes[j];
			}

			const unsigned char* buttons = glfwGetJoystickButtons(joy, &count);
			if (count != state.Buttons.size())
				state.Buttons.resize(count);
			for (int j = 0; j < count && j < JOYSTICK_BUTTON_COUNT; j++)
			{
				state.LastButtons[j] = state.Buttons[j];
				state.Buttons[j] = buttons[j];
			}
		}
	}
}

bool InputManager::GetJoystickButton(Joystick joystick, JoystickButtons button)
{
	if (_joyState.find(joystick) != _joyState.end())
	{
		return _joyState[joystick].Buttons[button] == 1;
	}
	return false;
}

bool InputManager::GetJoystickButtonPressed(Joystick joystick, JoystickButtons button)
{
	if (_joyState.find(joystick) != _joyState.end())
	{
		return		_joyState[joystick].Buttons[button]
				&&	!_joyState[joystick].LastButtons[button];
	}
	return false;
}

float InputManager::GetJoystickAxis(Joystick joystick, JoystickAxes axis)
{
	if (_joyState.find(joystick) != _joyState.end())
	{
		return _joyState[joystick].Axes[axis];
	}
	return 0.0f;
}

std::vector<Joystick> InputManager::GetActiveJoysticks() const
{
	std::vector<Joystick> active;
	for(auto j : _joyState)
		active.push_back(j.first);
	return active;
}

void InputManager::AddJoystick(int joy)
{
	string name(glfwGetJoystickName(joy));	
	JoystickState state;
	state.Name = name + " " + to_string(joy);
	LOG("Joystick %s connected.", name.c_str());
	_joyState[joy] = state;
}

void InputManager::RemoveJoystick(int joy)
{	
	auto itr = _joyState.find(joy);
	if(itr != _joyState.end())
	{
		JoystickState& state = itr->second;
		LOG("Joystick %s disconnected.", state.Name.c_str());
		_joyState.erase(joy);
	}	
}

#ifdef INSPECTOR
void InputManager::Inspect()
{
	for (auto& i : _joyState)
	{
		auto& j = i.second;
		if (ImGui::CollapsingHeader(j.Name.c_str()))
		{
			int i = 0;
			for (auto& a : j.Axes)
			{
				ImGui::DragFloat((string("Axis ") + to_string(i)).c_str(), &a, 0.01f, -1.0f, 1.0f);
				i++;
			}

			i = 0;
			for (auto& b : j.Buttons)
			{
				ImGui::DragInt((string("Buttons ") + to_string(i)).c_str(), &b, 1, 0, 1);
				i++;
			}
		}
	}

	if(ImGui::Button("Add Virtual Joystick"))
	{
		AddVirtualJoystick();
	}
}

void InputManager::AddVirtualJoystick()
{
	int joy = ++_nextVirtual;

	JoystickState state;
	state.Name = "Virtual Joystick " + to_string(joy);
	LOG("Joystick %s connected.", state.Name.c_str());
	_joyState[joy] = state;
}
#endif

void InputManager::InitProfiles()
{
	/*
	Profile xboxOne;
	xboxOne.Buttons		= 
	{
		0,		// JOYSTICK_BUTTON_A
		1,		// JOYSTICK_BUTTON_B
		2,		// JOYSTICK_BUTTON_X
		3,		// JOYSTICK_BUTTON_Y
		// JOYSTICK_BUTTON_LB
		// JOYSTICK_BUTTON_RB
		// JOYSTICK_BUTTON_BACK
		// JOYSTICK_BUTTON_START
		// JOYSTICK_BUTTON_DPAD_UP
		// JOYSTICK_BUTTON_DPAD_DOWN
		// JOYSTICK_BUTTON_DPAD_LEFT
		// JOYSTICK_BUTTON_DPAD_RIGHT
		// JOYSTICK_BUTTON_R3
		// JOYSTICK_BUTTON_L3
	};
	xboxOne.Axes		= {0, 1,  5, 4};
	*/
}

bool InputManager::GetKeyOnce(char key)
{
	return (glfwGetKey(Game.Device().GetWindow(), key) ?
		(_keyOnce[key] ? false : (_keyOnce[key] = true)) : \
		(_keyOnce[key] = false));
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
bool InputManager::GetKey(int key)
{
	return glfwGetKey(Game.Device().GetWindow(), key) == GLFW_PRESS;
}
