# Plan de Acción - David (Persona 2: HTTP Protocol)

## Fase 0: Contratos `.hpp` (con el equipo)

Definir las interfaces compartidas **antes de escribir código**:
- `HTTPRequest.hpp` — contenedor de datos (method, path, queryString, headers, body, errorCode)
- `HTTPResponse.hpp` — serialización de respuestas
- `RequestParser.hpp` — máquina de estados incremental con `feed()`, `isComplete()`, `getRequest()`, `reset()`

**Decisión clave:** Raúl debe llamar a tu `parser.feed(buffer)` en cada lectura de socket. Solo tú sabes cuándo un request está completo (necesitas Content-Length o detectar chunked). No dejes que Raúl intente detectar la completitud por su cuenta.

---

## Fase 1: Tablas de lookup (sin dependencias)

1. **HTTPStatus** — mapeo código → reason phrase (200 OK, 404 Not Found, etc.)
2. **MimeTypes** — extensión → Content-Type (.html → text/html, .css → text/css, etc.)
3. **Páginas de error por defecto** — HTML mínimo generado para cada error code

---

## Fase 2: HTTPResponse (antes del parser)

- `serialize()` → produce `HTTP/1.1 200 OK\r\nHeaders\r\n\r\nbody`
- Siempre setear `Content-Length`, `Date`, `Server`, `Connection`
- Factory methods: `buildErrorResponse(code)`, `buildRedirectResponse(code, location)`
- Testearlo con datos mock inmediatamente

---

## Fase 3: Parser de Request Line + Headers (el grueso del trabajo)

### 3.1 — Máquina de estados
- Estados: `PARSING_REQUEST_LINE → PARSING_HEADERS → PARSING_BODY → COMPLETE | ERROR`
- `feed()` acumula en buffer interno, procesa líneas completas (`\r\n`)
- Cuidado: `\r` puede llegar en un feed y `\n` en el siguiente — trabajar siempre sobre el buffer acumulado

### 3.2 — Request line
- Parsear `METHOD SP URI SP HTTP/1.1\r\n`
- Separar URI en path + query string (al primer `?`)
- **Percent-decode** el path (`%20` → espacio). Implementar `percentDecode()`. Rechazar `%00`
- **Normalizar path**: resolver `.` y `..` para evitar directory traversal. Esto es **crítico de seguridad**

### 3.3 — Headers
- Format: `field-name: field-value\r\n`, normalizar nombres a **lowercase**
- Trim whitespace en valores
- Validaciones obligatorias:
  - `Host` obligatorio en HTTP/1.1 (400 si falta)
  - `Content-Length` debe ser entero no negativo
  - Si hay `Transfer-Encoding: chunked` + `Content-Length`, chunked tiene prioridad
  - Rechazar obsolete line folding (líneas que empiezan con espacio/tab) → 400

---

## Fase 4: Parsing del Body (la parte más difícil)

### 4.1 — Body de longitud fija
- Leer exactamente `Content-Length` bytes del buffer
- Verificar contra `client_max_body_size` ANTES de leer → 413 si excede

### 4.2 — Chunked transfer encoding
```
TAMAÑO_HEX\r\n
DATA\r\n
...
0\r\n
\r\n
```
- Sub-máquina de estados: `READING_CHUNK_SIZE → READING_CHUNK_DATA → READING_TRAILER`
- **Deschunkear completamente** — el CGI espera EOF, no chunked encoding
- Verificar tamaño acumulado contra `max_body_size` en cada chunk
- **Edge cases críticos:** chunk size split entre feeds, `\r\n` tras datos en feed separado, hex inválido, overflow

### 4.3 — Sin body
- GET/DELETE sin Content-Length ni Transfer-Encoding → COMPLETE directamente
- POST sin Content-Length ni chunked → 411 Length Required

---

## Fase 5: Puntos de integración

- **Keep-alive:** `shouldKeepAlive()` según `Connection` header y versión HTTP
- **`reset()`** para reutilizar parser en conexiones persistentes (preservar bytes sobrantes del buffer para pipelining)
- **`getState()`** para que Raúl sepa si hay un request parcial (timeout → 408)
- **`validate(maxBodySize)`** — validación final post-parseo

---

## Fase 6: Robustez (nunca crashear)

**Límites obligatorios:**
- Request line: max 8192 bytes
- Header individual: max 8192 bytes
- Total headers: max 32KB, max 100 headers
- Body: según `client_max_body_size` (default 1MB)

**Pitfalls C++98:**
- No existe `std::stoi`/`std::to_string` → usar `std::strtol` y `std::ostringstream`
- Todo `find()` → comprobar `npos` antes de usar el resultado
- Percent-decode ANTES de normalizar path (si no, `%2e%2e` bypasea tu check de `..`)
- Bytes sobrantes tras chunked `0\r\n\r\n` → pertenecen al siguiente request

---

## Orden de implementación resumido

```
Día 1     → Fase 0: .hpp con el equipo
Día 2     → Fase 1: StatusCodes + MimeTypes + ErrorPages
Día 2-3   → Fase 2: HTTPResponse + serialize()
Día 3-5   → Fase 3: RequestParser (request line + headers)
Día 5-7   → Fase 4: Body parsing (fixed + chunked)
Día 7-8   → Fase 5: Keep-alive, reset, validate
Día 8-9   → Fase 6: Tests de robustez con inputs malformados
Día 10+   → Integración con Raúl y Pau
```

---

## Ficheros a crear

```
includes/HTTPRequest.hpp
includes/HTTPResponse.hpp
includes/RequestParser.hpp
includes/HTTPStatus.hpp
includes/MimeTypes.hpp
srcs/http/HTTPRequest.cpp
srcs/http/HTTPResponse.cpp
srcs/http/RequestParser.cpp
srcs/http/HTTPStatus.cpp
srcs/http/MimeTypes.cpp
tests/test_parser.cpp       (main aislado para probar parsing)
tests/test_response.cpp     (main aislado para probar responses)
```

---

## Datos del HTTPRequest (miembros)

- `std::string method` — "GET", "POST", "DELETE"
- `std::string uri` — URI raw tal como llega
- `std::string path` — path decodificado y normalizado
- `std::string queryString` — todo tras `?`, raw
- `std::string httpVersion` — "HTTP/1.1" o "HTTP/1.0"
- `std::map<std::string, std::string> headers` — nombres en lowercase
- `std::string body` — body decodificado (unchunked si era chunked)
- `int errorCode` — 0 si válido, sino el código HTTP de error
- `bool isComplete` — señal de que el parseo terminó

---

## Datos del HTTPResponse (miembros)

- `int statusCode`
- `std::string statusMessage`
- `std::map<std::string, std::string> headers`
- `std::string body`
- `std::string serialize()` — produce la respuesta HTTP completa como string

---

## Estados del RequestParser

```cpp
enum ParseState {
    PARSING_REQUEST_LINE,
    PARSING_HEADERS,
    PARSING_BODY,
    COMPLETE,
    ERROR
};
```

Miembros internos:
- `std::string _buffer` — datos acumulados
- `HTTPRequest _request` — request en construcción
- `size_t _contentLength`
- `bool _isChunked`
- `size_t _maxBodySize` — desde config

Métodos públicos:
- `void feed(const std::string& data)`
- `bool isComplete() const`
- `HTTPRequest getRequest() const`
- `void reset()`
- `ParseState getState() const`

---

## Status codes a soportar

| Código | Reason Phrase | Uso |
|--------|--------------|-----|
| 200 | OK | Respuesta exitosa |
| 201 | Created | POST/upload exitoso |
| 204 | No Content | DELETE exitoso |
| 301 | Moved Permanently | Redirección permanente |
| 302 | Found | Redirección temporal |
| 400 | Bad Request | Request malformado |
| 403 | Forbidden | Sin permisos |
| 404 | Not Found | Recurso no encontrado |
| 405 | Method Not Allowed | Método no permitido en ruta |
| 408 | Request Timeout | Timeout parcial |
| 411 | Length Required | POST sin Content-Length ni chunked |
| 413 | Content Too Large | Body excede max_body_size |
| 414 | URI Too Long | URI demasiado largo |
| 500 | Internal Server Error | Error interno |
| 501 | Not Implemented | Transfer-Encoding no soportado |
| 502 | Bad Gateway | CGI devuelve output inválido |
| 504 | Gateway Timeout | CGI tarda demasiado |
| 505 | HTTP Version Not Supported | Versión HTTP no soportada |

---

## MIME types mínimos

| Extensión | Content-Type |
|-----------|-------------|
| .html, .htm | text/html |
| .css | text/css |
| .js | application/javascript |
| .jpg, .jpeg | image/jpeg |
| .png | image/png |
| .gif | image/gif |
| .ico | image/x-icon |
| .svg | image/svg+xml |
| .txt | text/plain |
| .json | application/json |
| .xml | application/xml |
| .pdf | application/pdf |
| default | application/octet-stream |

---

## Pitfalls críticos

1. **`\r\n` split entre feeds** — trabajar siempre sobre buffer acumulado, nunca sobre chunks individuales
2. **`find()` retorna `npos`** — comprobar SIEMPRE antes de usar en `substr()`
3. **Percent-decode antes de normalizar path** — `%2e%2e` debe resolverse como `..` DESPUÉS de decodificar
4. **Bytes sobrantes tras chunked** — pertenecen al siguiente request (pipelining)
5. **C++98** — no hay `std::stoi`, `std::to_string`, initializer lists. Usar `strtol`, `ostringstream`
6. **Case sensitivity** — header NAMES lowercase, header VALUES sin tocar (excepto `Connection` que es case-insensitive)
7. **Host con puerto** — `Host: localhost:8080` es válido, no rechazar
8. **Buffer invalidation** — tras `erase()` recalcular todo desde el inicio del buffer
