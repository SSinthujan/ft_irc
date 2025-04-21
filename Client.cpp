/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ssitchsa <ssitchsa@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 10:36:12 by ssitchsa           #+#    #+#             */
/*   Updated: 2025/04/14 16:48:02 by ssitchsa          ###   ########.fr       */
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