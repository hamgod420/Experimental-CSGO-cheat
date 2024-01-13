#pragma once
#include "..\..\Utils\GlobalVars.h"

class visuals
{
public:
	void setup();
private:
	struct
	{
		int left, top, right, bottom;
	}box;

	Color color;
	Color textcolor;
	Color skelecolor;
	DWORD font;
	int offset_y;
	void setup_visuals(C_BaseEntity* entity, float alpha, matrix3x4_t* bone_matrix = nullptr);
};
extern visuals esp;