#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>



struct ColorV2
{
	float red, green, blue, alpha;
};

enum 
{
	check_box,
	slider,
	combo_box,
	multi_box
};

class Menu
{
public:
	bool menuOpened;
	void Render();

	struct
	{
		bool Aimbot;
		int Hitchance;
		int HitchanceValue;
		int Mindmg;
		bool Resolver;
		int BodyScale;
		int HeadScale;
		bool MultiPoint;
		bool DelayShot;
		bool IgnoreLimbs;
		bool Autostop;
		bool FixShotPitch;
		bool PosBacktrack;
		bool ShotBacktrack;
		bool BaimLethal;
		bool BaimPitch;
		bool BaimInAir;

		bool Antiaim;
		bool RandJitterInRange;
		int	JitterRange;
		int	Fakelag;
		bool FakeLagOnPeek;
		bool ChokeShotOnPeek;
		bool attarget;

		bool Esp;
		int Font;
		ColorV2 FontColor = { 255.f,255.f,255.f,255.f };
		bool Name;
		int HealthVal;
		bool Weapon;
		bool Box;
		ColorV2 BoxColor = { 255.f,255.f,255.f,255.f };
		bool HealthBar;
		bool Skeleton[2] = {false,false};
		ColorV2 SkeletonColor = { 255.f,255.f,255.f,255.f };
		bool HitboxPoints;
		bool Chams;
		ColorV2 ChamsColor = { 255.f,255.f,255.f,255.f };
		bool NoZoom;
		bool NoScope;
		bool NoRecoil;
		int Fov;
		bool Crosshair;

		bool Bhop;
		bool AutoStrafe;
		bool LegitBacktrack;
		bool Ak47meme;
		int	Test;
	}Config;

private:
	struct
	{
		float x = 0.f, y = 0.f;
	}Pos; // lol

	enum
	{
		check_box,
		slider,
		combo_box,
		multi_box
	};

	int ControlsX;
	int GroupTabBottom;
	int OffsetY;
	int OldOffsetY;
	int TabOffset;
	int SubTabOffset;
	float SubTabSize; // cpp fuckin sux had to make this a float or the whole thing crashes
	float TabSize;
	int GroupTabPos[4];

	int TabNum = 0;
	int SubTabNum = 0;
	int PreviousControl = -1;

	void Tab(std::string name);
	void SubTab(std::string name);
	void CheckBox(std::string name, bool* item);
	void Slider(int max, std::string name, int* item);
	void ComboBox(std::string name, std::vector< std::string > itemname, int* item);
	void MultiComboBox(std::string name, std::vector< std::string > itemname, bool* item);
	void ColorPicker(std::string name, ColorV2& item);
};

extern Menu g_Menu;