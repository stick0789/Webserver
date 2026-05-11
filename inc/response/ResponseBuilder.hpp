/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 09:53:34 by marvin            #+#    #+#             */
/*   Updated: 2026/05/06 09:53:34 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSEBUILDER_HPP
#define RESPONSEBUILDER_HPP

#include "../inc/request/HTTPRequest.hpp"
#include "../inc/request/LocationConfig.hpp"
#include "../inc/parser/ServerConfig.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include "../inc/mime/MimeTypes.hpp"
#include "../inc/utils/Utils.hpp"
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

class ResponseBuilder
{
    private:
            std::string             _fullPath;
            int                     _cgi;
            bool                    _autoindex; 
            std::string             _redirectUrl; 
            std::string             _contentType; 
            size_t                  _contentLength;
            
            LocationConfig          _location; //copia la configuracio especifica de la ruta
            const ServerConfig*     _serverConf; //per consultat limits de body o pagines de error
            const HTTPRequest&      _request; //de on trec els metodes
            MimeTypes               _mime; //extensions (.png) a (image/png)
            CGIHandler&             _cgiObj;


            int     parsingPath();
            int     buildBody();
            int     buildHtmlIndex();
            void    buildErrorBody();
            int     readFile();
            void    parsingCGIResponse();
            void    parseAndSetCgiHeads(std::string headPart);


    public:
                ResponseBuilder();
                ResponseBuilder(const ResponseBuilder &src);
                ResponseBuilder &operator=(const ResponseBuilder&src);
                ~ResponseBuilder();

                void    buildResponse();
                void    setHeaders();
                void    setContentType();
                void    setContentLength();
                void    setConnection();
                void    setLocation();
                void    handleCGI();

                HTTPResponse&           _response; //a on enviare el resultat

                HTTPResponse    getResponse() const;     
                std::string     getFullPath() const {return this->_fullPath;}
};

#endif