#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include <deque>

#define TIME_TO_TICKS( dt )		( (int)( 0.5 + (float)(dt) / global_vars->intervalPerTick ) )
#define TICKS_TO_TIME( t )		( global_vars->intervalPerTick *( t ) )

struct PlayerRecords
{
	matrix3x4_t Matrix[128];
	float Velocity;
	float SimTime;
	bool Shot;
};

class LagComp
{
public:
	int ShotTick[65];
	std::deque<PlayerRecords> PlayerRecord[65] = {  };
	void StoreRecord(C_BaseEntity* pEnt);
	void ClearRecords(int i);
	float LerpTime();
	bool ValidTick(int tick);

	template<class T, class U>
	T clamp(T in, U low, U high);
private:
};
extern LagComp g_LagComp;