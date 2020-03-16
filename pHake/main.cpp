#include "Gui/pGui.hpp"
#include <iostream>
#include <Windows.h>
#include "pHake.h"
#include "pTimer.hpp"

pGui* menu;
pHake* game;

struct vector2
{
	float x, y;
};

struct settings
{
	bool godmode = false;
	bool neverwanted = false;
	bool rploop = false;
	bool trigger = false;
	bool weaponmax = false;
	bool fly = false;

	float flySpeed = 5.f;
	float kmh = 0.f;

	std::string boostPlayer = "default";
	std::string boostVehicle = "default";
}settings;


void lMouseDown()
{
	INPUT    Input = { 0 };

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	::SendInput(1, &Input, sizeof(INPUT));
}

void lMouseUp()
{
	INPUT    Input = { 0 };

	Input.type = INPUT_MOUSE;
	Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	::SendInput(1, &Input, sizeof(INPUT));
}

void suicide()
{
	game->player->write<float>(0x280, 0.f);
	menu->notification.add("Player health set to 0");
}

float getYaw()
{
	float angle = 0;
	float x = game->mem.read<float>(game->_base + 0x26242B8);
	float y = game->mem.read<float>(game->_base + 0x26242E8);

	if (x < 0 && y > 0)
	{
		angle = ((((x * -1) * 100) * 0.9) - 90) * -1;
	}

	if (x > 0 && y > 0)
	{
		angle = ((((x * -1) * 100) * 0.9) - 90) * -1;
	}

	if (x > 0 && y < 0)
	{
		angle = ((((x * -1) * 100) * 0.9) + 270);
	}

	if (x < 0 && y < 0)
	{
		angle = ((((x * -1) * 100) * 0.9) + 270);
	}
	return angle;
}

float getPitch()
{
	float angle = 0;
	float x = game->mem.read<float>(game->_base + 0x2915F28);

	angle = (((x * -1) * 100) + 90);
	return x;
}

void freezeWorld(bool value)
{
	if (value)
	{
		BYTE freezeOn[4] = { 0x90, 0x90, 0x90, 0x90 };
		WriteProcessMemory(game->mem.hProcess, (void*)(game->_base + 0x1429EC3), &freezeOn, sizeof(freezeOn), NULL);
		game->player->write<unsigned char>(0x10A8, 1); // disable ragdoll
	}
	else
	{
		BYTE freezeOff[4] = { 0x0F, 0x29, 0x48, 0x50 }; // check if position is already restored
		game->player->write<unsigned char>(0x10A8, 32); // enable ragdoll
		WriteProcessMemory(game->mem.hProcess, (void*)(game->_base + 0x1429EC3), &freezeOff, sizeof(freezeOff), NULL);
	}
}

bool isWorldFrozen()
{
	BYTE isOn[4];
	ReadProcessMemory(game->mem.hProcess, (void*)(game->_base + 0x1429EC3), &isOn, sizeof(isOn), NULL);

	if (isOn[0] == 0x90) // check if position is already freezed
		return true;
	else
		return false;
}

void tpToWaypoint()
{
	vector2 waypoint = game->mem.read<vector2>(game->_base + 0x1F5EA30);

	if (waypoint.x != 64000 && waypoint.y != 64000)
	{
		if (game->player->read<int32_t>(0xe44) == 0)
		{
			game->playerPos->write<float>(0x50, waypoint.x);
			game->playerPos->write<float>(0x54, waypoint.y);
			game->playerPos->write<float>(0x58, -200.f);
		}
		else
		{
			game->playerVehiclePos->write<float>(0x50, waypoint.x);
			game->playerVehiclePos->write<float>(0x54, waypoint.y);
			game->playerVehiclePos->write<float>(0x58, -200.f);
		}
		menu->notification.add("Teleported to Waypoint");
	}
	else
	{
		menu->notification.add("Please set a Waypoint");
	}
}

void boostPlayer()
{
	if (settings.boostPlayer == "default")
	{
		settings.boostPlayer = "fast";
		game->playerInfo->write<float>(0x14C, 2.5);
		game->playerInfo->write<float>(0x148, 2.5);
		game->player->write<unsigned char>(0x10A8, 1);

		menu->notification.add("Player mode set to " + settings.boostPlayer);
		return;
	}

	if (settings.boostPlayer == "fast")
	{
		settings.boostPlayer = "max";
		game->playerInfo->write<float>(0x14C, 2500);
		game->playerInfo->write<float>(0x148, 2500);
		game->player->write<unsigned char>(0x10A8, 1);

		menu->notification.add("Player mode set to " + settings.boostPlayer);
		return;
	}

	if (settings.boostPlayer == "max")
	{
		settings.boostPlayer = "default";
		game->playerInfo->write<float>(0x14C, 1);
		game->playerInfo->write<float>(0x148, 1);
		game->player->write<unsigned char>(0x10A8, 32);

		menu->notification.add("Player mode set to " + settings.boostPlayer);
		return;
	}
}

void boostVehicle()
{
	if (settings.boostVehicle == "default")
	{
		settings.boostVehicle = "race";

		game->playerVehicle->write<float>(0xC3C, 20.f);
		game->playerVehicleHandling->write<float>(0x88, 3.f);
		game->playerVehicleHandling->write<float>(0x90, 3.f);
		game->playerVehicleHandling->write<float>(0x4C, 3.f);
		game->playerVehicleHandling->write<float>(0xF8, 0.f);

		menu->notification.add("Vehicle mode set to " + settings.boostVehicle);
		return;
	}
	else if (settings.boostVehicle == "race")
	{
		settings.boostVehicle = "max";

		game->playerVehicle->write<float>(0xC3C, 25.f);
		game->playerVehicleHandling->write<float>(0x88, 5.f);
		game->playerVehicleHandling->write<float>(0x90, 5.f);
		game->playerVehicleHandling->write<float>(0x4C, 20.f);
		game->playerVehicleHandling->write<float>(0xF8, 0.f);

		menu->notification.add("Vehicle mode set to " + settings.boostVehicle);
		return;
	}
	else if (settings.boostVehicle == "max")
	{
		settings.boostVehicle = "fly";

		game->playerVehicle->write<float>(0xC3C, -10.f);
		game->playerVehicleHandling->write<float>(0x88, 2.f);
		game->playerVehicleHandling->write<float>(0x90, 2.f);
		game->playerVehicleHandling->write<float>(0x4C, 2.f);
		game->playerVehicleHandling->write<float>(0xF8, 0.f);

		menu->notification.add("Vehicle mode set to " + settings.boostVehicle);
		return;
	}
	else if (settings.boostVehicle == "fly")
	{
		settings.boostVehicle = "default";

		game->playerVehicle->write<float>(0xC3C, 9.8);
		game->playerVehicleHandling->write<float>(0x88, 2.f);
		game->playerVehicleHandling->write<float>(0x90, 2.f);
		game->playerVehicleHandling->write<float>(0x4C, 1.f);
		game->playerVehicleHandling->write<float>(0xF8, 0.f);

		menu->notification.add("Vehicle mode set to " + settings.boostVehicle);
		return;
	}
}

void lGodmode()
{
	if (settings.godmode)
	{
		if (game->player->read<bool>(0x189) != 1 || game->playerVehicle->read<bool>(0x189) != 1)
		{
			game->player->write<bool>(0x189, 1);
			game->playerVehicle->write<bool>(0x189, 1);
		}
	}
	else
	{
		if (game->player->read<bool>(0x189) == 1 || game->playerVehicle->read<bool>(0x189) == 1)
		{
			game->player->write<bool>(0x189, 0);
			game->playerVehicle->write<bool>(0x189, 0);
		}
	}
}

void lNeverWanted()
{
	if (settings.neverwanted)
	{
		if (game->playerInfo->read<uint16_t>(0x848) > 0)
		{
			game->playerInfo->write<uint16_t>(0x848, 0);
		}
	}
}

void lRpLoop()
{
	if (settings.rploop)
	{
		for (int i = 0; i <= 5; i++)
		{
			game->playerInfo->write<uint16_t>(0x848, i);
		}
	}
}

void lTrigger()
{
	if (settings.trigger)
	{
		if (game->mem.read<uint16_t>(game->_base + 0x1F47430) != 0 && game->mem.read<uint16_t>(game->_base + 0x1F47430) < 3)
		{
			lMouseDown();
			Sleep(1);
			lMouseUp();
		}
	}
}

void lWeaponMax()
{
	if (settings.weaponmax)
	{
		if (game->playerWeaponinfo->read<float>(0xB0) != 99999.f ||  game->playerWeaponinfo->read<float>(0x28C) != 99999.f)
		{
			game->playerWeaponinfo->write<float>(0xB0, 99999.f);
			game->playerWeaponinfo->write<float>(0x12C, 9999.f);
			game->playerWeaponinfo->write<float>(0x28C, 99999.f);
			game->playerWeaponinfoAmmoinfo->write<float>(0x18, 99999.f);
		}
	}
	else
	{
		if (game->playerWeaponinfo->read<float>(0xB0) == 99999.f || game->playerWeaponinfo->read<float>(0x12C) == 99999.f)
		{
			game->playerWeaponinfo->write<float>(0xB0, 100.f);
			game->playerWeaponinfo->write<float>(0x12C, 1.f);
		}
	}
}

void lFly()
{
	if (settings.fly)
	{
		if (HIBYTE(GetAsyncKeyState(0x57)))
		{
			float yaw = getYaw() + 90;
			float pitch = getPitch() + 90;

			if (!isWorldFrozen())
			{
				game->player->write<unsigned char>(0x10A8, 1);
				freezeWorld(true);
			}

			float newX = game->playerPos->read<float>(0x50) + ((settings.flySpeed * cos(yaw * 3.14 / 180)) * -1);
			float newY = game->playerPos->read<float>(0x54) + (settings.flySpeed * sin(yaw * 3.14 / 180));
			float newZ = game->playerPos->read<float>(0x58) + (((settings.flySpeed * 40.f)* cos(pitch * 3.14 / 180)) * -1);

			game->playerPos->write<float>(0x50, newX);
			game->playerPos->write<float>(0x54, newY);
			game->playerPos->write<float>(0x58, newZ);

			if (game->player->read<int32_t>(0xe44) == 0)
			{
				game->playerPos->write<float>(0x50, newX);
				game->playerPos->write<float>(0x54, newY);
				game->playerPos->write<float>(0x58, newZ);
			}
			else
			{
				game->playerVehiclePos->write<float>(0x50, newX);
				game->playerVehiclePos->write<float>(0x54, newY);
				game->playerVehiclePos->write<float>(0x58, newZ);
			}
		}
	}
	else
	{
		if (isWorldFrozen())
		{
			game->player->write<unsigned char>(0x10A8, 32);
			freezeWorld(false);
		}
	}
}

void mainLoop()
{
	pTimer* timer = new pTimer;
	timer->add(lGodmode, 1000);
	timer->add(lWeaponMax, 750);
	timer->add(lNeverWanted, 250);
	timer->add(lRpLoop, 0);
	timer->add(lTrigger, 0);
	timer->add(lFly, 10);

	while (true)
	{
		game->update();
		timer->update();
		settings.kmh = 3.6 * game->mem.read<float>(game->_base + 0x2576BC0);

		if (HIBYTE(GetAsyncKeyState(VK_MENU)))
		{
			menu->toggle();
			Sleep(150);
		}
		if (HIBYTE(GetAsyncKeyState(VK_NUMPAD0)))
		{
			tpToWaypoint();
			Sleep(150);
		}
		if (HIBYTE(GetAsyncKeyState(VK_NUMPAD1)))
		{
			boostPlayer();
			Sleep(150);
		}
		if (HIBYTE(GetAsyncKeyState(VK_NUMPAD2)))
		{
			boostVehicle();
			Sleep(150);
		}
		Sleep(1);
	}
}

void exitProgram()
{
	game->mem.closeHandle();
	//delete game;
	//delete menu;
	exit(-1);
}

int main()
{
	FreeConsole();

	menu = new pGui;
	game = new pHake;

	std::thread t = std::thread(mainLoop);

	menu->create("Grand Theft Auto V");
	menu->entries.addBool("Godmode", settings.godmode);
	menu->entries.addBool("NeverWanted", settings.neverwanted);
	menu->entries.addBool("Trigger", settings.trigger);
	menu->entries.addBool("RpLoop", settings.rploop);
	menu->entries.addBool("Weaponmax", settings.weaponmax);
	menu->entries.addBool("Fly", settings.fly);
	menu->entries.addFloat("Flyspeed", settings.flySpeed, 0.5, 0.5);
	menu->entries.addFloat("Km/h", settings.kmh, 0, 0);
	menu->entries.addFunction("Boost Player", boostPlayer);
	menu->entries.addFunction("Boost Vehicle", boostVehicle);
	menu->entries.addFunction("Tp to Waypoint", tpToWaypoint);
	menu->entries.addFunction("Suicide", suicide);
	menu->entries.addFunction("Exit", exitProgram);
	menu->Loop();

	return 0;
}