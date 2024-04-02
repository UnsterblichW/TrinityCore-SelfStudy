

#pragma once

#include "Socket.h"


class EchoSession : public Socket<EchoSession>
{
    typedef Socket<EchoSession> EchoSocket;
public:
    EchoSession(tcp::socket&& socket);

public:
    // 连接建立成功接下来干啥
    virtual void Start() override;

    // 异步把数据包发送出去、future 异步处理数据库的回调
    virtual bool Update() override;

    void SendPacket(const uint8_t* msg, std::size_t len);

protected:
    // 断开连接的回调
    virtual void OnClose() override;
    // 数据包到达后，解析协议以及分发数据包
    virtual void ReadHandler() override;
};
