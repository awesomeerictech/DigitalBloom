// AndroidServiceJNI.cpp
#include <jni.h>
#include <string>
#include <mutex>
#include <memory>
#include <map>
#include <android/log.h>

#define LOG_TAG "AndroidServiceJNI"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#include <QObject>
#include <QMetaObject>

// include your Qt/CPP server headers (ensure include paths set correctly)
#include "StreamServer.hpp"
#include "QxHttpServerController.h"
#include "servercontrollerwt.h"

// We keep shared pointers inside a map protected by mutex
static std::mutex g_mutex;
static std::map<int, std::shared_ptr<void>> g_servers;

// helpers to wrap QObject lifetime safely: create on Qt main thread if needed
static void start_qobject_on_main_thread(QObject* obj, const char* method) {
    // Use Qt metaobject invocation to call start on the main (GUI) thread
    if (!obj) return;
    QMetaObject::invokeMethod(obj, method, Qt::QueuedConnection);
}

extern "C" JNIEXPORT jint JNICALL
Java_org_stlltd_digitalbloom_DigitalBloomService_nativeStartServer(JNIEnv *env, jclass /*cls*/, jint serverId, jstring jIniPath, jstring jSslPath) {
    std::lock_guard<std::mutex> guard(g_mutex);
    int id = static_cast<int>(serverId);
    ALOGI("nativeStartServer id=%d", id);

    if (g_servers.find(id) != g_servers.end()) {
        ALOGI("server %d already started", id);
        return 0;
    }

    const char *iniPath = jIniPath ? env->GetStringUTFChars(jIniPath, nullptr) : nullptr;
    const char *sslPath = jSslPath ? env->GetStringUTFChars(jSslPath, nullptr) : nullptr;

    try {
        if (id == 0) {
            // StreamServer (Qt QObject)
            // Create on Qt main thread to ensure proper QObject affinity
            std::shared_ptr<StreamServer> ss;
            // create and start on main thread
            QMetaObject::invokeMethod(qApp, [&ss, iniPath, sslPath]() {
                ss = std::make_shared<StreamServer>(nullptr);
                // set ini/ssl via available API if you added them, otherwise StreamServer reads Downloads
                // e.g., ss->setIniPath(QString::fromUtf8(iniPath));
                ss->startServer();
            }, Qt::BlockingQueuedConnection);
            g_servers[id] = ss;
            ALOGI("StreamServer started");
        } else if (id == 1) {
            // QxHttpServerController
            std::shared_ptr<QxHttpServerController> ctrl;
            QMetaObject::invokeMethod(qApp, [&ctrl]() {
                ctrl = std::make_shared<QxHttpServerController>(nullptr);
                // if you want to apply stream IP: find StreamServer and call applyListenIpFrom
                QObject *streamObj = qApp->findChild<QObject*>("StreamServer");
                // Start controller (use the correct method name from your class)
                QMetaObject::invokeMethod(ctrl.get(), "startController", Qt::QueuedConnection);
            }, Qt::BlockingQueuedConnection);
            g_servers[id] = ctrl;
            ALOGI("QxHttpServerController started");
        } else if (id == 2) {
            // ServerControllerWT
            std::shared_ptr<ServerControllerWT> ctrl;
            QMetaObject::invokeMethod(qApp, [&ctrl]() {
                ctrl = std::make_shared<ServerControllerWT>(nullptr);
                QMetaObject::invokeMethod(ctrl.get(), "startController", Qt::QueuedConnection);
            }, Qt::BlockingQueuedConnection);
            g_servers[id] = ctrl;
            ALOGI("ServerControllerWT started");
        } else {
            ALOGE("unknown serverId: %d", id);
            if (iniPath) env->ReleaseStringUTFChars(jIniPath, iniPath);
            if (sslPath) env->ReleaseStringUTFChars(jSslPath, sslPath);
            return -2;
        }
    } catch (const std::exception &ex) {
        ALOGE("exception: %s", ex.what());
        if (iniPath) env->ReleaseStringUTFChars(jIniPath, iniPath);
        if (sslPath) env->ReleaseStringUTFChars(jSslPath, sslPath);
        return -3;
    } catch (...) {
        ALOGE("unknown exception while starting server %d", id);
        if (iniPath) env->ReleaseStringUTFChars(jIniPath, iniPath);
        if (sslPath) env->ReleaseStringUTFChars(jSslPath, sslPath);
        return -4;
    }

    if (iniPath) env->ReleaseStringUTFChars(jIniPath, iniPath);
    if (sslPath) env->ReleaseStringUTFChars(jSslPath, sslPath);
    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_org_stlltd_digitalbloom_DigitalBloomService_nativeStopServer(JNIEnv *env, jclass /*cls*/, jint serverId) {
    std::lock_guard<std::mutex> guard(g_mutex);
    int id = static_cast<int>(serverId);
    ALOGI("nativeStopServer id=%d", id);

    auto it = g_servers.find(id);
    if (it == g_servers.end()) {
        ALOGI("server %d not running", id);
        return 0;
    }

    try {
        if (id == 0) {
            auto ss = std::static_pointer_cast<StreamServer>(it->second);
            if (ss) {
                // Stop via Qt main thread
                QMetaObject::invokeMethod(ss.get(), "stopServer", Qt::BlockingQueuedConnection);
            }
        } else if (id == 1) {
            auto ctrl = std::static_pointer_cast<QxHttpServerController>(it->second);
            if (ctrl) QMetaObject::invokeMethod(ctrl.get(), "stopController", Qt::BlockingQueuedConnection);
        } else if (id == 2) {
            auto ctrl = std::static_pointer_cast<ServerControllerWT>(it->second);
            if (ctrl) QMetaObject::invokeMethod(ctrl.get(), "stopController", Qt::BlockingQueuedConnection);
        }
    } catch (const std::exception &ex) {
        ALOGE("exception while stopping: %s", ex.what());
    } catch (...) {
        ALOGE("unknown exception while stopping server %d", id);
    }

    g_servers.erase(it);
    ALOGI("server %d stopped", id);
    return 0;
}
