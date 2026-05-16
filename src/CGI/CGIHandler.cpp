/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI .cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 09:56:39 by marvin            #+#    #+#             */
/*   Updated: 2026/05/05 12:26:17 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/CGI/CGIHandler.hpp"
#include <iostream>
#include <unistd.h>    
#include <sys/wait.h> 
#include <signal.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>

CGIHandler::CGIHandler(const HTTPRequest& req, HTTPResponse& res, const std::vector<uint8_t>& body) : 
    _cgiPid(-1), 
    _fullPath(""),
    _chEnv(NULL),
    _args(NULL),
    _body(body),
    _request(req),
    _response(res)
    
{}

CGIHandler::CGIHandler(const CGIHandler &src) :
    _cgiPid(src._cgiPid), 
    _fullPath(src._fullPath),
    _chEnv(src._chEnv),
    _args(src._args),
    _body(src._body),
    _request(src._request),
    _response(src._response)
{}

CGIHandler  &CGIHandler::operator=(const CGIHandler &src)
{
    if (this != &src)
    {
        _cgiPid = src._cgiPid; 
        _fullPath = src._fullPath;
    }
    return (*this);
}

CGIHandler::~CGIHandler()
{
    freeMemory();
}

void    CGIHandler::setCgiPath(const std::string& path)
{
    this->_cgiPath = path;
}

void    CGIHandler::freeMemory()
{
    if (_chEnv)
    {
        for (int i = 0; _chEnv[i]; i++)
        {
            free(_chEnv[i]);
            _chEnv = NULL;
        }
        delete[](_chEnv);
        _chEnv = NULL;
    }
    if (_args)
    {
        for (int i = 0; _args[i]; i++)
        {
            free(_args[i]);
            _args = NULL;
        }
        delete[](_args);
        _args = NULL;
    }
}

//void    CGIHandler::initEnv()
void    CGIHandler::initEnv(const LocationConfig& loc)
{
    //std::vector<LocationConfig>::const_iterator it_loc;
    int poz = findStart(_cgiPath, "cgi-bin/");
    _env["AUTH_TYPE"] = "Basic";
	_env["CONTENT_LENGTH"] = _request.getHeader("content-length");
	_env["CONTENT_TYPE"] = _request.getHeader("content-type");
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SCRIPT_NAME"] = _cgiPath;
    _env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > _cgiPath.size()) ? "" : _cgiPath.substr(poz + 8, _cgiPath.size())); // check dif cases after put right parametr from the response
    //_env["PATH_INFO"] = getPathInfo(_request.getPath(), it_loc->getCgiExtension());
    _env["PATH_INFO"] = getPathInfo(_request.getPath(), loc.getCgiExtension());
    //_env["PATH_TRANSLATED"] = it_loc->getRoot() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
    _env["PATH_TRANSLATED"] = loc.getRoot() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
    _env["QUERY_STRING"] = decode(_request.getQueryString());
    _env["REMOTE_ADDR"] = _request.getHeader("host");
	poz = findStart(_request.getHeader("host"), ":");
    _env["SERVER_NAME"] = (poz > 0 ? _request.getHeader("host").substr(0, poz) : "");
    _env["SERVER_PORT"] = (poz > 0 ? _request.getHeader("host").substr(poz + 1, _request.getHeader("host").size()) : "");
    _env["REQUEST_METHOD"] = _request.getMethod();
    _env["HTTP_COOKIE"] = _request.getHeader("cookie");
    //_env["DOCUMENT_ROOT"] = it_loc->getRoot();
    _env["DOCUMENT_ROOT"] = loc.getRoot();
	_env["REQUEST_URI"] = _request.getPath() + _request.getQueryString();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["REDIRECT_STATUS"] = "200";
	_env["SERVER_SOFTWARE"] = "AMANIX";
    _chEnv = new char*[_env.size() + 1];

    std::map<std::string, std::string>::iterator it;
    size_t i = 0;
    for (it = _env.begin(); it != _env.end(); it++)
    {
        std::string fullEnv = it->first + "=" + it->second;
        _chEnv[i] = ft_strdup(fullEnv.c_str());
        i++;
    }
    _chEnv[i] = NULL;

    _args = new char*[3];
    //_args[0] = ft_strdup(it_loc->getCgiPass().c_str());
    _args[0] = ft_strdup(loc.getCgiPass().c_str());
    _args[1] = ft_strdup(_cgiPath.c_str());
    _args[2] = NULL;
}

int CGIHandler::execute()
{
    if (pipe(_pipeOut) < 0 || pipe(_pipeIn) < 0)
    {
        _response.setStatusCode(500);
        return (1);
    }
    _cgiPid = fork();
    if (_cgiPid < 0)
    {
        close(_pipeOut[0]); close(_pipeOut[1]);
        close(_pipeIn[0]); close(_pipeIn[1]);
        _response.setStatusCode(500);
        return (1);
    }
    if (_cgiPid == 0)
    {
        dup2(_pipeOut[1], STDOUT_FILENO);
        dup2(_pipeIn[0], STDIN_FILENO);
        close(_pipeOut[0]);
        close(_pipeOut[1]);
        close(_pipeIn[0]);
        close(_pipeIn[1]);
        execve(_args[0], _args, _chEnv);
        exit(1);
        return (0);
    }
    else
    {
        close(_pipeOut[1]);
        close(_pipeIn[0]);
        if (_request.getMethod() == "POST")
        {
            
            const std::vector<uint8_t>& bodyData = _request.getBody();
            std::string body(reinterpret_cast<const char*>(bodyData.data()), bodyData.size()); 
            if (!body.empty())
                write(_pipeIn[1], body.c_str(), body.size());
        }
        close(_pipeIn[1]); 
    }
    return (_pipeOut[0]);
}