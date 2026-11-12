////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef PasswordHash_h__
#define PasswordHash_h__

#include "Define.h"
#include <string>

// Name and password must already be normalized with Utf8ToUpperOnlyLatin
namespace PasswordHash
{
    // SHA256(HEX_UPPER(SHA256(name)) + ":" + password), hex upper, byte-reversed. Only used to check old rows
    TC_COMMON_API std::string CalculateLegacy(std::string const& name, std::string const& password);

    // Accepts both formats. needsUpgrade is set when the match was on the legacy format
    TC_COMMON_API bool Verify(std::string const& stored, std::string const& name, std::string const& password, bool* needsUpgrade = nullptr);

    // Spends the time of a real Argon2id check, so an unknown account answers as slowly as a known one
    TC_COMMON_API void SpendVerifyTime(std::string const& password);
}

#endif // PasswordHash_h__
