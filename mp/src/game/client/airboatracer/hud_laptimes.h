#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include "ar_shareddefs.h"

using namespace vgui;

class CHudLaptimes : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudLaptimes, Panel);

public:
	CHudLaptimes(const char *pElementName);
	void Init(void);
	void Clear(void);
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void MsgFunc_LapTime(bf_read &msg);
	void LevelInit(void);
	void LevelShutdown(void);

protected:
	virtual void Paint();
	float m_fLaptimes[MAX_LAPS];

	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HudNumbers");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hTextFontSmall, "TextFontSmall", "DefaultSmall");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");

	CPanelAnimationVarAliasType(float, label_lap_x, "label_lap_x", "10", "proportional_float");
	CPanelAnimationVarAliasType(float, label_lap_y, "label_lap_y", "5", "proportional_float");
	CPanelAnimationVarAliasType(float, label_value_x, "label_value_x", "10", "proportional_float");
	CPanelAnimationVarAliasType(float, label_value_y, "label_value_y", "30", "proportional_float");
};