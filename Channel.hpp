/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssitchsa <ssitchsa@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 12:08:51 by ssitchsa           #+#    #+#             */
/*   Updated: 2025/04/11 16:47:32 by ssitchsa          ###   ########.fr       */
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
        std::string GetChannelName(){return channelName;};

        //Topic
        std::string GetTopic(){return topic;};
        void SetTopic(std::string s){topic = s;};
        
        void AddMember(std::string);
        
        //password
        void SetPassword(std::string s){password = s;};
        std::string GetPassword(){return password;};
        
        //invite only
        void SetInviteOnly(){inviteOnly = true;};
        void RemoveInviteOnly(){inviteOnly = false;};
        bool CheckInviteOnly(){return inviteOnly;};
        //Getters
        std::string GetChannelName() const {return channelName;};
        // std::string GetTopic(){return topic;};
        // void GetRoles(std::string level);
        // void WhosOnline();
        int CheckRole(std::string name);
};

#endif