/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   web_server.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 23:17:33 by pmorello          #+#    #+#             */
/*   Updated: 2026/05/13 23:19:54 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEB_SERVER_HPP
#define WEB_SERVER_HPP

#include "../inc/request/HTTPRequest.hpp"
#include "../inc/request/LocationConfig.hpp"
#include "../inc/parser/ServerConfig.hpp"
#include "../inc/server/Server.hpp"
#include "../inc/response/HTTPResponse.hpp"
#include "../inc/mime/MimeTypes.hpp"
#include "../inc/utils/BuilderUtils.hpp"
#include "../inc/CGI/CGIHandler.hpp"
#include "../inc/server/Client.hpp"
#include "../inc/utils/CGIUtils.hpp"
#include "../inc/utils/Debug.hpp"
#include "../inc/utils/HTTPStatus.hpp"
#include "../inc/utils/Utils.hpp"

#endif