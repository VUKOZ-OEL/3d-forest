/**
 *
 *  ConfigLoader.h
 *  An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

#pragma once

#include <json/json.h>
#include <string>
#include <trantor/utils/NonCopyable.h>

namespace drogon
{
class ConfigLoader : public trantor::NonCopyable
{
  public:
    explicit ConfigLoader(const std::string &configFile) noexcept(false);
    explicit ConfigLoader(const JsonCpp::Value &data);
    explicit ConfigLoader(JsonCpp::Value &&data);
    ~ConfigLoader();

    const JsonCpp::Value &jsonValue() const
    {
        return configJsonRoot_;
    }

    void load() noexcept(false);

  private:
    std::string configFile_;
    JsonCpp::Value configJsonRoot_;
};
}  // namespace drogon
