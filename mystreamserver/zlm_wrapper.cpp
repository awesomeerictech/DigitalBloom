// zlm_wrapper.cpp
// compile with -std=c++11

#include "zlm_wrapper_api.h"

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <memory>
#include <map>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>

#include "Util/MD5.h"
#include "Util/logger.h"
#include "Util/SSLBox.h"
#include "Util/onceToken.h"
#include "Network/TcpServer.h"
#include "Poller/EventPoller.h"

#include "Common/config.h"
#include "Rtsp/UDPServer.h"
#include "Rtsp/RtspSession.h"
#include "Rtmp/RtmpSession.h"
#include "Shell/ShellSession.h"
#include "Rtmp/FlvMuxer.h"
#include "Player/PlayerProxy.h"
#include "Http/WebSocketSession.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;

// preserve the exact port/default block you specified
namespace mediakit {
namespace Http {
#define HTTP_FIELD "http."
#define HTTP_PORT 8184
const string kPort = HTTP_FIELD"port";
#define HTTPS_PORT 4184
const string kSSLPort = HTTP_FIELD"sslport";
onceToken token1([](){
    mINI::Instance()[kPort] = HTTP_PORT;
    mINI::Instance()[kSSLPort] = HTTPS_PORT;
},nullptr);
} // Http

namespace Shell {
#define SHELL_FIELD "shell."
#define SHELL_PORT 9000
const string kPort = SHELL_FIELD"port";
onceToken token_shell([](){
    mINI::Instance()[kPort] = SHELL_PORT;
},nullptr);
} // Shell

namespace Rtsp {
#define RTSP_FIELD "rtsp."
#define RTSP_PORT 554
#define RTSPS_PORT 322
const string kPort = RTSP_FIELD"port";
const string kSSLPort = RTSP_FIELD"sslport";
onceToken token_rtsp([](){
    mINI::Instance()[kPort] = RTSP_PORT;
    mINI::Instance()[kSSLPort] = RTSPS_PORT;
},nullptr);
} // Rtsp

namespace Rtmp {
#define RTMP_FIELD "rtmp."
#define RTMP_PORT 1938
#define RTMPS_PORT 19380
const string kPort = RTMP_FIELD"port";
const string kSSLPort = RTMP_FIELD"sslport";
onceToken token_rtmp([](){
    mINI::Instance()[kPort] = RTMP_PORT;
    mINI::Instance()[kSSLPort] = RTMPS_PORT;
},nullptr);
} // Rtmp
} // mediakit

#define REALM "STREAM SERVER"
static map<string,FlvRecorder::Ptr> s_mapFlvRecorder;
static mutex s_mtxFlvRecorder;

// prefer toolkit::File if available; fallback to std::ifstream
static bool file_exists_portable(const std::string &path) {
#if 1
    // try to use toolkit::File if symbol exists at compile time
    // many ZLToolKit versions provide File::is_file or File::file_exist - try both via SFINAE not possible here,
    // so use the most common: File::is_file (if not present, compile will fail and you'll switch to fallback)
    // To be safe we attempt std::ifstream always:
    std::ifstream ifs(path.c_str());
    return ifs.good();
#else
    // alternative: return File::is_file(path); // enable if your File API exactly matches
#endif
}

// helper: check whether an IP string is loopback or wildcard
static bool is_loopback_or_wildcard(const std::string &ip) {
    if (ip.empty()) return true;
    // IPv4 wildcard
    if (ip == "0.0.0.0") return true;
    // IPv6 wildcard
    if (ip == "::" || ip == "::0") return true;
    // loopback IPv4
    if (ip.rfind("127.", 0) == 0) return true;
    // loopback IPv6
    if (ip == "::1") return true;
    return false;
}

// initEventListener preserved (use your test_server.cpp listeners)
// (same as previously provided but using only public interfaces used in test_server.cpp)
void initEventListener() {
    static onceToken s_token([]() {
        // OnGetRtspRealm
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastOnGetRtspRealm,
                                             [](BroadcastOnGetRtspRealmArgs){
                                                 DebugL << "OnGetRtspRealm: " << args.getUrl() << " " << args.params;
                                                 if (string("1") == args.stream) {
                                                     invoker(REALM);
                                                 } else {
                                                     invoker("");
                                                 }
                                             });

        // OnRtspAuth
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastOnRtspAuth,
                                             [](BroadcastOnRtspAuthArgs){
                                                 DebugL << "OnRtspAuth: " << args.getUrl() << " " << args.params;
                                                 string user = user_name;
                                                 if (user == "test0") { invoker(false, "pwd0"); return; }
                                                 if (user == "test1") { auto encrypted_pwd = MD5(user + ":" + REALM + ":" + "pwd1").hexdigest(); invoker(true, encrypted_pwd); return; }
                                                 if (user == "test2" && must_no_encrypt) { invoker(true, "pwd2"); return; }
                                                 invoker(false, user);
                                             });

        // MediaPublish
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPublish,
                                             [](BroadcastMediaPublishArgs){
                                                 DebugL << "MediaPublish: " << args.getUrl() << " " << args.params;
                                                 invoker("", ProtocolOption());
                                             });

        // MediaPlayed
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPlayed,
                                             [](BroadcastMediaPlayedArgs){
                                                 DebugL << "MediaPlayed: " << args.getUrl() << " " << args.params;
                                                 invoker("");
                                             });

        // ShellLogin
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastShellLogin,
                                             [](BroadcastShellLoginArgs){
                                                 DebugL << "ShellLogin: " << user_name << " " << passwd;
                                                 invoker("");
                                             });

        // MediaChanged - flv recorder example
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaChanged,
                                             [](BroadcastMediaChangedArgs){
                                                 auto tuple = sender.getMediaTuple();
                                                 if (sender.getSchema() == RTMP_SCHEMA && tuple.app == "live") {
                                                     lock_guard<mutex> lck(s_mtxFlvRecorder);
                                                     auto key = tuple.shortUrl();
                                                     if (bRegist) {
                                                         DebugL << "Start FLV record: " << sender.getUrl();
                                                         GET_CONFIG(string, http_root, Http::kRootPath);
                                                         auto path = http_root + "/" + key + "_" + to_string(time(NULL)) + ".flv";
                                                         FlvRecorder::Ptr recorder(new FlvRecorder);
                                                         try {
                                                             recorder->startRecord(EventPollerPool::Instance().getPoller(),
                                                                                   dynamic_pointer_cast<RtmpMediaSource>(sender.shared_from_this()), path);
                                                             s_mapFlvRecorder[key] = recorder;
                                                         } catch (std::exception &ex) {
                                                             WarnL << ex.what();
                                                         }
                                                     } else {
                                                         s_mapFlvRecorder.erase(key);
                                                     }
                                                 }
                                             });

        // NotFoundStream
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastNotFoundStream,
                                             [](BroadcastNotFoundStreamArgs){
                                                 DebugL << "NotFoundStream: " << args.getUrl() << " " << args.params;
                                             });

        // FlowReport
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastFlowReport,
                                             [](BroadcastFlowReportArgs){
                                                 DebugL << "FlowReport: " << args.getUrl() << " " << args.params
                                                        << " bytes=" << totalBytes << " duration=" << totalDuration;
                                             });

    }, nullptr);
}

// zlm_handle_t impl (Option B, no signals)
struct zlm_handle_t {
    struct Impl;
    Impl* impl;
};

struct zlm_handle_t::Impl {
    std::thread thr;
    std::atomic_bool running{false};
    std::atomic_bool stopRequested{false};
    std::mutex mtx;

    std::string listenIp;
    uint16_t listenPort = 0;

    std::string iniPath;
    std::string downloadPath;
    std::string sslPath;

    // Set Http::kRootPath to downloads (if available) before start
    void applyDownloadsRootToConfig() {
        // try to use toolkit::File or std helpers to locate downloads? wrapper is standalone - cannot call Qt here.
        // We'll prefer an ini-provided kRootPath; if ini doesn't set, users call zlm_start_with_paths passing ini that sets kRootPath.
        // But we can set a default here if environment variable DOWNLOADS_PATH is set (optional).
        // For cross-platform correctness, we expect the Qt side to set ini/Http::kRootPath before calling start.
    }

    int start() {
        if (running.load()) return 1;
        stopRequested.store(false);

        const std::string ini = iniPath;
        const std::string download_dir = downloadPath;
        const std::string ssl = sslPath;
        const std::string ip  = listenIp;
        const uint16_t port   = listenPort;

        thr = std::thread([this, ini, download_dir, ssl, ip, port]() {
            try {



                // load configuration: prefer provided ini path from Qt
                if (!ini.empty()) {
                    InfoL << "zlm_wrapper: loading ini from: " << ini;
                    loadIniConfig(ini.c_str());
                } else {
                    InfoL << "zlm_wrapper: calling loadIniConfig(nullptr)";
                    loadIniConfig(nullptr);
                }

                // set Http::kRootPath if not already set by ini: (we rely on Qt side to set Http::kRootPath)
                // but if someone passed an env var "ZLM_DOWNLOADS_ROOT" we could use it (optional)
                // Keep this minimal here: do not try to call Qt functions in wrapper.

                initEventListener();

                // SSL
                if (!ssl.empty() && file_exists_portable(ssl)) {
                    SSL_Initor::Instance().loadCertificate(ssl.c_str());
                    SSL_Initor::Instance().trustCertificate(ssl.c_str());
                    SSL_Initor::Instance().ignoreInvalidCertificate(false);
                    InfoL << "zlm_wrapper: loaded SSL from: " << ssl;
                } else {
                    InfoL << "zlm_wrapper: ssl not provided or missing: " << ssl;
                }

                // Validate listen ip (must not be loopback/wildcard)
                if (!ip.empty() && is_loopback_or_wildcard(ip)) {
                    WarnL << "zlm_wrapper: refused to bind to loopback/wildcard address: " << ip;
                    running.store(false);
                    return;
                }

                std::string listen = ip; // empty => bind all (but we won't allow loopback-only)
                uint16_t shellPort = (uint16_t)mINI::Instance()[Shell::kPort];
                uint16_t rtspPort  = (uint16_t)mINI::Instance()[Rtsp::kPort];
                uint16_t rtspsPort = (uint16_t)mINI::Instance()[Rtsp::kSSLPort];
                uint16_t rtmpPort  = port ? port : (uint16_t)mINI::Instance()[Rtmp::kPort];
                uint16_t rtmpsPort = (uint16_t)mINI::Instance()[Rtmp::kSSLPort];
                uint16_t httpPort  = (uint16_t)mINI::Instance()[Http::kPort];
                uint16_t httpsPort = (uint16_t)mINI::Instance()[Http::kSSLPort];

                InfoL << "zlm_wrapper: starting servers: rtmp=" << rtmpPort << " rtmps=" << rtmpsPort << " rtsp=" << rtspPort
                      << " http=" << httpPort << " https=" << httpsPort << " listen=" << listen;

                // create servers
                // TcpServer::Ptr shellSrv(new TcpServer());
                // TcpServer::Ptr rtspSrv(new TcpServer());
                // TcpServer::Ptr rtmpSrv(new TcpServer());
                TcpServer::Ptr httpSrv(new TcpServer());
                // TcpServer::Ptr rtmpSslSrv(new TcpServer());
                // TcpServer::Ptr httpsSrv(new TcpServer());
                // TcpServer::Ptr rtspSslSrv(new TcpServer());

                // start: if listen empty -> bind all interfaces, but ensure not binding to pure loopback only
                if (listen.empty()) {
                    // WARNING: listen empty binds all interfaces (including loopback). Qt side should ensure this only happens if acceptable.
                    // shellSrv->start<ShellSession>(shellPort);
                    // rtspSrv->start<RtspSession>(rtspPort);
                    // rtmpSrv->start<RtmpSession>(rtmpPort);
                    httpSrv->start<HttpSession>(httpPort);
                    // if (rtspsPort) rtspSslSrv->start<RtspSessionWithSSL>(rtspsPort);
                    // if (httpsPort) httpsSrv->start<HttpsSession>(httpsPort);
                    // if (rtmpsPort) rtmpSslSrv->start<RtmpSessionWithSSL>(rtmpsPort);
                } else {
                    // shellSrv->start<ShellSession>(shellPort, listen);
                    // rtspSrv->start<RtspSession>(rtspPort, listen);
                    // rtmpSrv->start<RtmpSession>(rtmpPort, listen);
                    httpSrv->start<HttpSession>(httpPort, listen);
                    // if (rtspsPort) rtspSslSrv->start<RtspSessionWithSSL>(rtspsPort, listen);
                    // if (httpsPort) httpsSrv->start<HttpsSession>(httpsPort, listen);
                    // if (rtmpsPort) rtmpSslSrv->start<RtmpSessionWithSSL>(rtmpsPort, listen);
                }

                // reload config listener
                NoticeCenter::Instance().addListener(ReloadConfigTag, Broadcast::kBroadcastReloadConfig,
                                                     [&](BroadcastReloadConfigArgs) {
                                                         try {
                                                             // if (rtmpsPort != mINI::Instance()[Rtmp::kSSLPort].as<uint16_t>()) {
                                                             //     rtmpsPort = mINI::Instance()[Rtmp::kSSLPort];
                                                             //     rtmpSslSrv->start<RtmpSessionWithSSL>(rtmpsPort, listen);
                                                             //     InfoL << "Restart rtmps:" << rtmpsPort;
                                                             // }
                                                             // if (httpsPort != mINI::Instance()[Http::kSSLPort].as<uint16_t>()) {
                                                             //     httpsPort = mINI::Instance()[Http::kSSLPort];
                                                             //     httpsSrv->start<HttpsSession>(httpsPort, listen);
                                                             //     InfoL << "Restart https:" << httpsPort;
                                                             // }
                                                             // if (rtmpPort != mINI::Instance()[Rtmp::kPort].as<uint16_t>()) {
                                                             //     rtmpPort = mINI::Instance()[Rtmp::kPort];
                                                             //     rtmpSrv->start<RtmpSession>(rtmpPort, listen);
                                                             //     InfoL << "Restart rtmp:" << rtmpPort;
                                                             // }
                                                         } catch (std::exception &ex) {
                                                             WarnL << "reload error: " << ex.what();
                                                         }
                                                     });

                running.store(true);

                // main loop: poll stopRequested every 200ms
                while (!stopRequested.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }

                // cleanup: reset shared_ptrs so destructors close listen sockets
                // httpsSrv.reset();
                // rtmpSslSrv.reset();
                // rtmpSrv.reset();
                // rtspSrv.reset();
                // rtspSslSrv.reset();
                // shellSrv.reset();
                httpSrv.reset();

                {
                    lock_guard<mutex> lck(s_mtxFlvRecorder);
                    s_mapFlvRecorder.clear();
                }

                running.store(false);
            } catch (std::exception &ex) {
                WarnL << "exception in wrapper thread: " << ex.what();
                running.store(false);
            } catch (...) {
                WarnL << "unknown exception in wrapper thread";
                running.store(false);
            }
        });

        return 0;
    }

    int stop() {
        if (!running.load()) return 0;
        stopRequested.store(true);
        if (thr.joinable()) thr.join();
        return 0;
    }
};

// C API
zlm_handle_t* zlm_create() {
    zlm_handle_t* h = (zlm_handle_t*)std::malloc(sizeof(zlm_handle_t));
    if (!h) return nullptr;
    h->impl = new zlm_handle_t::Impl();
    return h;
}

void zlm_destroy(zlm_handle_t* h) {
    if (!h) return;
    h->impl->stop();
    delete h->impl;
    std::free(h);
}

int zlm_start_with_paths(zlm_handle_t* h, const char* ini_path,const char* download_path, const char* ssl_pem_path) {
    if (!h) return -1;
    if (ini_path) h->impl->iniPath = std::string(ini_path);
    if (download_path) h->impl->downloadPath = std::string(download_path);
    if (ssl_pem_path) h->impl->sslPath = std::string(ssl_pem_path);
    return h->impl->start();
}

int zlm_stop(zlm_handle_t* h) {
    if (!h) return -1;
    return h->impl->stop();
}

int zlm_set_listen(zlm_handle_t* h, const char* ip, uint16_t port) {
    if (!h) return -1;
    std::string sip = ip ? std::string(ip) : std::string();
    if (!sip.empty() && is_loopback_or_wildcard(sip)) {
        WarnL << "zlm_set_listen: refused to set loopback/wildcard IP: " << sip;
        return -1; // invalid
    }
    if (ip) h->impl->listenIp = sip;
    if (port) h->impl->listenPort = port;

    if (h->impl->running.load()) {
        h->impl->stop();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        h->impl->start();
    }
    return 0;
}

int zlm_is_running(zlm_handle_t* h) {
    if (!h) return 0;
    return h->impl->running.load() ? 1 : 0;
}
