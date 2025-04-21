/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssitchsa <ssitchsa@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:14 by ssitchsa           #+#    #+#             */
/*   Updated: 2025/04/14 16:04:38 by ssitchsa          ###   ########.fr       */
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
class Client;
class Channel;

class Server
{
    private:
        int Port;
        int ServerSocketFD;
        static bool Signal;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds;
        std::vector<Channel> channels;
        std::string serverPassword;
    
    public:
        //Sever Starter
        Server(){ServerSocketFD = -1;};
        void ServerSocket();
        void ServerInit();
        
        //Client Calls
        void AcceptNewClient();
        void ReceiveNewData(int fd);
        void ParseLaunch(std::string &o, int);
        std::vector<std::string> SplitCmd(std::string &s);

        //Channel
        bool CheckIfChannelExists(std::string);

        //Signal
        static void SignalHadler(int signum);

        //Getters
        Client* GetClient(int fd);
        std::string GetPassword(){return serverPassword;};

        //Close
        void CloseFds();
        void CleanClients(int fds);    

        //Else
        std::vector<std::string> SplitTmpBuffer(std::string);

        //Cmds

        void Join(std::vector<std::string>, int fd);
};


#endif