/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:42:45 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/20 18:00:43 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <signal.h>

/*
#include <unistd.h>
*/
int main(int ac, char **av)
{
	Server ser;
	try
	{
		signal(SIGINT, Server::SignalHadler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, Server::SignalHadler);
		ser.inputCheck(ac, av);
		std::cout << "---- SERVER ----" << std::endl; //-> catch the signal (ctrl + \)
		ser.ServerInit(); //-> initialize the server
	}
	catch(const std::exception& e)
	{
		ser.CloseFds(); //-> close the file descriptors
		std::cerr << e.what() << std::endl;
	}
	std::cout << "The Server Closed!" << std::endl;
}
