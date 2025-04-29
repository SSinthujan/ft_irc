/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 13:39:02 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/29 02:37:57 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"


Channel::Channel()
{

}

Channel::Channel(std::string name)
    : channelName(name),
      topic("null"),
      maxUsers(-1),
      roles(),
      members(),
      invitedUsers(),
      password(""),
      topicRestricted(false),
      keyEnabled(false),
      inviteOnly(false)
{}
// === Gestion des rôles (opérateurs) ===
int Channel::CheckRole(std::string name)
{
    std::map<std::string, int>::iterator it;
    for (it = roles.begin(); it != roles.end(); it++)
    {
        if (it->first == name)
            return it->second;
    }
    return -1;
}

void Channel::SetOperator(std::string const &nickname)
{
    roles[nickname] = 1;
}

void Channel::RemoveOperator(std::string const &nickname)
{
    roles.erase(nickname);
}

bool Channel::IsOperator(std::string const &nickname) const
{
    std::map<std::string, int>::const_iterator it = roles.find(nickname);
    return it != roles.end() && it->second == 1;
}

// === Gestion des membres ===
void Channel::AddMember(std::string &nickname, int fd)
{
    if (members.size() < (size_t)maxUsers || maxUsers == -1)
        members[nickname] = fd; // 1 = membre standard
}

void Channel::AddMemberInvite(std::string &nickname, int fd)
{
        members[nickname] = fd;
}

void Channel::RemoveMember(const std::string &nickname)
{
    members.erase(nickname);
}

bool Channel::HasMember(std::string const &nickname)
{
    return members.find(nickname) != members.end();
}

bool Channel::IsFull() const
{
    return maxUsers != -1 && members.size() >= (size_t)maxUsers;
}

// === Gestion des invitations (MODE +i) ===
void Channel::AddInvite(std::string const &nickname)
{
    invitedUsers.push_back(nickname);
}

bool Channel::IsInvited(std::string const &nickname) const
{
    return std::find(invitedUsers.begin(), invitedUsers.end(), nickname) != invitedUsers.end();
}

void Channel::RemoveInvite(std::string const &nickname)
{
    std::vector<std::string>::iterator it = std::find(invitedUsers.begin(), invitedUsers.end(), nickname);
    if (it != invitedUsers.end())
        invitedUsers.erase(it);
}

// === Gestion de la clé (MODE +k) ===
void Channel::EnableKey(std::string const &pass)
{
    password = pass;
    keyEnabled = true;
}

void Channel::DisableKey()
{
    password = "";
    keyEnabled = false;
}

void Channel::Broadcast(const std::string &msg, const std::map<int, Client> &clients)
{
    for (std::map<std::string, int>::iterator it = members.begin(); it != members.end(); ++it)
    {
        int fd = it->second;
        std::map<int, Client>::const_iterator cli = clients.find(fd);
        if (cli != clients.end())
        {
            cli->second.sendMsg(msg);
        }
    }
}

void Channel::Broadcast2(const std::string &msg, const std::map<int, Client> &clients, const std::string& sender)
{
    for (std::map<std::string, int>::const_iterator it = members.begin(); it != members.end(); ++it)
    {
        const std::string& nickname = it->first;
        int fd = it->second;

        // On ne renvoie pas au sender
        if (nickname == sender)
            continue;

        std::map<int, Client>::const_iterator cli = clients.find(fd);
        if (cli != clients.end())
        {
            cli->second.sendMsg(msg);
        }
    }
}

std::vector<std::string> Channel::GetMembers() const
{
    std::vector<std::string> list;
    for (std::map<std::string, int>::const_iterator it = members.begin(); it != members.end(); ++it)
        list.push_back(it->first);
    return list;
}

bool Channel::IsEmpty() const
{
    return members.empty();
}

const std::map<std::string, int> &Channel::GetMembers2() const
{
    return members;
}

