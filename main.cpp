/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pmorello <pmorello@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/14 13:16:48 by pmorello          #+#    #+#             */
/*   Updated: 2026/04/14 13:18:02 by pmorello         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "web_server.hpp"

int main()
{
    // 1. Crear un vector de configuracions de Location
    std::vector<LocationConfig> mock_locations;

    // 2. Crear i configurar una instància de LocationConfig
    LocationConfig test_loc;
    
    // Aquí configures el que el teu CGIHandler necessiti (exemples):
    // test_loc.setCgiPass("/usr/bin/python3");
    // test_loc.setCgiExtension(".py");
    // test_loc.setRoute("/cgi-bin/");

    // Afegir-la al vector
    mock_locations.push_back(test_loc);

    // 3. Crear les altres dependències (Request i Response)
    HTTPRequest  req;
    HTTPResponse res;
    std::string  body = "dades de prova";

    // 4. Instanciar el Handler
    CGIHandler cgi(req, res, body);

    // 5. Cridar al mètode passant l'iterador al primer element del vector
    // mock_locations.begin() retorna precisament el tipus: 
    // std::vector<LocationConfig>::iterator
    cgi.reqToMap(mock_locations.begin());

    // 6. Executar per provar
    cgi.execute();

    return 0;
}