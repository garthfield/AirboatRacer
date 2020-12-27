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
DECLARE_HUD_MESSAGE(CHudAirboatRacer, Lap);

CHudAirboatRacer::CHudAirboatRacer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAirboatRacer")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudAirboatRacer::Init(void) {
	HOOK_HUD_MESSAGE(CHudAirboatRacer, Lap);
}

void CHudAirboatRacer::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
}

void CHudAirboatRacer::MsgFunc_Lap(bf_read &msg) {
	msg.ReadString(m_szLapInfo, sizeof(m_szLapInfo));
	Msg("Received Lap: %s\n", m_szLapInfo);
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

	const char *pszLabelLap = "LAP";
	wchar_t sLabelLap[256];
	wchar_t sValueLap[256];


	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelSpeed, sLabelSpeed, sizeof(sLabelSpeed));
	g_pVGuiLocalize->ConvertANSIToUnicode(pszValueSpeed, sValueSpeed, sizeof(sValueSpeed));

	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelLap, sLabelLap, sizeof(sLabelLap));
	g_pVGuiLocalize->ConvertANSIToUnicode(m_szLapInfo, sValueLap, sizeof(sValueLap));

	// Set text colour
	Color cColor = m_TextColor;
	surface()->DrawSetTextColor(cColor[0], cColor[1], cColor[2], cColor[3]);

	// Speed Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(label_speed_x, label_speed_y);
	surface()->DrawUnicodeString(sLabelSpeed);

	// Speed Number
	surface()->DrawSetTextFont(m_hNumberFont);
	surface()->DrawSetTextPos(value_speed_x, value_speed_y);
	surface()->DrawUnicodeString(sValueSpeed);

	// Lap Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(label_lap_x, label_lap_y);
	surface()->DrawUnicodeString(sLabelLap);

	// Lap Number
	surface()->DrawSetTextFont(m_hNumberFont);
	surface()->DrawSetTextPos(value_lap_x, value_lap_y);
	surface()->DrawUnicodeString(sValueLap);
}