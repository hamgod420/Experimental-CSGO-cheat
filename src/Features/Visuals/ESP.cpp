#include "ESP.h"
#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\SDK\ISurface.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"

visuals esp;
namespace csgo_esp_helpers {
	/* set to string */
	std::string string_upper(std::string strToConvert) {
		std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);
		return strToConvert;
	}

	/* fix networked name */
	std::string fix_networked_name(std::string name) {
		std::string cname = name;

		if (cname[0] == 'C')
			cname.erase(cname.begin());

		auto start_of_weap = cname.find("Weapon");
		if (start_of_weap != std::string::npos)
			cname.erase(cname.begin() + start_of_weap, cname.begin() + start_of_weap + 6);

		return cname;
	}
}
void visuals::setup_visuals(C_BaseEntity* entity, float alpha, matrix3x4_t* bone_matrix) {
	/* variables */
	Vector min, max;
	entity->GetRenderBounds(min, max);
	Vector pos, pos_3d, top, top_3d;
	pos_3d = entity->GetAbsOrigin() - Vector(0, 0, 10);
	top_3d = pos_3d + Vector(0, 0, max.z + 15);

	/* screen size */
	int screen_x, screen_y;
	engine->GetScreenSize(screen_x, screen_y);

	/* world to screen */
	if (Utils::world_to_screen(pos_3d, pos) && Utils::world_to_screen(top_3d, top)) {
		/* variables */
		int height = (pos.y - top.y);
		int width = height / 2;

		/* name esp */
		if (g_Menu.Config.Name) {
			PlayerInfo_t ent_info;
			if (!engine->GetPlayerInfo(entity->EntIndex(), &ent_info)) return;

			std::string name = ent_info.szName;
			surface->DrawT(pos.x, top.y - 14, Color(255, 255, 255, alpha), g::name_font, true, name.c_str());
		}

		/* box esp */
		if (g_Menu.Config.Box) {
			surface->OutlinedRect(pos.x - width / 2, top.y, width, height, Color(255, 255, 255, alpha));
			surface->OutlinedRect(pos.x - width / 2 + 1, top.y + 1, width - 2, height - 2, Color(20, 20, 20, alpha));
			surface->OutlinedRect(pos.x - width / 2 - 1, top.y - 1, width + 2, height + 2, Color(20, 20, 20, alpha));
		}

		/* healthbar */
		if (g_Menu.Config.HealthBar) {
			/* color system */
			int enemy_hp = entity->GetHealth();
			int hp_red = 255 - (enemy_hp * 2.55);
			int hp_green = enemy_hp * 2.55;
			Color health_color = Color(hp_red, hp_green, 1, alpha);

			/* offset */
			float offset = (height / 4.f) + 5;
			int hp = height - ((height * enemy_hp) / 100);

			surface->FilledRect((pos.x - width / 2) - 7, top.y - 1, (pos.x - width / 2) - 3, top.y + height + 1, Color(20, 20, 20, alpha)); //	intense maths

			surface->Line((pos.x - width / 2) - 6, top.y + hp, (pos.x - width / 2) - 6, top.y + height - 1, health_color); //	could have done a rect here,
			surface->Line((pos.x - width / 2) - 5, top.y + hp, (pos.x - width / 2) - 5, top.y + height - 1, health_color); //	but fuck it

			if (enemy_hp < 100)
				surface->DrawT((pos.x - width / 2) - 5, top.y + hp - 2, Color(255, 255, 255, alpha), g::esp_font, true, csgo_esp_helpers::string_upper(std::to_string(enemy_hp)).c_str());

		}

		/* weapon esp */
		if (g_Menu.Config.Weapon) {
			/* get weapon */
			auto weapon = entity->GetActiveWeapon();
			if (!weapon) return;

			/* weapon name */
			std::string weap_name = weapon->GetName();

			/* yoy */
			surface->DrawT(pos.x, pos.y + 1, Color(255, 255, 255, alpha), g::esp_font, true, csgo_esp_helpers::string_upper(weap_name).c_str());
		}
	}
}

/* alpha */
float render_alpha[64];

void visuals::setup() {
	/* checks */
	if (!g::pLocalEntity || !engine->IsInGame() || !g_Menu.Config.Esp)  return;

	/* setup */
	for (int i = 1; i <= global_vars->maxClients; i++) {
		/* checks */
		auto entity = g_pEntityList->GetClientEntity(i);
		if (!entity) continue;
		if (!g::pLocalEntity)  return;
		if (entity == g::pLocalEntity)  continue;
		if (entity->GetTeam() == g::pLocalEntity->GetTeam()) continue;

		if ((entity->IsDormant() || entity->GetHealth() <= 0) && render_alpha[entity->EntIndex()] > 0)
			render_alpha[entity->EntIndex()] -= 5;
		else if (render_alpha[entity->EntIndex()] < 210 && !(entity->IsDormant() || entity->GetHealth() <= 0))
			render_alpha[entity->EntIndex()] += 5;

		if (!entity->IsAlive() || entity->IsDormant() && render_alpha[entity->EntIndex()] <= 0)
			continue;

		setup_visuals(entity, render_alpha[entity->EntIndex()]);
	}
}
