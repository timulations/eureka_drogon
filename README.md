# eureka_drogon
Minimalistic Eureka client for web services built on Drogon C++17 for service discovery with your Drogon servers.

## Getting Started
Clone this repo.
Then, the only two things you really need are 
- `drogon_plugin_eurekaClient.cc`
- `drogon_plugin_eurekaClient.h`

Copy those two files into your `plugins/` folder of your Drogon project. 

And in your `config.json` file, add a section for configuring the Drogon Eureka client:
```json
      "plugins": [
          {
              "name": "drogon::plugin::eurekaClient",
              "dependencies": [],
              "config": {
                  "eurekaHostName": "localhost",  
                  "eurekaPort": "8761",     
                  "appName": "my_drogon_service", 
                  "appPort": "5000",
                  "dataCenterInfo": {
                      "@class": "com.netflix.appinfo.InstanceInfo$DefaultDataCenterInfo", 
                      "name": "MyOwn"
                  }
              }
          }
      ]
```

Here is the JSON spec
```json
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
```

With this, your Drogon service will start to send registration requests and regular heartbeats to the Eureka server on start-up so that it is discoverable by other services within your Eureka cluster.

To discover other services in the Eureka cluster within Drogon, use the following functions:
```cpp
    // return all discovered services (including self) currently registered on the Eureka registry
    void getAllServices(std::function<void(std::vector<eurekaDiscoveredApp>)> cb);

    // return all discovered service instances with application name matching appName
    void getAppServices(const std::string& appName, std::function<void(std::vector<eurekaDiscoveredApp>)> cb);

    // return the discovered service instance with matching appName and appHostName
    void getService(const std::string& appName, const std::string& appHostName, std::function<void(std::optional<eurekaDiscoveredApp>)> cb);
```

For example, in `controllers/api_v1_eureka.cc`. Simply get the plugin pointer, then away you go!
```cpp
    auto *eurekaClientPtr = app().getPlugin<drogon::plugin::eurekaClient>();
    eurekaClientPtr->getAppServices(appName, [resp=std::move(resp), callback=std::move(callback)](std::vector<drogon::plugin::eurekaDiscoveredApp> allApps) {
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
```


## Demo
Build and run the Drogon server, then go to: `http://localhost:5000/api/v1/eureka/`. For the supported endpoints for querying discovered services, see `controllers/api_v1_eureka.h`
