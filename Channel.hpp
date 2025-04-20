/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 12:08:51 by dakojic           #+#    #+#             */
/*   Updated: 2025/04/21 01:23:43 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>


class Channel
{
    private:
        std::string channelName;
        std::string topic;
        int maxUsers;
        std::map<std::string, int> roles;
        std::map<std::string, int> members;
        std::string password;
        bool inviteOnly;
        Channel();

    public:
        Channel(std::string name){channelName = name; maxUsers = -1;};

        //Topic
        std::string GetTopic(){return topic;};
        void SetTopic(std::string s){topic = s;};
        
        void AddMember(std::string &);
        void RemoveMember(std::string &);
        void SetMaxUsers(int n) { maxUsers = n; };
        
        //password
        void SetPassword(std::string s){password = s;};
        std::string GetPassword(){return password;};
        
        //invite only
        void SetInviteOnly(){inviteOnly = true;};
        void RemoveInviteOnly(){inviteOnly = false;};
        bool CheckInviteOnly(){return inviteOnly;};
        //Getters
        std::string GetChannelName() const {return channelName;};
        void SetChannelName(std::string &s) { channelName = s; };
        // void GetRoles(std::string level);
        // void WhosOnline();
        int CheckRole(std::string name);
        bool IsFull() const;
};

#endif