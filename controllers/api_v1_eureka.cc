#include "api_v1_eureka.h"
#include "drogon_plugin_eurekaClient.h"

using namespace api::v1;

// Add definition of your processing function here

// Add definition of your processing function here
void eureka::index(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setBody("Hello, World!");

    callback(resp);
}

void eureka::getAllDiscovered(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback)
{
    auto resp = HttpResponse::newHttpResponse();
    
    auto *eurekaClientPtr = app().getPlugin<drogon::plugin::eurekaClient>();
    eurekaClientPtr->getAllDiscoveredServices([resp=std::move(resp), callback=std::move(callback)](std::vector<drogon::plugin::eurekaDiscoveredApp> allApps) {
        std::stringstream ss;
        for (const auto& discoveredApp: allApps) {
            ss << discoveredApp.appName << " found at " << discoveredApp.appIpAddr << ":" << discoveredApp.appPort << ", ";
        }

        resp->setBody(ss.str());
        callback(resp);
    });
}

void eureka::getInstancesFor(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string& appName)
{
    auto resp = HttpResponse::newHttpResponse();
    
    auto *eurekaClientPtr = app().getPlugin<drogon::plugin::eurekaClient>();
    eurekaClientPtr->getAllInstancesInfo(appName, [resp=std::move(resp), callback=std::move(callback)](std::vector<drogon::plugin::eurekaDiscoveredApp> allApps) {
        std::stringstream ss;

        if (!allApps.empty()) {
            for (const auto& discoveredApp: allApps) {
                ss << discoveredApp.appName << " found at " << discoveredApp.appIpAddr << ":" << discoveredApp.appPort << ", ";
            }
        } else {
            ss << "No matching apps found";
        }


        resp->setBody(ss.str());
        callback(resp);
    });
}

void eureka::getInstance(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string& appName, const std::string& appHostName)
{
    auto resp = HttpResponse::newHttpResponse();
    
    auto *eurekaClientPtr = app().getPlugin<drogon::plugin::eurekaClient>();
    eurekaClientPtr->getInstanceInfo(appName, appHostName, [resp=std::move(resp), callback=std::move(callback), appName, appHostName](std::optional<drogon::plugin::eurekaDiscoveredApp> appInstanceOpt) {
        std::stringstream ss;
        
        if (appInstanceOpt.has_value()) {
            ss << appInstanceOpt.value().appName << "@" << appInstanceOpt.value().appHostName << " found at " << appInstanceOpt.value().appIpAddr << ":" << appInstanceOpt.value().appPort << ", ";
        } else {
            ss << "No app with name=" << appName << "@" << appHostName << " was fonud in the Eureka registry";
        }

        resp->setBody(ss.str());
        callback(resp);
    });
}