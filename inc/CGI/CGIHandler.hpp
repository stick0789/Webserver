/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 13:44:52 by marvin            #+#    #+#             */
/*   Updated: 2026/05/13 20:02:36 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

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
#include "../inc/utils/CGIUtils.hpp"
#include "../inc/response/ResponseBuilder.hpp"


class CGIHandler
{
    private:
            pid_t   _cgiPid;
            int     _pipeIn[2];
            int     _pipeOut[2];
            std::map<std::string, std::string> _env;
            std::string _fullPath;
            char**  _chEnv;
            char**  _args;
            std::string _cgiPath;
            
            const HTTPRequest&      _request;
            HTTPResponse&           _response;

            void    freeMemory();
            
    public:
            CGIHandler(const HTTPRequest& req, HTTPResponse& res, const std::vector<uint8_t>& body);
            CGIHandler(const CGIHandler &src);
            CGIHandler  &operator=(const CGIHandler &src);
            ~CGIHandler();

            int    execute();
            void   initEnv(const LocationConfig* location);
        
            void        setCgiPath(const std::string& path);
};

#endif