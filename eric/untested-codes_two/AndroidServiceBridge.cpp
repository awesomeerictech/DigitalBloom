/*!
 * DigitalBloom Qt JNI Bridge
 * Implements the native C++ functions declared in DigitalBloomService.java
 * by calling the appropriate methods on the ServerManager singleton.
 * This simplified file is the JNI glue using the two-call architecture.
 */

#include <jni.h>
#include <QtGlobal>

#if defined(Q_OS_ANDROID)

#include <QJniObject>
#include <QJniEnvironment>
#include <QtCore/private/qandroidextras_p.h>
#include "ServerManager.h" // Assumed header for ServerManager singleton

// Android logging helper using QDebug
#define LOG_TAG "DigitalBloomJNI"
#include <QDebug>
#define LOG_ANDROID(level, ...) qDebug() << LOG_TAG << ":" << level << ":" << __VA_ARGS__

// --- JNI Implementations (Called from Java) ---

extern "C" JNIEXPORT void JNICALL
Java_org_stlltd_digitalbloom_DigitalBloomService_nativeStartAllServers(JNIEnv *env, jobject thiz)
{
    LOG_ANDROID("INFO", "JNI: nativeStartAllServers() called. Triggering ServerManager::startAllServers().");
    
    // Call the C++ singleton method to start all servers (1, 2, and 3 in sequence)
    ServerManager::instance()->startAllServers();
}

extern "C" JNIEXPORT void JNICALL
Java_org_stlltd_digitalbloom_DigitalBloomService_nativeStopAllServers(JNIEnv *env, jobject thiz)
{
    LOG_ANDROID("INFO", "JNI: nativeStopAllServers() called. Triggering ServerManager::stopAllServers().");
    
    // Call the C++ singleton method to safely shut down all servers
    ServerManager::instance()->stopAllServers();
}

#endif // Q_OS_ANDROID