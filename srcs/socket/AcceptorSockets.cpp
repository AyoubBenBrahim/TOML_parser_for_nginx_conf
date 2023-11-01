#include "AcceptorSockets.hpp"


AcceptorSockets::AcceptorSockets(in_addr host, int port, int max_clients)
{
    this->listen_port = port;
    this->host = host;
    this->backlogQueueMax = (max_clients == 0) ? 100 : max_clients;
    this->_addrlen = sizeof(_addr);
}

AcceptorSockets::~AcceptorSockets()
{
}

void AcceptorSockets::socketAPI()
{
    this->create_socket();
    this->bind_socket();
    this->listen_socket();
}

// AcceptorSockets &AcceptorSockets::operator=(const AcceptorSockets &src)
// {
//     if (this != &src)
//     {
//         this->_AcceptorSocketFd = src.getSocketFd();
//         this->_addr = src.getSockAddr();
//         this->_addrlen = src.getAddrLen();
//         std::vector<int> copy = src.getClient();
//         for (auto it = copy.begin(); it != copy.end(); it++)
//         {
//             this->_clientsFD.push_back(*it);
//         }

//         // for (int index = 0; index < MAX_CLIENTS; index++)
//         // {
//         //     this->fds[index].fd = src.getFds()[index].fd;
//         //     this->fds[index].events = src.getFds()[index].events;
//         //     this->fds[index].revents = src.getFds()[index].revents;
//         // }
//     }
//     return *this;
// }

int AcceptorSockets::getSocketFd() const
{
    return this->_AcceptorSocketFd;
}

struct sockaddr_in AcceptorSockets::getSockAddr() const
{
    return this->_addr;
}

socklen_t AcceptorSockets::getAddrLen() const
{
    return this->_addrlen;
}

void AcceptorSockets::create_socket()
{
    _AcceptorSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_AcceptorSocketFd == -1)
    {
        throw std::runtime_error("socket() failed");
    }
}

void AcceptorSockets::bind_socket()
{
    /*
        _addr.sin_addr.s_addr = inet_addr(host.c_str());
        assigns the IP address specified by the host variable

        INADDR_ANY represents the IP address "0.0.0.0",
        indicating that the server socket should bind and
        listen on all available network interfaces on the system.
        This allows the server to accept incoming connections
        on any IP address associated with the machine.
    */

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(this->listen_port);
    _addr.sin_addr.s_addr = host.s_addr;
    // _addr.sin_addr.s_addr = INADDR_ANY;
    // memcpy(&_addr.sin_addr, &host, sizeof(host));

    // Set SO_REUSEADDR option
    int reuseAddr = 1;
    if (setsockopt(_AcceptorSocketFd, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) == -1)
    {
        throw std::runtime_error("setsockopt() failed");
    }

    if (bind(_AcceptorSocketFd, (struct sockaddr *)&_addr, sizeof(_addr)) == -1)
    {
        throw std::runtime_error("bind() failed");
    }

    if (fcntl(_AcceptorSocketFd, F_SETFL, O_NONBLOCK) == -1)
    {
        throw std::runtime_error("fcntl() failed");
    }
}

void AcceptorSockets::listen_socket()
{
    if (listen(_AcceptorSocketFd, this->backlogQueueMax) == -1)
    {
        throw std::runtime_error("listen() failed");
    }
    // std::cout << "Listening on port " << this->listen_port << std::endl;
}

int AcceptorSockets::accept_socket(int clientCount)
{
    int newClientFd = accept(_AcceptorSocketFd, (struct sockaddr *)&_addr, &_addrlen);
    std::cout << "newClient just connected: " << newClientFd << std::endl;
    if (newClientFd == -1)
    {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
        {
            throw std::runtime_error("accept() Just for check");
        }
        else
        {
            throw std::runtime_error("accept() failed");
        }
        return -1;
    }
    if (!checkMaxClients(clientCount + 1))
    {
        close(newClientFd);
        return -503; // HTTP status code 503 indicates that the server is temporarily unable to handle the request due to being overloaded 
    }
    return newClientFd;
}

bool AcceptorSockets::checkMaxClients(int clientCount)
{
    if (clientCount > this->backlogQueueMax)
    {
        std::cout << "ERROR: Max clients connections reached\n";
        return false;
    }
    return true;
}