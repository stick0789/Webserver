/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 19:28:18 by pmorello          #+#    #+#             */
/*   Updated: 2026/05/13 20:04:04 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIUTILS_HPP
#define CGIUTILS_HPP

#include <iostream>
#include <unistd.h>    
#include <sys/wait.h> 
#include <sys/types.h>
#include <signal.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include "../inc/request/HTTPRequest.hpp"
#include "../inc/request/LocationConfig.hpp"
#include "../inc/parser/ServerConfig.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include "../inc/response/ResponseBuilder.hpp"

template <typename T>
std::string     ToString(const T val);
unsigned int    fromHexToDec(const std::string &nb);
char*           ft_strdup(const char *str);
int             findStart(const std::string path, const std::string delim);
std::string     decode(const std::string& path);
std::string     getPathInfo(const std::string& path, std::string extension);

#endif