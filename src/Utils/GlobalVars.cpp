#include "GlobalVars.h"

namespace g
{
    CUserCmd*      pCmd         = nullptr;
    C_BaseEntity*  pLocalEntity = nullptr;
    std::uintptr_t uRandomSeed  = NULL;
	Vector         OriginalView;
	bool           bSendPacket  = true;
	bool		   LagPeek      = false;
	int            TargetIndex  = -1;
	Vector         EnemyEyeAngs[65];
	Vector         AimbotHitbox[65][28];
	Vector         RealAngle;
	Vector         FakeAngle;
	bool           Shot[65];
	bool           Hit[65];
	Vector LastAimVec;
	int            MissedShots[65];
	int MissedShotsResolve[64];
	std::string ResolverStage[64];
	matrix3x4_t	LastAimMatrix[128];
	DWORD esp_font;
	DWORD CourierNew;
	DWORD Tahoma;
	DWORD name_font;
}
