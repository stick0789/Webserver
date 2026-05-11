#include "../../inc/server/Client.hpp"
#include <iostream>
#include <unistd.h>

Client::Client(void)
    :_fd(-1),
    _lastActivity(time(NULL)),
    _responseFd(-1),
    _config(NULL)
{}

Client::Client(int fd, const ServerConfig *config)
    :_fd(fd),
    _lastActivity(time(NULL)),
    _responseFd(-1),
    _config(config)
{
    if (config)
        _parser.setMaxBodySize(config->getClientMaxBodySize());
}

Client::Client(const Client &other)
    :_fd(other._fd),
    _lastActivity(other._lastActivity),
    _requestBuffer(other._requestBuffer),
    _responseFd(other._responseFd),
    _config(other._config)
{}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
    {
        this->_fd = other._fd;
        this->_lastActivity = other._lastActivity;
        this->_requestBuffer = other._requestBuffer;
        this->_responseFd = other._responseFd;
        this->_config = other._config;
    }
    return (*this);
}

Client::~Client(void)
{
    if (this->hasResponse())
        close(this->_responseFd);
}

void Client::updateActivity(void)
{
    this->_lastActivity = time(NULL);
}

bool Client::hasTimedOut(int timeoutLimit) const
{
    return ((time(NULL) - this->_lastActivity) > timeoutLimit);
}

void Client::appendRequest(const char *data, ssize_t size)
{
    this->_requestBuffer.append(data, size);
}

void Client::setResponseFd(int fd)
{
    this->_responseFd = fd;
}

bool Client::hasResponse(void) const
{
    return (this->_responseFd >= 0);
}

void Client::clearRequest(void)
{
    this->_requestBuffer.clear();
}

int Client::getFd(void) const
{
    return (this->_fd);
}

const std::string &Client::getRequestBuffer(void) const
{
    return (this->_requestBuffer);
}

int Client::getResponseFd(void) const
{
   return (this->_responseFd);
}

const ServerConfig *Client::getConfig(void) const
{
    return (this->_config);
}

/*
    recicla esto

    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead < 0)
    {
        /*
            Since we set the O_NONBLOCK, 
            EWOULDBLOCK / EAGAIN: "No data available right now. Try again later."
            return true because is not a error, just nothing to say.
        *//*
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true);
        std::cout << "\033[1;31m[ERROR] recv() failed with fd: " << fd << "\033[0m" << std::endl;
        return (false);
    }
    else if (bytesRead == 0)
    {
        std::cout << "\033[93m[INFO] Conection closed with fd: " << fd << ")\033[0m" << std::endl;
        return false;
    }
    size_t currentSize = this->_clientBuffers[fd].size();
    size_t maxBody = it->second->getClientMaxBodySize();
    if (currentSize + bytesRead > maxBody)
    {
        std::cout << "\033[1;31m[ERROR] Request bigger than client_max_body_size ("
                    << maxBody << " bytes) with fd: " << fd << "\033[0m" << std::endl; 
        // error 413 here?
        return (false);
    }
    buffer[bytesRead] = '\0';
    this->_clientBuffers[fd].append(buffer, bytesRead);
    #ifdef DEBUG
        std::cout << "[DEBUG] Receibed" << bytesRead << "bytes from fd " << fd << std:endl;
    #endif
*/