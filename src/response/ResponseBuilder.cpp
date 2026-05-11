/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/04 15:14:30 by marvin            #+#    #+#             */
/*   Updated: 2026/05/09 14:51:37 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/response/ResponseBuilder.hpp"

/* utils ResponseBuilder */

static void LocationMatchRequest(const std::string &reqPath, const std::vector<LocationConfig> &locsPath, std::string &urlMatch)
{
    size_t  longestMatch = 0;
    std::string res = "";
    std::vector<LocationConfig>::const_iterator it;
    for (it = locsPath.begin(); it != locsPath.end(); it++)
    {
        if (reqPath.compare(0, it->getPath().length(), it->getPath()) == 0)
        {
            if (it->getPath() == "/" || reqPath.length() == it->getPath().length() || reqPath[it->getPath().length()] == '/')
            {
                if (reqPath.length() > longestMatch)
                    longestMatch = it->getPath().length();
                    urlMatch = it->getPath();
            }
        }
    }
}

static int  validMethods(const std::string &method, const LocationConfig *ptr)
{
    const std::vector<std::string> &methods = ptr->getAllowedMethods();
    if (methods.empty())
        return (0);
    std::vector<std::string>::const_iterator it;
    for (it = methods.begin(); it != methods.end(); it++)
    {
        if (*it == method)
            return (0);
    }
    return (1);
}

static bool isDirectory(const std::string &path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == 0)
        return (S_ISDIR(s.st_mode));
    return (false);
    
}

static bool fileExist(std::string &path)
{
    struct stat s;
    return (stat(path.c_str(), &s) == 0);
}

static std::string removeBoundary(const std::string &body, std::string &bdary)
{
    std::string buffer;
    std::string newBody;
    std::string filename;
    bool isBoundary = false;
    bool isContent = false;

    std::string start_delim = "--" + bdary;
    std::string end_delim = "--" + bdary + "--";

    if (body.find(start_delim) != std::string::npos && body.find(end_delim) != std::string::npos)
    {
        for (size_t i = 0; i < body.size(); i++)
        {
            buffer.clear();
            while (i < body.size() && body[i] != '/n')
            {
                buffer += body[i];
                i++;
            }
            if (!buffer.compare(0, end_delim.length(), end_delim))
            {
                if (isContent && !newBody.empty())
                    newBody.erase(newBody.end() - 1);
                break;
            }
            if (!buffer.compare(0, start_delim.length(), start_delim))
            {
                isBoundary = true;
                isContent = false;
                continue;
            }
            if (isBoundary)
            {
                if (!buffer.compare(0, 31, "Content-Disposition: form-data;"))
                {
                    size_t start = buffer.find("filename=/");
                    if (start != std::string::npos)
                    {
                        size_t end = buffer.find("/", start + 10);
                        if (end != std::string::npos)
                        {
                            filename = buffer.substr(start + 10, end - (start + 10));
                        }
                    }
                }
                else if(!buffer.compare(0, 1, "/r"))
                {
                    isBoundary = false;
                    isContent = true;
                }
            }
            else if (isContent)
            {
                newBody += (buffer + "/n");
            }
        }
    }
    return (newBody);
}



HTTPResponse    ResponseBuilder::getResponse() const
{
    return (this->_response);
}

int  ResponseBuilder::readFile()
{
    std::ifstream file(_fullPath.c_str(), std::ios::binary);
    if (!file)
        return (1);
    std::stringstream ss;
    ss << file.rdbuf();
    _response.setBody(ss.str());
    return (0);
}

/* Extreu el path i fa algunes validacions 1.Mapeig Logic, 2.validacio de Regles, 3.URL a fullPath, 4.Resolucio de recurs */
int    ResponseBuilder::parsingPath()
{
    
    std::string urlMatch = "";
    LocationMatchRequest(_request.getPath(), _serverConf->getLocations(), urlMatch);
    if (urlMatch.empty())
    {
        _response.setStatusCode(404);
        return (1);
    }
    LocationConfig *confLoc = NULL; //es un punter a on guardarem la configuracio de Location copiada.
    for (size_t i = 0; i < _serverConf->getLocations().size(); i++)
    {
        if (_serverConf->getLocations()[i].getPath() == urlMatch)
        {
            _location = _serverConf->getLocations()[i];
            confLoc = &_location;
            break ;
        }
    }
    if (!confLoc)
    {
        _response.setStatusCode(404);
        return (1);
    }
    if (validMethods(_request.getMethod(), confLoc))
    {
        _response.setStatusCode(405);
        return (1);
    }
    if (_request.getBody().size() > _serverConf->getClientMaxBodySize())
    {
        _response.setStatusCode(413);
        return (1);
    }
    if (confLoc->getRedirectCode() != 0)
    {
        _response.setStatusCode(confLoc->getRedirectCode());
        _redirectUrl = confLoc->getRedirectUrl();
        return (1);
    }
    if (confLoc->getPath().find("cgi-bin") != std::string::npos || !confLoc->getCgiExtension().empty())
    {
        _cgi = 1;
        return (0);
    }
    _fullPath = confLoc->getRoot() + _request.getPath();
    if (isDirectory(_fullPath))
    {
        if (_fullPath[_fullPath.length() - 1] != '/')
        {
            _response.setStatusCode(301);
            _redirectUrl = _request.getPath() + "/";
            return (1);
        }
        const std::vector<std::string>& indexFiles = confLoc->getIndexFiles();
        for (size_t i = 0; i < indexFiles.size(); i++)
        {
            std::string indexPath = _fullPath + indexFiles[i];
            if (fileExist(indexPath))
            {
                _fullPath = indexPath;
                return (0);
            }
        }
        if (confLoc->getAutoindex())
        {
            _autoindex = true;
            return (0);
        }
        _response.setStatusCode(403);
        return (1);
    }
    if (!fileExist(_fullPath))
    {
        _response.setStatusCode(404);
        return (1);
    }
    return (0);
}

/* Monta la pagina en cas de que surti algun error */
void    ResponseBuilder::buildErrorBody()
{
    int code = _response.getStatusCode();
    if (_serverConf && _serverConf->getErrorPages().count(code))
    {
        _fullPath = _location.getRoot() + _serverConf->getErrorPages().at(code);
        if (readFile())
            return ;
    }
    _response = HTTPResponse::buildErrorResponse(code);
}

/* Monta la pagina en cas de que no hi hagui fitxer dins */
int ResponseBuilder::buildHtmlIndex()
{
    DIR *directory = opendir(_fullPath.c_str());
    if (directory == NULL) 
        return (1);
    std::ostringstream oss;
    oss << "<html><head><title>Index of ";
    oss << _request.getPath() << "</title></head>";
    oss << "<body><h1>Index of " << _request.getPath() << "</h1><hr><table>";
    
    struct dirent *entity;
    while((entity = readdir(directory)) != NULL)
    {
        if(std::string(entity->d_name) == ".") 
            continue;
        oss << "<tr><td><a href=\"" << entity->d_name << "\">";
        oss << entity->d_name << "</a></td></tr>";
    }
    oss << "</table><hr></body></html>";
    closedir(directory);
    _response.setBody(oss.str());
    _contentType = "text/html"; 
    return (0);
}

void    ResponseBuilder::setContentType()
{
    if (_contentType == "text/plain" && !_fullPath.empty())
    {
        size_t dot_pos = _fullPath.find_last_of(".");
        if (dot_pos != std::string::npos)
            _contentType = _mime.getMimeType(_fullPath.substr(dot_pos));
    }
    _response.setHeader("Content-Type", _contentType);
}

void    ResponseBuilder::setContentLength()
{
    std::stringstream ss;
    ss << _response.getBody().size();
    _response.setHeader("Content-Length", ss.str());
}

void    ResponseBuilder::setConnection()
{
    if (_request.getHeader("connection") == "keep-alive")
        _response.setHeader("Connection", "keep-alive");
    else
        _response.setHeader("Connection", "close");
}

void    ResponseBuilder::setLocation()
{
    if (_response.getStatusCode() >= 300 && _response.getStatusCode() < 400 && !_redirectUrl.empty())
        _response.setHeader("Location", _redirectUrl);
}
/* Posa el missatge als headers */
void ResponseBuilder::setHeaders()
{
    setContentType();
    setContentLength();
    setConnection();
    setLocation();
}

/* Un cop amb el path parsejat, i amb les flags comprovades monta el cos depenen del metodes */
int ResponseBuilder::buildBody()
{
    if (_request.getBody().size() > _serverConf->getClientMaxBodySize())
    {
        _response.setStatusCode(413);
        buildErrorBody();
        return (1);
    }
    if (parsingPath())
    {
        buildErrorBody();
        return (1);
    }
    if (_cgi == 1 || _autoindex == true)
        return (0);
    std::string method = _request.getMethod();
    if (method == "GET")
    {
        if (readFile()) 
        {
            _response.setStatusCode(404);
            buildErrorBody();
            return (1);
        }
    }
    else if (method == "POST" || method == "PUT")
    {
        std::ofstream   file(_fullPath.c_str(), std::ios::binary);
        if (!file.is_open())
        {
            _response.setStatusCode(500);
            buildErrorBody();
            return (1);
        }
        std::string contentType = _request.getHeader("content-Type");
        const std::string& rawData = _request.getBody();
        if (contentType.find("multipart/form-data") != std::string::npos)
        {
            size_t  boundaryPos = contentType.find("boundary=");
            if (boundaryPos != std::string::npos)
            {
                std::string boundary = contentType.substr(boundaryPos + 9);
                std::string cleanBody = removeBoundary(rawData, boundary);
                file.write(cleanBody.c_str(), cleanBody.length());
            }
            else
                file.write(rawData.data(), rawData.size());
        }
        else
            file.write(rawData.data(), rawData.size());
        file.close();
        _response.setStatusCode(201);
    }
    else if (method == "DELETE")
    {
        if (std::remove(_fullPath.c_str()))
            _response.setStatusCode(404);
        else
            _response.setStatusCode(204);
    }
    if (_response.getStatusCode() == 0)
        _response.setStatusCode(200);
    return (0);
}

/* Funcio principal on monta tot */
void    ResponseBuilder::buildResponse()
{
    if (buildBody())
        return ;
    if (_cgi)
        return ;
    if (_autoindex)
    {
        if (buildHtmlIndex())
        {
            _response.setStatusCode(500);
            buildErrorBody();
            return ;
        }
        else
            _response.setStatusCode(200);
    }
    setHeaders();
}


void    ResponseBuilder::parsingCGIResponse()
{
    std::string cgiResponse = _cgiObj.getCgiResponse(); 
    std::string cgiHead; 
    std::string cgiBody;
    size_t  sep = cgiResponse.find("/r/n/r/n");
    if (sep != std::string::npos)
    {
        cgiHead = cgiResponse.substr(0, sep);
        cgiBody = cgiResponse.substr(sep + 4);
    }
    else
    {
        sep = cgiResponse.find("/n/n");
        if (sep != std::string::npos)
        {
            cgiHead = cgiResponse.substr(0, 2);
            cgiBody = cgiResponse.substr(sep + 2);
        }
        else
        {
            cgiBody = cgiResponse;
        }
    }
    if(!cgiHead.empty())
        parseAndSetCgiHeads(cgiHead);
    _response.setBody(cgiBody);
}

void    ResponseBuilder::parseAndSetCgiHeads(std::string headPart)
{
    std::stringstream ss;
    ss << headPart;
    std::string line;
    while (std::getline(ss, line))
    {
       if (!line.empty() && line[line.size() - 1] == '/r')
            line.erase(line.size() - 1);
       if (line.empty())
            continue;
        size_t  colonPos = line.find(":");
        if (colonPos != std::string::npos)
        {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            size_t  first = value.find_first_not_of(" ");
            if (first != std::string::npos)
                value = value.substr(first);
            if (key == "Status")
            {
                int code = std::atoi(value.substr(0, 3).c_str());
                _response.setStatusCode(code);
            }
            else if (key == "Content-Type")
                _response.setHeader("Content-Type", value);
            else if (key == "Location")
            {
                _response.setStatusCode(302);
                _response.setHeader("Location", value);
            }
            else
                _response.setHeader(key, value);
        }
    }
}