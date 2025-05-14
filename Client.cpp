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
    username = info[0];
    mode = info[1];
    realname = info[2];
    if(info.size() > 3 && info[3][0] == ':')
    {
        realname = info[3].substr(1);
        for(size_t i = 4; i < info.size(); i++)
            realname +=" " + info[i];
    }
}

void Client::sendMsg(const std::string &msg) const
{
    send(_fd, msg.c_str(), msg.length(), 0);
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
