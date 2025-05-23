/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 23:33:18 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/05/16 17:28:56 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"


void Server::nick(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 1 || args[0].empty())
        return;
    // client.SetNickname(args[0]);
    // client.SetNick(true);  // Set the nick flag to true
    // std::cout << "\033[32mNICK command has been detected\033[0m" << std::endl;
    if (client.GetPass())
        {
            if (args[0][0] != '#')
            {
                bool nicknameInUse = false;
                for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
                {
                    if (it->second.GetNickname() == args[0])
                    {
                        nicknameInUse = true;
                        break;
                    }
                }

                if (nicknameInUse)
                {
                    // Nickname déjà pris, envoie 433
                    std::string response = ":irc.server 433 * " + args[0] + " :Nickname is already in use\r\n";
                    send(client.GetFd(), response.c_str(), response.length(), 0);
                }
                else
                {

                    // Nickname libre, on le change
                    std::string oldNick = client.GetNickname();
                    client.SetNickname(args[0]);
                    client.SetNick(true);
                    std::string nickMsg = ":" + oldNick + "!user@host NICK :" + client.GetNickname() + "\r\n";
                    send(client.GetFd(), nickMsg.c_str(), nickMsg.length(), 0);
                    std::cout << ":" << client.GetNickname() << "!@ NICK " << client.GetNickname() << std::endl;
                }
            }
            else
            {
                // Nick invalide (commence par #)
                std::string response = ":irc.server 432 * " + args[0] + " :Erroneus nickname\r\n";
                send(client.GetFd(), response.c_str(), response.length(), 0);
            }
        }
}

void Server::topic(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    std::cout << "[DEBUG] Args:" << std::endl;
    for (size_t i = 0; i < args.size(); ++i) 
    {
        std::cout << "args[" << i << "] = \"" << args[i] << "\"" << std::endl;
    }
    if (args.size() < 1 || args[0].empty()  )
        return;

    std::string channelName = args[0];

    if (!CheckIfChannelExists(channelName)) {
        client.sendMsg(":" + std::string("irc.server 403 ") + client.GetNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    Channel& channel = chan[channelName];

    if (!channel.HasMember(client.GetNickname())) {
        client.sendMsg(":" + std::string("irc.server 442 ") + client.GetNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }

    // === GET topic ===
    if (args.size() == 1) 
    {
        const std::string& topic = channel.GetTopic();
        if (topic == "null") {
            client.sendMsg(":" + std::string("irc.server 331 ") + client.GetNickname() + " " + channelName + " :No topic is set\r\n");
        } else {
            client.sendMsg(":" + std::string("irc.server 332 ") + client.GetNickname() + " " + channelName + " :" + topic + "\r\n");
        }
        return;
    }

    // === SET topic ===
    std::string subject;
    for (size_t i = 1; i < args.size(); ++i) {
        subject += args[i];
        if (i != args.size() - 1)
            subject += " ";
    }
    if (channel.IsTopicRestricted() && !channel.IsOperator(client.GetNickname())) {
        client.sendMsg(":" + std::string("irc.server 482 ") + client.GetNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    channel.SetTopic(subject);
    std::string settopicMsg = ":" + client.GetNickname() + " TOPIC " + args[0] + " :" + subject + "\r\n";;
    channel.Broadcast(settopicMsg, clients);
}


void Server::join(Client &client ,std::vector<std::string> &args, int fd)
{
    (void)fd;
    std::vector<std::string> channelsToJoin;
    std::vector<std::string> keys;
    std::string nickname = client.GetNickname();
    
    if(args.size() < 1 || args[0].empty() || args[0][0] == ' ')
        return ;
    channelsToJoin = SplitByComma(args[0]);
    if (args.size() > 1)
        keys = SplitByComma(args[1]);
    std::cout << "\033[32mJOIN command has been detected\033[0m" << std::endl;
    for (size_t i = 0; i < channelsToJoin.size(); ++i)
    {
        std::string channelName = channelsToJoin[i];
        if (channelName[0] != '#')
            channelName = "#" + channelName;
        std::string key = (i < keys.size()) ? keys[i] : "";
    
        Channel* channel;
    
        // Crée ou récupère le channel
        if (!CheckIfChannelExists(channelName))
        {
            chan[channelName] = Channel(channelName);
            channel = &chan[channelName];
            channel->SetOperator(nickname); // premier arrivé = opérateur
        }
        else
        {
             channel = &chan[channelName];
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

        channel->AddMember(nickname, fd);

        std::string joinMsg = ":" + nickname + " JOIN " + channelName + "\r\n";
        channel->Broadcast(joinMsg, clients);

        if (channel->GetTopic() == "null")
            client.sendMsg(std::string("irc.server 332 ") + nickname + " " + channelName + " :" + channel->GetTopic() + "\r\n");
        else
            client.sendMsg(std::string("irc.server 331 ") + nickname + " " + channelName + " :No topic is set\r\n"); // RPL_NOTOPIC
        std::string names;
        std::vector<std::string> members = channel->GetMembers();
        for (size_t j = 0; j < members.size(); ++j)
        {
            if (channel->IsOperator(members[j]))
                names += "@";
            names += members[j] + " ";
        }
        std::string response = ":" + std::string("irc.server 353 ") + client.GetNickname() + " = " + channelName + " :";
        for (std::vector<std::string>::iterator it = members.begin(); it != members.end(); ++it)
        {
            std::size_t index = std::distance(members.begin(), it);
            if (channel->IsOperator(members[index]))
                response += "@";
            response += *it + " ";
        }
        response += "\r\n"; 
        client.sendMsg(response);// RPL_NAMREPLY
        client.sendMsg( ":" + std::string("irc.server 366 ") + nickname + " " + channelName + " :End of /NAMES list\r\n"); // RPL_ENDOFNAMES
    }
    
};

void Server::names(Client &client, std::vector<std::string> &args, int fd)
{
    std::vector<std::string> channelToPrint;
    if(args.size() < 2 || args[0].empty() || args[0][0] == ' ')
        return ;
    if (args.size() > 1)
    {
        std::string str;
        std::stringstream ss(args[0]);
        
        while (std::getline(ss, str, ',')) 
        {
            if (!str.empty())
                channelToPrint.push_back(str);
        }
    }
    for (size_t i = 0; i < channelToPrint.size(); ++i)
    {
        std::string channelName = channelToPrint[i];
        if (CheckIfChannelExists(channelName))
        {
            Channel& channel = chan[channelName]; //besoin de join pour creer le channel
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

void Server::mode(Client &client, std::vector<std::string> &args, int fd)
{
    if (args.size() < 2) 
    {
        std::string error = ":irc.server 461 " + client.GetNickname() + " MODE :Not enough parameters\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    std::string channelName = args[0];
    std::string modeChanges = args[1];

    Channel *chan = GetChannel(channelName);
    if (!chan) {
        std::string error = ":irc.server 403 " + client.GetNickname() + " " + channelName + " :No such channel\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    if (!chan->IsOperator(client.GetNickname())) {
        std::string error = ":irc.server 482 " + client.GetNickname() + " " + channelName + " :You're not channel operator\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    bool adding = true;
    size_t paramIndex = 2;
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
                    if (adding && args.size() > paramIndex) {
                        chan->SetKey(args[paramIndex++]);
                    } else if (!adding) {
                        chan->RemoveKey();
                    }
                    break;
                case 'l':
                    if (adding && args.size() > paramIndex) 
                    {
                        std::stringstream ss(args[paramIndex]);
                        int limit;
                        ss >> limit;
                        if (!ss.fail()) {
                            chan->SetUserLimit(limit);
                            modeParams.push_back(args[paramIndex]);
                            paramIndex++;
                            responseModes += "l";
                        } else {
                            std::string error = ":irc.server 461 " + client.GetNickname() + " MODE :Invalid limit parameter\r\n";
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
                    if (args.size() > paramIndex) {
                        std::string targetNick = args[paramIndex++];
                        if (adding) chan->AddOperator(targetNick);
                        else chan->RemoveOperator(targetNick);
                    }
                    break;
                default:
                    response = ":irc.server 472 " + client.GetNickname() + " " + mode + " :is unknown mode char to me\r\n";
                    send(fd, response.c_str(), response.length(), 0);
                    break;
            }
        }
    }
    // Broadcast aux membres du canal le changement de mode
    std::string msg = ":" + client.GetNickname() + " MODE " + channelName + " " + modeChanges;
    for (size_t i = 2; i < args.size(); ++i) {
        msg += " " + args[i];
    }
    msg += "\r\n";
    chan->Broadcast(msg, this->clients);
}

void Server::part(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    std::string subject;
    if (args.size() < 1 || args[0].empty())
        return;
    std::cout << "\033[32mPART command has been detected\033[0m" << std::endl;
    if (!CheckIfChannelExists(args[0]))
    {
        std::string error = ":" + std::string("irc.server 403 ")+ client.GetNickname() + " " + args[0] + " :No such channel\r\n";
        client.sendMsg(error);
        return;
    }
    Channel& channel = chan[args[0]];
    if (!channel.HasMember(client.GetNickname()))
    {
        std::string error1 = ":" + std::string("irc.server 442 ") + client.GetNickname() + " " + args[0] + " :You're not on that channel\r\n";
        client.sendMsg(error1);
        return;
    }
    if (args.size() > 1)
    {
        for (size_t i = 1; i < args.size(); ++i)
        {
            subject += args[i];
            if (i != args.size() - 1)
                subject += " ";
        }
    }
    std::string prefix = client.GetNickname();
    std::string partMsg;
    if (args.size() > 1)
        partMsg = ":" + prefix + " PART " + args[0] + " :" + subject + "\r\n";
    else
        partMsg = ":" + prefix + " PART " + args[0] + "\r\n";
    channel.Broadcast(partMsg, clients);
    channel.RemoveMember(client.GetNickname());
   // client.sendMsg(partMsg);
   std::cout << client.GetUsername() << std::endl;
}

void Server::privmsg(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 2 || args[0].empty() || args[1].empty())
        return;
    std::cout << "\033[32mPRIVMSG command has been detected\033[0m" << std::endl;
    std::string message;
    for (size_t i = 1; i < args.size(); ++i)
    {
        message += args[i];
        if (i != args.size() - 1)
            message += " ";
    }
    std::string prefix = ":" + client.GetNickname() + "!" + client.GetUsername() + "@localhost";
    std::string prefix2 = ":" + client.GetNickname();
    std::string fullmsg = prefix + " PRIVMSG " + args[0] + " :" + message + "\r\n";
    std::string fullmsg2 = prefix2 + " PRIVMSG " + args[0] + " :" + message + "\r\n";
    if (args[0][0] == '#')
    {
        if (!CheckIfChannelExists(args[0]))
        {
            std::string error = ":" + std::string("irc.server 403 ")+ client.GetNickname() + " " + args[0] + " :No such channel\r\n";
            client.sendMsg(error);
            return;
        }

        Channel& channel = chan[args[0]];
        channel.Broadcast2(fullmsg, clients, client.GetNickname());
    }
    else
    {
        Client* target = GetClientByNickname(args[0]);
        if (!target)
        {
            std::string error = ":" + std::string("irc.server 401 ")+ client.GetNickname() + " " + args[0] + " :No such nick\r\n";
            client.sendMsg(error);
            return;
        }
        target->sendMsg(fullmsg2);
    }
}

void Server::invite(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 3 || args[0].empty() || args[1].empty())
        return ;
    std::string channelToInvit;
    std::string name;

    name = args[1];
    channelToInvit = args[0];
    if (!CheckIfChannelExists(channelToInvit))
    {
        std::string error = ":" + std::string("irc.server 403 ") + client.GetNickname() + " " + channelToInvit + " :No such channel\r\n";
        client.sendMsg(error);
        return;
    }
    Channel& channel = chan[channelToInvit];
    if (!(channel.HasMember(client.GetNickname())))
    {
        std::string error1 = ":" + std::string("irc.server 442 ") + client.GetNickname() + " " + channelToInvit + " :You're not on that channel\r\n";
        client.sendMsg(error1);
        return;
    }
   if (channel.HasMember(name))
   {
        std::string error2 = ":" + std::string("irc.server 443 ") + client.GetNickname() + name + " " + channelToInvit + " :is already on channel\r\n";
        client.sendMsg(error2);
        return;
   }
    Client* invitedClient = GetClientByNickname(name);
    if (invitedClient)
    {
        channel.AddMemberInvite(name, invitedClient->GetFd());
        std::string inviteMsg = ":" + client.GetNickname() + "!" + client.GetUsername() + "@" + "localhost" +
        " INVITE " + name + " :" + channelToInvit + "\r\n";
        invitedClient->sendMsg(inviteMsg);
        std::string joinMsg = ":" + name + " JOIN " + channelToInvit + "\r\n";
        channel.Broadcast(joinMsg, clients);
    }
    std::cout << "\033[32mINVITE command has been detected\033[0m" << std::endl;
}

void Server::kick(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 3 || args[0].empty() || args[1].empty())
        return ;
    std::string channelToKick;
    std::string name;
    std::string motif;

    name = args[1];
    channelToKick = args[0];
    if (args.size() > 3)
        motif = args[3];
    else
        motif = "banned from the channel";
    if (!CheckIfChannelExists(channelToKick))
    {
        std::string error = ":" + std::string("irc.server 403 ") + client.GetNickname() + " " + channelToKick + " :No such channel\r\n";
        client.sendMsg(error);
        return;
    }
    Channel& channel = chan[channelToKick];
    if (!(channel.IsOperator(client.GetNickname())))
    {
        std::string error2 = ":" + std::string("irc.server 482 ") + client.GetNickname() + " " + args[0] + " :You're not channel operator\r\n";
        client.sendMsg(error2);
        return;
    }
    if (!(channel.HasMember(name)))
    {
        std::string error2 = ":" + std::string("irc.server 441 ") + client.GetNickname() + " " + name + " " + args[0] + " :User not in channel\r\n";
        client.sendMsg(error2);
        return;
    }
    std::string prefix = client.GetNickname() + "!" + client.GetUsername() + "@localhost";
    std::string kickMessage = ":" + prefix + " KICK " + channelToKick + " " + name + " :" + motif + "\r\n";
    channel.Broadcast(kickMessage, clients);
    channel.RemoveMember(name);

    std::cout << "\033[32mKICK command has been detected\033[0m" << std::endl;
}

void Server::quit(Client &client, std::vector<std::string> &args, int fd)
{
    std::string reason;
    if (args.size() > 1)
    {
        for (size_t i = 1; i < args.size(); ++i)
        {
            reason += args[i];
            if (i != args.size() - 1)
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
    for (std::map<std::string, Channel>::iterator it = chan.begin(); it != chan.end(); )
    {
        Channel& channel = it->second;
        if (channel.HasMember(client.GetNickname()))
        {
            channel.RemoveMember(client.GetNickname());
            channel.Broadcast(msg, clients);
        }
        if (channel.IsEmpty())
        {
            std::map<std::string, Channel>::iterator tmp = it;
            ++it;
            chan.erase(tmp); // Attention: erase retourne le nouvel itérateur
        }
        else
            ++it;
    }
    CleanClient(fd);
    close(fd);
}

void Server::cap(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 1 || args[0].empty())
        return;
    std::cout << "\033[32mCAP command has been detected\033[0m" << std::endl;
    
    if (args[0] == "LS")
    {
        if (!(client.GetRegistered()))
        {
            std::string response = ":irc.server CAP * LS :multi-prefix\r\n";
            client.sendMsg(response);
        }
    }
    else if (args[0] == "END")
    {
        if (!(client.GetRegistered()) && (client.GetPass() && client.GetNick() && client.GetUser()))
        {
            client.SetRegistered(true);
            std::string welcome = ":irc.server 001 " + client.GetNickname() + " :Welcome to the IRC server " + client.GetNickname() + "!" + client.GetUsername() + "@" + client.GetipAddress() + "\r\n";
            client.sendMsg(welcome);
            
            // Message 002 (RPL_YOURHOST)
            std::string yourHost = ":irc.server 002 " + client.GetNickname() + " :Your host is irc.server, running version 1.0\r\n";
            client.sendMsg(yourHost);
            
            // Message 003 (RPL_CREATED)
            std::string created = ":irc.server 003 " + client.GetNickname() + " :This server was created today\r\n";
            client.sendMsg(created);
            
            // Message 004 (RPL_MYINFO)
            std::string myInfo = ":irc.server 004 " + client.GetNickname() + " irc.server 1.0 o o\r\n";
            client.sendMsg(myInfo);
            
            // Message 375 (RPL_MOTDSTART)
            std::string motdStart = ":irc.server 375 " + client.GetNickname() + " :- irc.server Message of the day - \r\n";
            client.sendMsg(motdStart);
            
            // Message 372 (RPL_MOTD)
            std::string motd = ":irc.server 372 " + client.GetNickname() + " :- Welcome to the IRC server\r\n";
            client.sendMsg(motd);
            
            // Message 376 (RPL_ENDOFMOTD)
            std::string endMotd = ":irc.server 376 " + client.GetNickname() + " :End of /MOTD command\r\n";
            client.sendMsg(endMotd);
        }
    }
    else if (args[0] == "REQ")
    {
        std::string response = ":irc.server CAP " + client.GetNickname() + " ACK :multi-prefix\r\n";
        client.sendMsg(response); 
    }
}

void Server::user(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 3)
        return;
    std::cout << "\033[32mUSER command has been detected\033[0m" << std::endl;
    
    if (!(client.GetUser()))
    {
        if (client.GetPass())
        {
            client.SetUser(args);
            client.SetBuser(true);
        }
    }
    else
    {
        std::string error = ":irc.server 462 " + client.GetNickname() + " :You may not reregister\r\n";
        client.sendMsg(error);
    }
}

void Server::pass(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 1 || args[0].empty())
        return;
    std::cout << "\033[32mPASS command has been detected\033[0m" << std::endl;
    
    if (args[0] != this->_password)
    {
        std::string errorMsg = ":irc.server 464 * :Password incorrect\r\n";
        client.sendMsg(errorMsg);
    }
    else
        client.SetPass(true);
}

void Server::ping(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 2 || args[0].empty())
        return;
    std::cout << "\033[32mPING command has been detected\033[0m" << std::endl;
    
    if (client.GetRegistered())
    {
        std::string response = "PONG :" + args[0] + "\r\n";
        client.sendMsg(response);
    }
}
