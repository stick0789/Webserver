/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BuilderUtils.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 19:02:00 by pmorello          #+#    #+#             */
/*   Updated: 2026/05/13 19:03:33 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILDERUTILS_HPP
#define BUILDERUTILS_HPP

#include "../inc/request/HTTPRequest.hpp"
#include "../inc/request/LocationConfig.hpp"
#include "../inc/parser/ServerConfig.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include "../inc/CGI/CGIHandler.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

void LocationMatchRequest(const std::string &reqPath, const std::vector<LocationConfig> &locsPath, std::string &urlMatch);
int  validMethods(const std::string &method, const LocationConfig *ptr);
bool isDirectory(const std::string &path);
bool fileExist(std::string &path);
std::string removeBoundary(const std::string &body, std::string &bdary);



#endif