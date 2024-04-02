
#include "EchoSession.h"
#include "MessageBuffer.h"

#include <string_view>


EchoSession::EchoSession(tcp::socket && socket) : Socket(std::move(socket))
{

}

void EchoSession::Start()
{
    std::string ip_address = GetRemoteIpAddress().to_string();
    std::cout << "Accepted connection from " << ip_address << std::endl;

    AsyncRead();
}

bool EchoSession::Update()
{
    return EchoSocket::Update();
}

void EchoSession::OnClose()
{
    std::string ip_address = GetRemoteIpAddress().to_string();
    std::cout << "Closed connection at " << ip_address << std::endl;
    EchoSocket::OnClose();
}

void EchoSession::ReadHandler()
{
    MessageBuffer& packet = GetReadBuffer();
    // 分隔完整的数据包
    const char *rptr = reinterpret_cast<const char*>(packet.GetReadPointer());
    size_t len = packet.GetActiveSize();

    std::string_view buf(rptr, len);

    size_t pos = buf.find_first_of("\r\n");
    if (pos == std::string_view::npos)
        return;
    std::cout << "C:" << std::string(rptr, pos+2);

    SendPacket(packet.GetReadPointer(), pos);

    packet.ReadCompleted(pos+2);
    AsyncRead();
}

void EchoSession::SendPacket(const uint8_t* msg, std::size_t len)
{
    MessageBuffer buf(len+4);
    buf.Write("S:", 2);
    buf.Write(msg, len);
    buf.Write("\r\n", 2);
    QueuePacket(std::move(buf));
}
