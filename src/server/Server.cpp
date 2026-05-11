#include "../inc/server/Server.hpp"
#include "../inc/parser/RequestParser.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include <iostream>
#include <sys/types.h> //System types (size_t, ssize_t...)
#include <sys/socket.h> //socket(), bind(), listen(), accept(), setsockopt(), sockaddr
#include <netinet/in.h> // sockaddr_in, htons(), htonl()
#include <fcntl.h> //To enable non-blocking mode fcntl() and O_NONBLOCK
#include <cstring> // memset()
#include <poll.h> //I/O multiplexing poll()
#include <unistd.h> //close for fd
#include <netdb.h> //getaddrinfo, freeaddrinfo
#include <sstream> //to turn int to string
#include <arpa/inet.h>> //for inet_ntoa

void printServerInfo(std::string &host, int port, const std::vector<std::string> &names)
{

    std::cout << std::endl << "\033[96m--- [SERVER] listening on ---\033[0m" << std::endl
    << "-IP/HOST: " << host << std::endl
    << "-PORT: " << port << std::endl;
    bool validName = false;
            for(size_t n = 0; n < names.size(); n++)
            {
                if (!names[n].empty())
                {
                    if (!validName)
                    {
                        std::cout << "-SERVER NAME(S): " << std::endl;
                        validName = true;
                    }
                    std::cout << "\t" << names[n] << std::endl;
                }
            }
}

static int responseToFd(const std::string &responseStr)
{
    int pipeFds[2];
    if (pipe(pipeFds) < 0)
    {
        std::cout << "\033[1;31m[ERROR] pipe() failed.\033[0m" << std::endl;
        return (-1);
    }
 
    // Escribir toda la respuesta en el extremo de escritura y cerrarlo
    // Para respuestas grandes esto bloquearía si el pipe se llena,
    // pero en webserv de 42 las respuestas caben en el buffer del kernel (64KB típico).
    ssize_t written = write(pipeFds[1], responseStr.c_str(), responseStr.size());
    close(pipeFds[1]);
 
    if (written < 0)
    {
        close(pipeFds[0]);
        return (-1);
    }
    return (pipeFds[0]); // fd de lectura
}


bool Server::setupSockets(void)
{
    std::vector<std::string>    openedHosts;
    std::vector<int>            openedPorts;

    //first we go over all servers
    for (size_t i = 0; i < this->_allServers.size(); i++)
    {
        std::string host = this->_allServers[i].getHost();
        const std::vector<int> &ports = this->_allServers[i].getPorts();

        //now we have to go over all ports
        for (size_t p = 0; p < ports.size(); p++)
        {
            int port = ports[p];

            bool isOpened = false;
            for (size_t j = 0; j < openedPorts.size(); j++)
            {
                if (openedHosts[j] == host && openedPorts[j] == port)
                {
                     isOpened = true;
                    break;
                }
            }
            if (isOpened)
                continue;
            int listenFd = socket(AF_INET, SOCK_STREAM, 0);
            if (listenFd < 0)
            {
                std::cout << "\033[1;31m[ERROR] Socket creation failed.\033[0m" << std::endl;
                return (false);
            }
            
            int opt = 1;
            if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
            {
                #ifdef DEBUG
                    std::cout << "[DEBUG-SERVER] Socket config failed." << std::endl;
                #endif
                close(listenFd);
                return (false);
            }
            if (fcntl(listenFd, F_SETFL,O_NONBLOCK) < 0)
            {
                std::cout << "\033[1;31m[ERROR] fcntl failed to set NONBLOCK.\033[0m" << std::endl;
                close(listenFd);
                return (false);
            }
            struct addrinfo hints, *res;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype  = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            std::stringstream ss;
            ss << port;
            std::string portStr = ss.str();

            if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0)
            {
                std::cout << "\033[1;31m[ERROR] getaddrinfo failed for host '" << host << "' and port '" << port << "'.\033[0m" << std::endl;
                close(listenFd);
                return (false);
            }

            if (bind(listenFd, res->ai_addr, res->ai_addrlen) < 0)
            {
                std::cout << "\033[1;31m[ERROR] Port " << port << " on host '" << host
                << "' is already in use or cannot be bound.\n\tMake sure no other server is running on this port and que la IP existe.\033[0m" << std::endl;
                freeaddrinfo(res);
                close(listenFd);
                return (false);
            }

            freeaddrinfo(res);
                //SOMAXCONN is a macro that defines max pending connections queue for a listening socket. (usually 128 on linux)
            if (listen(listenFd, SOMAXCONN) < 0)
            {
                std::cout << "\033[1;31m[ERROR] listen() failed.\033[0m" << std::endl;
                close(listenFd);
                return (false);
            }

            struct pollfd pfd;
            pfd.fd = listenFd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            this->_fds.push_back(pfd);
            this->_listenFds.push_back(listenFd);

            openedHosts.push_back(host);
            openedPorts.push_back(port);

            const std::vector<std::string> &names = this->_allServers[i].getServerNames();
            printServerInfo(host, port, names);
        }

    }
    return (!this->_listenFds.empty()); 
}

bool Server::acceptNewConnection(int fd)
{

    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &clientLen);
    const ServerConfig *config = NULL;

    if (clientFd < 0)
    {
        std::cout << "\033[1;31m[ERROR]  accept() failed. \033[0m" << std::endl;
        return (false);
    }
    if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
    {
        std::cout << "\033[1;31m[ERROR]  New connection fcntl() failed to set O_NONBLOCK. \033[0m" << std::endl;
        close(clientFd);
        return (false);
    }
    for (size_t i = 0; i < this->_listenFds.size(); i++)
    {
        if (this->_listenFds[i] == fd)
        {
            config = &this->_allServers[i];
            break;
        }
    }
    if (config)
        this->_clients[clientFd] = Client(clientFd, config);
    else
    {
        std::cout << "\033[1;31m[ERROR] No ServerConfig found for listening fd " << fd << ". Closing client.\033[0m" << std::endl;
        close(clientFd);
        return false;
    }

    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    this->_fds.push_back(pfd);

    /*
        OS stores IP as a huge unsigned int,
        inet_ntoa convers it to readeable format X.X.X.X
        
        Internet reverses bits to transfer data, so
        ntohs has to reverse them again to be printable
        Ex: port 80 gets stored as hex 00 50; but
        if you try to print it it will reverse them and 
        print 50 00. 
        So 80 becomes 20480.
    */
    std::cout << "\033[92m-New client-\nfd: " << clientFd << std::endl
                << "\tIP:" << inet_ntoa(clientAddr.sin_addr) << std::endl
                << "\tPORT: " << ntohs(clientAddr.sin_port) << "\033[0m" << std::endl;

    return (true);
}

/*
    I need to get the client_max_body_size from the .conf file.
    so I needed to somehow map a fd to a ServerConfig, that way
    with this->_fdToServerConfig[clientFd] = config;
    Each client knows what is its config and I can use getClientMaxBodySize();
*/
bool Server::readFromClient(int fd)
{
    char buffer[READ_BUFFER];
    std::map<int, Client>::iterator it = this->_clients.find(fd);
    if (it == this->_clients.end())
        return (false);
    Client &client = it->second;

    ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true);
        std::cout << "\033[1;31m[ERROR] recv() failed for fd: " << fd << "\033[0m" << std::endl;
        return (false);
    }

    if (bytesRead == 0)
    {
        std::cout << "\033[93m[INFO] Connection closed with fd: " << fd << "\033[0m" << std::endl;
        return (false);
    }
    
    buffer[bytesRead] = '\0';
    //client.appendRequest(buffer, bytesRead);
    client.getParser().feed(std::string(buffer, bytesRead));
    #ifdef DEBUG
        std::cout << "[DEBUG] Received " << bytesRead << " bytes from fd " << fd << std::endl;
    #endif
/**
 * if (client.getRequestBuffer().find("\r\n\r\n") != std::string::npos)
    {
        #ifdef DEBUG
            std::cout << "[DEBUG] Request complete for fd " << fd << std::endl;
        #endif

        //httphandler calls here
        RequestParser handler;
        handler.feed(client);

        if (client.hasResponse())
            setClientEvents(fd, POLLOUT);
    }
 * -- */
    if (!client.getParser().isComplete())
    {
        return (true);
    }
    HTTPRequest request = client.getParser().getRequest();
    if (request.getErrorCode() != 0)
    {
        std::cout << "\033[93m[INFO] Parse error " << request.getErrorCode()
                  << " for fd " << fd << "\033[0m" << std::endl;
 
        HTTPResponse errResp = HTTPResponse::buildErrorResponse(request.getErrorCode());
        std::string serialized = errResp.serialize();
 
        int responseFd = responseToFd(serialized);
        if (responseFd < 0)
            return (false);
 
        client.setResponseFd(responseFd);
        setClientEvents(fd, POLLOUT);
        return (true);
    }
    
    HTTPResponse response = HTTPResponse::buildErrorResponse(501);
    std::string serialized = response.serialize();
 
    int responseFd = responseToFd(serialized);
    if (responseFd < 0)
        return (false);
 
    client.setResponseFd(responseFd);
    setClientEvents(fd, POLLOUT);
 
    // ── Keep-alive: resetear el parser para el siguiente request ───────────
    if (request.shouldKeepAlive())
        client.getParser().reset();

    return (true);
}

// fd should contain everything so just send it to client
bool Server::sendToClient(int fd)
{
    std::map<int, Client>::iterator it = this->_clients.find(fd);
    if (it == this->_clients.end())
        return (false);
    Client &client = it->second;
    int bodyFd = client.getResponseFd();
    if (bodyFd < 0)
        return (true); //nothing to send
    char buffer[READ_BUFFER];
    ssize_t bytesRead = read(bodyFd, buffer, sizeof(buffer));

    if (bytesRead < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (true); //retry
        
        std::cout << "\033[1;31m[ERROR] read() failed on fd " 
                  << bodyFd << ": " << strerror(errno) << "\033[0m" << std::endl;
        return (false);
    }
    if (bytesRead == 0)
    {
        close(bodyFd);
        client.setResponseFd(-1);
        client.clearRequest();
        #ifdef DEBUG
            std::cout << "[DEBUG] Response fully sent to fd " << fd << std::endl;
        #endif
        //keep-alive
        setClientEvents(fd, POLLIN);
        return (true);
    }
    ssize_t totalSent = 0;
    while (totalSent < bytesRead)
    {
        ssize_t sent = send(fd, buffer+totalSent, bytesRead - totalSent, MSG_DONTWAIT);
        if (sent < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return (true);

            std::cout << "\033[1;31m[ERROR] send() failed for fd: " 
                      << fd << "\033[0m" << std::endl;
            return (false);
        }
        totalSent += sent;
    }

    #ifdef DEBUG
        std::cout << "[DEBUG] Sent " << totalSent << " bytes to fd " << fd << std::endl;
    #endif

    return (true);
}

bool Server::isListening(int fd)
{
    for (size_t i = 0; i < this->_listenFds.size(); i++)
    {
        if (fd == this->_listenFds[i])
            return (true);
    }
    return (false);
}

std::string Server::getRawRequest(int client_fd) const
{
    std::map<int, Client>::const_iterator it = this->_clients.find(client_fd);
    if (it != this->_clients.end())
        return (it->second.getRequestBuffer());
    return ("");
}

void Server::kickClient(int fd)
{
    std::map<int, Client>::iterator it = this->_clients.find(fd);
    if (it != this->_clients.end())
    {
        if (it->second.hasResponse())
            close(it->second.getResponseFd());
    }
    close(fd);
    for (size_t i = 0; i < this->_fds.size(); i++)
    {
        if (this->_fds[i].fd == fd)
        {
            this->_fds.erase(_fds.begin() + i); // erase needs a iterator, so begin creates the iterator at 0, then we add 'i' so we erase the right one
            break;
        }
    }
    this->_clients.erase(fd);
    #ifdef DEBUG
        std::cout << "[DEBUG] client with fd '" << fd << "' was kicked." << std::endl;
    #endif
}

void Server::setClientEvents(int fd, short events)
{
    for (size_t i = 0; i < this->_fds.size(); i++)
    {
        if (this->_fds[i].fd == fd)
        {
            this->_fds[i].events = events;
            break;
        }
    }
}

void Server::checkTimeouts(void)
{
    for (std::map<int, Client>::iterator it = this->_clients.begin();
        it != this->_clients.end();)
    {
        if(it->second.hasTimedOut(IDLE_TIMEOUT / 1000))
        {
            kickClient(it->first);
            this->_clients.erase(it++);
        }
        else
            ++it;
    }
}

void Server::cleanup(void)
{
    for (size_t i = 0; i < this->_fds.size(); i++)
    {
        if (this->_fds[i].fd >= 0)
            close(this->_fds[i].fd);
    }
    this->_fds.clear();
    #ifdef DEBUG
        std::cout << "\033[31mAll resources cleaned!\033[0m" << std::endl;
    #endif
}

Server::Server(void)
{
    #ifdef DEBUG
        std::cout << "Default server created" << std::endl;
    #endif
}

Server::Server(const ConfigParser &configs):_allServers(configs.getParsedServerConfigs())
{
    #ifdef DEBUG
        std::cout << "Server created using config file" << std::endl;
    #endif
}

Server::Server(const Server &other)
    :_allServers(other._allServers),
    _fds(other._fds),
    _listenFds(other._listenFds)
{
    #ifdef DEBUG
        std::cout << "Server created as a copy" << std::endl;
    #endif
}

Server &Server::operator=(const Server &other)
{
    if (this != &other)
    {
        _allServers = other._allServers;
        _fds = other._fds;
        _listenFds = other._listenFds;
    }
    #ifdef DEBUG
        std::cout << "Server assigned as a copy" << std::endl;
    #endif
    return (*this);
}

Server::~Server(void)
{
    cleanup();
    #ifdef DEBUG
        std::cout << "\033[1;32mServer successfully destroyed!\033[0m" << std::endl;
    #endif
}

bool Server::run(void)
{
    if (!setupSockets())
        return (false);

    bool activeServer = true;
    while (activeServer)
    {
        checkTimeouts();
        int pollRes = poll(&this->_fds[0], this->_fds.size(), POLL_TIMEOUT);
        if (pollRes < 0)
        {
            activeServer = false;
            break;
        }
        if (pollRes == 0)
            continue;
        for (int i = this->_fds.size() - 1; i >= 0 ; i--)
        {
            if (this->_fds[i].revents == 0)
                continue;
            
            int fd = _fds[i].fd;
            
            if(isListening(fd))
            {
                if (!acceptNewConnection(fd))
                    activeServer = false;
                continue;
            }
            /*
                POLLRDHUP is kind of new and linux-specific so need to be protected
                POLLIN - there's data waiting to be read.
                POLLPRI - there's urgent data waiting to be read.
                POLLOUT - we can write.
                POLLHUP - client disconnected or connection lost
                POLLERR - socket is broken
                POLLNVAL - invalid fd, probably closed but not removed from
                            _fds vector. So poll tried to check a closed fd
                POLLRDHUP - its like a nice goodbye. The client finished "talking" and left.
            */
            # ifdef POLLRDHUP
                if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL | POLLRDHUP))
            # else
                if (_fds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
            # endif
            {
                kickClient(fd);
                continue;
            }
            if (_fds[i].revents & POLLIN)
            {
                if (!readFromClient(fd))
                    kickClient(fd);
                else
                    this->_clients[fd].updateActivity();
            }
            if (_fds[i].revents & POLLOUT)
            {
                if (!sendToClient(fd))
                {
                    kickClient(fd);
                    continue;
                }
                else
                    this->_clients[fd].updateActivity();
            }
        }
    }
    return (activeServer);
}