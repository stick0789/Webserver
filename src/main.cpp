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


/* ************************************************************************** */
/*                                                                            */
/*   main.cpp  —  Prueba de integración: Server + RequestParser              */
/*                                                                            */
/*   Uso:                                                                     */
/*     ./webserv                        (usa conf/Testing.conf por defecto)   */
/*     ./webserv conf/Default.conf      (archivo de configuración custom)     */
/*                                                                            */
/*   Para probar desde otro terminal:                                         */
/*     curl -v http://127.0.0.1:8080/home/                                   */
/*     curl -v -X POST http://127.0.0.1:8080/upload/ -d "hola"               */
/*     curl -v http://127.0.0.1:8080/ruta/que/no/existe                      */
/*                                                                            */
/* ************************************************************************** */

#include "web_server.hpp"   // Global project include 

#include <iostream>
#include <csignal>

// Variable global para controlar el cierre limpio y evitar leaks en Valgrind
// (Asegúrate de que webserv.run() use un bucle tipo 'while(g_running)')
volatile bool g_running = true; 

void goodByeHandler(int sig)
{
    (void)sig;
    g_running = false;
    std::cout << std::endl << "\033[1;32m[webserv] Server stopped. Goodbye!\033[0m" << std::endl;
}

int main(int argc, char *argv[])
{
    // Signals 
    signal(SIGINT, goodByeHandler);
    signal(SIGTERM, goodByeHandler);
    signal(SIGPIPE, SIG_IGN); // Prevents crashes

    if(argc > 2)
    {
        std::cout << "⚠️Wrong usage!⚠️" << std::endl;
        std::cout << "executable should be executed as follows:" << std::endl;
        std::cout << "./webserv [configuration file]" << std::endl;
        return (1);
    }

    ConfigParser configs;
    bool success = false;

    // Config parse
    if(argc == 2)
        success = configs.parseConfigFile(argv[1]);
    else
        success = configs.parseConfigFile();

    // Security check
    if (success && configs.getParsedServerConfigs().empty())
    {
        std::cerr << "\033[1;31m[ERROR] No server blocks found in config.\033[0m" << std::endl;
        success = false;
    }

    #ifdef DEBUG
        if (success) {
            printParsedConfig(configs);
        }
    #endif

    if (success)
    {   //before running, I check if all configured files and folders actually exists
        if (!runConfigPreflight(configs.getParsedServerConfigs()))
        {
            std::cerr << "[ERROR] Preflight failed. Check config paths/permissions." << std::endl;
            return (1);
        }
        Server webserv(configs);
        success = webserv.run();
    }

    /*
        !success because true = 1, false = 0
        but return expects 0 for no error, anything else for error
    */
    return (!success);
}

/*
#include "web_server.hpp"   // Include global del proyecto

#include <iostream>
#include <csignal>

// ─── Signal handler para Ctrl+C limpio ──────────────────────────────────────

static volatile bool g_running = true;

static void signalHandler(int sig)
{
    (void)sig;
    g_running = false;
    std::cout << "\n\033[93m[INFO] Signal received. Shutting down...\033[0m" << std::endl;
}

// ─── main ────────────────────────────────────────────────────────────────────

int main(int argc, char **argv)
{
    // Capturar Ctrl+C para salir limpiamente
    signal(SIGINT,  signalHandler);
    signal(SIGTERM, signalHandler);
    // Ignorar SIGPIPE (evita crash si el cliente cierra la conexión mientras enviamos)
    signal(SIGPIPE, SIG_IGN);

    // ── 1. Seleccionar el archivo de configuración ──────────────────────────
    std::string configFile = "Testing.conf";
    if (argc == 2)
        configFile = argv[1];
    else if (argc > 2)
    {
        std::cerr << "\033[1;31m[ERROR] Usage: " << argv[0]
                  << " [config_file]\033[0m" << std::endl;
        return (1);
    }

    std::cout << "\033[96m[INFO] Loading config: " << configFile << "\033[0m" << std::endl;

    // ── 2. Parsear el archivo de configuración ──────────────────────────────
    ConfigParser config;
    if (!config.parseConfigFile(configFile))
    {
        std::cerr << "\033[1;31m[ERROR] Failed to parse config file: "
                  << configFile << "\033[0m" << std::endl;
        return (1);
    }
    if (config.getParsedServerConfigs().empty())
    {
        std::cerr << "\033[1;31m[ERROR] No server blocks found in config.\033[0m" << std::endl;
        return (1);
    }

    std::cout << "\033[92m[OK] Config parsed: "
              << config.getParsedServerConfigs().size()
              << " server(s) found.\033[0m" << std::endl;

    // ── 3. Crear el servidor y arrancarlo ───────────────────────────────────
    Server server(config);

    std::cout << "\033[96m[INFO] Starting server... (Ctrl+C to stop)\033[0m" << std::endl;

    if (!server.run())
    {
        std::cerr << "\033[1;31m[ERROR] Server exited with error.\033[0m" << std::endl;
        return (1);
    }

    std::cout << "\033[92m[INFO] Server stopped cleanly.\033[0m" << std::endl;
    return (0);
}*/
/*
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
*/