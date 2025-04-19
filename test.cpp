#include <iostream>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sstream> 
// Classes

class Client
{
    private:
        int Fd;
        std::string IpAddress;
        std::string true_buffer;
    public:
        Client(){};
        
        int GetFd(){return Fd;};
        std::string GetBuffer();
        void AddToBuffer(char *str);
        void SetFd(int fd){Fd = fd;};
        void SetIpAdress(std::string IpAdr){IpAddress = IpAdr;};
        void ClearBuffer(){true_buffer.clear();};
};

class Server
{
    private:
        int Port;
        int ServerSocketFD;
        static bool Signal;
        std::vector<Client> clients;
        std::vector<struct pollfd> fds;
    
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

        //Signal
        static void SignalHadler(int signum);

        //Getters
        Client* GetClient(int fd);

        //Close
        void CloseFds();
        void CleanClients(int fds);    

        //Else
        std::vector<std::string> SplitTmpBuffer(std::string);


};

// Methodes

void Client::AddToBuffer(char* str)
{
    true_buffer += str;
}
// Close 
std::string Client::GetBuffer()
{
    return true_buffer;
}
Client* Server::GetClient(int fd)
{
    for(size_t i = 0; i < this->clients.size(); i++)
    {
        if(this->clients[i].GetFd() == fd)
            return &this->clients[i];
    }
    return NULL;
}
void Server::CleanClients(int fd)
{
    for(size_t i = 0; i < fds.size(); i++)
    {
        if(fds[i].fd == fd)
        {
            fds.erase(fds.begin() + i);
            break ;
        }
    }
    for(size_t i = 0; i < clients.size(); i++)
    {
        if(clients[i].GetFd() == fd)
        {
            clients.erase(clients.begin() + i);
            break ;
        }
    }
};

void Server::CloseFds()
{
    for(size_t i = 0; i < clients.size(); i++)
    {
        std::cout << "Client <" << clients[i].GetFd() << "> Disconnected" <<  std::endl;
        close(clients[i].GetFd());       
    }
    if(ServerSocketFD != -1)
    {
        std::cout << "Server <" << ServerSocketFD << "> disconnected" << std::endl;
        close(ServerSocketFD);
    }
};

// Signal

bool Server::Signal = false;

void Server::SignalHadler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Signal Received" << std::endl;
    Server::Signal = true;
};

std::vector<std::string> Server::SplitTmpBuffer(std::string str) // A revoir ??
{
    std::vector<std::string> split;
    std::istringstream iss(str);
    std::string tmp;
    while(std::getline(iss, tmp))
    {
        size_t pos = tmp.find_first_of("\r\n");
        if(pos != std::string::npos)
            tmp = tmp.substr(0, pos);
        split.push_back(tmp);
    }
    return split;
    
};

std::vector<std::string> Server::SplitCmd(std::string &cmd) // A revoir ??
{
    std::vector<std::string> vec;
    std::istringstream stm(cmd);
    std::string token;
    while(stm >> token)
    {
        vec.push_back(token);
        token.clear();
    }
    return vec;
};
void Server::ParseLaunch(std::string &str, int fd)
{
    if(str.empty())
        return ;
    std::vector<std::string> split = SplitCmd(str);
    size_t sep = str.find_first_of(" \t\v");
    if(sep != std::string::npos)
        str = str.substr(sep);
    if(split[0][0] == '/')
        std::cout <<"IS A COMMAND" << std::endl;
    else
        std::cout<<"IS A MESSAGE"<<std::endl;

}

void Server::ReceiveNewData(int fd)
{
    char buff[1024];
    memset(buff, 0, sizeof(buff));
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);
    Client *tmp_client = GetClient(fd);
    std::vector<std::string> split;
    if(bytes <= 0)
    {
        std::cout << "Client <" << fd<<"> disconnected" << std::endl;
        CleanClients(fd);
        close(fd);
    }
    else
    {
        tmp_client->AddToBuffer(buff);
        if(tmp_client->GetBuffer().find_first_of("\r\n") == std::string::npos)
            return ;
        split = SplitTmpBuffer(tmp_client->GetBuffer());
        for(size_t i = 0; i < split.size(); i++)
            this->ParseLaunch(split[i], fd);
        tmp_client->ClearBuffer();
    }
};

void Server::AcceptNewClient()
{
    Client newClient;
    struct sockaddr_in clientAdress;
    struct pollfd newFd;
    socklen_t len = sizeof(clientAdress);

    int incommingfd = accept(ServerSocketFD, (sockaddr *)&clientAdress, &len);
    if(incommingfd == -1)
    {
        std::cout << "Accept() failed" << std::endl;
        return ;
    }
    if(fcntl(incommingfd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "fcntl() failed" << std::endl;
        return ;
    }
    newFd.fd = incommingfd;
    newFd.events = POLLIN;
    newFd.revents = 0;
    
    newClient.SetFd(incommingfd);
    newClient.SetIpAdress(inet_ntoa(clientAdress.sin_addr));
    clients.push_back(newClient);
    fds.push_back(newFd);

    std::cout << "Client <" << incommingfd << "> connected" << std::endl;
};

void Server::ServerSocket()
{
    int en = 1;
    struct sockaddr_in address;
    struct pollfd newFd;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(this->Port);
    
    ServerSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(ServerSocketFD == -1)
        throw(std::runtime_error("socket() failed"));
    if(setsockopt(ServerSocketFD, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == 1)
        throw(std::runtime_error("setsockopt() failed to set option SO_REUSEADDR"));
    if(bind(ServerSocketFD, (struct sockaddr *) &address, sizeof(address)) == -1)
        throw(std::runtime_error("failed to bind() socket"));
    if(listen(ServerSocketFD, SOMAXCONN) == -1)
        throw(std::runtime_error("listen() failed"));
    newFd.fd = ServerSocketFD;
    newFd.events = POLLIN;
    newFd.revents = 0;
    fds.push_back(newFd);
};

void Server::ServerInit()
{
    this->Port = 4444;
    ServerSocket();
    std::cout<<"Server <" << ServerSocketFD << "> connected" << std::endl;
    std::cout<<"Waiting for incomming connections..."<<std::endl;
    while(Server::Signal == false)
    {
        if((poll(&fds[0], fds.size(), -1) == -1) && Server::Signal == false)
            throw(std::runtime_error("poll() failed"));
        else
        {
            for(size_t i = 0; i < fds.size(); i++)
            {
                if(fds[i].revents & POLLIN)
                {
                    if(fds[i].fd == ServerSocketFD)
                        AcceptNewClient();
                    else
                        ReceiveNewData(fds[i].fd);

                }
            }
        }
    }
    CloseFds();
}

int main()
{
	Server ser;
	std::cout << "---- SERVER ----" << std::endl;
	try{
		signal(SIGINT, Server::SignalHadler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::SignalHadler); //-> catch the signal (ctrl + \)
		ser.ServerInit(); //-> initialize the server
	}
	catch(const std::exception& e){
		ser.CloseFds(); //-> close the file descriptors
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}
