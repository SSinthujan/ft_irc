/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 13:39:02 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/23 02:01:04 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"


Channel::Channel()
{

}

Channel::Channel(std::string name)
    : channelName(name),
      topic(""),
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
void Channel::AddMember(std::string &nickname)
{
    if (members.size() < (size_t)maxUsers || maxUsers == -1)
        members[nickname] = 1; // 1 = membre standard
}

void Channel::RemoveMember(std::string &nickname)
{
    members.erase(nickname);
}

bool Channel::HasMember(std::string const &nickname) const
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
        for (std::map<int, Client>::const_iterator cli = clients.begin(); cli != clients.end(); ++cli)
        {
            if (cli->second.GetNickname() == it->first)
            {
                cli->second.sendMsg(msg);
            }
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
