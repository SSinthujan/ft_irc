/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:10 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/27 03:38:11 by almichel         ###   ########.fr       */
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
        int _fd;
        int num;
        std::string nickname;
        std::string ipAddress;
        std::string true_buffer;
        std::string username;
        std::string mode;
        std::string realname;
        
        bool registered;
        bool pass;
        bool end;
        bool buser;
        bool nick;
    public:
        Client(){_fd = -1 ,nick = false; buser = false ; end = false; pass = false; registered = false; nickname = "nickname"; username = "username"; mode = "mode"; realname = "realname"; true_buffer = "";};
        
        int GetFd(){return _fd;};
        int GetPass(){return pass;};
        void SetPass(bool set){pass = set;};
        int GetNick(){return nick;};
        void SetNick(bool set){nick = set;};
        int GetEnd(){return end;};
        void SetEnd(bool set){end = set;};
        int GetUser(){return buser;};
        void SetBuser(bool set){buser = set;};
        int GetRegistered() { return registered; };
        void SetRegistered(bool set) {registered = set;};
        
        std::string GetBuffer(){return true_buffer;};
        void AddToBuffer(char *str){true_buffer += str;};
        void SetNickname(std::string s){nickname = s;};
        void SetFd(int fd){_fd = fd;};
        void SetIpAdress(std::string IpAdr){ipAddress = IpAdr;};
        void ClearBuffer(){true_buffer.clear();};
        std::string GetNickname()const {return nickname;};
        void SetUser(std::vector<std::string>);
        int GetNum(){return num;};
        std::string GetUsername(){return username;};
        std::string GetipAddress(){return ipAddress;};
        void sendMsg(const std::string &msg) const;
        std::string get_command();



};

#endif