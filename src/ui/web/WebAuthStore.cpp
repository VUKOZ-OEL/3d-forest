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

/** @file WebAuthStore.cpp */

// Include 3D Forest.
#include <WebAuthStore.hpp>

// Include 3rd party.
#include <drogon/utils/Utilities.h>
#include <json/json.h>
#include <trantor/utils/Utilities.h>

// Include std.
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>

#ifndef _WIN32
    #include <sys/stat.h>
#endif

namespace
{
using Bytes = std::vector<std::uint8_t>;
using Key = std::array<std::uint8_t, 32>;

Key sha256(const std::uint8_t *data, std::size_t size)
{
    const auto hash = trantor::utils::sha256(data, size);
    Key result{};
    std::copy(hash.bytes, hash.bytes + result.size(), result.begin());
    return result;
}

Key hmac(const std::uint8_t *key,
         std::size_t keySize,
         const std::uint8_t *data,
         std::size_t dataSize)
{
    std::array<std::uint8_t, 64> block{};
    if (keySize > block.size())
    {
        const auto reduced = sha256(key, keySize);
        std::copy(reduced.begin(), reduced.end(), block.begin());
    }
    else if (keySize)
        std::copy(key, key + keySize, block.begin());
    Bytes inner(block.size() + dataSize);
    Bytes outer(block.size() + 32);
    for (std::size_t i = 0; i < block.size(); ++i)
    {
        inner[i] = block[i] ^ 0x36;
        outer[i] = block[i] ^ 0x5c;
    }
    std::copy(data, data + dataSize, inner.begin() + block.size());
    const auto innerHash = sha256(inner.data(), inner.size());
    std::copy(innerHash.begin(), innerHash.end(), outer.begin() + block.size());
    return sha256(outer.data(), outer.size());
}

Key hmac(const Key &key, const std::string &message)
{
    return hmac(key.data(),
                key.size(),
                reinterpret_cast<const std::uint8_t *>(message.data()),
                message.size());
}

Bytes randomBytes(std::size_t size)
{
    Bytes result(size);
    if (!trantor::utils::secureRandomBytes(result.data(), result.size()))
        throw std::runtime_error("Could not obtain secure random bytes");
    return result;
}

void writeFile(const std::filesystem::path &path, const Bytes &bytes)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file || !file.write(reinterpret_cast<const char *>(bytes.data()),
                             static_cast<std::streamsize>(bytes.size())))
        throw std::runtime_error("Could not write web account file: " +
                                 path.string());
}

Bytes readFile(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    return Bytes(std::istreambuf_iterator<char>(file),
                 std::istreambuf_iterator<char>());
}

#ifndef _WIN32
void requirePrivatePath(const std::filesystem::path &path, bool directory)
{
    struct stat info
    {
    };
    if (::stat(path.string().c_str(), &info) != 0 ||
        (info.st_mode & (S_IRWXG | S_IRWXO)) != 0 ||
        (directory && !S_ISDIR(info.st_mode)))
        throw std::runtime_error("Restrict web account permissions for " +
                                 path.string());
}
#endif

bool validUsername(const std::string &username)
{
    return !username.empty() && username.size() <= 64 &&
           std::all_of(username.begin(),
                       username.end(),
                       [](unsigned char c) {
                           return std::isalnum(c) || c == '_' || c == '-' ||
                                  c == '.';
                       });
}

std::string wrapVerifier(const Key &pepper,
                         const std::string &username,
                         const std::string &saltHex,
                         const Key &verifier,
                         std::string &nonceHex,
                         std::string &macHex)
{
    const auto nonce = randomBytes(16);
    nonceHex = webAuthHex(nonce.data(), nonce.size());
    Bytes cipher(verifier.begin(), verifier.end());
    std::string input = "3DForest credential wrap v1";
    input.append(reinterpret_cast<const char *>(nonce.data()), nonce.size());
    input.push_back('0');
    const auto stream = hmac(pepper, input);
    for (std::size_t i = 0; i < cipher.size(); ++i)
        cipher[i] ^= stream[i];
    const auto mac =
        hmac(pepper,
             "3DForest credential v1\n" + username + "\n" + saltHex + "\n" +
                 nonceHex + "\n" + webAuthHex(cipher.data(), cipher.size()));
    macHex = webAuthHex(mac.data(), mac.size());
    return webAuthHex(cipher.data(), cipher.size());
}

Key unwrapVerifier(const Key &pepper,
                   const std::string &username,
                   const std::string &saltHex,
                   const std::string &nonceHex,
                   const std::string &cipherHex,
                   const std::string &macHex)
{
    Bytes nonce(16), cipher(32);
    Key suppliedMac{};
    if (!webAuthUnhex(nonceHex, nonce.data(), nonce.size()) ||
        !webAuthUnhex(cipherHex, cipher.data(), cipher.size()) ||
        !webAuthUnhex(macHex, suppliedMac.data(), suppliedMac.size()))
        throw std::runtime_error("Invalid web account record");
    const auto expectedMac =
        hmac(pepper,
             "3DForest credential v1\n" + username + "\n" + saltHex + "\n" +
                 nonceHex + "\n" + cipherHex);
    if (!webAuthEqual(expectedMac.data(),
                      suppliedMac.data(),
                      expectedMac.size()))
        throw std::runtime_error("Web account record authentication failed");
    std::string input = "3DForest credential wrap v1";
    input.append(reinterpret_cast<const char *>(nonce.data()), nonce.size());
    input.push_back('0');
    const auto stream = hmac(pepper, input);
    Key result{};
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = cipher[i] ^ stream[i];
    return result;
}

JsonCpp::Value loadUsers(const std::filesystem::path &path)
{
    JsonCpp::Value result{JsonCpp::objectValue};
    if (!std::filesystem::exists(path))
        return result;
    std::ifstream file(path, std::ios::binary);
    JsonCpp::CharReaderBuilder builder;
    builder["collectComments"] = false;
    std::string errors;
    if (!JsonCpp::parseFromStream(builder, file, &result, &errors) ||
        !result.isObject())
        throw std::runtime_error("Could not parse web account file: " + errors);
    return result;
}

void saveUsers(const std::filesystem::path &path, const JsonCpp::Value &users)
{
    JsonCpp::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    const auto content = JsonCpp::writeString(builder, users);
    const auto temp = path.string() + ".tmp";
    {
        std::ofstream file(temp, std::ios::binary | std::ios::trunc);
        if (!file || !file.write(content.data(),
                                 static_cast<std::streamsize>(content.size())))
            throw std::runtime_error("Could not write web accounts file");
    }
#ifndef _WIN32
    if (::chmod(temp.c_str(), S_IRUSR | S_IWUSR) != 0)
        throw std::runtime_error(
            "Could not restrict permissions on accounts file");
#endif
    std::error_code ec;
    std::filesystem::rename(temp, path, ec);
    if (ec)
    {
        std::filesystem::remove(path, ec);
        ec.clear();
        std::filesystem::rename(temp, path, ec);
    }
    if (ec)
        throw std::runtime_error("Could not replace web accounts file: " +
                                 ec.message());
}

} // namespace

std::array<std::uint8_t, 32> webAuthHmac(
    const std::array<std::uint8_t, 32> &key,
    const std::string &message)
{
    return hmac(key, message);
}

std::array<std::uint8_t, 32> webAuthPbkdf2(const std::string &password,
                                           const std::string &salt,
                                           std::uint32_t count)
{
    if (!count || count > 2000000)
        throw std::invalid_argument("Invalid PBKDF2 iteration count");
    std::string first = salt;
    first.append("\0\0\0\1", 4);
    auto u = hmac(reinterpret_cast<const std::uint8_t *>(password.data()),
                  password.size(),
                  reinterpret_cast<const std::uint8_t *>(first.data()),
                  first.size());
    Key result = u;
    for (std::uint32_t i = 1; i < count; ++i)
    {
        u = hmac(reinterpret_cast<const std::uint8_t *>(password.data()),
                 password.size(),
                 u.data(),
                 u.size());
        for (std::size_t j = 0; j < result.size(); ++j)
            result[j] ^= u[j];
    }
    return result;
}

std::string webAuthHex(const std::uint8_t *data, std::size_t size)
{
    static constexpr char digits[] = "0123456789abcdef";
    std::string result(size * 2, '0');
    for (std::size_t i = 0; i < size; ++i)
    {
        result[i * 2] = digits[data[i] >> 4];
        result[i * 2 + 1] = digits[data[i] & 15];
    }
    return result;
}

bool webAuthUnhex(const std::string &text,
                  std::uint8_t *output,
                  std::size_t outputSize)
{
    if (text.size() != outputSize * 2)
        return false;
    auto digit = [](unsigned char c) -> int
    {
        if (c >= '0' && c <= '9')
            return c - '0';
        if (c >= 'a' && c <= 'f')
            return c - 'a' + 10;
        if (c >= 'A' && c <= 'F')
            return c - 'A' + 10;
        return -1;
    };
    for (std::size_t i = 0; i < outputSize; ++i)
    {
        const int high = digit(static_cast<unsigned char>(text[i * 2]));
        const int low = digit(static_cast<unsigned char>(text[i * 2 + 1]));
        if (high < 0 || low < 0)
            return false;
        output[i] = static_cast<std::uint8_t>((high << 4) | low);
    }
    return true;
}

bool webAuthEqual(const std::uint8_t *a,
                  const std::uint8_t *b,
                  std::size_t size)
{
    std::uint8_t diff = 0;
    for (std::size_t i = 0; i < size; ++i)
        diff |= a[i] ^ b[i];
    return diff == 0;
}

WebAuthStore::WebAuthStore(std::string directory)
    : directory_(std::move(directory))
{
    const std::filesystem::path root(directory_);
    const auto usersPath = root / "users.json";
    if (!std::filesystem::exists(usersPath))
        return;
#ifndef _WIN32
    requirePrivatePath(root, true);
    requirePrivatePath(usersPath, false);
    requirePrivatePath(root / "pepper.bin", false);
#endif
    const auto pepper = readFile(root / "pepper.bin");
    if (pepper.size() != pepper_.size())
        throw std::runtime_error("Missing or invalid web account pepper file");
    std::copy(pepper.begin(), pepper.end(), pepper_.begin());
    const auto records = loadUsers(usersPath);
    for (const auto &username : records.getMemberNames())
    {
        if (!validUsername(username))
            throw std::runtime_error("Invalid username in web account file");
        const auto &record = records[username];
        if (!record["iterations"].isUInt() ||
            record["iterations"].asUInt() != iterations)
            throw std::runtime_error("Unsupported web account hash settings");
        Credential credential;
        credential.salt = record["salt"].asString();
        credential.verifier = unwrapVerifier(pepper_,
                                             username,
                                             credential.salt,
                                             record["nonce"].asString(),
                                             record["wrapped"].asString(),
                                             record["mac"].asString());
        credential.known = true;
        users_.emplace(username, std::move(credential));
    }
}

WebAuthStore::Credential WebAuthStore::credential(
    const std::string &username) const
{
    const auto it = users_.find(username);
    if (it != users_.end())
        return it->second;
    Credential result;
    const auto salt = randomBytes(32);
    result.salt = webAuthHex(salt.data(), salt.size());
    const auto verifier = randomBytes(32);
    std::copy(verifier.begin(), verifier.end(), result.verifier.begin());
    return result;
}

std::chrono::seconds WebAuthStore::lockoutRemaining(
    const std::string &username)
{
    std::lock_guard<std::mutex> lock(failuresMutex_);
    const auto it = failures_.find(username);
    if (it == failures_.end())
        return std::chrono::seconds::zero();
    const auto now = std::chrono::steady_clock::now();
    if (now >= it->second.lockedUntil)
    {
        it->second.count = 0;
        it->second.lockedUntil = {};
        return std::chrono::seconds::zero();
    }
    return std::chrono::ceil<std::chrono::seconds>(it->second.lockedUntil - now);
}

void WebAuthStore::recordFailedLogin(const std::string &username)
{
    std::lock_guard<std::mutex> lock(failuresMutex_);
    auto &failure = failures_[username];
    if (std::chrono::steady_clock::now() < failure.lockedUntil)
        return;
    if (++failure.count >= 5)
    {
        failure.count = 0;
        failure.lockedUntil = std::chrono::steady_clock::now() +
                              std::chrono::minutes(5);
    }
}

void WebAuthStore::recordSuccessfulLogin(const std::string &username)
{
    std::lock_guard<std::mutex> lock(failuresMutex_);
    failures_.erase(username);
}

void WebAuthStore::addUser(const std::string &directory,
                           const std::string &username,
                           const std::string &password)
{
    if (!validUsername(username))
        throw std::invalid_argument("Username must be 1-64 characters: "
                                    "letters, digits, dot, dash, underscore");
    if (password.size() < 8 || password.size() > 1024)
        throw std::invalid_argument("Password must contain 8-1024 bytes");
    const std::filesystem::path root(directory);
#ifndef _WIN32
    const bool createRoot = !std::filesystem::exists(root);
#endif
    std::filesystem::create_directories(root);
#ifndef _WIN32
    if (createRoot)
    {
        if (::chmod(root.string().c_str(), S_IRWXU) != 0)
            throw std::runtime_error(
                "Could not restrict permissions on account directory");
    }
    else
        requirePrivatePath(root, true);
#endif
    const auto pepperPath = root / "pepper.bin";
    const auto usersPath = root / "users.json";
    Key pepper{};
    if (std::filesystem::exists(pepperPath))
    {
#ifndef _WIN32
        requirePrivatePath(pepperPath, false);
#endif
        const auto bytes = readFile(pepperPath);
        if (bytes.size() != pepper.size())
            throw std::runtime_error("Invalid web account pepper file");
        std::copy(bytes.begin(), bytes.end(), pepper.begin());
    }
    else
    {
        if (std::filesystem::exists(usersPath))
            throw std::runtime_error(
                "Account database exists but pepper file is missing");
        const auto bytes = randomBytes(pepper.size());
        std::copy(bytes.begin(), bytes.end(), pepper.begin());
        writeFile(pepperPath, bytes);
#ifndef _WIN32
        if (::chmod(pepperPath.string().c_str(), S_IRUSR | S_IWUSR) != 0)
            throw std::runtime_error(
                "Could not restrict permissions on pepper file");
#endif
    }
    if (std::filesystem::exists(usersPath))
    {
#ifndef _WIN32
        requirePrivatePath(usersPath, false);
#endif
    }
    auto users = loadUsers(usersPath);
    if (users.isMember(username))
        throw std::invalid_argument("That username already exists");
    const auto saltBytes = randomBytes(32);
    const auto saltHex = webAuthHex(saltBytes.data(), saltBytes.size());
    const auto verifier = webAuthPbkdf2(password, saltHex, iterations);
    std::string nonceHex, macHex;
    const auto wrapped =
        wrapVerifier(pepper, username, saltHex, verifier, nonceHex, macHex);
    JsonCpp::Value record{JsonCpp::objectValue};
    record["salt"] = saltHex;
    record["iterations"] = iterations;
    record["nonce"] = nonceHex;
    record["wrapped"] = wrapped;
    record["mac"] = macHex;
    users[username] = std::move(record);
    saveUsers(usersPath, users);
}
