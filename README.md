# Webserver

*This project has been created as part of the 42 curriculum by rmanzana, pmorello, jaacosta.*

Description
-----------

This is a small C++ webserver made by three 42 students as a learning project. The goal was to implement a basic HTTP server that can serve static files, run simple CGI scripts (PHP/Python), and handle uploads for specific locations.

Instructions
------------

Compilation

```bash
make
# For a debug build with symbols and `DEBUG` enabled:
make debug
```

Run

```bash
# Example: run with the CGI test config
./webserver conf/CGItest.conf
# Press Ctrl+C to stop the server (or use `kill` if backgrounded).
```

Quick test (from the server machine)

```bash
curl -v -H "Host: cgitest.local:8080" http://127.0.0.1:8080/
```

Notes
- Config files are in the `conf/` folder. Set `host` to `0.0.0.0` to listen on all interfaces.
- Use the `www/` folder for site content (see provided examples: `CGItest`, `simpleHTML`).

Resources
---------

- HTTP basics: MDN Web Docs — HTTP Overview: https://developer.mozilla.org/en-US/docs/Web/HTTP/Overview
- CGI reference: CGI Programming/FAQ (w3.org archives) and PHP CGI docs: https://www.php.net/manual/en/install.fpm.php
- Sockets and POSIX: Beejs Guide to Network Programming, and Unix Network Programming by W. Richard Stevens
- Poll/select: Linux `poll(2)` manual and related tutorials

How AI was used
----------------

We used AI (mainly Gemini) to help with debugging and refactoring tasks during development.
AI was used for code suggestions, writing small patches, and proposing debugging steps; all final edits and merges were reviewed and applied by the team.

Additional notes
----------------

- This server uses C++98 and POSIX APIs (sockets, `poll`, `fork`/`execve` for CGI).
- It's single-threaded and uses a simple event loop — expect limitations under load.

Authors
-------

- rmanzana
- pmorello
- jaacosta

If you want, we can add build badges, contribution notes, or a short walkthrough of the main source files.
