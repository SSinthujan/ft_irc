/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:18 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/28 17:49:58 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
    ServerSocketFD = -1;
    this->quit_flag = false;
}

void Server::inputCheck(int ac, char **av)
{

    if (ac != 3)
        throw (std::runtime_error("Wrong args : ./ircserv <port> <password>"));

    _portStr = av[1];
    _password = av[2];

    char* end = NULL;
    long port = strtol(_portStr.c_str(), &end, 10);

    if (*end != '\0')
        throw(std::runtime_error("Port must be a valid number (digits only)"));

    if (port < 1024 || port > 65535)
        throw(std::runtime_error("Port must be between 1024 and 65535"));

    Port = static_cast<int>(port);
}
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
    /* 1. Remove the pollfd that matches fd */
    for (std::vector<struct pollfd>::iterator it = fds.begin();
         it != fds.end(); ++it)
    {
        if (it->fd == fd)
        {
            fds.erase(it);          // erase → O(N) but only once
            break;
        }
    }

    /* 2. Remove the Client whose key is fd (O(log N)) */
    clients.erase(fd);              // does nothing if fd not present
}

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
    if(split[0] == "CAP" && split.size() > 1 && split[1] == "LS")
    {
        if (!(tmp->GetRegistered()))
        {
            std::cout <<"FD : " << fd << std::endl;
            std::string response = ":irc.server CAP * LS :multi-prefix\r\n";
            send(tmp->GetFd(), response.c_str(), response.length(), 0);
        }
    }
    else if(split[0] == "CAP" && split.size() > 1 && split[1] == "END")
    {
        if (!(tmp->GetRegistered()) && (tmp->GetPass() && tmp->GetNick() && tmp->GetUser()))
        {
            tmp->SetRegistered(true);
            std::string welcome = ":irc.server 001 " + tmp->GetNickname() + " :Welcome to the IRC server " + tmp->GetNickname() + "!" + tmp->GetUsername() + "@" + tmp->GetIpAddress() + "\r\n";
            send(tmp->GetFd(), welcome.c_str(), welcome.length(), 0);
            
            // Message 002 (RPL_YOURHOST)
            std::string yourHost = ":irc.server 002 " + tmp->GetNickname() + " :Your host is irc.server, running version 1.0\r\n";
            send(tmp->GetFd(), yourHost.c_str(), yourHost.length(), 0);
            
            // Message 003 (RPL_CREATED)
            std::string created = ":irc.server 003 " + tmp->GetNickname() + " :This server was created today\r\n";
            send(tmp->GetFd(), created.c_str(), created.length(), 0);
            
            // Message 004 (RPL_MYINFO)
            std::string myInfo = ":irc.server 004 " + tmp->GetNickname() + " irc.server 1.0 o o\r\n";
            send(tmp->GetFd(), myInfo.c_str(), myInfo.length(), 0);
            
            // Message 375 (RPL_MOTDSTART)
            std::string motdStart = ":irc.server 375 " + tmp->GetNickname() + " :- irc.server Message of the day - \r\n";
            send(tmp->GetFd(), motdStart.c_str(), motdStart.length(), 0);
            
            // Message 372 (RPL_MOTD)
            std::string motd = ":irc.server 372 " + tmp->GetNickname() + " :- Welcome to the IRC server\r\n";
            send(tmp->GetFd(), motd.c_str(), motd.length(), 0);
            
            // Message 376 (RPL_ENDOFMOTD)
            std::string endMotd = ":irc.server 376 " + tmp->GetNickname() + " :End of /MOTD command\r\n";
            send(tmp->GetFd(), endMotd.c_str(), endMotd.length(), 0);
        }
    }
    else if(split[0] == "CAP" && split.size() > 1 && split[1] == "REQ")
    {
        std::string response = ":irc.server CAP " + tmp->GetNickname() + " ACK :multi-prefix\r\n";
        send(tmp->GetFd(), response.c_str(), response.length(), 0); 
    }
    else if(split[0] == "NICK" && split.size() > 1)
    {
        if (tmp->GetPass())
        {
            std::string oldNick = tmp->GetNickname();
            tmp->SetNickname(split[1]);
            tmp->SetNick(true);
            std::cout << ":" << tmp->GetNickname() << "!@ NICK " << tmp->GetNickname() << std::endl;
        }
    }
    else if(split[0] == "USER" && split.size() > 4)
    {
        if (!(tmp->GetUser()))
        {
            if (tmp->GetPass())
            {
                tmp->SetUser(split);
                tmp->SetBuser(true);
            }
        }
        else
            std::cout << "Unknown command: " << split[0] << std::endl;
        
    }
    else if(split[0] == "PING" && split.size() > 1)
    {
        if(tmp->GetRegistered())
        {
            std::string response = "PONG :" + split[1] + "\r\n";
            send(tmp->GetFd(), response.c_str(), response.length(), 0);
            std::cout << "\033[32mPING command has been detected\033[0m" << std::endl;
        }
    }
    else if(split[0] == "PASS")
    {
        if(split.size() > 1 && split[1] != this->_password)
        {
            std::string errorMsg = ":irc.server 464 * :Password incorrect\r\n";
            send(tmp->GetFd(), errorMsg.c_str(), errorMsg.length(), 0);
        }
        else
            tmp->SetPass(true);
    }
    else if(split[0] == "JOIN" && split.size() > 1)
    {
        if (tmp->GetRegistered())
            Join(*tmp, split, fd);
    }
    else if (split[0] == "MODE")
    {
        if (tmp->GetRegistered())
            HandleMode(tmp, split, fd);
    }
    else if (split[0] == "QUIT")
    {
        if (tmp->GetRegistered())
        {
            Quit(*tmp, split, fd);
            this->quit_flag = true;
        }
    }
    else if (split[0] == "NAMES")
    {
        if (tmp->GetRegistered())
        {
            Names(*tmp, split, fd);
        }
    }
    else if (split[0] == "KICK")
    {
        if (tmp->GetRegistered())
        {
            Kick(*tmp, split);
        }
    }
    else if (split[0] == "INVITE")
    {
        if (tmp->GetRegistered())
        {
            Invite(*tmp, split);
        }
    }
    else
    {
        std::cout << "Unknown command: " << split[0] << std::endl;
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
    return channels.find(str) != channels.end();
};

Client* Server::GetClientByNickname(const std::string& nickname)
{
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second.GetNickname() == nickname)
            return &(it->second);
    }
    return NULL;
}

void Server::Invite(Client &client, std::vector<std::string> str)
{
    if (str.size() < 3 || str[1].empty() || str[2].empty())
        return ;
    std::string channelsToInvit;
    std::string name;

    name = str[2];
    channelsToInvit = str[1];
    if (!CheckIfChannelExists(channelsToInvit))
        return;
    Channel& channel = channels[channelsToInvit];
    if (!(channel.HasMember(client.GetNickname())))
        return;
   if (channel.HasMember(name))
        return;
    if (channel.CheckInviteOnly())
        if (!(channel.IsOperator(client.GetNickname())))
            return;
    if (channel.IsFull())
        return;
    Client* invitedClient = GetClientByNickname(name);
    if (invitedClient)
    {
        channel.AddMember(name, invitedClient->GetFd());
        std::string inviteMsg = ":" + client.GetNickname() + "!" + client.GetUsername() + "@" + "localhost" +
        " INVITE " + name + " :" + channelsToInvit + "\r\n";
        invitedClient->sendMsg(inviteMsg);
        std::string joinMsg = ":" + name + " JOIN " + channelsToInvit + "\r\n";
        channel.Broadcast(joinMsg, clients);
    }

    std::cout << "\033[32mINVITE command has been detected\033[0m" << std::endl;
}

void Server::Kick(Client &client, std::vector<std::string> str)
{
    if (str.size() < 3 || str[1].empty() || str[2].empty())
        return ;
    std::string channelsToKick;
    std::string name;
    std::string motif;

    name = str[2];
    channelsToKick = str[1];
    if (str.size() > 3)
        motif = str[3];
    else
        motif = "banned from the channel";
    if (!CheckIfChannelExists(channelsToKick))
        return;
    Channel& channel = channels[channelsToKick];
    if (!(channel.IsOperator(client.GetNickname())))
        return;
    if (!(channel.HasMember(name)))
        return;
    std::string prefix = client.GetNickname() + "!user@localhost";
    std::string kickMessage = ":" + prefix + " KICK " + channelsToKick + " " + name + " :" + motif + "\r\n";
    channel.Broadcast(kickMessage, clients);
    channel.RemoveMember(name);
    std::cout << "\033[32mKICK command has been detected\033[0m" << std::endl;
}

void Server::Names(Client &client, std::vector<std::string> str, int fd)
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
            const std::map<std::string, int>& channelMembers = channel.GetMembers2();
            std::string response = ":" + std::string("irc.server 353 ") + client.GetNickname() + " = " + channelName + " :";
            for (std::map<std::string, int>::const_iterator it = channelMembers.begin(); it != channelMembers.end(); ++it)
            {
                const std::string &nickname = it->first;
                if (channel.IsOperator(nickname))
                    response += "@";
                response += nickname + " ";
            }
            response += "\r\n";
            send(fd, response.c_str(), response.length(), 0);
            std::string endResponse = ":" + std::string("irc.server 366 ") + client.GetNickname() + " " + channelName + " :End of /NAMES list.\r\n";
            send(fd, endResponse.c_str(), endResponse.length(), 0);
        }
    }
    std::cout << "\033[32mNAMES command has been detected\033[0m" << std::endl;
    
}

void Server::Quit(Client &client, std::vector<std::string> str, int fd)
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
    std::cout << "\033[32mQUIT command has been detected\033[0m" << std::endl;
    std::cout << "Client <" << fd << "> disconnected (" << reason << ")" << std::endl;
    std::string msg = client.GetNickname() + " disconnected (" + reason + ")\r\n";
    
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); )
    {
        Channel& channel = it->second;
        
        // Vérifie si le client est dans ce channel
        if (channel.HasMember(client.GetNickname()))
        {
            channel.RemoveMember(client.GetNickname());
            channel.Broadcast(msg, clients);
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
            ++it;
        }
    }
    CleanClients(fd);
    close(fd);
  
}

void Server::Join(Client &client ,std::vector<std::string> str, int fd)
{
    (void)fd;
    std::vector<std::string> channelsToJoin;
    std::vector<std::string> keys;
    std::string nickname = client.GetNickname();
    
    if(str.size() < 2 || str[1].empty() || str[1][0] == ' ')
        return ;
    channelsToJoin = SplitByComma(str[1]);
    if (str.size() > 2)
        keys = SplitByComma(str[2]);
    std::cout << "\033[32mJOIN command has been detected\033[0m" << std::endl;
    for (size_t i = 0; i < channelsToJoin.size(); ++i)
    {
        std::string channelName = channelsToJoin[i];
        std::string key = (i < keys.size()) ? keys[i] : "";
    
        Channel* channel;
    
        // Crée ou récupère le channel
        if (!CheckIfChannelExists(channelName))
        {
            channels[channelName] = Channel(channelName);
            channel = &channels[channelName];
            channel->SetOperator(nickname); // premier arrivé = opérateur
        }
        else
        {
             channel = &channels[channelName];
        }
    
        // Déjà membre ?
        if (channel->HasMember(nickname))
            continue;
    
        // Vérification mode +i (invite-only)
         if (channel->CheckInviteOnly() && !channel->IsInvited(nickname))
        {
            client.sendMsg("473 " + nickname + " " + channelName + " :Cannot join channel (+i)\r\n");
            continue;
        }
            // Vérification mode +k (clé)
        if (channel->IsKeyEnabled() && channel->GetPassword() != key)
        {
            client.sendMsg("475 " + nickname + " " + channelName + " :Cannot join channel (+k)\r\n");
            continue;
        }

        // Vérification canal plein
        if (channel->IsFull())
        {
            client.sendMsg("471 " + nickname + " " + channelName + " :Cannot join channel (+l)\r\n");
            continue;
        }

        // Ajout du membre
        channel->AddMember(nickname, fd);

        // Broadcast JOIN à tous les membres du channel
        std::string joinMsg = ":" + nickname + " JOIN " + channelName + "\r\n";
        channel->Broadcast(joinMsg, clients);

        // Envoi du topic s’il existe
        if (!channel->GetTopic().empty())
        {
            client.sendMsg("332 " + nickname + " " + channelName + " :" + channel->GetTopic() + "\r\n"); // RPL_TOPIC
        }

        // Envoi de la liste des utilisateurs (RPL_NAMREPLY + RPL_ENDOFNAMES)
        std::string names;
        std::vector<std::string> members = channel->GetMembers();
        for (size_t j = 0; j < members.size(); ++j)
        {
            if (channel->IsOperator(members[j]))
                names += "@";
            names += members[j] + " ";
        }

        client.sendMsg("353 " + nickname + " = " + channelName + " :" + names + "\r\n"); // RPL_NAMREPLY
        client.sendMsg("366 " + nickname + " " + channelName + " :End of /NAMES list\r\n"); // RPL_ENDOFNAMES
    }
    
};

void Server::HandleMode(Client *client, const std::vector<std::string> &split, int fd)
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
    std::cout << "\033[32mMODE command has been detected\033[0m" << std::endl;
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
    memset(buff, 0, sizeof(buff));
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    Client *tmp_client = GetClient(fd);
    if (!tmp_client)
        return;
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
        {
            this->ParseLaunch(split[i], fd);
        }
        if (split[0] != "QUIT")
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
    //this->Port = 4444;
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