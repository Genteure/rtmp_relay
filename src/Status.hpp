//
//  rtmp_relay
//

#pragma once

#include <memory>
#include "StatusSender.hpp"

namespace relay
{
    class Relay;

    enum class ReportType
    {
        TEXT,
        HTML,
        JSON
    };

    class Status
    {
    public:
        Status(cppsocket::Network& aNetwork, Relay& aRelay, const std::string& address);

        Status(const Status&) = delete;
        Status& operator=(const Status&) = delete;
        Status(Status&& other) = delete;
        Status& operator=(Status&& other) = delete;

        void update(float delta);

    private:
        void handleAccept(cppsocket::Socket& acceptor, cppsocket::Socket& clientSocket);

        cppsocket::Network& network;
        cppsocket::Socket socket;
        Relay& relay;

        std::vector<std::unique_ptr<StatusSender>> statusSenders;
    };
}
