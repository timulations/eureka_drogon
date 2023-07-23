#include <drogon/drogon.h>
#include "drogon_plugin_eurekaClient.h"

using namespace drogon;

int main() {
    //Set HTTP listener address and port
    //app().addListener("0.0.0.0",80);
    //Load config file
    app().loadConfigFile("../config.json");
    //Run HTTP framework,the method will block in the internal event loop

    app().registerHandler(
        "/",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&callback) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Hello, World!");

            callback(resp);
        },
        {Get});  


    app().registerHandler(
        "/get_discovered",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&callback) {
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
        },
        {Get}); 

    app().run();
    return 0;
}

