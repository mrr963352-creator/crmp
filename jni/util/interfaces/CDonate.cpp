#include <GLES2/gl2.h>

#include "util/CJavaWrapper.h"
#include "../main.h"

#include "..//keyboard.h"
#include "..//chatwindow.h"
#include "..//CSettings.h"
#include "../net/netgame.h"

#include "CDonate.h"

extern CKeyBoard* pKeyBoard;
extern CChatWindow* pChatWindow;
extern CSettings* pSettings;
extern CNetGame* pNetGame;

jobject CDonate::activity;

void CNetGame::packetDonate(Packet *p)
{
    Log("Processing donate packet");
    RakNet::BitStream bs((unsigned char*)p->data, p->length, false);

    bs.IgnoreBits(40); // skip packet and rpc id

    int8_t typeInfo;
    bs.Read(typeInfo);
    Log("Donate packet type: %d", typeInfo);

    if (typeInfo == 0) 
    {
        Log("Opening donate interface");
        CDonate::openDonate();
    }

    else if (typeInfo == 1) 
    {
        Log("Closing donate interface");
        CDonate::closeDonate();
    }
    else if (typeInfo == 2)
    {
        uint8_t max_items;
        bs.Read(max_items);
        Log("Adding %d donate categories", max_items);
        
        for (int i = 0; i < max_items; i ++)
        {
            int8_t icon;
            bs.Read(icon);
            
            char text[256];
            memset(text, 0, 256);
            
            uint8_t bLen;
            bs.Read(bLen);
            bs.Read(text, bLen);
            text[bLen] = '\0';
            
            CDonate::donateAddCategory(icon, text);
        }
    }
    else if (typeInfo == 3)
    {
        uint8_t max_items;
        bs.Read(max_items);
        Log("Adding %d donate items", max_items);
        
        for (int q = 0; q < max_items; q ++)
        {
            int16_t position;
            bs.Read(position);
            
            int8_t type;
            bs.Read(type);
            
            int32_t model;
            bs.Read(model);
            
            uint8_t color1;
            bs.Read(color1);
            
            uint8_t color2;
            bs.Read(color2);
            
            float rotX;
            bs.Read(rotX);
            
            float rotY;
            bs.Read(rotY);
            
            float rotZ;
            bs.Read(rotZ);
            
            float zoom;
            bs.Read(zoom);
            
            char text[3][256];
            for (int i = 0; i < 3; i ++) memset(text[i], 0, 256);
            for (int i = 0; i < 2; i ++)
            {
                uint8_t bLen;
                bs.Read(bLen);
                bs.Read(text[i], bLen);
                text[i][bLen] = '\0';
            }
            
            bool special;
            bs.Read(special);
            
            uint8_t bLen;
            bs.Read(bLen);
            bs.Read(text[2], bLen);
            text[2][bLen] = '\0';
            
            CDonate::donateAddItem(position, type, model, color1, color2, rotX, rotY, rotZ, zoom, text[0], text[1], special, text[2]);
        }
    }
    else if (typeInfo == 4) 
    {
        Log("Clearing donate items");
        CDonate::donateItemClear();
    }
    else if (typeInfo == 5)
    {
        char text[3][256];
        for (int i = 0; i < 3; i ++)
        {
            memset(text[i], 0, 256);
            
            uint8_t bLen;
            bs.Read(bLen);
            bs.Read(text[i], bLen);
            text[i][bLen] = '\0';
        }
        
        Log("Updating donate info");
        CDonate::donateUpdateInfo(text[0], text[1], text[2]);
    }
    Log("Finished processing donate packet");
}

void CDonate::openDonate()
{
    Log("[CDonate] Attempting to open donate interface");
    
    if(!g_pJavaWrapper) 
    {
        Log("[CDonate] Error: JavaWrapper is null");
        return;
    }

    JNIEnv* env = g_pJavaWrapper->GetEnv();
    if (!env)
    {
        Log("[CDonate] Error: Failed to get JNIEnv");
        return;
    }
    
    if(!activity) 
    {
        Log("[CDonate] Error: Activity is null");
        return;
    }

    jclass clazz = env->GetObjectClass(activity);
    if(!clazz) 
    {
        Log("[CDonate] Error: Failed to get activity class");
        return;
    }

    jmethodID method = env->GetMethodID(clazz, OBFUSCATE("openDonate"), OBFUSCATE("()V"));
    if(!method) 
    {
        Log("[CDonate] Error: openDonate method not found");
        env->DeleteLocalRef(clazz);
        return;
    }

    Log("[CDonate] Calling openDonate method");
    env->CallVoidMethod(activity, method);
    
    if (env->ExceptionCheck())
    {
        Log("[CDonate] Exception occurred during openDonate call");
        env->ExceptionClear();
    }
    
    Log("[CDonate] openDonate call completed");
    env->DeleteLocalRef(clazz);
}

void CDonate::closeDonate()
{
    Log("[CDonate] Closing donate interface");
    JNIEnv* env = g_pJavaWrapper->GetEnv();

    if (!env)
    {
        Log("[CDonate] Error: No JNIEnv");
        return;
    }
    
    if(!activity)
    {
        Log("[CDonate] Error: Activity is null");
        return;
    }

    jclass clazz = env->GetObjectClass(activity);
    if(!clazz)
    {
        Log("[CDonate] Error: Failed to get activity class");
        return;
    }

    jmethodID method = env->GetMethodID(clazz, OBFUSCATE("closeDonate"), OBFUSCATE("()V"));
    if(!method)
    {
        Log("[CDonate] Error: closeDonate method not found");
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(activity, method);
    
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
    }
    
    env->DeleteLocalRef(clazz);
}

void CDonate::donateAddCategory(int8_t icon, char* text)
{
    Log("[CDonate] Adding donate category: icon=%d, text=%s", icon, text);
    JNIEnv* env = g_pJavaWrapper->GetEnv();

    if (!env)
    {
        Log("[CDonate] Error: No JNIEnv");
        return;
    }

    jbyteArray bytes = env->NewByteArray(strlen(text));
    env->SetByteArrayRegion(bytes, 0, strlen(text), (jbyte*)text);
    
    jclass clazz = env->GetObjectClass(activity);
    if(!clazz)
    {
        Log("[CDonate] Error: Failed to get activity class");
        env->DeleteLocalRef(bytes);
        return;
    }

    jmethodID method = env->GetMethodID(clazz, OBFUSCATE("donateAddCategory"), OBFUSCATE("(I[B)V"));
    if(!method)
    {
        Log("[CDonate] Error: donateAddCategory method not found");
        env->DeleteLocalRef(bytes);
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(activity, method, icon, bytes);
    
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
    }
    
    env->DeleteLocalRef(bytes);
    env->DeleteLocalRef(clazz);
}

void CDonate::donateAddItem(int16_t position, int8_t type, int32_t model, uint8_t color1, uint8_t color2, float rotX, float rotY, float rotZ, float zoom, char* text, char* text2, bool special, char* text3)
{
    Log("[CDonate] Adding donate item: pos=%d, type=%d, model=%d", position, type, model);
    JNIEnv* env = g_pJavaWrapper->GetEnv();

    if (!env)
    {
        Log("[CDonate] Error: No JNIEnv");
        return;
    }

    jbyteArray bytes = env->NewByteArray(strlen(text));
    env->SetByteArrayRegion(bytes, 0, strlen(text), (jbyte*)text);

    jbyteArray bytes2 = env->NewByteArray(strlen(text2));
    env->SetByteArrayRegion(bytes2, 0, strlen(text2), (jbyte*)text2);

    jbyteArray bytes3 = env->NewByteArray(strlen(text3));
    env->SetByteArrayRegion(bytes3, 0, strlen(text3), (jbyte*)text3);
    
    jclass clazz = env->GetObjectClass(activity);
    if(!clazz)
    {
        Log("[CDonate] Error: Failed to get activity class");
        env->DeleteLocalRef(bytes);
        env->DeleteLocalRef(bytes2);
        env->DeleteLocalRef(bytes3);
        return;
    }

    jmethodID method = env->GetMethodID(clazz, OBFUSCATE("donateAddItem"), OBFUSCATE("(IIIIIFFFF[B[BZ[B)V"));
    if(!method)
    {
        Log("[CDonate] Error: donateAddItem method not found");
        env->DeleteLocalRef(bytes);
        env->DeleteLocalRef(bytes2);
        env->DeleteLocalRef(bytes3);
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(activity, method, position, type, model, color1, color2, rotX, rotY, rotZ, zoom, bytes, bytes2, special, bytes3);
    
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
    }
    
    env->DeleteLocalRef(bytes);
    env->DeleteLocalRef(bytes2);
    env->DeleteLocalRef(bytes3);
    env->DeleteLocalRef(clazz);
}

void CDonate::donateItemClear()
{
    Log("[CDonate] Clearing donate items");
    JNIEnv* env = g_pJavaWrapper->GetEnv();

    if (!env)
    {
        Log("[CDonate] Error: No JNIEnv");
        return;
    }
    
    jclass clazz = env->GetObjectClass(activity);
    if(!clazz)
    {
        Log("[CDonate] Error: Failed to get activity class");
        return;
    }

    jmethodID method = env->GetMethodID(clazz, OBFUSCATE("donateItemClear"), OBFUSCATE("()V"));
    if(!method)
    {
        Log("[CDonate] Error: donateItemClear method not found");
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(activity, method);
    
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
    }
    
    env->DeleteLocalRef(clazz);
}

void CDonate::donateUpdateInfo(char* text, char* text2, char* text3)
{
    Log("[CDonate] Updating donate info");
    JNIEnv* env = g_pJavaWrapper->GetEnv();

    if (!env)
    {
        Log("[CDonate] Error: No JNIEnv");
        return;
    }

    jbyteArray bytes = env->NewByteArray(strlen(text));
    env->SetByteArrayRegion(bytes, 0, strlen(text), (jbyte*)text);

    jbyteArray bytes2 = env->NewByteArray(strlen(text2));
    env->SetByteArrayRegion(bytes2, 0, strlen(text2), (jbyte*)text2);

    jbyteArray bytes3 = env->NewByteArray(strlen(text3));
    env->SetByteArrayRegion(bytes3, 0, strlen(text3), (jbyte*)text3);
    
    jclass clazz = env->GetObjectClass(activity);
    if(!clazz)
    {
        Log("[CDonate] Error: Failed to get activity class");
        env->DeleteLocalRef(bytes);
        env->DeleteLocalRef(bytes2);
        env->DeleteLocalRef(bytes3);
        return;
    }

    jmethodID method = env->GetMethodID(clazz, OBFUSCATE("donateUpdateInfo"), OBFUSCATE("([B[B[B)V"));
    if(!method)
    {
        Log("[CDonate] Error: donateUpdateInfo method not found");
        env->DeleteLocalRef(bytes);
        env->DeleteLocalRef(bytes2);
        env->DeleteLocalRef(bytes3);
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(activity, method, bytes, bytes2, bytes3);
    
    if (env->ExceptionCheck())
    {
        env->ExceptionClear();
    }
    
    env->DeleteLocalRef(bytes);
    env->DeleteLocalRef(bytes2);
    env->DeleteLocalRef(bytes3);
    env->DeleteLocalRef(clazz);
}

extern "C"
{
    JNIEXPORT void JNICALL Java_com_rockstargames_gui_donate_DonateManager_SendResponse(JNIEnv* pEnv, jobject thiz, jint i, jint i2, jint i3)
    {
        Log("[CDonate] Sending donate response: %d, %d, %d", i, i2, i3);
        if(!pNetGame)
        {
            Log("[CDonate] Error: pNetGame is null");
            return;
        }

        RakNet::BitStream bsParams;
        bsParams.Write((uint8_t)ID_CUSTOM_RPC);
        bsParams.Write((uint32_t)RPC_Donate);

        bsParams.Write((int32_t)i);
        bsParams.Write((int32_t)i2);
        bsParams.Write((int32_t)i3);
        
        if(!pNetGame->GetRakClient())
        {
            Log("[CDonate] Error: RakClient is null");
            return;
        }

        pNetGame->GetRakClient()->Send(&bsParams, HIGH_PRIORITY, RELIABLE, 0);
        Log("[CDonate] Response sent successfully");
    }
}

void CDonate::setActivity(JNIEnv* env, jobject thiz)
{
    Log("[CDonate] Setting activity");
    if(!env)
    {
        Log("[CDonate] Error: JNIEnv is null");
        return;
    }

    if(activity)
    {
        Log("[CDonate] Warning: Activity already set, deleting old reference");
        env->DeleteGlobalRef(activity);
    }

    activity = env->NewGlobalRef(thiz);
    if(!activity)
    {
        Log("[CDonate] Error: Failed to create global reference");
    }
    else
    {
        Log("[CDonate] Activity set successfully");
    }
}