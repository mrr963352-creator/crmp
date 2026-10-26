#include "main.h"
#include "game/game.h"
#include "game/RW/RenderWare.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "playertags.h"
#include "CSettings.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CGUI *pGUI;
extern CSettings *pSettings;

playerTags CPlayerTags::m_PlayerTags[MAX_PLAYERS];

CPlayerTags::CPlayerTags()
{
	Log("Loading afk_icon..");
	m_pAfk_icon = (RwTexture*)LoadTextureFromDB(CMain::GetCustomStorage(), "afk_icon");
	m_pVoice_icon = (RwTexture*)LoadTextureFromDB(CMain::GetCustomStorage(), "micro_icon");
	m_pKeyboard_icon = (RwTexture*)LoadTextureFromDB(CMain::GetCustomStorage(), "keyboard_icon");
	
	HealthBarBDRColor = ImColor( 0x00, 0x00, 0x00, 0xFF );

	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		m_bChatBubbleStatus[i] = 0;
		m_pSzText[i] = nullptr;
		m_pSzTextWithoutColors[i] = nullptr;
		m_iVoiceTime[i] = 0;
		m_iLastVoiceTimeUpdated[i] = 0;
		
		m_PlayerTags[i].m_bTagStatus = false;
		m_PlayerTags[i].m_iTagType = 0;
	}
}

CPlayerTags::~CPlayerTags() {}

#include "voice/CVoiceChatClient.h"
extern CVoiceChatClient* pVoice;

static stVoiceSort aVoiceSortTime[MAX_PLAYERS];
#include <algorithm>

void CPlayerTags::Render()
{
	if (!pSettings->GetReadOnly().iNameTag)
		return;

	VECTOR VecPos;
	MATRIX4X4 matLocal, matPlayer;
	int dwHitEntity;
	char szNickBuf[50];
	int iVoiceCounter = 0;
	ImVec2 basePos = ImVec2(pGUI->ScaleX(10.0f), pGUI->ScaleY(600.0f));

	if(pNetGame && pNetGame->m_bShowPlayerTags)
	{
		CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
		pGame->FindPlayerPed()->GetMatrix(&matLocal);

		for(PLAYERID playerId: pPlayerPool->m_PlayersCount)
		{
			aVoiceSortTime[playerId].m_iPlayerID = playerId;
			aVoiceSortTime[playerId].m_iVoiceTime = 999999;

			if(pPlayerPool->GetSlotState(playerId) == true)
			{
				CRemotePlayer* pPlayer = pPlayerPool->GetAt(playerId);

				if(pPlayer && pPlayer->IsActive() && pPlayer->m_bShowNameTag)
				{
					CPlayerPed* pPlayerPed = pPlayer->GetPlayerPed();
					
					if (m_bChatBubbleStatus[playerId])
					{
						if (!pPlayerPed)
						{
							ResetChatBubble(playerId);
							continue;
						}
						if (pPlayerPed->GetDistanceFromCamera() <= m_fDistance[playerId])
						{
							if (!pPlayerPed->IsAdded()) continue;
							VecPos.X = 0.0f;
							VecPos.Y = 0.0f;
							VecPos.Z = 0.0f;
							pPlayerPed->GetBonePosition(8, &VecPos);
							DrawChatBubble(playerId, &VecPos, pPlayerPed->GetDistanceFromCamera());
						}
						if (GetTickCount() - m_dwStartTime[playerId] >= m_dwTime[playerId])
						{
							ResetChatBubble(playerId);
						}
					}
					
					if(pPlayerPed->GetDistanceFromCamera() <= pNetGame->m_fNameTagDrawDistance)
					{
						if(!pPlayerPed->IsAdded()) continue;
						VecPos.X = 0.0f;
						VecPos.Y = 0.0f;
						VecPos.Z = 0.0f;
						pPlayerPed->GetBonePosition(8, &VecPos);

						CAMERA_AIM *pCam = GameGetInternalAim();
						dwHitEntity = 0;

						if(pNetGame->m_bNameTagLOS)
						{
							/*dwHitEntity = ScriptCommand(&get_line_of_sight, 
								VecPos.X, VecPos.Y, VecPos.Z,
								pCam->pos1x, pCam->pos1y, pCam->pos1z,
								1, 0, 0, 1, 0);*/
							dwHitEntity = 1;
						}

						if (!pNetGame->m_bNameTagLOS || dwHitEntity)
						{
							if (pVoice)
							{
								bool bVoice = (GetTickCount() - pVoice->m_aLastPushTime[playerId] <= 500);

								sprintf(szNickBuf, "%s %d", pPlayerPool->GetPlayerName(playerId), playerId);
								Draw(&VecPos, szNickBuf,
									pPlayer->GetPlayerColor(),
									pPlayerPed->GetDistanceFromCamera(),
									pPlayer->m_fReportedHealth,
									pPlayer->m_fReportedArmour,
									pPlayer->IsAFK(), bVoice, playerId);

								if (bVoice)
								{
									pVoice->SetPlayerVoiceStatus(playerId, true);
									if (m_iVoiceTime[playerId] == 0 || m_iVoiceTime[playerId] == 999999)
									{
										m_iVoiceTime[playerId] = 1000 + (rand() % 20);
										m_iLastVoiceTimeUpdated[playerId] = GetTickCount();
										aVoiceSortTime[playerId].m_iVoiceTime = m_iVoiceTime[playerId];
									}
									else
									{
										m_iVoiceTime[playerId] += GetTickCount() - m_iLastVoiceTimeUpdated[playerId];
										aVoiceSortTime[playerId].m_iVoiceTime = m_iVoiceTime[playerId];
										m_iLastVoiceTimeUpdated[playerId] = GetTickCount();
									}
								}
								else
								{
									m_iVoiceTime[playerId] = 999999;
									aVoiceSortTime[playerId].m_iVoiceTime = m_iVoiceTime[playerId];
									m_iLastVoiceTimeUpdated[playerId] = 0;
								}
							}
							else
							{
								sprintf(szNickBuf, "%s %d", pPlayerPool->GetPlayerName(playerId), playerId);
								Draw(&VecPos, szNickBuf,
									pPlayer->GetPlayerColor(),
									pPlayerPed->GetDistanceFromCamera(),
									pPlayer->m_fReportedHealth,
									pPlayer->m_fReportedArmour,
									pPlayer->IsAFK(), 0, playerId);
							}
						}
					}
				}
			}
		}
		
		if (pGame->IsToggledHUDElement(HUD_ELEMENT_VOICE) && pSettings->GetWrite().iVoiceList)
		{
			int n = sizeof(aVoiceSortTime) / sizeof(aVoiceSortTime[0]);

			struct 
			{
				bool operator()(stVoiceSort a, stVoiceSort b)
				{
					return a.m_iVoiceTime < b.m_iVoiceTime;
				}
			} customLess;

			std::sort(aVoiceSortTime, aVoiceSortTime + n, customLess);
			for (int i = 0; i <= 6; i++)
			{
				if (aVoiceSortTime[i].m_iVoiceTime >= 999990) break;
				if (pPlayerPool->GetSlotState(aVoiceSortTime[i].m_iPlayerID) == false) continue;
				sprintf(szNickBuf, "%s (%d)", pPlayerPool->GetPlayerName(aVoiceSortTime[i].m_iPlayerID), aVoiceSortTime[i].m_iPlayerID);
				basePos.y += pGUI->GetFontSize() * 1.1f;
				ImVec2 iconPos = basePos;
				iconPos.x += ImGui::CalcTextSize(szNickBuf).x + (pGUI->GetFontSize() * 0.6f);
				if (m_pVoice_icon)
				{
					ImVec2 a = ImVec2(iconPos.x, iconPos.y);
					ImVec2 b = ImVec2(iconPos.x + pGUI->GetFontSize(), iconPos.y + pGUI->GetFontSize());
					ImGui::GetBackgroundDrawList()->AddImage((ImTextureID)m_pVoice_icon->raster, a, b);
				}

				pGUI->RenderText(basePos, 0xFFFFFFFF, true, szNickBuf);

				iVoiceCounter++;
			}
		}
	}
}

bool ProcessInlineHexColor(const char* start, const char* end, ImVec4& color);
void TextWithColors(ImVec2 pos, ImColor col, const char* szStr, const char* szStrWithoutColors = nullptr);
void FilterColors(char* szStr);

void CPlayerTags::AddChatBubble(PLAYERID playerId, char* szText, uint32_t dwColor, float fDistance, uint32_t dwTime)
{
	if (m_bChatBubbleStatus[playerId])
	{
		ResetChatBubble(playerId);
		m_dwColors[playerId] = dwColor;
		m_fDistance[playerId] = fDistance;
		m_dwTime[playerId] = dwTime;
		m_dwStartTime[playerId] = GetTickCount();
		m_bChatBubbleStatus[playerId] = 1;
		m_fTrueX[playerId] = -1.0f;
		cp1251_to_utf8(m_pSzText[playerId], szText);
		cp1251_to_utf8(m_pSzTextWithoutColors[playerId], szText);
		FilterColors(m_pSzTextWithoutColors[playerId]);
		const char* pText = m_pSzTextWithoutColors[playerId];
		m_iOffset[playerId] = 0;
		while (*pText)
		{
			if (*pText == '\n')
			{
				m_iOffset[playerId]++;
			}
			pText++;
		}
		return;
	}
	m_dwColors[playerId] = dwColor;
	m_fDistance[playerId] = fDistance;
	m_dwTime[playerId] = dwTime;
	m_dwStartTime[playerId] = GetTickCount();
	m_bChatBubbleStatus[playerId] = 1;
	m_fTrueX[playerId] = -1.0f;
	m_pSzText[playerId] = new char[1024];
	m_pSzTextWithoutColors[playerId] = new char[1024];
	cp1251_to_utf8(m_pSzText[playerId], szText);
	cp1251_to_utf8(m_pSzTextWithoutColors[playerId], szText);
	FilterColors(m_pSzTextWithoutColors[playerId]);
	const char* pText = m_pSzTextWithoutColors[playerId];
	m_iOffset[playerId] = 0;
	while (*pText)
	{
		if (*pText == '\n')
		{
			m_iOffset[playerId]++;
		}
		pText++;
	}
}

void CPlayerTags::ResetChatBubble(PLAYERID playerId)
{
	if (m_bChatBubbleStatus[playerId])
	{
		m_dwTime[playerId] = 0;
	}
	m_bChatBubbleStatus[playerId] = 0;
}

void CPlayerTags::DrawChatBubble(PLAYERID playerId, VECTOR* vec, float fDistance)
{
	VECTOR TagPos;

	TagPos.X = vec->X;
	TagPos.Y = vec->Y;
	TagPos.Z = vec->Z;
	TagPos.Z += 0.45f + (fDistance * 0.0675f) + ((float)m_iOffset[playerId] * pGUI->ScaleY(0.35f));

	VECTOR Out;
	((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(SA_ADDR(0x54EEC0 + 1)))(&TagPos, &Out, 0, 0, 0, 0);

	if (Out.Z < 1.0f)
		return;

	ImVec2 pos = ImVec2(Out.X, Out.Y);

	if (m_fTrueX[playerId] < 0)
	{
		char* curBegin = m_pSzTextWithoutColors[playerId];
		char* curPos = m_pSzTextWithoutColors[playerId];
		while (*curPos != '\0')
		{
			if (*curPos == '\n')
			{
				float width = ImGui::CalcTextSize(curBegin, (char*)(curPos - 1)).x;
				if (width > m_fTrueX[playerId])
				{
					m_fTrueX[playerId] = width;
				}

				curBegin = curPos + 1;
			}

			curPos++;
		}

		if (m_fTrueX[playerId] < 0)
		{
			m_fTrueX[playerId] = ImGui::CalcTextSize(m_pSzTextWithoutColors[playerId]).x;
		}
	}

	pos.x -= (m_fTrueX[playerId] / 2);

	TextWithColors(pos, __builtin_bswap32(m_dwColors[playerId]), m_pSzText[playerId]);
}

// =============================================================
// Draw — ник + жёлтый бокс с ID + HP / ARMOR бары с числами
// =============================================================
/*void CPlayerTags::Draw(VECTOR* vec, char* szName, uint32_t dwColor,
    float fDist, float fHealth, float fArmour, bool bAfk, bool bVoice, int playerID)
{
    if (!pGame->IsToggledHUDElement(HUD_ELEMENT_TAGS))
        return;

    VECTOR TagPos;
    TagPos.X = vec->X;
    TagPos.Y = vec->Y;
    TagPos.Z = vec->Z + 0.25f + (fDist * 0.0475f);

    VECTOR Out;
    ((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(SA_ADDR(0x54EEC0 + 1)))
        (&TagPos, &Out, 0, 0, 0, 0);

    if (Out.Z < 1.0f)
        return;

    // --- Разбить "Nickname ID" на nick и id ---
    std::string fullText = szName;
    std::string nick, id;

    size_t p1 = fullText.rfind(" ");
    if (p1 != std::string::npos)
    {
        nick = fullText.substr(0, p1);
        id   = fullText.substr(p1 + 1);
    }
    else
    {
        nick = fullText;
        id = "";
    }

    char nickBuff[256];
    char idBuff[64];

    cp1251_to_utf8(nickBuff, nick.c_str());
    cp1251_to_utf8(idBuff, id.c_str());

    float fontSize = pGUI->GetFontSize();
    float padX     = pGUI->ScaleX(8.0f);
    float padY     = pGUI->ScaleY(1.0f);
    float gapNI    = pGUI->ScaleX(4.0f);  // зазор между ником и боксом

    float nickW  = ImGui::CalcTextSize(nickBuff).x;
    float idW    = ImGui::CalcTextSize(idBuff).x;
    float idBoxW = idW + padX * 2.0f;
    float totalW = nickW + gapNI + idBoxW;

    // Центрируем всю строку по X
    ImVec2 basePos(Out.X - totalW / 2.0f, Out.Y);

    // ── НИК ──────────────────────────────────────────────────
    ImVec2 nickPos    = basePos;
    ImVec2 nickShadow = ImVec2(basePos.x + 1, basePos.y + 1);

    pGUI->RenderText(nickShadow, ImColor(0, 0, 0, 200), true, nickBuff);
    pGUI->RenderText(nickPos,    ImColor(dwColor),       true, nickBuff);

    // ── ID БОКС (жёлтый) ─────────────────────────────────────
    float idBoxX = basePos.x + nickW + gapNI;

    ImVec2 idMin(idBoxX,          basePos.y - padY);
    ImVec2 idMax(idBoxX + idBoxW, basePos.y + fontSize + padY);

    // Заливка
    ImGui::GetBackgroundDrawList()->AddRectFilled(
        idMin, idMax,
        ImColor(255, 220, 0, 255),
        pGUI->ScaleY(4.0f));

    // Обводка
    ImGui::GetBackgroundDrawList()->AddRect(
        idMin, idMax,
        ImColor(0, 0, 0, 120),
        pGUI->ScaleY(4.0f));

    // Текст ID по центру бокса
    ImVec2 idTextPos(
        idBoxX + (idBoxW - idW) / 2.0f,
        basePos.y);

    pGUI->RenderText(ImVec2(idTextPos.x + 1, idTextPos.y + 1), ImColor(0, 0, 0, 255), true, idBuff);
    pGUI->RenderText(idTextPos, ImColor(255, 255, 255, 255), true, idBuff);

    // ── HP / ARMOR БАРЫ ──────────────────────────────────────
    int iHP     = (int)ImClamp(fHealth,  0.0f, 100.0f);
    int iArmour = (int)ImClamp(fArmour,  0.0f, 100.0f);

    bool hasArmour = iArmour > 0;

    float cellW = totalW * 0.45f;
    float cellH = fontSize * 0.55f;
    float gap   = pGUI->ScaleX(2.0f);

    float barsY      = basePos.y + fontSize * 1.2f;
    float totalBarsW = hasArmour ? (cellW * 2.0f + gap) : cellW;
    float barsX      = Out.X - totalBarsW / 2.0f;

    // --- HP ---
    ImVec2 hpMin(barsX, barsY);
    ImVec2 hpMax(barsX + cellW, barsY + cellH);

    float  hpPercent = iHP / 100.0f;
    ImVec2 hpFillMax(hpMin.x + cellW * hpPercent, hpMax.y);

    ImGui::GetBackgroundDrawList()->AddRectFilled(hpMin, hpMax,    ImColor(60,  0,  0, 255), pGUI->ScaleY(4.0f));
    ImGui::GetBackgroundDrawList()->AddRectFilled(hpMin, hpFillMax, ImColor(220, 40, 40, 255), pGUI->ScaleY(4.0f));
    ImGui::GetBackgroundDrawList()->AddRect      (hpMin, hpMax,    ImColor(0,   0,  0, 255), pGUI->ScaleY(4.0f));

    char  hpBuf[8];
    sprintf(hpBuf, "%d", iHP);

    float  hpTw  = ImGui::CalcTextSize(hpBuf).x;
    ImVec2 hpText(hpMin.x + (cellW - hpTw) / 2.0f, hpMin.y + padY);

    pGUI->RenderText(ImVec2(hpText.x + 1, hpText.y + 1), ImColor(0, 0, 0, 180), true, hpBuf);
    pGUI->RenderText(hpText, ImColor(255, 255, 255, 255), true, hpBuf);

    // --- ARMOR (только если > 0) ---
    if (hasArmour)
    {
        float armX = barsX + cellW + gap;

        ImVec2 armMin(armX, barsY);
        ImVec2 armMax(armX + cellW, barsY + cellH);

        float  armPercent = iArmour / 100.0f;
        ImVec2 armFillMax(armMin.x + cellW * armPercent, armMax.y);

        ImGui::GetBackgroundDrawList()->AddRectFilled(armMin, armMax,    ImColor(40,  40,  40, 255), pGUI->ScaleY(4.0f));
        ImGui::GetBackgroundDrawList()->AddRectFilled(armMin, armFillMax, ImColor(160, 160, 160, 255), pGUI->ScaleY(4.0f));
        ImGui::GetBackgroundDrawList()->AddRect      (armMin, armMax,    ImColor(0,   0,   0, 255), pGUI->ScaleY(4.0f));

        char  armBuf[8];
        sprintf(armBuf, "%d", iArmour);

        float  armTw  = ImGui::CalcTextSize(armBuf).x;
        ImVec2 armText(armMin.x + (cellW - armTw) / 2.0f, armMin.y + padY);

        pGUI->RenderText(ImVec2(armText.x + 1, armText.y + 1), ImColor(0, 0, 0, 180), true, armBuf);
        pGUI->RenderText(armText, ImColor(255, 255, 255, 255), true, armBuf);
    }
}*/
// =============================================================
// Helpers
// =============================================================

float CPlayerTags::GetIdBadgeFontSize()
{
    float idFontSize = pGUI->ScaleX(11.0f);
    const float minSize = pGUI->ScaleY(0.78f);
    if (idFontSize < minSize)
        idFontSize = minSize;

    const float maxSize = pGUI->GetFontSize() * 1.00f;
    if (idFontSize > maxSize)
        idFontSize = maxSize;

    const float minReadable = pGUI->GetFontSize() * 0.88f;
    if (idFontSize < minReadable)
        idFontSize = minReadable;
    return idFontSize;
}

void CPlayerTags::DrawShadowedBarText(ImDrawList* dl, ImFont* font, float fontSize, ImVec2 pos, const char* text)
{
    dl->AddText(font, fontSize, ImVec2(pos.x + 1.0f, pos.y + 1.0f), PlayerTagStyle::kBarTextShadow, text);
    dl->AddText(font, fontSize, pos, PlayerTagStyle::kBarText, text);
}

void CPlayerTags::DrawHealthBar(ImDrawList* dl, ImFont* font, float centerX, float topY, float health, float armour, float idFontSize)
{
    if (health < 0.0f)
        return;

    const float scaleX     = pGUI->ScaleX(1.0f);
    const float layoutScale = pGUI->ScaleY(1.0f);

    float barFontSize = layoutScale * 15.0f;
    if (barFontSize > idFontSize * 0.7f)
        barFontSize = idFontSize * 0.7f;

    const float barHalfW   = scaleX * 70.0f;
    const float barRounding = layoutScale * 2.0f;
    float barHeight         = layoutScale * 11.0f;
    const float minBarHeight = layoutScale * 3.0f + barFontSize;
    if (barHeight < minBarHeight)
        barHeight = minBarHeight;

    if (health > 100.0f) health = 100.0f;
    if (armour < 0.0f)   armour = 0.0f;
    if (armour > 100.0f) armour = 100.0f;

    const ImVec2 barMin(centerX - barHalfW, topY);
    const ImVec2 barMax(centerX + barHalfW, topY + barHeight);

    dl->AddRectFilled(barMin, barMax, PlayerTagStyle::kHealthBarBg, barRounding);

    if (armour <= 0.0f) {
        const float fillW = (health / 100.0f) * (barHalfW * 2.0f);
        dl->AddRectFilled(barMin, ImVec2(barMin.x + fillW, barMax.y), PlayerTagStyle::kHealthFill, barRounding);
    } else {
        const float hpFillW = (health / 50.0f) * barHalfW;
        dl->AddRectFilled(barMin, ImVec2(barMin.x + hpFillW, barMax.y), PlayerTagStyle::kHealthFill, barRounding);

        const ImVec2 arMin(centerX, barMin.y);
        const ImVec2 arMax(centerX + (armour / 100.0f) * barHalfW, barMax.y);
        dl->AddRectFilled(arMin, arMax, PlayerTagStyle::kArmourFill, barRounding);

        dl->AddLine(
            ImVec2(centerX, barMin.y + 1.0f),
            ImVec2(centerX, barMax.y - 1.0f),
            IM_COL32(0, 0, 0, 0xFF), 1.4f
        );
    }

    dl->AddRect(barMin, barMax, PlayerTagStyle::kBarBorder, barRounding, 0, PlayerTagStyle::kBarBorderThick);

    char valueBuf[8];
    snprintf(valueBuf, sizeof(valueBuf), "%d", static_cast<int>(health + 0.5f));

    ImVec2 textSize = font->CalcTextSizeA(barFontSize, FLT_MAX, 0.0f, valueBuf);
    float textY = topY + (barHeight - textSize.y) * 0.5f;
    float textX = centerX - textSize.x * 0.5f;
    if (armour > 0.0f)
        textX = barMin.x + barHalfW * 0.5f - textSize.x * 0.5f;

    DrawShadowedBarText(dl, font, barFontSize, ImVec2(textX, textY), valueBuf);

    if (armour > 0.0f) {
        snprintf(valueBuf, sizeof(valueBuf), "%d", static_cast<int>(armour + 0.5f));
        textSize = font->CalcTextSizeA(barFontSize, FLT_MAX, 0.0f, valueBuf);
        textX = centerX + barHalfW * 0.5f - textSize.x * 0.5f;
        DrawShadowedBarText(dl, font, barFontSize, ImVec2(textX, textY), valueBuf);
    }

    HealthBarBDR1 = barMin;
    HealthBarBDR2 = barMax;
}

// =============================================================
// Draw
// =============================================================
void CPlayerTags::Draw(VECTOR* vec, char* szName, uint32_t dwColor,
    float fDist, float fHealth, float fArmour, bool bAfk, bool bVoice, int playerID)
{
    if (!pGame->IsToggledHUDElement(HUD_ELEMENT_TAGS)) return;

    VECTOR TagPos;
    TagPos.X = vec->X;
    TagPos.Y = vec->Y;
    TagPos.Z = vec->Z + 0.25f + (fDist * 0.0475f);

    VECTOR Out;
    ((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(SA_ADDR(0x54EEC0 + 1)))(&TagPos, &Out, 0, 0, 0, 0);

    if (Out.Z < 1.0f)
        return;

    // Split "Nickname ID" -> nick + id
    std::string fullText = szName;
    size_t spacePos = fullText.rfind(' ');
    std::string nick = (spacePos != std::string::npos) ? fullText.substr(0, spacePos) : fullText;
    std::string idStr = (spacePos != std::string::npos) ? fullText.substr(spacePos + 1) : "";

    char nickBuff[256];
    char idBuf[32];
    cp1251_to_utf8(nickBuff, nick.c_str());
    snprintf(idBuf, sizeof(idBuf), "%s", idStr.c_str());

    ImDrawList* const dl   = ImGui::GetBackgroundDrawList();
    ImFont* const font     = ImGui::GetFont();
    const float idFontSize = GetIdBadgeFontSize();
    const float padX       = pGUI->ScaleX(PlayerTagStyle::kIdBadgePadX);
    const float padY       = pGUI->ScaleY(PlayerTagStyle::kIdBadgePadY);
    const float nickGap    = pGUI->ScaleX(PlayerTagStyle::kNickIdGap);
    const float fontH      = pGUI->GetFontSize();

    const ImVec2 nickSize = ImGui::CalcTextSize(nickBuff);
    const float  nickH    = nickSize.y > fontH ? nickSize.y : fontH;

    const ImVec2 idTextSize  = font->CalcTextSizeA(idFontSize, FLT_MAX, 0.0f, idBuf);
    const ImVec2 idBadgeSize(idTextSize.x + padX * 2.0f, idTextSize.y + padY * 2.0f);

    const float totalW  = nickSize.x + nickGap + idBadgeSize.x;
    const float rowLeft = Out.X - totalW * 0.5f;

    // Nick
    ImVec2 nickPos(rowLeft, Out.Y);
    pGUI->RenderText(nickPos, __builtin_bswap32(dwColor | 0xFF), true, nickBuff);

    // ID badge
    const ImVec2 idBadgeMin(rowLeft + nickSize.x + nickGap, Out.Y + (nickH - idBadgeSize.y) * 0.5f);
    const ImVec2 idBadgeMax(idBadgeMin.x + idBadgeSize.x, idBadgeMin.y + idBadgeSize.y);
    const float  idRounding = idBadgeSize.y * 0.35f < pGUI->ScaleY(6.0f) ? idBadgeSize.y * 0.35f : pGUI->ScaleY(6.0f);

    dl->AddRectFilled(idBadgeMin, idBadgeMax, PlayerTagStyle::kIdBadgeFill, idRounding);
    dl->AddRect      (idBadgeMin, idBadgeMax, PlayerTagStyle::kIdBadgeBorder, idRounding, 0, PlayerTagStyle::kIdBorderThick);

    // Center ID text inside badge
    const ImVec2 idTextPos(
        idBadgeMin.x + (idBadgeSize.x - idTextSize.x) * 0.5f,
        idBadgeMin.y + (idBadgeSize.y - idTextSize.y) * 0.5f
    );
    dl->AddText(font, idFontSize, idTextPos, PlayerTagStyle::kIdBadgeText, idBuf);

    // Health + armour bar
    float rowBottom = Out.Y + nickH;
    DrawHealthBar(dl, font, Out.X, rowBottom + pGUI->ScaleY(3.0f), fHealth, fArmour, idFontSize);

    // AFK icon (left of bar)
    const float iconSize   = fontH * 1.2f;
    const float rightEdge  = rowLeft + totalW;
    const float rightAnchor = rightEdge > HealthBarBDR2.x ? rightEdge : HealthBarBDR2.x;

    if (bAfk && m_pAfk_icon)
    {
        const float leftAnchor = rowLeft < HealthBarBDR1.x ? rowLeft : HealthBarBDR1.x;
        ImVec2 a(leftAnchor - iconSize - pGUI->ScaleX(4.0f), HealthBarBDR1.y);
        ImVec2 b(a.x + iconSize, a.y + iconSize);
        dl->AddImage((ImTextureID)m_pAfk_icon->raster, a, b);
    }

    // Keyboard icon (commented out, kept for future)
    /*if (bKeyboard && m_pKeyboard_icon)
    {
        ImVec2 a(rightAnchor + pGUI->ScaleX(4.0f), Out.Y);
        ImVec2 b(a.x + iconSize, a.y + iconSize);
        dl->AddImage((ImTextureID)m_pKeyboard_icon->raster, a, b);
    }*/

    // Voice icon
    VECTOR TagPos_voice;
    TagPos_voice.X = vec->X;
    TagPos_voice.Y = vec->Y;
    TagPos_voice.Z = vec->Z + 0.65f + (fDist * 0.0475f);

    VECTOR Out_voice;
    ((void (*)(VECTOR*, VECTOR*, float*, float*, bool, bool))(SA_ADDR(0x54EEC0 + 1)))(&TagPos_voice, &Out_voice, 0, 0, 0, 0);

    if (Out_voice.Z < 1.0f)
        return;

    if (bVoice && m_pVoice_icon)
    {
        const float half = (pGUI->GetFontSize() * 1.3f) * 0.5f;
        ImVec2 a(Out_voice.X - half * 1.5f, Out_voice.Y);
        ImVec2 b(Out_voice.X + half * 1.5f, Out_voice.Y + pGUI->GetFontSize() * 1.3f);
        dl->AddImage((ImTextureID)m_pVoice_icon->raster, a, b);
    }
}

void CPlayerTags::UpdatePlayerTags(PLAYERID playerId, bool tagStatus, char* szText, uint8_t tagType, uint32_t tagColor, uint32_t tagColorBox)
{
	m_PlayerTags[playerId].m_bTagStatus = tagStatus;
	cp1251_to_utf8(m_PlayerTags[playerId].m_TagText, szText);
	m_PlayerTags[playerId].m_iTagType     = tagType;
	m_PlayerTags[playerId].m_iTagColor    = tagColor;
	m_PlayerTags[playerId].m_iTagColorBox = tagColorBox;
}