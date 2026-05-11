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

CGIHandler::CGIHandler(const HTTPRequest& req, HTTPResponse& res, std::string body) : 
    _cgiPid(-1), 
    _exitStatus(0),
    _fullPath(""),
    _cgiResponse(""),
    _request(req),
    _response(res)
{}

CGIHandler::CGIHandler(const CGIHandler &src) :
    _cgiPid(src._cgiPid), 
    _exitStatus(src._exitStatus),
    _fullPath(src._fullPath),
    _cgiResponse(src._cgiResponse),
    _request(src._request),
    _response(src._response)
{}

CGIHandler  &CGIHandler::operator=(const CGIHandler &src)
{
    if (this != &src)
    {
        _cgiPid = src._cgiPid; 
        _exitStatus = src._exitStatus;
        _fullPath = src._fullPath;
        _cgiResponse = src._cgiResponse;
    }
    return (*this);
}

CGIHandler::~CGIHandler()
{
    freeMemory();
}

/* CGI Utils */
template <typename T>
std::string ToString(const T val)
{
    std::string stream;
    stream << val;
    return (stream.str());
}

static unsigned int    fromHexToDec(const std::string &nb)
{
    unsigned int x;
    std::stringstream ss;
    ss << nb;
    ss >> std::hex >> x;
    return (x);
}

static char*   ft_strdup(const char *str)
{
    if (!str)
        return (NULL);
    size_t  i = 0;
    while (str[i] != '/0')
        i++;
    char* res = new char[i + 1]; 
    if (!res)
        return (NULL);
    i = 0;
    while (str[i])
    {
        res[i] = str[i];
        i++; 
    }
    res[i] = '/0';
    return (res);
}

static int     findStart(const std::string path, const std::string delim)
{
    if (path.empty())
        return (-1);
    size_t poz = path.find(delim);
    if (poz != std::string::npos)
        return (poz);
    else
        return (-1);
}

static std::string decode(const std::string& path)
{
    std::string res = path;
    size_t token = res.find("%");
    while (token != std::string::npos)
    {
        if (res.length() < token + 2)
            break ;
        char decimal = fromHexToDec(res.substr(token + 1, 2));
        res.replace(token, 3, ToString(decimal));
        token = res.find("%");
    }
    return (res);
}

static std::string getPathInfo(const std::string& path, std::string extension)
{
	if (extension.empty())
        return "";
    size_t start = path.find(extension);
	if (start == std::string::npos)
		return "";
    size_t  end = start + extension.length();
	if (end >= path.size())
		return "";
	std::string tmp = path.substr(end);
	if (!tmp.empty() || tmp[0] != '/')
		return "";
    size_t  querypos = tmp.find("?");
	if (querypos != std::string::npos)
        return tmp;
    return (tmp.substr(0, querypos));
}

void    CGIHandler::freeMemory()
{
    if (_chEnv)
    {
        for (int i = 0; _chEnv[i]; i++)
            delete[]_chEnv[i];
        delete[](_chEnv);
        _chEnv = NULL;
    }
    if (_args)
    {
        for (int i = 0; _args[i]; i++)
            delete[]_args[i];
        delete[](_args);
        _args = NULL;
    }
}

void    CGIHandler::reqToMap(const std::vector<LocationConfig>::iterator it_loc)
{
    int poz = findStart(_cgiPath, "cgi-bin/");
    _env["AUTH_TYPE"] = "Basic";
	_env["CONTENT_LENGTH"] = _request.getHeader("content-length");
	_env["CONTENT_TYPE"] = _request.getHeader("content-type");
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SCRIPT_NAME"] = _cgiPath;
    _env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > _cgiPath.size()) ? "" : _cgiPath.substr(poz + 8, _cgiPath.size())); // check dif cases after put right parametr from the response
    _env["PATH_INFO"] = getPathInfo(_request.getPath(), it_loc->getCgiExtension());
    _env["PATH_TRANSLATED"] = it_loc->getRoot() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
    _env["QUERY_STRING"] = decode(_request.getQueryString());
    _env["REMOTE_ADDR"] = _request.getHeader("host");
	poz = findStart(_request.getHeader("host"), ":");
    _env["SERVER_NAME"] = (poz > 0 ? _request.getHeader("host").substr(0, poz) : "");
    _env["SERVER_PORT"] = (poz > 0 ? _request.getHeader("host").substr(poz + 1, _request.getHeader("host").size()) : "");
    _env["REQUEST_METHOD"] = _request.getMethod();
    _env["HTTP_COOKIE"] = _request.getHeader("cookie");
    _env["DOCUMENT_ROOT"] = it_loc->getRoot();
	_env["REQUEST_URI"] = _request.getPath() + _request.getQueryString();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["REDIRECT_STATUS"] = "200";
	_env["SERVER_SOFTWARE"] = "AMANIX";
    _chEnv = new char*[_env.size() + 1];
    std::map<std::string, std::string>::iterator it = _env.begin();
    for (int i = 0; it != _env.end(); it++, i++)
    {
        std::string tmp = it->first + "=" + it->second;
        _chEnv[i] = ft_strdup(tmp.c_str());
    }
    _args = new char*[3];
    _args[0] = ft_strdup(it_loc->getCgiPass().c_str());
    _args[1] = ft_strdup(_response._buildRes.getFullPath().c_str());
    _args[2] = NULL;

}


void    CGIHandler::execute()
{
    if (pipe(_pipeOut) < 0 || pipe(_pipeIn) < 0)
    {
        _response.setStatusCode(500);
    }
    _cgiPid = fork();
    if (_cgiPid < 0)
    {
        close(_pipeOut[0]); close(_pipeOut[1]);
        close(_pipeIn[0]); close(_pipeIn[1]);
        _response.setStatusCode(500);
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
    }
    else
    {
        close(_pipeOut[1]);
        close(_pipeIn[0]);
        if (_request.getMethod() == "POST")
        {
            std::string body = _request.getBody();
            if (!body.empty())
                write(_pipeIn[1], body.c_str(), body.size());
        }
        close(_pipeIn[1]); 
        char buffer[4096];
        int bytes_read;
        while ((bytes_read = read(_pipeOut[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            _cgiResponse.append(buffer, bytes_read);
        }
        close(_pipeOut[0]);
        int status;
        waitpid(_cgiPid, &status, 0);
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != 0)
                _response.setStatusCode(502);;
        }
        else
            _response.setStatusCode(504);
    }
}