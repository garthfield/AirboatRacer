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
	char m_szLapInfo[256];
	int m_nPowerup1; // Nitro
	int m_nPowerup2; // Jump
	int m_nPowerup3; // Shockwave
	int m_iPowerupType;

	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HudNumbers");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");

	CPanelAnimationVarAliasType(float, label_speed_x, "label_speed_x", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, label_speed_y, "label_speed_y", "30", "proportional_float");
	CPanelAnimationVarAliasType(float, value_speed_x, "value_speed_x", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, value_speed_y, "value_speed_y", "30", "proportional_float");

	CPanelAnimationVarAliasType(float, label_lap_x, "label_lap_x", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, label_lap_y, "label_lap_y", "30", "proportional_float");
	CPanelAnimationVarAliasType(float, value_lap_x, "value_lap_x", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, value_lap_y, "value_lap_y", "30", "proportional_float");

	CPanelAnimationVarAliasType(float, label_powerup_x, "label_powerup_x", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, label_powerup_y, "label_powerup_y", "10", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_powerup_x, "icon_powerup_x", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_powerup_y, "icon_powerup_y", "10", "proportional_float");
};