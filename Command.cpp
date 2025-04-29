/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssitchsa <ssitchsa@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 23:33:18 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/28 23:33:56 by ssitchsa         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::nick(Client &client, std::vector<std::string> &args, int fd){
    fd++;
    client.SetNickname(args[0]);
}

// void Server::join(Client &client, std::vector<std::string> &args, int fd)
// {

// }

void Server::names(Client &client, std::vector<std::string> &args, int fd)
{
    std::vector<std::string> channelToPrint;
    if(args.size() < 2 || args[1].empty() || args[1][0] == ' ')
        return ;
    if (args.size() > 1)
    {
        std::string str;
        std::stringstream ss(args[1]);
        
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
    std::cout << "Client <" << fd << "> disconnected (" << reason << ")" << std::endl;
    //BroadcastQuitMessage(clientFd, reason);
    for (std::map<std::string, Channel>::iterator it = chan.begin(); it != chan.end(); )
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
            chan.erase(tmp); // Attention: erase retourne le nouvel itérateur
        }
        else
        {
            ++it; // sinon, on avance normalement
        }
    }
    CleanClient(fd);
    close(fd);
}

void Server::mode(Client &client, std::vector<std::string> &args, int fd)
{
    if (args.size() < 3) 
    {
        std::string error = ":irc.server 461 " + client.GetNickname() + " MODE :Not enough parameters\r\n";
        send(fd, error.c_str(), error.length(), 0);
        return;
    }

    std::string channelName = args[1];
    std::string modeChanges = args[2];

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
    for (size_t i = 3; i < args.size(); ++i) {
        msg += " " + args[i];
    }
    msg += "\r\n";
    chan->Broadcast(msg, this->clients);
}

void Server::part(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    std::string subject;
    if (args.size() < 2 || args[1].empty())
        return;
    std::cout << "\033[32mPART command has been detected\033[0m" << std::endl;
    if (!CheckIfChannelExists(args[1]))
    {
        std::string error = ":" + std::string("irc.server 403 ")+ client.GetNickname() + " " + args[1] + " :No such channel\r\n";
        client.sendMsg(error);
        return;
    }
    Channel& channel = channel[args[1]];
    if (!channel.HasMember(client.GetNickname()))
    {
        std::string error1 = ":" + std::string("irc.server 442 ") + client.GetNickname() + " " + args[1] + " :You're not on that channel\r\n";
        client.sendMsg(error1);
        return;
    }
    if (args.size() > 2)
    {
        for (size_t i = 2; i < args.size(); ++i)
        {
            subject += args[i];
            if (i != args.size() - 1)
                subject += " ";
        }
    }
    std::string prefix = client.GetNickname() + "!" + client.GetUsername() + "@" + "localhost";
    std::string partMsg;
    if (args.size() > 2)
        partMsg = ":" + prefix + " PART " + args[1] + " :" + subject + "\r\n";
    else
        partMsg = ":" + prefix + " PART " + args[1] + "\r\n";
    std::cout << client.GetUsername() << std::endl;
    channel.RemoveMember(client.GetNickname());
    channel.Broadcast(partMsg, clients);
    client.sendMsg(partMsg);
}

void Server::privmsg(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 3 || args[1].empty() || args[2].empty())
        return;
    std::cout << "\033[32mPRIVMSG command has been detected\033[0m" << std::endl;
    std::string message;
    for (size_t i = 2; i < args.size(); ++i)
    {
        message += args[i];
        if (i != args.size() - 1)
            message += " ";
    }
    std::string prefix = ":" + client.GetNickname() + "!" + client.GetUsername() + "@localhost";
    std::string prefix2 = ":" + client.GetNickname();
    std::string fullmsg = prefix + " PRIVMSG " + args[1] + " :" + message + "\r\n";
    std::string fullmsg2 = prefix2 + " PRIVMSG " + args[1] + " :" + message + "\r\n";
    if (args[1][0] == '#')
    {
        if (!CheckIfChannelExists(args[1]))
        {
            std::string error = ":" + std::string("irc.server 403 ")+ client.GetNickname() + " " + args[1] + " :No such channel\r\n";
            client.sendMsg(error);
            return;
        }

        Channel& channel = channel[args[1]];
        channel.Broadcast2(fullmsg, clients, client.GetNickname());
    }
    else
    {
        Client* target = GetClientByNickname(args[1]);
        if (!target)
        {
            std::string error = ":" + std::string("irc.server 401 ")+ client.GetNickname() + " " + args[1] + " :No such nick\r\n";
            client.sendMsg(error);
            return;
        }
        target->sendMsg(fullmsg2);
    }
}

void Server::invite(Client &client, std::vector<std::string> &args, int fd)
{
    (void)fd;
    if (args.size() < 3 || args[1].empty() || args[2].empty())
        return ;
    std::string channelToInvit;
    std::string name;

    name = args[2];
    channelToInvit = args[1];
    if (!CheckIfChannelExists(channelToInvit))
    {
        std::string error = ":" + std::string("irc.server 403 ") + client.GetNickname() + " " + channelToInvit + " :No such channel\r\n";
        client.sendMsg(error);
        return;
    }
    Channel& channel = channel[channelToInvit];
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
    if (args.size() < 3 || args[1].empty() || args[2].empty())
        return ;
    std::string channelToKick;
    std::string name;
    std::string motif;

    name = args[2];
    channelToKick = args[1];
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
    Channel& channel = channel[channelToKick];
    if (!(channel.IsOperator(client.GetNickname())))
    {
        std::string error2 = ":" + std::string("irc.server 482 ") + client.GetNickname() + " " + str[1] + " :You're not channel operator\r\n";
        client.sendMsg(error2);
        return;
    }
    if (!(channel.HasMember(name)))
    {
        std::string error2 = ":" + std::string("irc.server 441 ") + client.GetNickname() + " " + name + " " + str[1] + " :You're not channel operator\r\n";
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
    for (std::map<std::string, Channel>::iterator it = channel.begin(); it != channel.end(); )
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
            channel.erase(tmp); // Attention: erase retourne le nouvel itérateur
        }
        else
            ++it;
    }
    CleanClient(fd);
    close(fd);
  
}