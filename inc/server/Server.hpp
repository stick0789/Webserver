#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include <map>
# include <string>


# include "../parser/ServerConfig.hpp"
# include "../parser/ConfigParser.hpp"
# include "Client.hpp"

# define POLL_TIMEOUT   1000    //ms, for poll()
# define IDLE_TIMEOUT   30000   //ms, idle client
# define READ_TIMEOUT   5000    //ms, for a complex read
# define KEEP_TIMEOUT   15000   //for keep-alive petitions

# define READ_BUFFER    4096    //"chunk" size to read from client.

class Request;
class client;

class Server {
    private:
        std::vector<ServerConfig>   _allServers;
        std::vector<struct pollfd>  _fds;
        std::vector<int>            _listenFds;
        std::map<int, Client>       _clients;

        bool setupSockets(void);           // socket(), bind(), listen()
        bool acceptNewConnection(int fd);    // accept()
        bool readFromClient(int fd);   // recv() -> Read and store bytes
        bool sendToClient(int fd);     // send() -> Sends response to Protocol Handler
        bool isListening(int fd);   //fd is on _listen_fds?
        
        std::string  getRawRequest(int client_fd) const; //Returns the content of _clientBuffers[client_fd]

        void kickClient(int fd);
        void setClientEvents(int fd, short events);
        void checkTimeouts(void);
        void cleanup(void);
        
    public:
        Server(void);
        Server(const ConfigParser &configs);
        Server(const Server &other);
        Server &operator=(const Server &other);
        ~Server(void);

        bool run(void);
};

#endif
