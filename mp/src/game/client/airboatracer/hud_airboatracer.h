#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudAirboatRacer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudAirboatRacer, Panel);

public:
	CHudAirboatRacer(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);

protected:
	virtual void Paint();
	int m_iSpeed;

	CPanelAnimationVar(vgui::HFont, m_hLabelSpeedFont, "LabelSpeedFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hValueSpeedFont, "ValueSpeedFont", "Default");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");

	CPanelAnimationVarAliasType(float, label_speed_x, "label_speed_x", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, label_speed_y, "label_speed_y", "30", "proportional_float");
	CPanelAnimationVarAliasType(float, value_speed_x, "value_speed_x", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, value_speed_y, "value_speed_y", "30", "proportional_float");
};