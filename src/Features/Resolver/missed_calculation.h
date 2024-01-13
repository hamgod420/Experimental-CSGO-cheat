#pragma once
#include "..\..\Utils\GlobalVars.h"
#include "..\..\SDK\CGlobalVarsBase.h"
#include "..\..\SDK\IClientMode.h"
#include <deque>

class COBB
{
public:
	Vector vecbbMin;
	Vector vecbbMax;
	matrix3x4_t* boneMatrix;
	int hitgroup;

	COBB(void) {};
	COBB(const Vector& bbMin, const Vector& bbMax, matrix3x4_t* matrix, int ihitgroup) { vecbbMin = bbMin; vecbbMax = bbMax; boneMatrix = matrix; hitgroup = ihitgroup; };
};

class CSphere
{
public:
	Vector m_vecCenter;
	float   m_flRadius = 0.f;
	//float   m_flRadius2 = 0.f; // r^2

	CSphere(void) {};
	CSphere(const Vector& vecCenter, float flRadius, int hitgroup) { m_vecCenter = vecCenter; m_flRadius = flRadius; Hitgroup = hitgroup; };

	int Hitgroup;
	bool intersectsRay(const C_Ray& ray);
	bool intersectsRay(const C_Ray& ray, Vector& vecIntersection);
};

struct ShotInfo
{
	C_BaseEntity* Player;
	float TimeFired;
	float TimeProcessed;
	bool WasProcessed;
	int Hitgroup;
	int Damage;
	std::string ResolverStage;
	Vector ShootPos;
	Vector AimPos;
	matrix3x4_t Matrix[128];

	bool processed_trace;
};

struct HitMarker
{
	Vector AimPos;
	Vector ImpactPos;
	float Time;
	float Alpha;
};

class MissedShots {
public:
	void FSN(ClientFrameStage_t curStage);
	void AddSnapshot(C_BaseEntity* player);
	//void EventCallback(IGameEvent* game_event);
	void ProcessSnapshots();

	Vector InterceptPos;
	std::vector<ShotInfo> ShotRecords;
	std::vector<HitMarker> HitmarkerInfo;
	std::vector<Vector> BulletImpacts;
}; extern MissedShots* g_MissedShots;