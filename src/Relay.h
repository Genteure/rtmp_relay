//
//  rtmp_relay
//

#pragma once

#include <utility>
#include <vector>
#include <memory>
#include "Network.h"
#include "Socket.h"
#include "Status.h"
#include "Server.h"

#if !defined(_MSC_VER)
#include <sys/syslog.h>
#endif

namespace relay
{
    class Status;

    class Relay
    {
    public:
        static uint64_t nextId() { return ++currentId; }

        Relay(cppsocket::Network& aNetwork);

        Relay(const Relay&) = delete;
        Relay(Relay&&) = delete;
        Relay& operator=(const Relay&) = delete;
        Relay& operator=(Relay&&) = delete;

        bool init(const std::string& config);
        void close();

        void run();

        void getInfo(std::string& str, ReportType reportType) const;

        void openLog();
        void closeLog();

        const Connection::Description* getConnectionDescription(const std::pair<uint32_t, uint16_t>& address, Connection::StreamType type, const std::string& applicationName, const std::string& streamName) const;

    private:
        void handleAccept(cppsocket::Socket& acceptor, cppsocket::Socket& clientSocket);

        static uint64_t currentId;
        bool active = true;

        cppsocket::Network& network;
        std::unique_ptr<Status> status;
        std::chrono::steady_clock::time_point previousTime;

        std::vector<std::unique_ptr<Server>> servers;
        std::vector<std::unique_ptr<Connection>> connections;

        std::vector<cppsocket::Socket> acceptors;

#if !defined(_MSC_VER)
        std::string syslogIdent;
        int syslogFacility = LOG_USER;
#endif
    };
}
