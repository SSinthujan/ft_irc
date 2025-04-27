/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:18 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/27 19:24:56 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//
Server::Server()
{
    ServerSocketFD = -1;
    this->quit_flag = false;
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
  
void Server::names(Client &client, std::vector<std::string> str, int fd)
{
    std::vector<std::string> channelsToPrint;
    if(str.size() < 2 || str[1].empty() || str[1][0] == ' ')
        return ;
    if (str.size() > 1)
        channelsToPrint = SplitByComma(str[1]);

    for (size_t i = 0; i < channelsToPrint.size(); ++i)
    {
        std::string channelName = channelsToPrint[i];

        if (CheckIfChannelExists(channelName))
        {
            Channel& channel = channels[channelName];
            std::vector<std::string> members = channel.GetMembers();
            std::string response = ":" + std::string("irc.server 353 ") + client.GetNickname() + " = " + channelName + " :";
            for (std::vector<std::string>::iterator it = members.begin(); it != members.end(); ++it)
            {
                std::size_t index = std::distance(members.begin(), it);
                if (channel.IsOperator(members[index]))
                    response += "@";
                response += *it + " ";
            }
            response += "\r\n";
            send(fd, response.c_str(), response.length(), 0);
            std::string endResponse = ":" + std::string("irc.server 366 ") + client.GetNickname() + " " + channelName + " :End of /NAMES list.\r\n";
            send(fd, endResponse.c_str(), endResponse.length(), 0);
        }
    }
    
}

void Server::quit(Client &client, std::vector<std::string> str, int fd)
{
    std::string reason;
    if (str.size() > 1)
    {
        for (size_t i = 1; i < str.size(); ++i)
        {
            reason += str[i];
            if (i != str.size() - 1)
                reason += " ";
        }
        if (!reason.empty() && reason[0] == ':')
            reason = reason.substr(1);
    }
    else
        reason = "Client quit";
    std::cout << "Client <" << fd << "> disconnected (" << reason << ")" << std::endl;
    //BroadcastQuitMessage(clientFd, reason);
    
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); )
    {
        Channel& channel = it->second;
        
        // Vérifie si le client est dans ce channel
        if (channel.HasMember(client.GetNickname()))
        {
            channel.RemoveMember(client.GetNickname());

            // Broadcast aux autres membres que ce client a QUIT (optionnel, si tu veux faire propre)
            // channel.Broadcast(client, ":" + client.GetNickname() + " QUIT :" + reason);
        }

        // Après suppression, si le channel est vide -> on le supprime
        if (channel.IsEmpty())
        {
            std::map<std::string, Channel>::iterator tmp = it;
            ++it;
            channels.erase(tmp); // Attention: erase retourne le nouvel itérateur
        }
        else
        {
            ++it; // sinon, on avance normalement
        }
    }
    CleanClients(fd);
    close(fd);
}
  
void Server::mode(Client *client, const std::vector<std::string> &split, int fd)
{
    if (split.size() < 3) 
    {
        std::string error = ":irc.server 461 " + client->GetNickname() + " MODE :Not enough parameters\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    std::string channelName = split[1];
    std::string modeChanges = split[2];

    Channel *chan = GetChannel(channelName);
    if (!chan) {
        std::string error = ":irc.server 403 " + client->GetNickname() + " " + channelName + " :No such channel\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    if (!chan->IsOperator(client->GetNickname())) {
        std::string error = ":irc.server 482 " + client->GetNickname() + " " + channelName + " :You're not channel operator\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    bool adding = true;
    size_t paramIndex = 3;
    std::string responseModes;
    std::vector<std::string> modeParams;
    
    for (size_t i = 0; i < modeChanges.length(); ++i)
    {
        char mode = modeChanges[i];
        if (mode == '+') {
            adding = true;
        } else if (mode == '-') {
            adding = false;
        } else {
            std::string response;
            switch (mode) {
                case 'i':
                if (adding)
                    chan->SetInviteOnly();
                else
                    chan->RemoveInviteOnly();
                responseModes += "i";
                    break;
                case 't':
                    chan->SetTopicRestricted(adding);
                    break;
                case 'k':
                    if (adding && split.size() > paramIndex) {
                        chan->SetKey(split[paramIndex++]);
                    } else if (!adding) {
                        chan->RemoveKey();
                    }
                    break;
                case 'l':
                    if (adding && split.size() > paramIndex) 
                    {
                        std::stringstream ss(split[paramIndex]);
                        int limit;
                        ss >> limit;
                        if (!ss.fail()) {
                            chan->SetUserLimit(limit);
                            modeParams.push_back(split[paramIndex]);
                            paramIndex++;
                            responseModes += "l";
                        } else {
                            std::string error = ":irc.server 461 " + client->GetNickname() + " MODE :Invalid limit parameter\r\n";
                            send(fd, error.c_str(), error.length(), 0);
                            return;
                        }
                    } 
                    else if (!adding) 
                    {
                        chan->RemoveUserLimit();
                        responseModes += "l";
                    }
                    break;
                case 'o':
                    if (split.size() > paramIndex) {
                        std::string targetNick = split[paramIndex++];
                        if (adding) chan->AddOperator(targetNick);
                        else chan->RemoveOperator(targetNick);
                    }
                    break;
                default:
                    response = ":irc.server 472 " + client->GetNickname() + " " + mode + " :is unknown mode char to me\r\n";
                    send(fd, response.c_str(), response.length(), 0);
                    break;
            }
        }
    }

    // Broadcast aux membres du canal le changement de mode
    std::string msg = ":" + client->GetNickname() + " MODE " + channelName + " " + modeChanges;
    for (size_t i = 3; i < split.size(); ++i) {
        msg += " " + split[i];
    }
    msg += "\r\n";
    chan->Broadcast(msg, this->clients);
}

Channel* Server::GetChannel(const std::string& name) 
{
    std::map<std::string, Channel>::iterator it = channels.find(name);
    if (it != channels.end())
        return &(it->second);
    return NULL;
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
    //this->Port = 4444;
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