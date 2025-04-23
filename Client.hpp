/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:10 by dakojic           #+#    #+#             */
/*   Updated: 2025/04/23 01:39:06 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h> 

class Client
{
    private:
        int Fd;
        int num;
        std::string nickname;
        std::string IpAddress;
        std::string true_buffer;
        std::string username;
        std::string mode;
        std::string realname;
        
        bool registered;
    public:
        Client(){registered = false; nickname = "nickname"; username = "username"; mode = "mode"; realname = "realname";};
        
        int GetFd(){return Fd;};
        std::string GetBuffer(){return true_buffer;};
        void AddToBuffer(char *str){true_buffer += str;};
        void SetNickname(std::string s){nickname = s;};
        void SetFd(int fd){Fd = fd;};
        void SetIpAdress(std::string IpAdr){IpAddress = IpAdr;};
        void ClearBuffer(){true_buffer.clear();};
        std::string GetNickname()const {return nickname;};
        void SetUser(std::vector<std::string>);
        int GetNum(){return num;};
        bool IsRegistered() const { return registered; };\
        std::string GetUsername(){return username;};
        std::string GetIpAddress(){return IpAddress;};
        void sendMsg(const std::string &msg) const;


};

#endif