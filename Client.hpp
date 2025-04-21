/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssitchsa <ssitchsa@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:10 by ssitchsa           #+#    #+#             */
/*   Updated: 2025/04/14 17:11:54 by ssitchsa          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <vector>

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
        std::string GetNickname(){return nickname;};
        void SetUser(std::vector<std::string>);
        int GetNum(){return num;};
        std::string GetUsername(){return username;};
        std::string GetIpAddress(){return IpAddress;}
};

#endif