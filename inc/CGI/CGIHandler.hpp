/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 13:44:52 by marvin            #+#    #+#             */
/*   Updated: 2026/05/09 14:55:41 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <unistd.h>    // La més important: conté fork(), pipe(), execve(), dup2(), close()
#include <sys/wait.h>  // Per a la funció waitpid() (evitar processos zombis)
#include <sys/types.h> // Defineix el tipus pid_t
#include <signal.h>    // Per gestionar senyals si cal matar processos (kill)
#include <string>
#include <stdio.h>
#include <stdlib.h>


#include "../inc/request/HTTPRequest.hpp"
#include "../inc/request/LocationConfig.hpp"
#include "../inc/parser/ServerConfig.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include "../inc/mime/MimeTypes.hpp"
#include "../inc/utils/Utils.hpp"
#include "../inc/response/ResponseBuilder.hpp"

// Arriba en CGIHandler.hpp, antes de la clase:
//class HTTPResponse;
//class HTTPRequest;

class CGIHandler
{
    private:
            pid_t   _cgiPid;
            int     _exitStatus;
            int     _pipeIn[2];
            int     _pipeOut[2];
            std::map<std::string, std::string> _env;
            std::string _fullPath;

            //per fer funcionar el execve
            char**  _chEnv;
            char**  _args;
            std::string _cgiPath;

            std::string _cgiResponse; //info que ha fet tot el pipe de tornada

            const HTTPRequest&      _request;
            LocationConfig          _location;
            HTTPResponse&            _response;

            void    freeMemory();


           

    public:
            CGIHandler(const HTTPRequest& req, HTTPResponse& res, std::string body);
            CGIHandler(const CGIHandler &src);
            CGIHandler  &operator=(const CGIHandler &src);
            ~CGIHandler();

            void execute();
            void   reqToMap(const std::vector<LocationConfig>::iterator it_loc);
            void    mapToCharPtr();
            void    argsToCharPtr();

            //getters
            std::string getCgiResponse() const {return (this->_cgiResponse);}
};

#endif