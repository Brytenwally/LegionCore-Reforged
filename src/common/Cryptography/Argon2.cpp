////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "Argon2.h"
#include "Log.h"
#include <cstring>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/params.h>
#include <openssl/rand.h>
#include <vector>

namespace
{
    constexpr char const* PREFIX = "$argon2id$";
    constexpr uint32 NEW_MEMORY_KIB = 65536;
    constexpr uint32 NEW_ITERATIONS = 3;
    constexpr uint32 NEW_LANES = 1;
    constexpr size_t SALT_LENGTH = 16;
    constexpr size_t HASH_LENGTH = 32;

    // Bounds for values read from the database, so a malformed or hostile row cannot make the server allocate gigabytes
    constexpr uint32 MAX_MEMORY_KIB = 262144;
    constexpr uint32 MAX_ITERATIONS = 20;
    constexpr uint32 MAX_LANES = 16;
    constexpr size_t MIN_SALT_LENGTH = 8;
    constexpr size_t MAX_SALT_LENGTH = 64;
    constexpr size_t MIN_HASH_LENGTH = 16;
    constexpr size_t MAX_HASH_LENGTH = 64;

    char const BASE64_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string Base64Encode(uint8 const* data, size_t len)
    {
        std::string out;
        out.reserve((len + 2) / 3 * 4);
        size_t i = 0;
        for (; i + 2 < len; i += 3)
        {
            uint32 v = (uint32(data[i]) << 16) | (uint32(data[i + 1]) << 8) | data[i + 2];
            out.push_back(BASE64_ALPHABET[(v >> 18) & 63]);
            out.push_back(BASE64_ALPHABET[(v >> 12) & 63]);
            out.push_back(BASE64_ALPHABET[(v >> 6) & 63]);
            out.push_back(BASE64_ALPHABET[v & 63]);
        }
        if (len - i == 1)
        {
            uint32 v = uint32(data[i]) << 16;
            out.push_back(BASE64_ALPHABET[(v >> 18) & 63]);
            out.push_back(BASE64_ALPHABET[(v >> 12) & 63]);
        }
        else if (len - i == 2)
        {
            uint32 v = (uint32(data[i]) << 16) | (uint32(data[i + 1]) << 8);
            out.push_back(BASE64_ALPHABET[(v >> 18) & 63]);
            out.push_back(BASE64_ALPHABET[(v >> 12) & 63]);
            out.push_back(BASE64_ALPHABET[(v >> 6) & 63]);
        }
        return out;
    }

    bool Base64Decode(std::string const& in, std::vector<uint8>& out)
    {
        out.clear();
        if (in.size() % 4 == 1)
            return false;

        uint32 acc = 0;
        int bits = 0;
        for (char c : in)
        {
            char const* pos = c ? strchr(BASE64_ALPHABET, c) : nullptr;
            if (!pos)
                return false;
            acc = (acc << 6) | uint32(pos - BASE64_ALPHABET);
            bits += 6;
            if (bits >= 8)
            {
                bits -= 8;
                out.push_back(uint8((acc >> bits) & 0xFF));
            }
        }
        return true;
    }

    bool ParseUInt(std::string const& s, uint32& out)
    {
        if (s.empty() || s.size() > 9)
            return false;
        uint32 v = 0;
        for (char c : s)
        {
            if (c < '0' || c > '9')
                return false;
            v = v * 10 + uint32(c - '0');
        }
        out = v;
        return true;
    }

    std::vector<std::string> Split(std::string const& s, char delim)
    {
        std::vector<std::string> parts;
        size_t start = 0;
        while (true)
        {
            size_t pos = s.find(delim, start);
            if (pos == std::string::npos)
            {
                parts.push_back(s.substr(start));
                return parts;
            }
            parts.push_back(s.substr(start, pos - start));
            start = pos + 1;
        }
    }

    // Fetched once and kept for the life of the process
    EVP_KDF* GetKdf()
    {
        static EVP_KDF* kdf = []()
        {
            EVP_KDF* fetched = EVP_KDF_fetch(nullptr, "ARGON2ID", nullptr);
            if (!fetched)
                TC_LOG_ERROR("misc", "Argon2id is not available in the linked OpenSSL (3.2 or later is required), password checks will fail");
            return fetched;
        }();
        return kdf;
    }

    bool Derive(std::string const& password, uint8 const* salt, size_t saltLen, uint32 iterations, uint32 lanes, uint32 memoryKiB, uint8* out, size_t outLen)
    {
        EVP_KDF* kdf = GetKdf();
        if (!kdf)
            return false;

        EVP_KDF_CTX* ctx = EVP_KDF_CTX_new(kdf);
        if (!ctx)
            return false;

        uint32 threads = 1;
        OSSL_PARAM params[] =
        {
            OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD, const_cast<char*>(password.data()), password.size()),
            OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, const_cast<uint8*>(salt), saltLen),
            OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ITER, &iterations),
            OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_THREADS, &threads),
            OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, &lanes),
            OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, &memoryKiB),
            OSSL_PARAM_construct_end()
        };

        bool const ok = EVP_KDF_derive(ctx, out, outLen, params) == 1;
        EVP_KDF_CTX_free(ctx);
        if (!ok)
            TC_LOG_ERROR("misc", "Argon2id derivation failed");
        return ok;
    }
}

bool Argon2::IsArgon2(std::string const& stored)
{
    return stored.compare(0, strlen(PREFIX), PREFIX) == 0;
}

std::string Argon2::Hash(std::string const& normalizedPassword)
{
    uint8 salt[SALT_LENGTH];
    if (RAND_bytes(salt, int(SALT_LENGTH)) != 1)
    {
        TC_LOG_ERROR("misc", "Argon2id: could not generate a random salt");
        return std::string();
    }

    uint8 hash[HASH_LENGTH];
    if (!Derive(normalizedPassword, salt, SALT_LENGTH, NEW_ITERATIONS, NEW_LANES, NEW_MEMORY_KIB, hash, HASH_LENGTH))
        return std::string();

    std::string result = "$argon2id$v=19$m=" + std::to_string(NEW_MEMORY_KIB) + ",t=" + std::to_string(NEW_ITERATIONS) + ",p=" + std::to_string(NEW_LANES)
        + "$" + Base64Encode(salt, SALT_LENGTH) + "$" + Base64Encode(hash, HASH_LENGTH);

    OPENSSL_cleanse(hash, sizeof(hash));
    return result;
}

bool Argon2::Verify(std::string const& normalizedPassword, std::string const& stored)
{
    if (!IsArgon2(stored))
        return false;

    // ["", "argon2id", "v=19", "m=..,t=..,p=..", salt, hash]
    std::vector<std::string> parts = Split(stored, '$');
    if (parts.size() != 6 || !parts[0].empty() || parts[2] != "v=19")
    {
        TC_LOG_ERROR("misc", "Argon2id: malformed stored hash");
        return false;
    }

    std::vector<std::string> costs = Split(parts[3], ',');
    uint32 memoryKiB = 0, iterations = 0, lanes = 0;
    if (costs.size() != 3 || costs[0].compare(0, 2, "m=") || costs[1].compare(0, 2, "t=") || costs[2].compare(0, 2, "p=")
        || !ParseUInt(costs[0].substr(2), memoryKiB) || !ParseUInt(costs[1].substr(2), iterations) || !ParseUInt(costs[2].substr(2), lanes))
    {
        TC_LOG_ERROR("misc", "Argon2id: malformed cost parameters in stored hash");
        return false;
    }

    if (iterations < 1 || iterations > MAX_ITERATIONS || lanes < 1 || lanes > MAX_LANES || memoryKiB < 8 * lanes || memoryKiB > MAX_MEMORY_KIB)
    {
        TC_LOG_ERROR("misc", "Argon2id: cost parameters of stored hash are out of range (m=%u t=%u p=%u)", memoryKiB, iterations, lanes);
        return false;
    }

    std::vector<uint8> salt, expected;
    if (!Base64Decode(parts[4], salt) || !Base64Decode(parts[5], expected)
        || salt.size() < MIN_SALT_LENGTH || salt.size() > MAX_SALT_LENGTH
        || expected.size() < MIN_HASH_LENGTH || expected.size() > MAX_HASH_LENGTH)
    {
        TC_LOG_ERROR("misc", "Argon2id: malformed salt or hash in stored hash");
        return false;
    }

    uint8 computed[MAX_HASH_LENGTH];
    if (!Derive(normalizedPassword, salt.data(), salt.size(), iterations, lanes, memoryKiB, computed, expected.size()))
        return false;

    bool const match = CRYPTO_memcmp(computed, expected.data(), expected.size()) == 0;
    OPENSSL_cleanse(computed, sizeof(computed));
    return match;
}
