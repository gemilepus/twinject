#include "stdafx.h"
#include "th10_player.h"
#include "config/th_config.h"
#include "hook/th_di8_hook.h"


void th10_player::onInit()
{
	th_player::onInit();
}

void th10_player::onTick()
{
	th_player::onTick();
}

void th10_player::onBeginTick()
{
	th_player::onBeginTick();
	this->doBulletPoll();
	this->doEnemyPoll();
	this->doPowerupPoll();
	this->doLaserPoll();
}

void th10_player::onAfterTick()
{
	th_player::onAfterTick();
}

void th10_player::draw(IDirect3DDevice9* d3dDev)
{
	th_player::draw(d3dDev);
}

void th10_player::handleInput(const BYTE diKeys[256], const BYTE press[256])
{
	th_player::handleInput(diKeys, press);
}

void th10_player::onEnableChanged(bool enable)
{
	th_player::onEnableChanged(enable);
}

void th10_player::doBulletPoll()
{
	// Code adapted from TH10_Collision_Points by binvec 
	// https://github.com/binvec/TH10_Collision_Points
	// Thanks!

	int base = *(int*)0x004776F0;
	if (!base) return;

	int ebx = base + 0x60;
	for (int i = 0; i < 2000; i++)
	{
		int edi = ebx + 0x400;
		int bp = *(int*)(edi + 0x46) & 0x0000FFFF;
		if (bp)
		{
			int eax = *(int*)0x477810;
			if (eax)
			{
				eax = *(int*)(eax + 0x58);
				if (!(eax & 0x00000400)) {
					float dx = *(float*)(ebx + 0x3C0);
					float dy = *(float*)(ebx + 0x3C4);
					float x = *(float*)(ebx + 0x3B4);
					float y = *(float*)(ebx + 0x3B8);
					float w = *(float*)(ebx + 0x3F0);
					float h = *(float*)(ebx + 0x3F4);

					vec2 sz = vec2(w, h);
					aabb a{
						vec2(x + th_param.GAME_WIDTH / 2, y) - sz / 2,
						vec2(dx,dy),
						sz
					};
					bullet b{ a };
					bullets.push_back(b);
				}
			}
		}
		ebx += 0x7F0;
	}
}

void th10_player::doEnemyPoll()
{
	// Code adapted from TH10_Collision_Points by binvec 
	// https://github.com/binvec/TH10_Collision_Points
	// Thanks!

	int base = *(int*)0x00477704;
	if (!base) return;

	int objBase = *(int*)(base + 0x58);
	if (objBase)
	{
		int objNext;
		do
		{
			int objAddr = *(int*)objBase + 0x103c;
			objNext = *(int*)(objBase + 4);

			unsigned int t = *(int*)(objAddr + 0x1444);
			if (!(t & 0x40)) {
				t = *(int*)(objAddr + 0x1444);
				if (!(t & 0x12)) {
					float x = *(float*)(objAddr + 0x2c);
					float y = *(float*)(objAddr + 0x30);
					float w = *(float*)(objAddr + 0xb8);
					float h = *(float*)(objAddr + 0xbc);
					float dx = *(float*)(objAddr + 0x2c + 0xc);
					float dy = *(float*)(objAddr + 0x30 + 0xc);

					vec2 sz = vec2(w, h);
					aabb a{
						vec2(x + th_param.GAME_WIDTH / 2, y) - sz / 2,
						vec2(dx, dy),
						sz
					};
					enemy e{ a };
					enemies.push_back(e);
				}
			}
			objBase = objNext;
		} while (objNext);
	}
}

void th10_player::doPowerupPoll()
{
	// Code adapted from TH10_Collision_Points by binvec 
	// https://github.com/binvec/TH10_Collision_Points
	// Thanks!

	int base = *(int*)0x00477818;
	if (!base) return;
	int esi = base + 0x14;
	int ebp = esi + 0x3b0;

	for (int i = 0; i < 2000; i++)
	{
		int eax = *(int*)(ebp + 0x2c);
		if (eax == 1)
		{
			float x = *(float*)(ebp - 0x4);
			float y = *(float*)ebp;
			float dx = *(float*)(ebp - 0x4 + 0xc);
			float dy = *(float*)(ebp + 0xc);

			vec2 sz(6, 6);

			aabb a{
				vec2(x + th_param.GAME_WIDTH / 2, y) - sz / 2,
				vec2(dx, dy),
				sz
			};
			powerup p{ a };
			powerups.push_back(p);
		}
		ebp += 0x3f0;
	}
}

void th10_player::doLaserPoll()
{
	// Code adapted from TH10_Collision_Points by binvec 
	// https://github.com/binvec/TH10_Collision_Points
	// Thanks!

	int base = *(int*)0x0047781C;
	if (!base) return;
	int ebx;
	int esi = *(int*)(base + 0x18);
	if (esi)
	{
		do {
			ebx = *(int*)(esi + 0x8);
			float x = *(float*)(esi + 0x24);
			float y = *(float*)(esi + 0x28);
			float arc = *(float*)(esi + 0x3c);
			float h = *(float*)(esi + 0x40);
			float w = *(float*)(esi + 0x44);
			float dx = *(float*)(esi + 0x24 + 0xc);
			float dy = *(float*)(esi + 0x28 + 0xc);

			obb a{
				vec2(x + th_param.GAME_WIDTH / 2, y),
				h, w / 4, arc,
				vec2(dx, dy),
			};
			laser l{ a };
			lasers.push_back(l);
			esi = ebx;
		} while (ebx);
	}
}

player th10_player::getPlayerEntity()
{
	// use static pointer to game object to get player offset
	PBYTE *PlayerPtrAddr = (PBYTE*)gs_ptr.plyr_pos;
	if (*PlayerPtrAddr) {
		PBYTE plyrAddr = *PlayerPtrAddr;

		vec2 sz = vec2(5, 5);
		aabb e {
			vec2(
				*(float*)(plyrAddr + 0x3C0) + th_param.GAME_WIDTH / 2,
				*(float*)(plyrAddr + 0x3C4)) - sz / 2,
			vec2(),
			// TODO assume 5x5 size, can probably adapt th15 code to find dimensions
			vec2(5,5),
		};
		return player{ e };
	}
	return player{ aabb() };
}


