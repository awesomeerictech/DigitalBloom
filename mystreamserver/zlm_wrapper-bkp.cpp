// zlm_wrapper.cpp
// Build with -std=c++11

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

// ---------------------------------------------------------------------------
// keep your exact port/default block (copied from test_server.cpp)
namespace mediakit {
////////////HTTP configuration///////////
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
}//namespace Http

////////////SHELL configuration///////////
namespace Shell {
#define SHELL_FIELD "shell."
#define SHELL_PORT 9000
const string kPort = SHELL_FIELD"port";
onceToken token1([](){
    mINI::Instance()[kPort] = SHELL_PORT;
},nullptr);
} //namespace Shell

////////////RTSP server configuration///////////
namespace Rtsp {
#define RTSP_FIELD "rtsp."
#define RTSP_PORT 554
#define RTSPS_PORT 322
const string kPort = RTSP_FIELD"port";
const string kSSLPort = RTSP_FIELD"sslport";
onceToken token1([](){
    mINI::Instance()[kPort] = RTSP_PORT;
    mINI::Instance()[kSSLPort] = RTSPS_PORT;
},nullptr);

} //namespace Rtsp

////////////RTMP server configuration///////////
namespace Rtmp {
#define RTMP_FIELD "rtmp."
#define RTMP_PORT 1938
#define RTMPS_PORT 19380
const string kPort = RTMP_FIELD"port";
const string kSSLPort = RTMP_FIELD"sslport";
onceToken token1([](){
    mINI::Instance()[kPort] = RTMP_PORT;
    mINI::Instance()[kSSLPort] = RTMPS_PORT;
},nullptr);

} //namespace RTMP

}  // namespace mediakit

#define REALM "STREAM SERVER"
static map<string,FlvRecorder::Ptr> s_mapFlvRecorder;
static mutex s_mtxFlvRecorder;

// portable file exists helper
static bool file_exists(const std::string &path) {
    std::ifstream ifs(path.c_str());
    return ifs.good();
}

// ---------------------------------------------------------------------------
// initEventListener implemented as in your test_server.cpp — exact APIs used
// ---------------------------------------------------------------------------
void initEventListener() {
    static onceToken s_token([]() {
        // Listen for kBroadcastOnGetRtspRealm event to decide if rtsp link needs authentication (traditional rtsp authentication scheme)
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastOnGetRtspRealm, [](BroadcastOnGetRtspRealmArgs) {
            DebugL << "RTSP authentication required event:" << args.getUrl() << " " << args.params;
            if (string("1") == args.stream) {
                // live/1 needs authentication
                // This stream needs authentication and sets realm
                invoker(REALM);
            } else {
                // Sometimes we need to query redis or database to determine if the stream needs authentication, which can be done asynchronously through invoker
                // This stream does not need authentication
                invoker("");
            }
        });

        // Listen for kBroadcastOnRtspAuth event to return the correct rtsp authentication username and password
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastOnRtspAuth, [](BroadcastOnRtspAuthArgs) {
            DebugL << "RTSP playback authentication:" << args.getUrl() << " " << args.params;
            DebugL << "RTSP user:" << user_name << (must_no_encrypt ? " Base64" : " MD5") << " login method";
            string user = user_name;
            // Assuming we read the database asynchronously
            if (user == "test0") {
                // Assuming the database stores plaintext
                invoker(false, "pwd0");
                return;
            }

            if (user == "test1") {
                // Assuming the database stores ciphertext
                auto encrypted_pwd = MD5(user + ":" + REALM + ":" + "pwd1").hexdigest();
                invoker(true, encrypted_pwd);
                return;
            }
            if (user == "test2" && must_no_encrypt) {
                // Assuming login is test2 and login is in base64 format; providing an encrypted password will cause authentication failure
                // You can block insecure encryption methods like base64 this way
                invoker(true, "pwd2");
                return;
            }

            // For other users, password equals username
            invoker(false, user);
        });


        // Listen for rtsp/rtmp publish event, return result to indicate whether publish permission is granted
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPublish, [](BroadcastMediaPublishArgs) {
            DebugL << "Publish authentication:" << args.getUrl() << " " << args.params;
            invoker("", ProtocolOption()); // authentication success
            // invoker("this is auth failed message"); // authentication failed
        });

        // Listen for rtsp/rtsps/rtmp/http-flv playback event, return result to indicate whether playback permission is granted
        // (rtsp can implement authentication via kBroadcastOnRtspAuth or this event)
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPlayed, [](BroadcastMediaPlayedArgs) {
            DebugL << "Playback authentication:" << args.getUrl() << " " << args.params;
            invoker(""); // authentication success
            // invoker("this is auth failed message"); // authentication failed
        });

        // Shell login event — allows logging into the server via shell to execute commands
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastShellLogin, [](BroadcastShellLoginArgs) {
            DebugL << "Shell login:" << user_name << " " << passwd;
            invoker(""); // authentication success
            // invoker("this is auth failed message"); // authentication failed
        });

        // Listen for rtsp/rtmp source registration or unregistration; used here to test saving RTMP as FLV recordings in the HTTP root directory
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaChanged, [](BroadcastMediaChangedArgs) {
            auto tuple = sender.getMediaTuple();
            if (sender.getSchema() == RTMP_SCHEMA && tuple.app == "live") {
                lock_guard<mutex> lck(s_mtxFlvRecorder);
                auto key = tuple.shortUrl();
                if (bRegist) {
                    DebugL << "Start recording RTMP:" << sender.getUrl();
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

        // Listen for playback failure (stream not found) event
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastNotFoundStream, [](BroadcastNotFoundStreamArgs) {
            /**
             * You can pull the stream again when this event is triggered to implement on-demand pulling.
             * After successfully pulling the stream, ZLMediaKit will immediately forward it to the player
             * (maximum waiting time is about 5 seconds — if pulling still fails after 5 seconds, the player will fail).
             */
            DebugL << "Stream not found event:" << args.getUrl() << " " << args.params;
        });


        // Listen for traffic-report events when playback or publishing ends
        NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastFlowReport, [](BroadcastFlowReportArgs) {
            DebugL << "Player (publisher) disconnected event:" << args.getUrl() << " " << args.params
                   << "\r\nTraffic used:" << totalBytes << " bytes, connection duration:" << totalDuration << " seconds";
        });


    }, nullptr);
}


// Ensure SIGHUP exists
#if !defined(SIGHUP)
#define SIGHUP 1
#endif

// ---------------------------------------------------------------------------
// Wrapper implementation (Option B: no signals, non-blocking thread)
// ---------------------------------------------------------------------------
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
    std::string sslPath;

    int start() {
        if (running.load()) return 1;
        stopRequested.store(false);

        const std::string ini = iniPath;
        const std::string ssl = sslPath;
        const std::string ip  = listenIp;
        const uint16_t port   = listenPort;

        thr = std::thread([this, ini, ssl, ip, port]() {
            try {
                Logger::Instance().add(std::make_shared<ConsoleChannel>());
                Logger::Instance().add(std::make_shared<FileChannel>());
                Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

                // load ini (prefer provided ini)
                if (!ini.empty()) {
                    InfoL << "zlm_wrapper: loading ini from: " << ini;
                    loadIniConfig(ini.c_str());
                } else {
                    InfoL << "zlm_wrapper: loadIniConfig(nullptr)";
                    loadIniConfig(nullptr);
                }

                // event listeners
                initEventListener();

                // SSL init if file exists
                if (!ssl.empty() && file_exists(ssl)) {
                    SSL_Initor::Instance().loadCertificate(ssl.c_str());
                    SSL_Initor::Instance().trustCertificate(ssl.c_str());
                    SSL_Initor::Instance().ignoreInvalidCertificate(false);
                    InfoL << "zlm_wrapper: loaded SSL from: " << ssl;
                } else {
                    InfoL << "zlm_wrapper: SSL path not provided or missing; skipping SSL init";
                }

                std::string listen = ip; // empty -> bind all

                uint16_t shellPort = (uint16_t)mINI::Instance()[Shell::kPort];
                uint16_t rtspPort  = (uint16_t)mINI::Instance()[Rtsp::kPort];
                uint16_t rtspsPort = (uint16_t)mINI::Instance()[Rtsp::kSSLPort];
                uint16_t rtmpPort  = port ? port : (uint16_t)mINI::Instance()[Rtmp::kPort];
                uint16_t rtmpsPort = (uint16_t)mINI::Instance()[Rtmp::kSSLPort];
                uint16_t httpPort  = (uint16_t)mINI::Instance()[Http::kPort];
                uint16_t httpsPort = (uint16_t)mINI::Instance()[Http::kSSLPort];

                InfoL << "zlm_wrapper: starting servers: rtmp=" << rtmpPort << " rtmps=" << rtmpsPort << " rtsp=" << rtspPort
                      << " http=" << httpPort << " https=" << httpsPort;

                // create server objects
                TcpServer::Ptr shellSrv(new TcpServer());
                TcpServer::Ptr rtspSrv(new TcpServer());
                TcpServer::Ptr rtmpSrv(new TcpServer());
                TcpServer::Ptr httpSrv(new TcpServer());
                TcpServer::Ptr rtmpSslSrv(new TcpServer());
                TcpServer::Ptr httpsSrv(new TcpServer());
                TcpServer::Ptr rtspSslSrv(new TcpServer());

                // start servers: if listen empty -> bind all, else bind ip
                if (listen.empty()) {
                    shellSrv->start<ShellSession>(shellPort);
                    rtspSrv->start<RtspSession>(rtspPort);
                    rtmpSrv->start<RtmpSession>(rtmpPort);
                    httpSrv->start<HttpSession>(httpPort);
                    if (rtspsPort) rtspSslSrv->start<RtspSessionWithSSL>(rtspsPort);
                    if (httpsPort) httpsSrv->start<HttpsSession>(httpsPort);
                    if (rtmpsPort) rtmpSslSrv->start<RtmpSessionWithSSL>(rtmpsPort);
                } else {
                    shellSrv->start<ShellSession>(shellPort, listen);
                    rtspSrv->start<RtspSession>(rtspPort, listen);
                    rtmpSrv->start<RtmpSession>(rtmpPort, listen);
                    httpSrv->start<HttpSession>(httpPort, listen);
                    if (rtspsPort) rtspSslSrv->start<RtspSessionWithSSL>(rtspsPort, listen);
                    if (httpsPort) httpsSrv->start<HttpsSession>(httpsPort, listen);
                    if (rtmpsPort) rtmpSslSrv->start<RtmpSessionWithSSL>(rtmpsPort, listen);
                }

                // reload config listener (restart by calling start again on servers as test_server.cpp does)
                NoticeCenter::Instance().addListener(ReloadConfigTag, Broadcast::kBroadcastReloadConfig, [&](BroadcastReloadConfigArgs) {
                    try {
                        if (shellPort != mINI::Instance()[Shell::kPort].as<uint16_t>()) {
                            shellPort = mINI::Instance()[Shell::kPort];
                            shellSrv->start<ShellSession>(shellPort, listen);
                            InfoL << "Restart the shell server:" << shellPort;
                        }
                        if (rtspPort != mINI::Instance()[Rtsp::kPort].as<uint16_t>()) {
                            rtspPort = mINI::Instance()[Rtsp::kPort];
                            rtspSrv->start<RtspSession>(rtspPort, listen);
                            InfoL << "Restart rtsp server" << rtspPort;
                        }
                        if (rtmpPort != mINI::Instance()[Rtmp::kPort].as<uint16_t>()) {
                            rtmpPort = mINI::Instance()[Rtmp::kPort];
                            rtmpSrv->start<RtmpSession>(rtmpPort, listen);
                            InfoL << "Restart rtmp server" << rtmpPort;
                        }
                        if (httpPort != mINI::Instance()[Http::kPort].as<uint16_t>()) {
                            httpPort = mINI::Instance()[Http::kPort];
                            httpSrv->start<HttpSession>(httpPort, listen);
                            InfoL << "Restart http server" << httpPort;
                        }
                        if (httpsPort != mINI::Instance()[Http::kSSLPort].as<uint16_t>()) {
                            httpsPort = mINI::Instance()[Http::kSSLPort];
                            httpsSrv->start<HttpsSession>(httpsPort, listen);
                            InfoL << "Restart https server" << httpsPort;
                        }
                        if (rtspsPort != mINI::Instance()[Rtsp::kSSLPort].as<uint16_t>()) {
                            rtspsPort = mINI::Instance()[Rtsp::kSSLPort];
                            rtspSslSrv->start<RtspSessionWithSSL>(rtspsPort, listen);
                            InfoL << "Restart rtsps server" << rtspsPort;
                        }
                        if (rtmpsPort != mINI::Instance()[Rtmp::kSSLPort].as<uint16_t>()) {
                            rtmpsPort = mINI::Instance()[Rtmp::kSSLPort];
                            rtmpSslSrv->start<RtmpSessionWithSSL>(rtmpsPort, listen);
                            InfoL << "Restart rtmps server" << rtmpsPort;
                        }
                    } catch (std::exception &ex) {
                        WarnL << "zlm_wrapper: reload error: " << ex.what();
                    }
                });

                running.store(true);

                // main loop: poll stopRequested
                while (!stopRequested.load()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }

                // request cleanup: reset shared_ptrs so destructors close sockets/listeners
                try { httpsSrv.reset(); } catch(...) {}
                try { rtmpSslSrv.reset(); } catch(...) {}
                try { rtmpSrv.reset(); } catch(...) {}
                try { rtspSrv.reset(); } catch(...) {}
                try { rtspSslSrv.reset(); } catch(...) {}
                try { shellSrv.reset(); } catch(...) {}
                try { httpSrv.reset(); } catch(...) {}

                // clear flv recorder map
                {
                    lock_guard<mutex> lck(s_mtxFlvRecorder);
                    s_mapFlvRecorder.clear();
                }

                running.store(false);

            } catch (const std::exception &ex) {
                WarnL << "zlm_wrapper: exception in wrapper thread: " << ex.what();
                running.store(false);
            } catch (...) {
                WarnL << "zlm_wrapper: unknown exception in wrapper thread";
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

// ---------------------------------------------------------------------------
// C API
// ---------------------------------------------------------------------------
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

int zlm_start_with_paths(zlm_handle_t* h, const char* ini_path, const char* ssl_pem_path) {
    if (!h) return -1;
    if (ini_path) h->impl->iniPath = std::string(ini_path);
    if (ssl_pem_path) h->impl->sslPath = std::string(ssl_pem_path);
    return h->impl->start();
}

int zlm_stop(zlm_handle_t* h) {
    if (!h) return -1;
    return h->impl->stop();
}

int zlm_set_listen(zlm_handle_t* h, const char* ip, uint16_t port) {
    if (!h) return -1;
    if (ip) h->impl->listenIp = std::string(ip);
    if (port) h->impl->listenPort = port;

    if (h->impl->running.load()) {
        // restart to apply changes (best-effort)
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
