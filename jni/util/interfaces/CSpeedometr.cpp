#include <GLES2/gl2.h>
#include "util/CJavaWrapper.h"
#include "../main.h"
#include "../keyboard.h"
#include "../chatwindow.h"
#include "../CSettings.h"
#include "../net/netgame.h"
#include "CSpeedometr.h"

extern CKeyBoard* pKeyBoard;
extern CChatWindow* pChatWindow;
extern CSettings* pSettings;
extern CNetGame* pNetGame;

jobject CSpeedometr::activity;
uint32_t CSpeedometr::CustomTickRate = 0;

void CSpeedometr::ShowSpeedometr()
{
    JNIEnv* env = g_pJavaWrapper->GetEnv();
    if (!env) return;
    
    jclass clazz = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(clazz, "showSpeedometr", "()V");
    env->CallVoidMethod(activity, method);
    env->DeleteLocalRef(clazz);
}

void CSpeedometr::HideSpeedometr(bool b)
{
    JNIEnv* env = g_pJavaWrapper->GetEnv();
    if (!env) return;
    
    jclass clazz = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(clazz, "hideSpeedometr", "(Z)V");
    env->CallVoidMethod(activity, method, b);
    env->DeleteLocalRef(clazz);
}

void CSpeedometr::UpdateDataSpeedometr(int Speed, int Fuel, int Health, int Milleage, 
                                     bool AlarmStatus, int Lights, int Engine, int Doors)
{
    JNIEnv* env = g_pJavaWrapper->GetEnv();
    if (!env) return;
    
    jclass clazz = env->GetObjectClass(activity);
    jmethodID method = env->GetMethodID(clazz, "updateDataSpeedometr", "(IIIIZIII)V");
    env->CallVoidMethod(activity, method, Speed, Fuel, Health, Milleage, AlarmStatus, Lights, Engine, Doors);
    env->DeleteLocalRef(clazz);
}

extern "C"
{
    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_onSpeedometrOpened(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame && pNetGame->GetRakClient()) {
                RakNet::BitStream bsParams;
                bsParams.Write((uint8_t)ID_CUSTOM_RPC);
                bsParams.Write((uint32_t)RPC_ModePlayer);
                bsParams.Write((uint8_t)1);
                pNetGame->GetRakClient()->Send(&bsParams, HIGH_PRIORITY, RELIABLE, 0);
            }
        }
    }
    
    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_onSpeedometrClicked(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame && pNetGame->GetRakClient()) {
                RakNet::BitStream bsParams;
                bsParams.Write((uint8_t)ID_CUSTOM_RPC);
                bsParams.Write((uint32_t)RPC_ModePlayer);
                bsParams.Write((uint8_t)1);
                pNetGame->GetRakClient()->Send(&bsParams, HIGH_PRIORITY, RELIABLE, 0);
            }
        }
    }

    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_toggleEngine(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame) {
                pNetGame->SendChatCommand("/en");
            }
        }
    }

    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_toggleLights(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame) {
                pNetGame->SendChatCommand("/light");
            }
        }
    }

    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_toggleDoors(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame) {
                pNetGame->SendChatCommand("/lk");
            }
        }
    }

    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_leftArrowPressed(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame) {
              //  pNetGame->SendChatCommand("/l");
              pNetGame->SendChatCommand("/left");
            }
        }
    }

    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_emergencyPressed(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame) {
                pNetGame->SendChatCommand("/emergency");
            }
        }
    }

    JNIEXPORT void JNICALL Java_com_rockstargames_gui_speedometr_SpeedometrManager_rightArrowPressed(
        JNIEnv* pEnv, jobject thiz)
    {
        if (GetTickCount() - CSpeedometr::CustomTickRate > 500)
        {
            CSpeedometr::CustomTickRate = GetTickCount();
            if(pNetGame) {
               // pNetGame->SendChatCommand("/r");
               pNetGame->SendChatCommand("/right");
            }
        }
    }
}

void CSpeedometr::setActivity(JNIEnv* env, jobject thiz)
{
    activity = env->NewGlobalRef(thiz);
}