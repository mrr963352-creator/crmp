#include "../main.h"
#include "../game/game.h"
#include "netgame.h"
#include "dialog.h"
#include "../chatwindow.h"
#include "../keyboard.h"

extern CGame *pGame;
extern CNetGame *pNetGame;
extern CKeyBoard *pKeyBoard;
extern CChatWindow *pChatWindow;
extern CDialogWindow *pDialogWindow;

CVehiclePool::CVehiclePool()
{
	for(VEHICLEID VehicleID = 0; VehicleID < MAX_VEHICLES; VehicleID++)
	{
		m_bVehicleSlotState[VehicleID] = false;
		m_pVehicles[VehicleID] = nullptr;
		m_pGTAVehicles[VehicleID] = nullptr;
	}
	m_VehiclesCount.clear();
}

CVehiclePool::~CVehiclePool()
{
	m_VehiclesCount.clear();
	for(VEHICLEID VehicleID = 0; VehicleID < MAX_VEHICLES; VehicleID++)
		Delete(VehicleID);
}

#include "../util/CJavaWrapper.h"
#include "../util/interfaces/CSpeedometr.h"
float VehicleFuel;
float TimeFuel;
float floatVehicleMillage;
int VehicleMilleage;

bool openSpeedometr = false;
bool ActivatedSpeedometr;
bool DeactivatedSpeedometr;
MATRIX4X4 matPos;
MATRIX4X4 nowPos;
uint32_t m_ulLastUpdateTime;
void CVehiclePool::UpdateSpeed()
{
    if(!pGame || !pGame->FindPlayerPed()) return;
    
    CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
    bool bInVehicle = pPlayerPed->IsInVehicle();
    bool bDriverSeat = bInVehicle && (pPlayerPed->GetVehicleSeatID() == 0);
    
    static bool bLastState = false;
    bool bCurrentState = false;
    static int iLastFuelPercent = -1; // Для отслеживания изменений топлива
    
// Упрощенная версия без проверки двигателя
if (bDriverSeat) 
{
    CVehicle* pVehicle = GetAt(FindIDFromGtaPtr(pPlayerPed->GetGtaVehicle()));
    if (pVehicle) 
    {
        // Проверяем условия видимости спидометра
        bool bCanShow = !(pKeyBoard && pKeyBoard->IsOpen()) &&
                      !(pDialogWindow && pDialogWindow->m_bIsActive) &&
                      pGame->IsToggledHUDElement(HUD_ELEMENT_HUD);
        
        if (bCanShow) 
        {
            // Расчет скорости
            VECTOR vecMoveSpeed;
            pVehicle->GetMoveSpeedVector(&vecMoveSpeed);
            int speed = ((sqrt((vecMoveSpeed.X * vecMoveSpeed.X) + 
                        (vecMoveSpeed.Y * vecMoveSpeed.Y)) * 10) / 1.65) * 30;
            
            // Обновление пробега (если нужно)
            uint32_t ulCurrentTime = GetTickCount();
            static uint32_t ulLastUpdateTime = 0;
            static MATRIX4X4 matLastPos;
            
            if (ulCurrentTime - ulLastUpdateTime >= 1000) 
            {
                ulLastUpdateTime = ulCurrentTime;
                
                MATRIX4X4 matCurrentPos;
                pVehicle->GetMatrix(&matCurrentPos);
                
                if (ulLastUpdateTime > 0) 
                {
                    float fDistance = GetDistanceBetween3DPoints(&matCurrentPos.pos, &matLastPos.pos);
                    floatVehicleMillage += fDistance / 1000;
                }
                matLastPos = matCurrentPos;
            }
            
            // Топливо просто берется из переменной (без влияния на двигатель)
            int iFuelPercent = static_cast<int>((VehicleFuel / 100.0f) * 100.0f);
            iFuelPercent = std::max(0, std::min(100, iFuelPercent)); // Ограничиваем 0-100
            
            // Обновление спидометра
            CSpeedometr::UpdateDataSpeedometr(
                speed, 
                iFuelPercent,
                (int)(pVehicle->GetHealth() / 10), 
                (int)floatVehicleMillage, 
                false, 
                (int)pVehicle->GetLightsState(),
                (int)pVehicle->GetEngineState(),
                (int)pVehicle->GetDoorState()
            );
            
            bCurrentState = true;
        }
    }
}
    
    // Управление видимостью
    if(bCurrentState != bLastState) {
        if(bCurrentState) {
            CSpeedometr::ShowSpeedometr();
        } else {
            CSpeedometr::HideSpeedometr(true);
            iLastFuelPercent = -1; // Сброс при выходе из машины
        }
        bLastState = bCurrentState;
    }
}

CVehicle* lastVehicle = nullptr;
VEHICLEID ClosetSoFar = INVALID_VEHICLE_ID;
void CVehiclePool::Process()
{
	CVehicle* pVehicle;
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	CLocalPlayer* pLocalPlayer = pPlayerPool->GetLocalPlayer();
	
	float fLeastDistance = 10000.0f;
	float fThisDistance = 0.0f;

	for (VEHICLEID x: m_VehiclesCount)
	{
		if (GetSlotState(x))
		{
			pVehicle = m_pVehicles[x];
			if (m_bIsActive[x])
			{
				fThisDistance = pVehicle->GetDistanceFromLocalPlayerPed();
				if(fThisDistance < fLeastDistance)
				{
					fLeastDistance = fThisDistance;
					ClosetSoFar = x;
					lastVehicle = pVehicle;
				}
				
				if(pVehicle->m_pVehicle != m_pGTAVehicles[x])
					m_pGTAVehicles[x] = pVehicle->m_pVehicle;

				pVehicle->ProcessDamage(x);
				pVehicle->Process();
			}

				/*if(pVehicle->GetHealth() == 0.0f)
				{
						NotifyVehicleDeath(x);
						continue;
				}

				if(pVehicle->GetVehicleSubtype() != VEHICLE_SUBTYPE_BOAT &&
					pVehicle->GetDistanceFromLocalPlayerPed() < 200.0f &&
					pVehicle->HasSunk())
				{
					NotifyVehicleDeath(x);
					continue;
				}


				if(pVehicle->m_pVehicle != m_pGTAVehicles[x])
					m_pGTAVehicles[x] = pVehicle->m_pVehicle;

				pVehicle->ProcessMarkers();*/
		}
	}
	
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if (pPlayerPed && ClosetSoFar != INVALID_VEHICLE_ID && GetSlotState(ClosetSoFar) && m_bIsActive[ClosetSoFar] && lastVehicle)
	{
		lastVehicle->ProcessContact(ClosetSoFar);
	}
}
#include "..//game/CCustomPlateManager.h"
bool CVehiclePool::New(NEW_VEHICLE *pNewVehicle)
{
	if(m_pVehicles[pNewVehicle->VehicleID] != nullptr)
	{
		pChatWindow->AddDebugMessage(OBFUSCATE("Warning: vehicle %u was not deleted"), pNewVehicle->VehicleID);
		Delete(pNewVehicle->VehicleID);
	}
	
	int iTryCount = 0;
	if (!pGame->IsModelLoaded(pNewVehicle->iVehicleType)) 
	{
		pGame->RequestModel(pNewVehicle->iVehicleType);
		pGame->LoadRequestedModels();
		while(!pGame->IsModelLoaded(pNewVehicle->iVehicleType))
		{
			usleep(1);
			if(iTryCount > 20)
			{
				Log(OBFUSCATE("[VehiclePool] Warning: Model %u wouldn't load in time!"), pNewVehicle->iVehicleType);
				return false;
			}

			iTryCount++;
		}
	}

	m_pVehicles[pNewVehicle->VehicleID] = pGame->NewVehicle(pNewVehicle->iVehicleType,
		pNewVehicle->vecPos.X, pNewVehicle->vecPos.Y, pNewVehicle->vecPos.Z, 
		pNewVehicle->fRotation, pNewVehicle->byteAddSiren);

	if(m_pVehicles[pNewVehicle->VehicleID])
	{
		CCustomPlateManager::PushPlate(pNewVehicle->VehicleID, 1, "", "");
		
		// colors
		if(pNewVehicle->aColor1 != -1 || pNewVehicle->aColor2 != -1)
		{
			m_pVehicles[pNewVehicle->VehicleID]->SetColor(
				pNewVehicle->aColor1, pNewVehicle->aColor2 );
		}

		// health
		m_pVehicles[pNewVehicle->VehicleID]->SetHealth(pNewVehicle->fHealth);

		// gta handle
		m_pGTAVehicles[pNewVehicle->VehicleID] = m_pVehicles[pNewVehicle->VehicleID]->m_pVehicle;
		m_bVehicleSlotState[pNewVehicle->VehicleID] = true;

		// interior
		if(pNewVehicle->byteInterior > 0)
			LinkToInterior(pNewVehicle->VehicleID, pNewVehicle->byteInterior);

		// damage status
		if(pNewVehicle->dwPanelDamageStatus || 
			pNewVehicle->dwDoorDamageStatus || 
			pNewVehicle->byteLightDamageStatus)
		{
			m_pVehicles[pNewVehicle->VehicleID]->UpdateDamageStatus(
				pNewVehicle->dwPanelDamageStatus, 
				pNewVehicle->dwDoorDamageStatus,
				pNewVehicle->byteLightDamageStatus, pNewVehicle->byteTireDamageStatus);
		}
		
		if(pNetGame->m_bManualVehicleEngineAndLight)
		{
			m_pVehicles[pNewVehicle->VehicleID]->SetEngineState(0);
			m_pVehicles[pNewVehicle->VehicleID]->SetLightsState(0);
		}
		
		m_vecSpawnPos[pNewVehicle->VehicleID].X = pNewVehicle->vecPos.X;
		m_vecSpawnPos[pNewVehicle->VehicleID].Y = pNewVehicle->vecPos.Y;
		m_vecSpawnPos[pNewVehicle->VehicleID].Z = pNewVehicle->vecPos.Z;
		m_fSpawnRotation[pNewVehicle->VehicleID] = pNewVehicle->fRotation;

		m_pVehicles[pNewVehicle->VehicleID]->SetWheelPopped(pNewVehicle->byteTireDamageStatus);

		m_bIsActive[pNewVehicle->VehicleID] = true;
		m_bIsWasted[pNewVehicle->VehicleID] = false;
		
		m_VehiclesCount.push_back((int16_t)pNewVehicle->VehicleID);
		return true;
	}

	return false;
}

bool CVehiclePool::Delete(VEHICLEID VehicleID)
{
	if(!GetSlotState(VehicleID) || !m_pVehicles[VehicleID])
		return false;

	m_bIsActive[VehicleID] = false;
	m_bVehicleSlotState[VehicleID] = false;
	delete m_pVehicles[VehicleID];
	m_pVehicles[VehicleID] = nullptr;
	m_pGTAVehicles[VehicleID] = nullptr;

	m_VehiclesCount.remove((int16_t)VehicleID);
	return true;
}

VEHICLEID CVehiclePool::FindIDFromGtaPtr(VEHICLE_TYPE *pGtaVehicle)
{
	for (VEHICLEID x: m_VehiclesCount)
	{
		if(pGtaVehicle == m_pGTAVehicles[x]) return x;
	}

	return INVALID_VEHICLE_ID;
}

VEHICLEID CVehiclePool::FindIDFromRwObject(RwObject* pRWObject)
{
	for (VEHICLEID x: m_VehiclesCount)
	{
		if (m_pGTAVehicles[x])
		{
			if (pRWObject == (RwObject*)(m_pGTAVehicles[x]->entity.m_RwObject)) return x;
		}
	}

	return INVALID_VEHICLE_ID;
}

int CVehiclePool::FindGtaIDFromID(int iID)
{
	if(m_pGTAVehicles[iID])
		return GamePool_Vehicle_GetIndex(m_pGTAVehicles[iID]);
	else
		return INVALID_VEHICLE_ID;
}

int CVehiclePool::FindNearestToLocalPlayerPed()
{
	return ClosetSoFar;
}

void CVehiclePool::LinkToInterior(VEHICLEID VehicleID, int iInterior)
{
	if(m_bVehicleSlotState[VehicleID])
		m_pVehicles[VehicleID]->LinkToInterior(iInterior);
}

void CVehiclePool::NotifyVehicleDeath(VEHICLEID VehicleID)
{
	if(pNetGame->GetPlayerPool()->GetLocalPlayer()->m_LastVehicle != VehicleID) return;
	Log(OBFUSCATE("CVehiclePool::NotifyVehicleDeath"));

	RakNet::BitStream bsDeath;
	bsDeath.Write(VehicleID);
	pNetGame->GetRakClient()->RPC(&RPC_VehicleDestroyed, &bsDeath, HIGH_PRIORITY, RELIABLE_SEQUENCED, 0, false, UNASSIGNED_NETWORK_ID, NULL);
	pNetGame->GetPlayerPool()->GetLocalPlayer()->m_LastVehicle = INVALID_VEHICLE_ID;
}

void CVehiclePool::AssignSpecialParamsToVehicle(VEHICLEID VehicleID, uint8_t byteObjective, uint8_t byteDoorsLocked)
{
	if(!GetSlotState(VehicleID)) return;
	CVehicle *pVehicle = m_pVehicles[VehicleID];

	if(pVehicle && m_bIsActive[VehicleID])
	{
		if(byteObjective)
		{
			pVehicle->m_byteObjectiveVehicle = 1;
			pVehicle->m_bSpecialMarkerEnabled = false;
		}

		pVehicle->SetDoorState(byteDoorsLocked);
	}
}