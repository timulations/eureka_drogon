/**
 *
 *  drogon_plugin_eurekaClient.h
 *
 */

#pragma once

#include <drogon/plugins/Plugin.h>
namespace drogon
{
namespace plugin
{

enum class eurekaState {
  UP,
  DOWN,
  STARTING,
  OUT_OF_SERVICE,
  UNKNOWN
};

constexpr unsigned eurekaClientDefaultHeartbeatPeriodSeconds = 30;


struct eurekaDiscoveredApp {
  std::string appName;
  std::string appHostName;
  std::string appIpAddr;
  std::string appPort;
  eurekaState appStatus;
};

/**
 * @brief This plugin is used to automatically manage register this Drogon service
 * with a Eureka service discovery server. It will periodically send heartbeats
 * along with the Drogon main event loop. 
 *
 * The json configuration is as follows. All fields are compulsory unless marked
 * with [OPTIONAL]
 *
 * @code
   {
      "name": "drogon::plugin::eurekaClient",
      "dependencies": [],
      "config": {
            "eurekaHostName": "x.x.x.x",        # host name or ip address of the running Eureka server
            "eurekaPort": "8761",               # port of the Eureka server
            "appName": "my_drogon_service",     # name of this service to be registered
            "appHostname": "x.x.x.x",           # [OPTIONAL] hostname of this service, if unspecified, the current IP will be used.
            "appIpAddr": "x.x.x.x",             # [OPTIONAL] ip address of this service, if unspecified, the current IP will be used.
            "appPort": "8080",                  # port to be used for Eureka communications. Can be the same as the current Drogon service port.
            "dataCenterInfo": {
                "@class": "com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo",   # class of data center
                "name": "MyOwn",                                                      # name of data center
            }
      }
   }
   @endcode
 *
 */
class DROGON_EXPORT eurekaClient : public drogon::Plugin<eurekaClient>
{
  public:
    eurekaClient() {}
    /// This method must be called by drogon to initialize and start the plugin.
    /// It must be implemented by the user.

    void initAndStart(const Json::Value &config) override;

    /// This method must be called by drogon to shutdown the plugin.
    /// It must be implemented by the user.
    void shutdown() override;

    // signal the Eureka server to update the state of this Drogon service
    void updateAppState(eurekaState newState, HttpReqCallback&& cb);

    // signal the Eureka server to update a metadata field value
    void updateMetadata(const std::string& key, const std::string& newValue, HttpReqCallback&& cb);

    void getAllDiscoveredServices(std::function<void(std::vector<eurekaDiscoveredApp>)> cb);

  private:
    // gets the local IP address of the current machine
    std::optional<std::string> getLocalIpAddress();

    std::string _eurekaHostName;
    std::string _eurekaPort;
    std::string _appName;
    std::string _appHostName;
};

}
}
