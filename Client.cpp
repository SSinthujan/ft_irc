/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: almichel <almichel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:12 by ssitchsa          #+#    #+#             */
/*   Updated: 2025/04/23 01:36:36 by almichel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

void Client::SetUser(std::vector<std::string> info)
{
    username = info[1];
    mode = info[2];
    realname = info[3];
    if(info.size() > 4 && info[4][0] == ':')
    {
        realname = info[4].substr(1);
        for(size_t i = 5; i < info.size(); i++)
            realname +=" " + info[i];
    }
}

void Client::sendMsg(const std::string &msg) const
{
    send(Fd, msg.c_str(), msg.length(), 0);
}

std::string Client::get_command() {
    std::string line;
    if (true_buffer.find_first_of("\r\n") != std::string::npos)
    {
        line = true_buffer.substr(0, true_buffer.find_first_of("\r\n"));
        if (true_buffer.find_first_of("\r\n") + 2 > true_buffer.size())
            true_buffer.clear();
        else
            true_buffer = true_buffer.substr(true_buffer.find_first_of("\r\n") + 2);
    }
    return (line);
}
