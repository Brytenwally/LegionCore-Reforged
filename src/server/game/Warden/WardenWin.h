////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2012 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#ifndef _WARDEN_WIN_H
#define _WARDEN_WIN_H

#include <map>
#include "Cryptography/ARC4.h"
#include "BigNumber.h"
#include "ByteBuffer.h"
#include "Warden.h"

class WorldSession;
class Warden;

class WardenWin : public Warden
{
    public:
        WardenWin(WorldSession* session);
        ~WardenWin() {}

        void InitializeModule();
        void InitializeMPQCheckFunc(ByteBuffer& buff);
        void InitializeLuaCheckFunc(ByteBuffer& buff);
        void InitializeTimeCheckFunc(ByteBuffer& buff);

        void HandleHashResult(ByteBuffer &buff) override;
        void HandleHashResultSpecial(ByteBuffer &buff) override;
        void HandleModuleFailed() override;
        void RequestBaseData() override;
        void SendExtendedData() override;

        void HandleData(ByteBuffer &buff) override;
        void HandleChecks(ByteBuffer &buff);

        void HandleExtendedData(ByteBuffer &buff) override;
        void HandleStringData(ByteBuffer &buff) override;
        void HandleFailedSync(uint32 clientSeqIndex);

        // temp
        void ActivateModule() override;

        void BuildBaseChecksRequest(ByteBuffer &buff);
        void BuildSequenceHeader(ByteBuffer &buff);
        void AddCheckData(uint16 id, ByteBuffer &buff, ByteBuffer &stringBuf, uint8 &index);

        template<class T>
        void WriteAddress(ByteBuffer & buff, T address);

    private:
        std::map<std::string, std::vector<uint16>> _baseChecksList;
        std::vector<uint16> _currentChecks;

        uint32 _sequencePacketIndex = 0;
};

#endif
