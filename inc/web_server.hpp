/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   web_server.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 11:35:08 by marvin            #+#    #+#             */
/*   Updated: 2026/05/07 11:35:08 by marvin           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef web_server.hpp
#define web_server.hpp


#include "../inc/CGI/CGIHandler.hpp"
#include "../inc/mime/MimeTypes.hpp"
#include "../inc/parser/ConfigParser.hpp"
#include "../inc/parser/RequestParser.hpp"
#include "../inc/parser/ServerConfig.hpp"
#include "../inc/parser/ConfigParser.tpp"
#include "../inc/request/HTTPRequest.hpp"
#include "../inc/request/LocationConfig.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include "../inc/response/ResponseBuilder.hpp"
#include "../inc/server/Server.hpp"
#include "../inc/utils/Debug.hpp"
#include "../inc/utils/HTTPStatus.hpp"
#include "../inc/utils/Utils.hpp"

#endif