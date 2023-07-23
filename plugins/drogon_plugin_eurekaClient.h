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

class eurekaClient : public drogon::Plugin<eurekaClient>
{
  public:
    eurekaClient() {}
    /// This method must be called by drogon to initialize and start the plugin.
    /// It must be implemented by the user.
    void initAndStart(const Json::Value &config) override;

    /// This method must be called by drogon to shutdown the plugin.
    /// It must be implemented by the user.
    void shutdown() override;
};

}
}
