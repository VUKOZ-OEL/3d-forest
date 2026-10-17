#include "JsonConfigAdapter.h"
#include <fstream>
#include <mutex>

using namespace drogon;

JsonCpp::Value JsonConfigAdapter::getJson(const std::string &content) const
    noexcept(false)
{
    static std::once_flag once;
    static JsonCpp::CharReaderBuilder builder;
    std::call_once(once, []() { builder["collectComments"] = false; });
    JSONCPP_STRING errs;
    std::unique_ptr<JsonCpp::CharReader> reader(builder.newCharReader());
    JsonCpp::Value root;
    if (!reader->parse(
            content.c_str(), content.c_str() + content.size(), &root, &errs))
    {
        throw std::runtime_error(errs);
    }
    return root;
}

std::vector<std::string> JsonConfigAdapter::getExtensions() const
{
    return {"json"};
}
