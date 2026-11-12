////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#include "PasswordHash.h"
#include "Argon2.h"
#include "SHA256.h"
#include "Util.h"
#include <openssl/crypto.h>

std::string PasswordHash::CalculateLegacy(std::string const& name, std::string const& password)
{
    SHA256Hash email;
    email.UpdateData(name);
    email.Finalize();

    SHA256Hash sha;
    sha.UpdateData(ByteArrayToHexStr(email.GetDigest(), email.GetLength()));
    sha.UpdateData(":");
    sha.UpdateData(password);
    sha.Finalize();

    return ByteArrayToHexStr(sha.GetDigest(), sha.GetLength(), true);
}

bool PasswordHash::Verify(std::string const& stored, std::string const& name, std::string const& password, bool* needsUpgrade)
{
    if (needsUpgrade)
        *needsUpgrade = false;

    if (Argon2::IsArgon2(stored))
        return Argon2::Verify(password, stored);

    std::string const legacy = CalculateLegacy(name, password);
    if (stored.size() != legacy.size() || CRYPTO_memcmp(stored.data(), legacy.data(), legacy.size()) != 0)
        return false;

    if (needsUpgrade)
        *needsUpgrade = true;
    return true;
}

void PasswordHash::SpendVerifyTime(std::string const& password)
{
    static std::string const dummy = Argon2::Hash("DUMMY");
    Argon2::Verify(password, dummy);
}
