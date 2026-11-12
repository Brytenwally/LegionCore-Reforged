////////////////////////////////////////////
///                                      ///
///              DEMONCORE               ///
///    Copyright 2026 Titans-Project     ///
///         All Rights Reserved.         ///
///                                      ///
///      Based on TrinityCore code       ///
///   Copyright 2008-2014 TrinityCore    ///
///   <http://www.trinitycore.org/>      ///
///                                      ///
////////////////////////////////////////////

#ifndef __WORLDSOCKET_H__
#define __WORLDSOCKET_H__

#include "DatabaseEnvFwd.h"
#include "MPSCQueue.h"
#include "Socket.h"
#include "Util.h"
#include "WorldPacketCrypt.h"
#include "WorldSession.h"
#include <safe_ptr.h>

struct z_stream_s;

class EncryptablePacket : public WorldPacket
{
public:
    EncryptablePacket(WorldPacket const& packet, bool encrypt) : WorldPacket(packet), _encrypt(encrypt) { }

    bool NeedsEncryption() const { return _encrypt; }

private:
    bool _encrypt;
};

namespace WorldPackets
{
    class ServerPacket;
    namespace Auth
    {
        class AuthSession;
        class AuthContinuedSession;
        class ConnectToFailed;
        class LogDisconnect;
        class Ping;
        class WardenData;
    }
}

#pragma pack(push, 1)

struct PacketHeader
{
    uint32 Size;
    uint16 Command;

    // 256 KB, as in later TrinityCore: past ~5,700 hotfixes the client's CMSG_HOTFIX_REQUEST outgrows 64 KB
    bool IsValidSize() { return Size < 0x40000; }
    bool IsValidOpcode() { return Command < NUM_OPCODE_HANDLERS; }
};

#pragma pack(pop)

class WorldSocket : public Socket<WorldSocket>
{
    static std::string const ServerConnectionInitialize;
    static std::string const ClientConnectionInitialize;
    static uint32 const MinSizeForCompression;

    static uint8 const AuthCheckSeed[16];
    static uint8 const SessionKeySeed[16];
    static uint8 const ContinuedSessionSeed[16];

    typedef Socket<WorldSocket> BaseSocket;

public:
    WorldSocket(boost::asio::ip::tcp::socket&& socket);
    ~WorldSocket();

    WorldSocket(WorldSocket const& right) = delete;
    WorldSocket& operator=(WorldSocket const& right) = delete;

    void Start() override;
    bool Update() override;

    void SendPacket(WorldPacket const& packet);

    ConnectionType GetConnectionType() const { return _type; }

    void SendAuthResponseError(uint32 code);
    void HandleEnableEncryptionAck();
    void SetWorldSession(WorldSessionPtr session);
    void SetSendBufferSize(std::size_t sendBufferSize) { _sendBufferSize = sendBufferSize; }

protected:
    void OnClose() override;
    void ReadHandler() override;
    bool ReadHeaderHandler();

    enum class ReadDataHandlerResult
    {
        Ok = 0,
        Error = 1,
        WaitingForQuery = 2
    };

    ReadDataHandlerResult ReadDataHandler();
private:
    void CheckIpCallback(PreparedQueryResult result);
    void InitializeHandler(boost::system::error_code const& error, std::size_t transferedBytes);
    void LogOpcodeText(OpcodeClient opcode, std::unique_lock<std::mutex> const& guard) const;
    void WritePacketToBuffer(EncryptablePacket const& packet, MessageBuffer& buffer);
    uint32 CompressPacket(uint8* buffer, WorldPacket const& packet);

    void HandleSendAuthSession();
    void HandleAuthSession(std::shared_ptr<WorldPackets::Auth::AuthSession> authSession);
    void HandleAuthSessionCallback(std::shared_ptr<WorldPackets::Auth::AuthSession> authSession, PreparedQueryResult result);
    void HandleAuthContinuedSession(std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession);
    void HandleAuthContinuedSessionCallback(std::shared_ptr<WorldPackets::Auth::AuthContinuedSession> authSession, PreparedQueryResult result);
    void HandleConnectToFailed(WorldPackets::Auth::ConnectToFailed& connectToFailed);

    bool HandlePing(WorldPackets::Auth::Ping& ping);
    bool HandleWardenData(WorldPackets::Auth::WardenData& packet);

    ConnectionType _type;
    uint64 _key;

    BigNumber _serverChallenge;
    WorldPacketCrypt _authCrypt;
    BigNumber _encryptSeed;
    BigNumber _decryptSeed;
    BigNumber _sessionKey;

    TimePoint _LastPingTime;
    uint32 _OverSpeedPings;
    uint32 _accountId;

    std::mutex _worldSessionLock;
    WorldSessionPtr _worldSession;
    bool _authed;

    MessageBuffer _headerBuffer;
    MessageBuffer _packetBuffer;
    MPSCQueue<EncryptablePacket> _bufferQueue;
    std::size_t _sendBufferSize;

    z_stream_s* _compressionStream;

    QueryCallbackProcessor _queryProcessor;
    std::string _ipCountry;
};

#endif
