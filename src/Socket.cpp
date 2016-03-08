//
//  rtmp_relay
//

#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include "Socket.h"
#include "Network.h"
#include "Utils.h"

Socket::Socket(Network& network, int sock):
    _network(network), _data(65536), _socket(sock)
{
    if (_socket <= 0)
    {
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        
        if (_socket < 0)
        {
            int error = errno;
            std::cerr << "Failed to create socket, error: " << error << std::endl;
        }
    }
    
    _network.addSocket(*this);
}

Socket::~Socket()
{
    _network.removeSocket(*this);
    if (_socket > 0) close(_socket);
}

Socket::Socket(Socket&& other):
    _network(other._network),
    _data(other._data),
    _dataSize(other._dataSize),
    _socket(other._socket),
    _connecting(other._connecting),
    _ready(other._ready),
    _blocking(other._blocking)
{
    _network.addSocket(*this);
    
    other._socket = 0;
    other._dataSize = 0;
    other._connecting = false;
    other._ready = false;
    other._blocking = true;
}

Socket& Socket::operator=(Socket&& other)
{
    _data = other._data;
    _dataSize = other._dataSize;
    _socket = other._socket;
    _connecting = other._connecting;
    _ready = other._ready;
    _blocking = other._blocking;
    
    other._socket = 0;
    other._dataSize = 0;
    other._connecting = false;
    other._ready = false;
    other._blocking = true;
    
    return *this;
}

bool Socket::connect(const std::string& address, uint16_t port)
{
    uint32_t ip;
    
    size_t i = address.find(':');
    std::string addressStr;
    std::string portStr;
    
    if (i != std::string::npos)
    {
        addressStr = address.substr(0, i);
        portStr = address.substr(i + 1);
    }
    else
    {
        addressStr = address;
        portStr = std::to_string(port);
    }
    
    addrinfo* result;
    if (getaddrinfo(addressStr.c_str(), portStr.empty() ? nullptr : portStr.c_str(), nullptr, &result) != 0)
    {
        int error = errno;
        std::cerr << "Failed to get address info, error: " << error << std::endl;
        return false;
    }
    
    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    ip = addr->sin_addr.s_addr;
    port = ntohs(addr->sin_port);
    
    freeaddrinfo(result);
    
    return connect(ip, port);
}

bool Socket::connect(uint32_t ipAddress, uint16_t port)
{
    if (_socket <= 0)
    {
        _socket = socket(AF_INET, SOCK_STREAM, 0);
        
        if (_socket < 0)
        {
            int error = errno;
            std::cerr << "Failed to create socket, error: " << error << std::endl;
            return false;
        }
    }
    
    std::cout << "Connecting to " << ipToString(ipAddress) << ":" << static_cast<int>(port) << std::endl;
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ipAddress;
    
    if (::connect(_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        if (errno == EINPROGRESS)
        {
            _connecting = true;
        }
        else
        {
            int error = errno;
            std::cerr << "Connection failed, error: " << error << std::endl;
            return false;
        }
    }
    else
    {
        // connected
        _ready = true;
    }
    
    return true;
}

bool Socket::setBlocking(bool blocking)
{
#ifdef WIN32
    unsigned long mode = blocking ? 0 : 1;
    if (ioctlsocket(_socket, FIONBIO, &mode) != 0)
    {
        return false;
    }
#else
    int flags = fcntl(_socket, F_GETFL, 0);
    if (flags < 0) return false;
    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    
    if (fcntl(_socket, F_SETFL, flags) != 0)
    {
        return false;
    }
#endif
    
    _blocking = blocking;
    
    return true;
}

bool Socket::send(std::vector<char> buffer)
{
    if (::send(_socket, buffer.data(), buffer.size(), 0) < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            int error = errno;
            std::cerr << "Failed to send data, error: " << error << std::endl;
            return false;
        }
    }
    
    return true;
}

bool Socket::read()
{
    ssize_t size = recv(_socket, _data.data() + _dataSize, _data.size() - _dataSize, 0);
    
    if (size < 0)
    {
        int error = errno;
        
        if (_connecting)
        {
            std::cerr << "Connection failed, error: " << error << std::endl;
            _connecting = false;
        }
        else
        {
            std::cerr << "Failed to read from socket, error: " << error << std::endl;
        }
        
        return false;
    }
    else if (size == 0)
    {
        std::cout << "Socket disconnected" << std::endl;
        _ready = false;
        return false;
    }
    
    _dataSize += size;
    
    return true;
}

bool Socket::write()
{
    if (_connecting)
    {
        _connecting = false;
        _ready = true;
        std::cout << "Connected" << std::endl;
    }
    
    return true;
}