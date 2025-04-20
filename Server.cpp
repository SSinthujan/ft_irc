/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dakojic <dakojic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:18 by dakojic           #+#    #+#             */
/*   Updated: 2025/04/14 17:23:40 by dakojic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Client* Server::GetClient(int fd)
{
    for(size_t i = 0; i < this->clients.size(); i++)
    {
        if(this->clients[i].GetFd() == fd)
            return &this->clients[i];
    }
    return NULL;
};

void Server::CleanClients(int fd)
{
    for(size_t i = 0; i < fds.size(); i++)
    {
        if(fds[i].fd == fd)
        {
            fds.erase(fds.begin() + i);
            break ;
        }
    }
    for(size_t i = 0; i < clients.size(); i++)
    {
        if(clients[i].GetFd() == fd)
        {
            clients.erase(clients.begin() + i);
            break ;
        }
    }
};

void Server::CloseFds()
{
    for(size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client <" << clients[i].GetFd() << "> Disconnected" <<  std::endl;
        close(clients[i].GetFd());       
    }
    if(ServerSocketFD != -1)
    {
        std::cout << "Server <" << ServerSocketFD << "> disconnected" << std::endl;
        close(ServerSocketFD);
    }
};

bool Server::Signal = false;

void Server::SignalHadler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Signal Received" << std::endl;
    Server::Signal = true;
};

std::vector<std::string> Server::SplitTmpBuffer(std::string str) // A revoir ??
{
    std::vector<std::string> split;
    std::istringstream iss(str);
    std::string tmp;
    while(std::getline(iss, tmp))
    {
        size_t pos = tmp.find_first_of("\r\n");
        if(pos != std::string::npos)
            tmp = tmp.substr(0, pos);
        split.push_back(tmp);
    }
    return split;
    
};

std::vector<std::string> Server::SplitCmd(std::string &cmd) // A revoir ??
{
    std::vector<std::string> vec;
    std::istringstream stm(cmd);
    std::string token;
    while(stm >> token)
    {
        vec.push_back(token);
        token.clear();
    }
    return vec;
};

void Server::ParseLaunch(std::string &str, int fd)
{
    if(str.empty())
        return ;
    std::vector<std::string> split = SplitCmd(str);
    size_t sep = str.find_first_of(" \t\v");

    Client *tmp = GetClient(fd);
    if(sep != std::string::npos)
        str = str.substr(sep);
    if(split[0] == "CAP" && split[1] == "LS")
    {
        std::cout <<"FD : " << fd << std::endl;
        std::string response = ": CAP * LS :multi-prefix";
        send(tmp->GetFd(), response.c_str(), response.length(), 0);
    }
    else if(split[0] == "CAP" && split[1] == "REQ")
    {
        std::string response = ":IRC-REQ CAP" +  tmp->GetNickname() + " ACK :multi-prefix";
        send(tmp->GetFd(), response.c_str(), response.length(), 0); 
    }
    else if(split[0] == "NICK")
    {
        tmp->SetNickname(str);
    }
    else if(split[0] == "USER")
    {
        tmp->SetUser(split);
    }
    else if(split[0] == "PING")
    {
        std::string response = "PONG " + str;
        send(tmp->GetFd(), response.c_str(), response.length(), 0);
        
    }
    else if(split[0] == "PASS")
    {
        if(split[1] != "-n")
        {
            std::cout << "WRONG PASSWORD" << std::endl;
        }
    }
    else
    {
        std::cout <<"MSG = " << split[0] << std::endl;
        return ;
    }
};

std::vector<std::string> SplitByComma(std::string str)
{
    std::vector<std::string> split;
    size_t start = 0;
    size_t end = str.find(',');
    while (end != std::string::npos)
    {
        split.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(',', start);
    }
    split.push_back(str.substr(start));
    return split;
};
bool Server::CheckIfChannelExists(std::string str)
{
    for (std::vector<Channel>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if(it->GetChannelName() == str)
            return true;
    }
    return false;
};
void Server::Join(std::vector<std::string> str, int fd)
{
    std::vector<std::string> channelsToJoin;
    
    if(str.empty() || str[1][0] == ' ')
    {
        std::cout<<"CANT JOIN"<< std::endl;
        return ;
    }
    channelsToJoin = SplitByComma(str[1]);
    for(std::vector<std::string>::iterator it = channelsToJoin.begin(); it != channelsToJoin.end(); it++)
    {
        if(CheckIfChannelExists(*it))
            std::cout<<"WE JOINING WITH " << fd<<std::endl;
        else
            std::cout<<"WE CREATING WITH "<<  fd<< std::endl;
    }
};
void Server::ReceiveNewData(int fd)
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    Client *tmp_client = GetClient(fd);
    std::vector<std::string> split;
    if(bytes <= 0)
    {
        std::cout << "Client <" << fd<<"> disconnected" << std::endl;
        CleanClients(fd);
        close(fd);
    }
    else
    {
        tmp_client->AddToBuffer(buff);
        if(tmp_client->GetBuffer().find_first_of("\r\n") == std::string::npos)
            return ;
        split = SplitTmpBuffer(tmp_client->GetBuffer());
        for(size_t i = 0; i < split.size(); i++)
            this->ParseLaunch(split[i], fd);
        tmp_client->ClearBuffer();
    }
};

void Server::AcceptNewClient()
{
    Client newClient;
    struct sockaddr_in clientAdress;
    struct pollfd newFd;
    socklen_t len = sizeof(clientAdress);

    int incommingfd = accept(ServerSocketFD, (sockaddr *)&clientAdress, &len);
    std::cout<<"INC FD " << incommingfd<<std::endl;
    if(incommingfd == -1)
    {
        std::cout << "Accept() failed" << std::endl;
        return ;
    }
    if(fcntl(incommingfd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "fcntl() failed" << std::endl;
        return ;
    }
    newFd.fd = incommingfd;
    newFd.events = POLLIN;
    newFd.revents = 0;
    
    newClient.SetFd(incommingfd);
    newClient.SetIpAdress(inet_ntoa(clientAdress.sin_addr));
    clients.push_back(newClient);
    fds.push_back(newFd);

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
    if(ServerSocketFD == -1)
        throw(std::runtime_error("socket() failed"));
    if(setsockopt(ServerSocketFD, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == 1)
        throw(std::runtime_error("setsockopt() failed to set option SO_REUSEADDR"));
    if(bind(ServerSocketFD, (struct sockaddr *) &address, sizeof(address)) == -1)
        throw(std::runtime_error("failed to bind() socket"));
    if(listen(ServerSocketFD, SOMAXCONN) == -1)
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
    std::cout<<"Server <" << ServerSocketFD << "> connected" << std::endl;
    std::cout<<"Waiting for incomming connections..."<<std::endl;
    while(Server::Signal == false)
    {
        if((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false)
            throw(std::runtime_error("poll() failed"));
        else
        {
            for(size_t i = 0; i < fds.size(); i++)
            {
                if(fds[i].revents & POLLIN)
                {
                    if(fds[i].fd == ServerSocketFD)
                        AcceptNewClient();
                    else
                        ReceiveNewData(fds[i].fd);

                }
            }
        }
    }
    CloseFds();
}
