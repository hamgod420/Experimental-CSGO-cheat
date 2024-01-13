#include "Menu.h"
#include "TGFCfg.h"
#include "..\SDK\Vector.h"
#include "..\SDK\ISurface.h"
#include "..\Utils\Color.h"
#include "..\Utils\GlobalVars.h"
#include "..\Utils\XorStr.h"

Menu g_Menu;

void Menu::Render()
{
	static bool Pressed = false;

	if (!Pressed && GetAsyncKeyState(VK_INSERT))
		Pressed = true;
	else if (Pressed && !GetAsyncKeyState(VK_INSERT))
	{
		Pressed = false;
		menuOpened = !menuOpened;
	}

	static Vector2D oldPos;
	static Vector2D mousePos;

	static int dragX = 0;
	static int dragY = 0;
	static int Width = 400;
	static int Height = 340;

	static int iScreenWidth, iScreenHeight;

	static std::string Title = XorStr("stackhack v3"); // insert p2c name
	static std::string bottomText = XorStr("copyright (c) stacker");

	static bool Dragging = false;
	bool click = false;

	if (menuOpened)
	{
		if (GetAsyncKeyState(VK_LBUTTON))
			click = true;

		engine->GetScreenSize(iScreenWidth, iScreenHeight);
		Vector2D MousePos = surface->GetMousePosition();

		if (Dragging && !click)
		{
			Dragging = false;
		}

		if (Dragging && click)
		{
			Pos.x = MousePos.x - dragX;
			Pos.y = MousePos.y - dragY;
		}

		if (surface->MouseInRegion(Pos.x, Pos.y, Width, 20))
		{
			Dragging = true;
			dragX = MousePos.x - Pos.x;
			dragY = MousePos.y - Pos.y;
		}

		if (Pos.x < 0)
			Pos.x = 0;
		if (Pos.y < 0)
			Pos.y = 0;
		if ((Pos.x + Width) > iScreenWidth)
			Pos.x = iScreenWidth - Width;
		if ((Pos.y + Height) > iScreenHeight)
			Pos.y = iScreenHeight - Height;

		surface->RoundedFilledRect(Pos.x, Pos.y, Width, Height, 5, Color(25, 25, 25, 225));
		surface->RoundedFilledRect(Pos.x, Pos.y + 20, Width, Height - 40, 3, Color(20, 20, 20, 255));

		GroupTabPos[0] = Pos.x + 85;
		GroupTabPos[1] = Pos.y + 25;
		GroupTabPos[2] = Width - 91;
		GroupTabPos[3] = Height - 50;

		ControlsX = GroupTabPos[0];
		GroupTabBottom = GroupTabPos[1] + GroupTabPos[3];

		surface->DrawT(Pos.x + 6, Pos.y + 2, Color(255, 255, 255, 255), g::CourierNew, false, Title.c_str()); // title
		surface->DrawT(Pos.x + 6, Pos.y + (Height - 18), Color(255, 255, 255, 255), g::CourierNew, false, bottomText.c_str()); // bottom text

		OffsetY = GroupTabPos[1] + 7;

		static bool CfgInitLoad = true;
		static bool CfgInitSave = true;

		static int SaveTab = 0;

		if (surface->MouseInRegion(Pos.x, Pos.y + 295, 80, 26))
		{
			if (CfgInitLoad && click)
			{
				SaveTab = 0;

				g_Config->Load();
				CfgInitLoad = false;
			}
		}
		else
			CfgInitLoad = true;

		if (surface->MouseInRegion(Pos.x, Pos.y + 270, 80, 26))
		{
			if (CfgInitSave && click)
			{
				SaveTab = 1;

				g_Config->Save();
				CfgInitSave = false;
			}
		}
		else
			CfgInitSave = true;

		surface->RoundedFilledRect(Pos.x, Pos.y + 270, 70, 50, 3, Color(119, 119, 119, 255));

		if (SaveTab == 0)
		{
			surface->RoundedFilledRect(Pos.x, Pos.y + 295, 80, 25, 3, Color(120, 209, 109, 255));
			surface->RoundedFilledRect(Pos.x, Pos.y + 295, 70, 25, 3, Color(53, 53, 53, 255));
		}
		else
		{
			surface->RoundedFilledRect(Pos.x, Pos.y + 270, 80, 25, 3, Color(120, 209, 109, 255));
			surface->RoundedFilledRect(Pos.x, Pos.y + 270, 70, 25, 3, Color(53, 53, 53, 255));
		}

		surface->DrawT(Pos.x + 6, Pos.y + 270 + 2, Color(255, 255, 255, 255), g::CourierNew, false, "save");
		surface->DrawT(Pos.x + 6, Pos.y + 295 + 2, Color(255, 255, 255, 255), g::CourierNew, false, "load");

		TabOffset = 0;
		SubTabOffset = 0;
		PreviousControl = -1;
		OldOffsetY = 0;

		Tab("aim");
		{
			SubTab("main");
			{
				std::string aimbotname = "active";
				if (Config.LegitBacktrack)
					aimbotname += " (turn off legitbacktrack)";
				CheckBox(aimbotname, &Config.Aimbot);
				Slider(100, "mindamage", &Config.Mindmg);
				CheckBox("autostop", &Config.Autostop);
				CheckBox("fix shot pitch", &Config.FixShotPitch);
				ComboBox("hitchance", { "off", "extra", "normal" }, &Config.Hitchance);
				if (Config.Hitchance != 0)
					Slider(100, "value", &Config.HitchanceValue);
				CheckBox("resolver", &Config.Resolver);
			}

			SubTab("position");
			{
				CheckBox("multipoint", &Config.MultiPoint);
				if (Config.MultiPoint)
				{
					Slider(100, "head scale", &Config.HeadScale);
					Slider(100, "body scale", &Config.BodyScale);
				}
				CheckBox("delay shot", &Config.DelayShot);
				CheckBox("ignore limbs on-move", &Config.IgnoreLimbs);
			}

			SubTab("target");
			{
				CheckBox("shot backtrack", &Config.ShotBacktrack);
				CheckBox("position backtrack", &Config.PosBacktrack);
				CheckBox("baim if lethal", &Config.BaimLethal);
				CheckBox("baim bad pitch", &Config.BaimPitch);
				CheckBox("baim in air", &Config.BaimInAir);
			}
		}
		
		Tab("visual");
		{
			SubTab("esp");
			{
				CheckBox("active", &Config.Esp);
				ComboBox("font", { "courier", "tahoma" }, &Config.Font);
				ColorPicker("font color", Config.FontColor);
				CheckBox("draw name", &Config.Name);
				CheckBox("draw weapon", &Config.Weapon);
				CheckBox("2d box", &Config.Box);
				ColorPicker("box color", Config.BoxColor);
				CheckBox("health bar", &Config.HealthBar);
				MultiComboBox("draw skeleton", { "normal", "backtrack" }, Config.Skeleton);
				ColorPicker("skele color", Config.SkeletonColor);
			}

			SubTab("render");
			{
				CheckBox("hitbox points", &Config.HitboxPoints);
				CheckBox("chams", &Config.Chams);
				ColorPicker("chams color", Config.ChamsColor);
				CheckBox("no zoom", &Config.NoZoom);
				CheckBox("no scope", &Config.NoScope);
				CheckBox("no recoil", &Config.NoRecoil);
				Slider(150, "fov", &Config.Fov);
				CheckBox("crosshair", &Config.Crosshair);
			}
		}
			
		Tab("antiaim");
		{
			std::string antiaimname = "hvh features";
			CheckBox(antiaimname, &Config.Antiaim);
			CheckBox("at target", &Config.attarget);
			CheckBox("random jitter in-range", &Config.RandJitterInRange);
			Slider(360, "jitter range", &Config.JitterRange);
			Slider(14, "fakelag", &Config.Fakelag);
			CheckBox("flag on peek", &Config.FakeLagOnPeek);
			if (Config.FakeLagOnPeek)
				CheckBox("choke shot", &Config.ChokeShotOnPeek);
		}

		Tab("misc");
		{
			CheckBox("bhop", &Config.Bhop);
			CheckBox("autostrafe", &Config.AutoStrafe);
		}

		TabSize = TabOffset;
		SubTabSize = SubTabOffset;
	}
}

void Menu::Tab(std::string name)
{
	int TabArea[4] = { Pos.x, Pos.y + 20 + (TabOffset * 25), 80, 26 };

	if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(TabArea[0], TabArea[1], TabArea[2], TabArea[3]))
		TabNum = TabOffset;

	if (TabOffset == 0)
		surface->RoundedFilledRect(TabArea[0], TabArea[1], 70, (TabSize * 25) , 3, Color(20, 20, 20, 255));

	if (TabOffset == TabNum)
	{
		surface->RoundedFilledRect(TabArea[0], TabArea[1], 70, TabArea[3], 3, Color(53, 53, 53, 255));
	}

	surface->DrawT(TabArea[0] + 6, TabArea[1] + 2, Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());

	TabOffset += 1;
	PreviousControl = -1;
}

void Menu::SubTab(std::string name)
{
	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	RECT TextSize = surface->GetTextSizeRect(g::CourierNew, name.c_str());

	static int TabSkip = 0;

	if (SubTabOffset == 0)
		surface->RoundedFilledRect(GroupTabPos[0], GroupTabPos[1], GroupTabPos[2], 21, 3, Color(20, 20, 20, 255));

	if (SubTabSize != 0 && TabSkip == TabNum)
	{
		if (TabNum >= SubTabSize)
			TabNum = 0;

		int TabLength = (GroupTabPos[2] / SubTabSize);

		int GroupTabArea[4] = { (GroupTabPos[0]) + (TabLength * SubTabOffset), GroupTabPos[1], TabLength, 21 };

		if ((GroupTabArea[0] + GroupTabArea[3]) <= (GroupTabPos[0] + GroupTabPos[2]))
		{
			int TextPosition[2] = { GroupTabArea[0] + (TabLength / 2) - (TextSize.right / 2), (GroupTabArea[1] + 10) - (TextSize.bottom / 2) };

			if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(GroupTabArea[0], GroupTabArea[1], GroupTabArea[2], GroupTabArea[3]))
				SubTabNum = SubTabOffset;

			int Offset = ((SubTabSize - 1) == SubTabOffset) ? 0 : 1;

			if (((SubTabSize - 1) == SubTabOffset) && (((TabLength * SubTabSize) > GroupTabPos[2]) || ((TabLength * SubTabSize) < GroupTabPos[2])))
				Offset = (GroupTabPos[2] - (TabLength * SubTabSize));

			if (SubTabNum == SubTabOffset)
				surface->RoundedFilledRect(GroupTabArea[0], GroupTabArea[1], GroupTabArea[2] + Offset, GroupTabArea[3], 3, Color(53, 53, 53, 255));

			surface->DrawT(TextPosition[0], TextPosition[1], Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());
		}
	}

	if (TabSkip != TabNum) // frame skip for drawing
		TabSkip = TabNum;

	if (SubTabOffset == SubTabNum)
		OffsetY += 20;

	SubTabOffset += 1;
	PreviousControl = -1;
}

void Menu::CheckBox(std::string name, bool* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool pressed = false;

	if (!GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 277, OffsetY, 16, 16))
	{
		if (pressed)
			*item = !*item;
		pressed = false;
	}

	if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 277, OffsetY, 16, 16) && !pressed)
		pressed = true;

	if (*item == true)
		surface->RoundedFilledRect(ControlsX + 277, OffsetY, 16, 16, 3, Color(133, 177, 255, 255));
	else
		surface->RoundedFilledRect(ControlsX + 277, OffsetY, 16, 16, 3, Color(45, 45, 45, 255));
		surface->OutlinedRect(ControlsX + 277, OffsetY, 16, 16, Color(65, 65, 65, 200));

	surface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());

	OldOffsetY = OffsetY;
	OffsetY += 26;
	PreviousControl = check_box;
}

void Menu::Slider(int max, std::string name, int* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	float pixelValue = max / 114.f;

	if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY + 5, 120, 8))
		*item = (surface->GetMousePosition().x - (ControlsX + 155)) * pixelValue;

	if (*item > max)
		*item = max;
	if (*item < 0)
		*item = 0;

	surface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());
	surface->RoundedFilledRect(ControlsX + 153, OffsetY + 5, 120, 10, 3, Color(45, 45, 45, 255));
	surface->RoundedFilledRect(ControlsX + 153, OffsetY + 5, 6 + (*item / pixelValue), 9, 3,Color(133, 177, 255, 255));

	surface->DrawT(ControlsX + 292, OffsetY + 1, Color(255, 255, 255, 255), g::CourierNew, true, std::to_string(*item).c_str());

	OldOffsetY = OffsetY;
	OffsetY += 26;
	PreviousControl = slider;
}

void Menu::ComboBox(std::string name, std::vector< std::string > itemname, int* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;

	if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	surface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());
	surface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 16, 3, Color(45, 45, 45, 255));
	surface->OutlinedRect(ControlsX + 153, OffsetY, 140, 16, Color(65, 65, 65, 200));

	if (open)
	{
		surface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 17 + (itemname.size() * 16), 3, Color(45, 45, 45, 255));
		surface->OutlinedRect(ControlsX + 153, OffsetY, 140, 17 + (itemname.size() * 16), Color(65, 65, 65, 200));
		for (int i = 0; i < itemname.size(); i++)
		{
			if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY + 16 + i * 16, 140, 16))
				*item = i;

			if (*item == i)
				surface->RoundedFilledRect(ControlsX + 154, OffsetY + 16 + (i * 16), 138, 16, 3, Color(53, 53, 53, 255));

			surface->DrawT(ControlsX + 159, OffsetY + 16 + (i * 16), Color(255, 255, 255, 255), g::CourierNew, false, itemname.at(i).c_str());
		}
	}

	surface->DrawT(ControlsX + 159, OffsetY, Color(255, 255, 255, 255), g::CourierNew, false, itemname.at(*item).c_str());

	OldOffsetY = OffsetY;

	if (open)
		OffsetY += 26 + (itemname.size() * 16);
	else
		OffsetY += 26;

	PreviousControl = combo_box;
}

void Menu::MultiComboBox(std::string name, std::vector< std::string > itemname, bool* item)
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	static bool multiPressed = false;
	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;
	std::string itemsSelected = "";
	int lastItem = 0;

	if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY, 140, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;

	surface->DrawT(ControlsX + 6, OffsetY, Color(255, 255, 255, 255), g::CourierNew, false, name.c_str());
	surface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 16, 5, Color(45, 45, 45, 255));
	surface->OutlinedRect(ControlsX + 153, OffsetY, 140, 16, Color(65, 65, 65, 200));
	if (open)
	{
		surface->RoundedFilledRect(ControlsX + 153, OffsetY, 140, 17 + (itemname.size() * 16), 5, Color(45, 45, 45, 255));
		surface->OutlinedRect(ControlsX + 153, OffsetY, 140, 17 + (itemname.size() * 16), Color(65, 65, 65, 200));
		for (int i = 0; i < itemname.size(); i++)
		{
			if (!GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY + 16 + (i * 16), 140, 16))
			{
				if (multiPressed)
					item[i] = !item[i];
				multiPressed = false;
			}

			if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + 153, OffsetY + 16 + (i * 16), 140, 16) && !multiPressed)
				multiPressed = true;

			if (item[i])
				surface->RoundedFilledRect(ControlsX + 154, OffsetY + 16 + (i * 16), 138, 16, 5, Color(120, 209, 109, 255));
			else
				surface->RoundedFilledRect(ControlsX + 154, OffsetY + 16 + (i * 16), 138, 16, 5, Color(192, 97, 108, 255));

			surface->DrawT(ControlsX + 159, OffsetY + 16 + (i * 16), Color(255, 255, 255, 255), g::CourierNew, false, itemname.at(i).c_str());
		}

	}

	bool items = false;

	// man look at all these for loops i sure am retarded

	for (int i = 0; i < itemname.size(); i++)
	{ 
		if (item[i]) 
		{
			if (lastItem < i)
				lastItem = i;
		}
	}

	for (int i = 0; i < itemname.size(); i++)
	{ 
		if (item[i]) 
		{
			items = true;
			RECT TextSize = surface->GetTextSizeRect(g::CourierNew, itemsSelected.c_str());
			RECT TextSizeGonaAdd = surface->GetTextSizeRect(g::CourierNew, itemname.at(i).c_str());
			if (TextSize.right + TextSizeGonaAdd.right < 130)
				itemsSelected += itemname.at(i) + ((lastItem == i) ? "" : ", ");
		}
	}

	if (!items)
		itemsSelected = "off";

	surface->DrawT(ControlsX + 159, OffsetY, Color(255, 255, 255, 255), g::CourierNew, false, itemsSelected.c_str());

	OldOffsetY = OffsetY;

	if (open)
		OffsetY += 26 + (itemname.size() * 16);
	else
		OffsetY += 26;

	PreviousControl = multi_box;
}

void Menu::ColorPicker(std::string name, ColorV2& item) // best coder in the universe
{
	if (GroupTabBottom <= OffsetY + 16)
		return;

	if (TabOffset - 1 != TabNum || TabOffset == 0)
		return;

	if (SubTabOffset != 0)
		if (SubTabOffset - 1 != SubTabNum)
			return;

	if (PreviousControl == slider || PreviousControl == -1)
		return;

	int CtrXoffset = 0;

	if (PreviousControl != check_box)
		CtrXoffset = 132;
	else
		CtrXoffset = 256;

	int yoffset = OldOffsetY + 10;
	int xoffset = ControlsX + 330;

	Color rainbow;

	bool pressed = false;
	bool open = false;
	static bool selectedOpened = false;
	static bool clickRest;
	static bool rest;
	static std::string nameSelected;

	if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + CtrXoffset, OldOffsetY, 16, 16) && !clickRest)
	{
		nameSelected = name;
		pressed = true;
		clickRest = true;
	}
	else if (!GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(ControlsX + CtrXoffset, OldOffsetY, 16, 16))
		clickRest = false;

	if (pressed)
	{
		if (!rest)
			selectedOpened = !selectedOpened;

		rest = true;
	}
	else
		rest = false;

	if (nameSelected == name)
		open = selectedOpened;
	
	if (open)
	{
		surface->RoundedFilledRect(xoffset, OldOffsetY, 100, 20, 5, Color(0, 0, 0, 255));
		surface->RoundedFilledRect(xoffset, OldOffsetY + 100, 100, 20, 5, Color(255, 255, 255, 255));

		if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(xoffset, OldOffsetY, 100, 10))
		{
			item.red = 0;
			item.green = 0;
			item.blue = 0;
			item.alpha = 255;
		}

		if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(xoffset, OldOffsetY + 110, 100, 10))
		{
			item.red = 255;
			item.green = 255;
			item.blue = 255;
			item.alpha = 255;
		}

		for (int i = 0; i < 100; i++)
		{
			if (xoffset >= ControlsX + 430)
			{
				xoffset -= 100;
				yoffset += 10;
			}

			float hue = (i * .01f);

			rainbow.FromHSV(hue, 1.f, 1.f);

			surface->FilledRect(xoffset , yoffset, 10, 10, rainbow);

			if (GetAsyncKeyState(VK_LBUTTON) && surface->MouseInRegion(xoffset, yoffset, 10, 10))
			{
				item.red = rainbow.red;
				item.green = rainbow.green;
				item.blue = rainbow.blue;
				item.alpha = 255.f;
			}			

			xoffset += 10;
		}
	}

	rainbow.red = item.red;
	rainbow.green = item.green;
	rainbow.blue = item.blue;
	rainbow.alpha = 255;

	surface->RoundedFilledRect(ControlsX + CtrXoffset, OldOffsetY, 16, 16, 5, rainbow);
}