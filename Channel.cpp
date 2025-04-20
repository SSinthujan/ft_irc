/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 13:39:02 by dakojic           #+#    #+#             */
/*   Updated: 2025/04/20 23:45:31 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

int Channel::CheckRole(std::string name)
{
    
    std::map<std::string, int>::iterator it;
    for(it = roles.begin(); it != roles.end(); it++)
    {
        if(it->first == name)
            return it->second;
    };
    return -1;
};

void Channel::AddMember(std::string& nickname)
{
    if (members.size() < (size_t)maxUsers || maxUsers == -1)
        members[nickname] = 1; // 1 = membre normal
}

void Channel::RemoveMember(std::string& nickname)
{
    members.erase(nickname);
}

bool Channel::IsFull() const 
{
    return maxUsers != -1 && members.size() >= (size_t)maxUsers;
}