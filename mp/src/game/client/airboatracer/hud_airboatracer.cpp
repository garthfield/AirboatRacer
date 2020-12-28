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
DECLARE_HUD_MESSAGE(CHudAirboatRacer, Powerup);

CHudAirboatRacer::CHudAirboatRacer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAirboatRacer")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	m_nPowerup1 = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nPowerup1, "sprites/hud/nitroicon", true, true);

	m_nPowerup2 = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nPowerup2, "sprites/hud/jumpicon", true, true);

	m_nPowerup3 = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nPowerup3, "sprites/hud/shockicon", true, true);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);

	m_iPowerupType = 1;
}

void CHudAirboatRacer::Init(void) {
	HOOK_HUD_MESSAGE(CHudAirboatRacer, Lap);
	HOOK_HUD_MESSAGE(CHudAirboatRacer, Powerup);
}

void CHudAirboatRacer::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
}

void CHudAirboatRacer::MsgFunc_Lap(bf_read &msg) {
	msg.ReadString(m_szLapInfo, sizeof(m_szLapInfo));
	Msg("Received Lap: %s\n", m_szLapInfo);
}

void CHudAirboatRacer::MsgFunc_Powerup(bf_read &msg) {
	m_iPowerupType = msg.ReadByte();
	Msg("Received Powerup Type: %d\n", m_iPowerupType);
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

	if (m_iPowerupType) {
		if (m_iPowerupType == 1) {
			surface()->DrawSetTexture(m_nPowerup1);
		}
		else if (m_iPowerupType == 2) {
			surface()->DrawSetTexture(m_nPowerup2);
		}
		else if (m_iPowerupType == 3) {
			surface()->DrawSetTexture(m_nPowerup3);
		}
		surface()->DrawTexturedRect(icon_powerup_x, icon_powerup_y, icon_powerup_x + 64, icon_powerup_y + 64);
	}

	char sSpeed[10];
	Q_snprintf(sSpeed, sizeof(sSpeed), "%d", m_iSpeed);

	const char *pszLabelSpeed = "SPEED", *pszValueSpeed = sSpeed;
	wchar_t sLabelSpeed[256];
	wchar_t sValueSpeed[256];

	const char *pszLabelLap = "LAP";
	wchar_t sLabelLap[256];
	wchar_t sValueLap[256];

	const char *pszLabelPowerup = "POWER-UP";
	wchar_t sLabelPowerup[256];

	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelPowerup, sLabelPowerup, sizeof(sLabelPowerup));

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

	// Powerup Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(label_powerup_x, label_powerup_y);
	surface()->DrawUnicodeString(sLabelPowerup);
}