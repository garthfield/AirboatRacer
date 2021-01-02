#include "cbase.h"
#include "hud.h"
#include "hud_laptimes.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudLaptimes);
DECLARE_HUD_MESSAGE(CHudLaptimes, LapTime);

CHudLaptimes::CHudLaptimes(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudLaptimes")
{
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	SetAlpha(255);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudLaptimes::Init(void) {
	HOOK_HUD_MESSAGE(CHudLaptimes, LapTime);
}

void CHudLaptimes::Clear(void)
{
	for (int i = 0; i < MAX_LAPS; i++) {
		m_fLaptimes[i] = NULL;
	}
}

void CHudLaptimes::LevelShutdown()
{
	Clear();
	SetVisible(false);
}

void CHudLaptimes::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
}

void CHudLaptimes::MsgFunc_LapTime(bf_read &msg) {
	int iLap = msg.ReadLong();
	m_fLaptimes[iLap-1] = msg.ReadFloat();
}

void CHudLaptimes::OnThink()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !pPlayer->IsInAVehicle()) {
		SetVisible(false);
		return;
	}

	SetVisible(true);

	BaseClass::OnThink();
}

void CHudLaptimes::Paint()
{
	SetPaintBorderEnabled(false);

	const char *pszLabelLap = "LAP";
	wchar_t sLabelLap[256];

	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelLap, sLabelLap, sizeof(sLabelLap));


	// Set text colour
	Color cColor = m_TextColor;
	surface()->DrawSetTextColor(cColor[0], cColor[1], cColor[2], cColor[3]);

	// Speed Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(label_lap_x, label_lap_y);
	surface()->DrawUnicodeString(sLabelLap);

	surface()->DrawSetColor(255, 255, 0, 50);
	surface()->DrawLine(0, 40, 300, 40);

	int iLapTimeLineHeight = 22;
	int iLapTimeY = label_value_y;
	for (int i = 0; i < MAX_LAPS; i++) {

		if (!m_fLaptimes[i])
			continue;

		char sLap[10];
		Q_snprintf(sLap, sizeof(sLap), "%d", i+1);
		const char *pszValueLap = sLap;
		wchar_t sValueLap[256];

		g_pVGuiLocalize->ConvertANSIToUnicode(pszValueLap, sValueLap, sizeof(sValueLap));

		char sTime[10];
		Q_snprintf(sTime, sizeof(sTime), "%.2f", m_fLaptimes[i]);
		const char *pszValueTime = sTime;
		wchar_t sValueTime[256];

		g_pVGuiLocalize->ConvertANSIToUnicode(pszValueTime, sValueTime, sizeof(sValueTime));

		surface()->DrawSetTextFont(m_hTextFontSmall);
		surface()->DrawSetTextPos(label_value_x, iLapTimeY);
		surface()->DrawUnicodeString(sValueLap);

		surface()->DrawSetTextFont(m_hTextFontSmall);
		surface()->DrawSetTextPos((label_value_x + 30), iLapTimeY);
		surface()->DrawUnicodeString(sValueTime);

		iLapTimeY += iLapTimeLineHeight;
	}
}