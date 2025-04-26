/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:18 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/23 01:47:31 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//
Server::Server()
{
    ServerSocketFD = -1;
}

void Server::inputCheck(int ac, char **av)
{

    if (ac != 3)
        throw(std::runtime_error("Wrong args : ./ircserv <port> <password>"));

    _portStr = av[1];
    _password = av[2];

    char *end = NULL;

    long port = strtol(_portStr.c_str(), &end, 10);

    if (!_portStr.c_str()[0] || *end != '\0')
        throw(std::runtime_error("Port must be a valid number (digits only)"));

    if (port < 1024 || port > 65535)
        throw(std::runtime_error("Port must be between 1024 and 65535"));

    Port = static_cast<int>(port);
}

Client *Server::GetClient(int fd)
{
    for (size_t i = 0; i < this->clients.size(); i++)
    {
        if (this->clients[i].GetFd() == fd)
            return &this->clients[i];
    }
    return NULL;
};

void Server::CleanClient(int fd)
{
    /* 1. Remove the pollfd that matches fd */
    for (std::vector<struct pollfd>::iterator it = fds.begin();
         it != fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            fds.erase(it); // erase → O(N) but only once
            break;
        }
    }

    /* 2. Remove the Client whose key is fd (O(log N)) */
    clients.erase(fd); // does nothing if fd not present
}

void Server::CloseFds()
{
    for (size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client <" << clients[i].GetFd() << "> Disconnected" << std::endl;
        close(clients[i].GetFd());
    }
    if (ServerSocketFD != -1)
    {
        std::cout << "Server <" << ServerSocketFD << "> disconnected" << std::endl;
        close(ServerSocketFD);
    }
};

bool Server::Signal = false;

void Server::SignalHandler(int signum)
{
    (void)signum;
    std::cout << std::endl
              << "Signal Received" << std::endl;
    Server::Signal = true;
};

bool Server::CheckIfChannelExists(std::string str)
{
    return channels.find(str) != channels.end();
};

void parseCmd(std::string &str){
    std::string prefix;
    std::string cmd;
    std::string suffix;
    std::vector<std::string> args;
    std::stringstream ss(str);
    std::string word;

    if(str[0] == ':') 
    {
        ss >> prefix;
        prefix = prefix.substr(1);
    }
    ss >> cmd;
    while (ss >> word)
    {
        if (word[0] == ':')
        {
            suffix = word.substr(1);
            break;
        }
        args.push_back(word);
    }
    if (cmd == "CAP"){

    }
    else if (cmd == "INVITE"){

    }
    else if (cmd == "JOIN"){
        
    }
    else if (cmd == "KICK"){
        
    }
    else if (cmd == "KILL"){
        
    }
    else if (cmd == "MODE"){
        
    }
    else if (cmd == "NAMES"){
        
    }
    else if (cmd == "NICK"){
        
    }
    else if (cmd == "NOTICE"){
        
    }
    else if (cmd == "OPER"){
        
    }
    else if (cmd == "PART"){
        
    }
    else if (cmd == "PASS"){
        
    }
    else if (cmd == "PASS"){
        
    }
    else if (cmd == "PING"){
        
    }
    else if (cmd == "PRIVMSG"){
        
    }
    else if (cmd == "QUIT"){
        
    }
    else if (cmd == "TOPIC"){
        
    }
    else if (cmd == "UNKNOWN"){
        
    }
    else if (cmd == "USER"){
        
    }
    else if (cmd == "WHO"){
        
    }
}

void Server::nick(Client &client, std::string &str){
    client.SetNickname(str);
    std::cout << 
}

void Server::join(Client &client ,std::vector<std::string> &str, int fd)
{

}

void Server::ReceiveNewData(int fd)
{
    char buff[1024];
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    buff[bytes] = 0;
    Client *tmp_client = GetClient(fd);
    if (!tmp_client)
        return;
    if (bytes <= 0)
    {
        std::cout << "Client <" << fd << "> disconnected" << std::endl;
        CleanClient(fd);
        close(fd);
        return;
    }
    tmp_client->AddToBuffer(buff);
    std::string line;
    while (tmp_client->GetBuffer().find_first_of("\r\n") != std::string::npos) {
       line = tmp_client->get_command();
       //gerer la commande
       std::cout << "commande : " << line << std::endl;
    }
    // split = SplitTmpBuffer(tmp_client->GetBuffer());
    // for (size_t i = 0; i < split.size(); i++)
    //     this->ParseLaunch(split[i], fd);
    // tmp_client->ClearBuffer();
};

void Server::AcceptNewClient()
{
    Client newClient;
    struct sockaddr_in clientAdress;
    struct pollfd newFd;
    socklen_t len = sizeof(clientAdress);

    int incommingfd = accept(ServerSocketFD, (sockaddr *)&clientAdress, &len);
    std::cout << "INC FD " << incommingfd << std::endl;
    if (incommingfd == -1)
    {
        std::cout << "Accept() failed" << std::endl;
        return;
    }
    if (fcntl(incommingfd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "fcntl() failed" << std::endl;
        return;
    }
    newFd.fd = incommingfd;
    newFd.events = POLLIN;
    newFd.revents = 0;
    fds.push_back(newFd);

    newClient.SetFd(incommingfd);
    newClient.SetIpAdress(inet_ntoa(clientAdress.sin_addr));
    clients[incommingfd] = newClient;

    std::cout << "Client <" << incommingfd << "> connected" << std::endl;
};

void Server::ServerSocket()
{
    int en = 1;
    struct sockaddr_in address;
    struct pollfd newFd;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->Port);

    ServerSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocketFD == -1)
        throw(std::runtime_error("socket() failed"));
    if (setsockopt(ServerSocketFD, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == 1)
        throw(std::runtime_error("setsockopt() failed to set option SO_REUSEADDR"));
    if (bind(ServerSocketFD, (struct sockaddr *)&address, sizeof(address)) == -1)
        throw(std::runtime_error("failed to bind() socket"));
    if (listen(ServerSocketFD, SOMAXCONN) == -1)
        throw(std::runtime_error("listen() failed"));
    newFd.fd = ServerSocketFD;
    newFd.events = POLLIN;
    newFd.revents = 0;
    fds.push_back(newFd);
};

void Server::ServerInit()
{
    this->Port = 4444;
    ServerSocket();
    std::cout << "Server <" << ServerSocketFD << "> connected" << std::endl;
    std::cout << "Waiting for incomming connections..." << std::endl;
    while (Server::Signal == false)
    {
        if ((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false)
            throw(std::runtime_error("poll() failed"));

        for (size_t i = 0; i < fds.size(); i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == ServerSocketFD)
                    AcceptNewClient();
                else
                    ReceiveNewData(fds[i].fd);
            }
        }
    }
    CloseFds();
}