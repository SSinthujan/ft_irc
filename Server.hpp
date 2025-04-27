/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:14 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/27 19:02:12 by almichel         ###   ########.fr       */
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
        bool quit_flag;
    
    public:
        //StartServer 
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

        void Names(Client &client, std::vector<std::string> str, int fd);

        //Signal
        static void SignalHandler(int signum);

        //Getters
        Client* GetClient(int fd);
        std::string GetPassword(){return serverPassword;};
        Channel* GetChannel(const std::string& name);
  
        //Close
        void CloseFds();
        void CleanClient(int fds);

        //Else
        std::vector<std::string> SplitTmpBuffer(std::string);

        //Cmds
        void nick(Client &client, std::string &str);
        void join(Client &client ,std::vector<std::string> &str, int fd);
        void quit(Client &client, std::vector<std::string> str, int fd);
        void mode(Client *client, const std::vector<std::string> &split, int fd);
        void names(Client &client, std::vector<std::string> str, int fd)
};


#endif