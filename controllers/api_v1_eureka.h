#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

namespace api
{
namespace v1
{
class eureka : public drogon::HttpController<eureka>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    // METHOD_ADD(eureka::get, "/{2}/{1}", Get); // path is /api/{arg2}/{arg1}
    // METHOD_ADD(eureka::your_method_name, "/{1}/{2}/list", Get); // path is /api/{arg1}/{arg2}/list
    // ADD_METHOD_TO(eureka::your_method_name, "/absolute/path/{1}/{2}/list", Get); // path is /absolute/path/{arg1}/{arg2}/list
    METHOD_ADD(eureka::index, "/", Get);
    METHOD_ADD(eureka::index, "", Get);
    METHOD_ADD(eureka::getAllDiscovered, "/get_all_discovered", Get);
    METHOD_ADD(eureka::getInstancesFor, "/search_instances/{1}", Get);
    METHOD_ADD(eureka::getInstance, "/search/{1}/{2}", Get);

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    // void get(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, int p1, std::string p2);
    // void your_method_name(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, double p1, int p2) const;
    void index(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void getAllDiscovered(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback);
    void getInstancesFor(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string& appName);
    void getInstance(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback, const std::string& appName, const std::string& appHostName);

};
}
}
