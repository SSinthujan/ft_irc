/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:14 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/23 01:26:50 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sstream> 
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>
#include <cstdlib> 
#include <map>

class Client;
class Channel;

class Server
{
    private:
        int Port;
        int ServerSocketFD;
        static bool Signal;
        std::map<int, Client> clients;
        std::vector<struct pollfd> fds;
        std::map<std::string, Channel> channels;
        std::string serverPassword;

        std::string _portStr;
        std::string _password;
    
    public:
        //Sever Starter
        Server();
        void inputCheck(int ac, char **av);
        void ServerSocket();
        void ServerInit();
        void parseCmd(std::string &str);
        
        //Client Calls
        void AcceptNewClient();
        void ReceiveNewData(int fd);

        //Channel
        bool CheckIfChannelExists(std::string);

        //Signal
        static void SignalHandler(int signum);

        //Getters
        Client* GetClient(int fd);
        std::string GetPassword(){return serverPassword;};

        //Close
        void CloseFds();
        void CleanClient(int fds);

        //Cmds
        void nick(Client &client, std::string &str);
        void join(Client &client ,std::vector<std::string> &str, int fd);
};


#endif