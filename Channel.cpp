/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dakojic <dakojic@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/11 13:39:02 by dakojic           #+#    #+#             */
/*   Updated: 2025/04/11 16:47:34 by dakojic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

int Channel::CheckRole(std::string name)
{
    
    std::map<std::string, int>::iterator it;
    for(it = roles.begin(); it != roles.end(); it++)
    {
        if(it->first == name)
            return it->second;
    };
    return -1;
};