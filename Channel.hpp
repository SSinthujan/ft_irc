/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 12:08:51 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/28 19:48:36 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include "Client.hpp"

class Client;

class Channel
{
    private:
        std::string channelName;
        std::string topic;
        int maxUsers;
        std::map<std::string, int> roles;
        std::map<std::string, int> members;
        std::vector<std::string> invitedUsers;
        std::string password;
        bool topicRestricted;
        bool keyEnabled;
        bool inviteOnly;

    public:
            Channel();
            Channel(std::string name);

           // === Informations générales
           std::string& GetChannelName()  { return channelName; };
           void SetChannelName(std::string &s) { channelName = s; };
           const std::map<std::string, int>& GetMembers2() const;
   
           // === Gestion du topic
           std::string GetTopic() { return topic; };
           void SetTopic(std::string s) { topic = s; };
           void SetTopicRestricted(bool b) { topicRestricted = b; };
           bool IsTopicRestricted() const { return topicRestricted; };
   
           // === Gestion des membres
           void AddMember(std::string &nickname, int fd);
           void RemoveMember(const std::string &nickname);
           bool HasMember(const std::string  &nickname);
           std::vector<std::string> GetMembers() const;
           bool IsFull() const;
           void SetMaxUsers(int n) { maxUsers = n; };
           bool IsEmpty() const;
   
           // === Gestion des opérateurs
           int CheckRole(std::string name);
           void SetOperator(std::string const &nickname);
           void RemoveOperator(std::string const &nickname);
           bool IsOperator(std::string const &nickname) const;
           void AddOperator(const std::string &nickname) { roles[nickname] = 1; };
   
           // === Mode +i : gestion du mode sur invitation 
           void SetInviteOnly() { inviteOnly = true; };
           void RemoveInviteOnly() { inviteOnly = false; };
           bool CheckInviteOnly() { return inviteOnly; };
           void AddInvite(std::string const &nickname);
           bool IsInvited(std::string const &nickname) const;
           void RemoveInvite(std::string const &nickname);
           void SetUserLimit(int limit) { maxUsers = limit; };
            void RemoveUserLimit() { maxUsers = -1; };
   
           // === Mode +k : gestion de la clé (mot de passe)
           void SetPassword(std::string s) { password = s; };
           std::string GetPassword() { return password; };
           void EnableKey(std::string const &pass);
           void DisableKey();
           bool IsKeyEnabled() const { return keyEnabled; };
           void SetKey(const std::string &key) { password = key; keyEnabled = true; }
            void RemoveKey() { password.clear(); keyEnabled = false; }

           //msg
           void Broadcast(const std::string &msg, const std::map<int, Client> &clients);
           void Broadcast2(const std::string &msg, const std::map<int, Client> &clients, const std::string& sender);

};

#endif