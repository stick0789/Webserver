/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BuilderUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 19:01:24 by pmorello          #+#    #+#             */
/*   Updated: 2026/05/13 19:03:06 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/utils/BuilderUtils.hpp"

void LocationMatchRequest(const std::string &reqPath, const std::vector<LocationConfig> &locsPath, std::string &urlMatch)
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

int  validMethods(const std::string &method, const LocationConfig *ptr)
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

bool isDirectory(const std::string &path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == 0)
        return (S_ISDIR(s.st_mode));
    return (false);
    
}

bool fileExist(std::string &path)
{
    struct stat s;
    return (stat(path.c_str(), &s) == 0);
}

std::string removeBoundary(const std::string &body, std::string &bdary)
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
