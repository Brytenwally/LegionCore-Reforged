////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
////////////////////////////////////////////

#ifndef Argon2_h__
#define Argon2_h__

#include "Define.h"
#include <string>

// PHC strings: $argon2id$v=19$m=<KiB>,t=<iterations>,p=<lanes>$<salt>$<hash>, standard base64 without padding (same as PHP password_hash)
namespace Argon2
{
    TC_COMMON_API bool IsArgon2(std::string const& stored);

    // Empty string on failure
    TC_COMMON_API std::string Hash(std::string const& normalizedPassword);

    // Any error counts as a mismatch
    TC_COMMON_API bool Verify(std::string const& normalizedPassword, std::string const& stored);
}

#endif // Argon2_h__
