#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudAirboatRacer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudAirboatRacer, Panel);

public:
	CHudAirboatRacer(const char *pElementName);
	void togglePrint();
	void Init(void);
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void MsgFunc_Lap(bf_read &msg);
	virtual void MsgFunc_Powerup(bf_read &msg);

protected:
	virtual void Paint();
	int m_iSpeed;
	int m_iHealth;
	char m_szLapInfo[256];
	int m_nPowerup1; // Nitro
	int m_nPowerup2; // Jump
	int m_nPowerup3; // Shockwave
	int m_iPowerupType;

	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HudNumbers");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");

	CPanelAnimationVarAliasType(float, label_speed_x, "label_speed_x", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, label_speed_y, "label_speed_y", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, value_speed_x, "value_speed_x", "50", "proportional_float");
	CPanelAnimationVarAliasType(float, value_speed_y, "value_speed_y", "32", "proportional_float");

	CPanelAnimationVarAliasType(float, label_lap_x, "label_lap_x", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, label_lap_y, "label_lap_y", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, value_lap_x, "value_lap_x", "50", "proportional_float");
	CPanelAnimationVarAliasType(float, value_lap_y, "value_lap_y", "2", "proportional_float");

	CPanelAnimationVarAliasType(float, label_powerup_x, "label_powerup_x", "110", "proportional_float");
	CPanelAnimationVarAliasType(float, label_powerup_y, "label_powerup_y", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_powerup_x, "icon_powerup_x", "165", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_powerup_y, "icon_powerup_y", "8", "proportional_float");

	CPanelAnimationVarAliasType(float, label_health_x, "label_health_x", "110", "proportional_float");
	CPanelAnimationVarAliasType(float, label_health_y, "label_health_y", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, value_health_x, "value_health_x", "155", "proportional_float");
	CPanelAnimationVarAliasType(float, value_health_y, "value_health_y", "32", "proportional_float");
};