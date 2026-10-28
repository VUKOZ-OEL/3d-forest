/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file WebAuthStore.hpp */

#pragma once

// Include std.
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

// Include local.
#include <ExportUiWeb.hpp>

/** File-backed web account credentials. */
class EXPORT_UI_WEB WebAuthStore
{
public:
    static constexpr std::uint32_t iterations = 600000;
    struct Credential
    {
        std::string salt;
        std::array<std::uint8_t, 32> verifier{};
        bool known{false};
    };

    explicit WebAuthStore(std::string directory);
    bool enabled() const { return !users_.empty(); }
    Credential credential(const std::string &username) const;
    std::chrono::seconds lockoutRemaining(const std::string &username);
    void recordFailedLogin(const std::string &username);
    void recordSuccessfulLogin(const std::string &username);
    static void addUser(const std::string &directory,
                        const std::string &username,
                        const std::string &password);

private:
    std::string directory_;
    std::array<std::uint8_t, 32> pepper_{};
    std::unordered_map<std::string, Credential> users_;
    struct LoginFailures
    {
        unsigned int count{0};
        std::chrono::steady_clock::time_point lockedUntil{};
    };
    std::mutex failuresMutex_;
    std::unordered_map<std::string, LoginFailures> failures_;
};

std::array<std::uint8_t, 32> webAuthPbkdf2(const std::string &password,
                                           const std::string &salt,
                                           std::uint32_t iterations);
std::array<std::uint8_t, 32> webAuthHmac(
    const std::array<std::uint8_t, 32> &key,
    const std::string &message);
std::string webAuthHex(const std::uint8_t *data, std::size_t size);
bool webAuthUnhex(const std::string &text,
                  std::uint8_t *output,
                  std::size_t outputSize);
bool webAuthEqual(const std::uint8_t *a,
                  const std::uint8_t *b,
                  std::size_t size);
