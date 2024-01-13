#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\SDK\IClientMode.h"
#include <deque>

struct Info
{
	Info() {}

	bool faking, is_pre_break_valid, pre_break_lby, last_moving_lby_valid, lby_changes, updated_large, left_trace, right_trace, left_damage, right_damage;
	float old_simulated, moving_time, last_delta, last_lby, lby_range, last_moving_simtime, predicted_yaw;
	Vector resolved_angles, dormant_origin, moving_origin;
};

struct LBYInfo
{
	// lby info
	float lby;
	float time;

	// player info
	float velocity;
};

class Resolver
{
public:
	void Apply(ClientFrameStage_t curStage);
private:

	void Resolve(C_BaseEntity* Entity);

	Info player_info[65];
	std::deque<LBYInfo> lby_info[64];

};
extern Resolver g_Resolver;