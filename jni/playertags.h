#pragma once

#include "../gui/gui.h"

namespace PlayerTagStyle {
	static constexpr ImU32 kIdBadgeFill     = IM_COL32(0xE8, 0xC4, 0x58, 0xFF);
	static constexpr ImU32 kIdBadgeBorder   = IM_COL32(0, 0, 0, 0xFF);
	static constexpr ImU32 kIdBadgeText     = IM_COL32(0x10, 0x10, 0x10, 0xFF);
	static constexpr ImU32 kHealthBarBg     = IM_COL32(0x1A, 0x18, 0x18, 0xE0);
	static constexpr ImU32 kHealthFill      = IM_COL32(0xC4, 0x2A, 0x32, 0xFF);
	static constexpr ImU32 kArmourFill      = IM_COL32(0xC0, 0xC4, 0xCC, 0xFF);
	static constexpr ImU32 kBarBorder       = IM_COL32(0, 0, 0, 0xFF);
	static constexpr ImU32 kBarText         = IM_COL32(0xF8, 0xFF, 0xFF, 0xFF);
	static constexpr ImU32 kBarTextShadow   = IM_COL32(0, 0, 0, 0xDC);
	static constexpr float kIdBadgePadX     = 6.0f;
	static constexpr float kIdBadgePadY     = 1.5f;
	static constexpr float kNickIdGap       = 5.0f;
	static constexpr float kBarBorderThick  = 1.6f;
	static constexpr float kIdBorderThick   = 1.6f;
}

struct stVoiceSort
{
	uint32_t m_iVoiceTime;
	PLAYERID m_iPlayerID;
};

struct playerTags
{
	bool m_bTagStatus;
	char m_TagText[128];
	uint8_t m_iTagType;
	uint32_t m_iTagColor;
	uint32_t m_iTagColorBox;
};

class CPlayerTags
{
public:
	CPlayerTags();
	~CPlayerTags();

	void Render();
	void AddChatBubble(PLAYERID playerId, char* szText, uint32_t dwColor, float fDistance, uint32_t dwTime);
	void ResetChatBubble(PLAYERID playerId);
	void DrawChatBubble(PLAYERID playerId, VECTOR* vec, float fDistance);
	
	static void UpdatePlayerTags(PLAYERID playerId, bool tagStatus, char* szText, uint8_t tagType, uint32_t tagColor, uint32_t tagColorBox);

private:
	ImVec2 HealthBarBDR1;
	ImVec2 HealthBarBDR2;
	ImVec2 HealthBarBG1;
	ImVec2 HealthBarBG2;
	ImVec2 HealthBar1;
	ImVec2 HealthBar2;

	ImColor HealthBarColor;
	ImColor HealthBarBGColor;
	ImColor HealthBarBDRColor;

	RwTexture* m_pAfk_icon;
	RwTexture* m_pVoice_icon;
	RwTexture* m_pKeyboard_icon;

	uint32_t m_dwColors[MAX_PLAYERS];
	float m_fDistance[MAX_PLAYERS];
	float m_fTrueX[MAX_PLAYERS];
	uint32_t m_dwTime[MAX_PLAYERS];
	uint32_t m_dwStartTime[MAX_PLAYERS];
	char* m_pSzText[MAX_PLAYERS];
	char* m_pSzTextWithoutColors[MAX_PLAYERS];
	bool m_bChatBubbleStatus[MAX_PLAYERS];
	int m_iOffset[MAX_PLAYERS];

	uint32_t m_iVoiceTime[MAX_PLAYERS];
	uint32_t m_iLastVoiceTimeUpdated[MAX_PLAYERS];
	PLAYERID m_iPlayerIDs[MAX_PLAYERS];
	
	static playerTags m_PlayerTags[MAX_PLAYERS];

	void Draw(VECTOR* vec, char* szNick, uint32_t dwColor, 
	float fDist, float fHealth, float fArmour, bool bAfk, bool bVoice, int playerID);

	void DrawHealthBar(ImDrawList* dl, ImFont* font, float centerX, float topY, float health, float armour, float idFontSize);
	float GetIdBadgeFontSize();
	void DrawShadowedBarText(ImDrawList* dl, ImFont* font, float fontSize, ImVec2 pos, const char* text);
};