//
//  rtmp_relay
//

#pragma once

#include <vector>

class Network;

class Socket
{
    friend Network;
public:
    Socket() = delete;
    Socket(Network& network, int sock = 0);
    virtual ~Socket();
    
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    
    Socket(Socket&& other);
    Socket& operator=(Socket&& other);
    
    bool connect(const std::string& address, uint16_t port = 0);
    bool connect(uint32_t ipAddress, uint16_t port);
    
    bool send(std::vector<char> buffer);
    
    int getSocketFd() const { return _socketFd; }
    
    bool isBlocking() const { return _blocking; }
    bool setBlocking(bool blocking);
    
    bool isConnecting() const { return _connecting; }
    bool isReady() const { return _ready; }
    
protected:
    virtual bool read();
    virtual bool write();
    
    Network& _network;
    
    int _socketFd = 0;
    
    std::vector<char> _data;
    
    bool _connecting = false;
    bool _ready = false;
    bool _blocking = true;
};
