#include "cbase.h"
#include "hud.h"
#include "hud_airboatracer.h"
#include "c_prop_vehicle.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudAirboatRacer);

CHudAirboatRacer::CHudAirboatRacer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAirboatRacer")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudAirboatRacer::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
}

void CHudAirboatRacer::OnThink()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !pPlayer->IsInAVehicle()) {
		SetVisible(false);
		return;
	}

	IClientVehicle *pVehicle = pPlayer->GetVehicle();
	C_PropVehicleDriveable *pDrivable = dynamic_cast<C_PropVehicleDriveable*>(pVehicle);
	m_iSpeed = pDrivable->GetSpeed();

	SetVisible(true);
	BaseClass::OnThink();
}

void CHudAirboatRacer::Paint()
{
	SetPaintBorderEnabled(false);

	char sSpeed[10];
	Q_snprintf(sSpeed, sizeof(sSpeed), "%d", m_iSpeed);

	const char *pszLabelSpeed = "SPEED", *pszValueSpeed = sSpeed;
	wchar_t sLabelSpeed[256];
	wchar_t sValueSpeed[256];

	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelSpeed, sLabelSpeed, sizeof(sLabelSpeed));
	g_pVGuiLocalize->ConvertANSIToUnicode(pszValueSpeed, sValueSpeed, sizeof(sValueSpeed));

	// Set text colour
	Color cColor = m_TextColor;
	surface()->DrawSetTextColor(cColor[0], cColor[1], cColor[2], cColor[3]);

	// Speed Label
	surface()->DrawSetTextFont(m_hLabelSpeedFont);
	surface()->DrawSetTextPos(label_speed_x, label_speed_y);
	surface()->DrawUnicodeString(sLabelSpeed);

	// Speed Number
	surface()->DrawSetTextFont(m_hValueSpeedFont);
	surface()->DrawSetTextPos(value_speed_x, value_speed_y);
	surface()->DrawUnicodeString(sValueSpeed);
}