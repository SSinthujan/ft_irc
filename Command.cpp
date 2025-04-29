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
    std::vector<std::string> channelsToPrint;
    if(args.size() < 2 || args[1].empty() || args[1][0] == ' ')
        return ;
    if (args.size() > 1)
    {
        std::string str;
        std::stringstream ss(args[1]);
        
        while (std::getline(ss, str, ',')) 
        {
            if (!str.empty())
                channelsToPrint.push_back(str);
        }
    }
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
    CleanClient(fd);
    close(fd);
}

void Server::mode(Client &client, std::vector<std::string> &args, int fd)
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
