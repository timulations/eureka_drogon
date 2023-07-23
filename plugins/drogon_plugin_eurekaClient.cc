/**
 *
 *  drogon_plugin_eurekaClient.cc
 *
 */

#include <drogon/drogon.h>
#include "drogon_plugin_eurekaClient.h"
#include "pugixml.hpp"

#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#else
/* TODO: implement ip address self-discovery for Windows */
#endif

using namespace drogon;
using namespace drogon::plugin;

const static std::unordered_map<std::string, eurekaState>  stateStr2stateEnum = {
    {"UP", eurekaState::UP},
    {"DOWN", eurekaState::DOWN},
    {"STARTING", eurekaState::STARTING},
    {"OUT_OF_SERVICE", eurekaState::OUT_OF_SERVICE},
    {"UNKNOWN", eurekaState::UNKNOWN}
};

static void sendHeartbeat(const std::string& eurekaHost, const std::string& eurekaPort, const std::string& appName, const std::string& appHostName)
{
    auto client = HttpClient::newHttpClient("http://" + eurekaHost + ":" + eurekaPort);
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Put);
    req->setPath("/eureka/apps/" + appName + "/" + appHostName);
    client->sendRequest(req, [](ReqResult result, const HttpResponsePtr &response) {});
    std::cout << "Sent heartbeat to " << eurekaHost << ":" << eurekaPort << ", for " << appName << " @ " << appHostName << std::endl;
}

void eurekaClient::initAndStart(const Json::Value &config)
{
    /// At initialization, register this current Drogon service with the specified Eureka server
    Json::Value registrationJson;
    Json::Value instanceJson;

    auto selfIpAddrOpt = getLocalIpAddress();
    auto selfIpAddr = selfIpAddrOpt.has_value() ? selfIpAddrOpt.value() : "127.0.0.1";

    instanceJson["hostName"] = config.get("appHostname", selfIpAddr).asString();
    instanceJson["app"] = config["appName"].asString();
    instanceJson["ipAddr"] = config.get("appIpAddr", selfIpAddr).asString();
    instanceJson["status"] = "UP";
    instanceJson["dataCenterInfo"] = config["dataCenterInfo"];
    
    Json::Value portJson;
    portJson["$"] = config["appPort"].asString();
    portJson["@enabled"] = "true";
    instanceJson["port"] = portJson;

    _eurekaHostName = config["eurekaHostName"].asString();
    _eurekaPort = config["eurekaPort"].asString();
    _appName = instanceJson["app"].asString();
    _appHostName = instanceJson["hostName"].asString();

    std::string url = "http://" + _eurekaHostName + ":" + _eurekaPort;
    auto client = HttpClient::newHttpClient(url);

    registrationJson["instance"] = instanceJson;
    auto req = HttpRequest::newHttpJsonRequest(registrationJson);
    req->setMethod(drogon::Post);
    req->setPath("/eureka/apps/" + config["appName"].asString());

    std::cout << "Sending registration request to " << _eurekaHostName << ":" << _eurekaPort << ", for app name: '" << _appName << "' @ " << _appHostName << std::endl;
    client->sendRequest(
        req,
        [eurekaHost=_eurekaHostName, eurekaPort=_eurekaPort, appName=_appName, appHostName=_appHostName]
        (ReqResult result, const HttpResponsePtr& response) {
            if (result != ReqResult::Ok) {
                std::cerr
                    << "Error while sending request to server! Result: "
                    << result << std::endl;
                return;
            }

            if (response->getStatusCode() != HttpStatusCode::k204NoContent) {
                std::cerr << "Eureka server responded with Error " << response->getStatusCode() << std::endl;
                std::cerr << response->getBody() << std::endl;
                return;
            }

            std::cout << "Eureka registration successful" << std::endl;

            /* after successful registration, start sending heartbeats to the Eureka server once every
             * eurekaClientDefaultHeartbeatPeriodSeconds seconds to let it know that this Drogon service
             * is still alive */
            app().getLoop()->runEvery(
                std::chrono::seconds(eurekaClientDefaultHeartbeatPeriodSeconds),
                [eurekaHost=std::move(eurekaHost), eurekaPort=std::move(eurekaPort), appName=std::move(appName), appHostName=std::move(appHostName)]() { 
                    sendHeartbeat(eurekaHost, eurekaPort, appName, appHostName); 
                }
            );
        }
    );
}

void eurekaClient::shutdown() 
{
    /// Shutdown the plugin
    updateAppState(
        eurekaState::DOWN,
        [eurekaHostName=_eurekaHostName, eurekaPort=_eurekaPort, appName=_appName, appHostName=_appHostName](ReqResult result, const HttpResponsePtr &response) {
            // De-register this Drogon service from the Eureka server
            auto client = HttpClient::newHttpClient("http://" + eurekaHostName + ":" + eurekaPort);
            auto req = HttpRequest::newHttpRequest();
            req->setMethod(drogon::Delete);
            req->setPath("/eureka/apps/" + appName + "/" + appHostName);
            client->sendRequest(req, [](ReqResult result, const HttpResponsePtr &response) {});
            std::cout << "Sent deregistration request to " << eurekaHostName << ":" << eurekaPort << ", for " << appName << " @ " << appHostName << std::endl;
        }
    );
}

void eurekaClient::updateAppState(eurekaState newState, HttpReqCallback&& cb) 
{
    auto client = HttpClient::newHttpClient("http://" + _eurekaHostName + ":" + _eurekaPort);
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Put);

    std::string path = "/eureka/apps/" + _appName + "/" + _appHostName + "/status?value=";
    switch (newState) {
    case eurekaState::UP:
        path += "UP";
        break;
    case eurekaState::DOWN:
        path += "DOWN";
        break;
    case eurekaState::STARTING:
        path += "STARTING";
        break;
    case eurekaState::UNKNOWN:
        path += "UNKNOWN";
        break;
    case eurekaState::OUT_OF_SERVICE:
        path += "OUT_OF_SERVICE";
        break;
    default: abort();
    }

    req->setPath(path);
    client->sendRequest(req, cb);
    std::cout << "Sent update state request to " << _eurekaHostName << ":" << _eurekaPort << ", for " << _appName << " @ " << _appHostName << std::endl;
}

void eurekaClient::updateMetadata(const std::string& key, const std::string& newValue, HttpReqCallback&& cb)
{
    auto client = HttpClient::newHttpClient("http://" + _eurekaHostName + ":" + _eurekaPort);
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Put);

    req->setPath("/eureka/apps/" + _appName + "/" + _appHostName + "/metadata?" + key + "=" + newValue);
    client->sendRequest(req, cb);
    std::cout << "Sent update metadata request to " << _eurekaHostName << ":" << _eurekaPort << ", for " << _appName << " @ " << _appHostName << std::endl;
}

void eurekaClient::getAllDiscoveredServices(std::function<void(std::vector<eurekaDiscoveredApp>)> cb)
{
    auto client = HttpClient::newHttpClient("http://" + _eurekaHostName + ":" + _eurekaPort);
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);

    req->setPath("/eureka/apps/");
    client->sendRequest(req, [cb = std::move(cb)](ReqResult result, const HttpResponsePtr& response) {
        if (response->getStatusCode() != HttpStatusCode::k200OK) {
            std::cerr << "Eureka server responded with Error " << response->getStatusCode() << std::endl;
            std::cerr << response->getBody() << std::endl;
            return;
        }

        pugi::xml_document doc;

        std::cout << response->getBody() << std::endl;

        pugi::xml_parse_result xml_parse_result = doc.load_string(std::string(response->getBody()).c_str());

        if (!xml_parse_result) {
            std::cerr << "Couldn't parse XML result" << std::endl;
            return;
        }

        std::vector<eurekaDiscoveredApp> ret;

        std::cout << "Discovered application: " << std::endl;
        for (pugi::xml_node application: doc.child("applications").children("application")) {
            
            for (pugi::xml_node appInstance: application.children("instance")) {
                std::cout << appInstance.child_value("status") << std::endl;
                ret.push_back(eurekaDiscoveredApp{
                    .appHostName = appInstance.child_value("hostName"),
                    .appIpAddr = appInstance.child_value("ipAddr"),
                    .appName = appInstance.child_value("app"),
                    .appPort = appInstance.child_value("port"),
                    .appStatus = stateStr2stateEnum.at(appInstance.child_value("status"))
                });
            }
        }

        cb(ret);
    });
    std::cout << "Sent fetch all services request to " << _eurekaHostName << ":" << _eurekaPort << std::endl;
}

std::optional<std::string> eurekaClient::getLocalIpAddress()
{
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback;

    if (sock == -1) {
        std::cerr << "Could not socket()\n";
        return std::nullopt;
    }

    std::memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        std::cerr << "Could not connect\n";
        return std::nullopt;
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) {
        close(sock);
        std::cerr << "Could not getsockname\n";
        return std::nullopt;
    }

    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == 0x0) {
        std::cerr << "Could not inet_ntop\n";
        return std::nullopt;
    } else {
        return buf;
    }
}