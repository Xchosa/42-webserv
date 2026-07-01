# Dispatcher – Regeln

Hier sind die Routing- und Verarbeitungsregeln des Dispatchers beschrieben.

## TODO
- handleDelete
- autoindex listet nur Verzeichnisse, keine Dateien

## 1. Überblick

`dispatch()` ist der zentrale Einstiegspunkt. Er bekommt einen fertig geparsten `HttpRequest`
und den passenden `ServerConfig*` und gibt entweder `DP_DONE` (Response fertig) oder
`DP_CGI_PENDING` (CGI-Prozess läuft noch) zurück.

## 2. Location-Matching (`findLocation`)

- Durchsucht alle Locations in `ServerConfig._locations` nach dem längsten Präfix-Match.
- Kein Treffer → `404`.

## 3. Dispatch-Reihenfolge

| Reihenfolge | Bedingung | Handler |
|---|---|---|
| 1 | Location hat `_redirect_code` | `handleRedirect` |
| 2 | Methode nicht in `_methods` | `405` |
| 3 | Methode ist `DELETE` | `handleDelete` |
| 4 | `_cgi_map` nicht leer → Erweiterung gefunden | `handleCgi` |
| 4a | CGI greift nicht (kein Match), POST + `_upload_store` | `handleUpload` |
| 4b | CGI greift nicht (kein Match), GET | `handleStatic` |
| 5 | POST + `_upload_store` (keine CGI-Map) | `handleUpload` |
| 6 | GET (keine CGI-Map, kein Upload) | `handleStatic` |
| 7 | Alles andere | `405` |

## 4. `handleRedirect`

- `3xx` → `Location`-Header mit `_redirect_url`, `Content-Length: 0`, kein Body.
- `2xx`/`4xx`/`5xx` mit optionaler `_redirect_url` → Wert als Plaintext-Body.
- Ohne `_redirect_url` → Default-Error-HTML als Body.

## 5. `handleStatic`

### Pfadauflösung

- Root beginnt mit '/'` → absolut zum Dateisystem, ansonsten relativ zur executable

### Datei-Logik

| Ergebnis von `stat()` | Verhalten |
|---|---|
| Fehler | `404` |
| Reguläre Datei | Inhalt lesen, MIME-Typ per Dateiendung |
| Verzeichnis mit `_index` | Index lesen |
| Verzeichnis + Index schlägt fehl + `_autoindex` | Autoindex-HTML |
| Verzeichnis + kein Index + kein `_autoindex` | `403` |
| Verzeichnis + kein Index + `_autoindex` | Autoindex-HTML |
| Sonstiger Dateityp | `502` |

### Autoindex

- Erzeugt ein HTML-Verzeichnis-Listing mit klickbaren Links.

## 6. `handleUpload`

- Erfordert `_upload_store` in `LocationConfig`.
- Upload-Basis: `getFullRootPath(lc) + "/" + upload_store`.
- Upload-Zielpfad: Upload-Basis + `request._path`.
- Fehlende Verzeichnisse werden automatisch angelegt.
- Datei wird binär geschrieben
- Antwort:
  - Datei existierte bereits → `200 OK`
  - Neue Datei → `201 Created`

## 7. `handleDelete`

- TODO @PAUL

## 8. `handleCgi`

### Extension-Erkennung

- Kein `.` im Request-Pfad → als statische Seite behandeln
- Letzte Erweiterung pruefen gegen `_cgi_map`.
- Kein Eintrag in `_cgi_map` → als statische Seite behandeln.
- `PATH_INFO`: Wenn nach der Erweiterung noch ein `/...` folgt, wird dieser Teil als
  `PATH_INFO`-Umgebungsvariable gesetzt

### Prozess-Start

- `fork()` + zwei Pipes:
  - `in_pipe`: Parent schreibt Request-Body → CGI `stdin`.
  - `out_pipe`: CGI `stdout` → Parent liest Response.
- Child: `dup2` für stdin/stdout, alle übrigen Pipe-Enden schließen,
  `chdir` in Location-Root, dann `execve`.
- Parent: nicht benötigte Pipe-Enden schließen; `out_pipe[0]` immer non-blocking.
  - Kein Request-Body → `in_pipe[1]` sofort schließen (`stdin_fd = -1`).
  - Mit Body → `in_pipe[1]` non-blocking, Body in `CgiSession._body` zwischenspeichern.

### CGI-Umgebungsvariablen

| Variable | Quelle |
|---|---|
| `GATEWAY_INTERFACE` | `CGI/1.1` (fix) |
| `SERVER_PROTOCOL` | `request._version` |
| `REQUEST_METHOD` | `request._method` |
| `SCRIPT_NAME` | Request-Pfad (ohne PATH_INFO) |
| `SCRIPT_FILENAME` | Absoluter Dateipfad des Scripts |
| `SERVER_PORT` | `sc->_listen_port` |
| `QUERY_STRING` | `request._query` |
| `SERVER_NAME` | `sc->_listen_host` (nur wenn gesetzt) |
| `CONTENT_LENGTH` | Länge von `request._body` (nur wenn vorhanden) |
| `CONTENT_TYPE` | `content-type`-Header (nur wenn vorhanden) |
| `REDIRECT_STATUS=200` | Nur bei PHP (Erweiterung `.php`) |
| `HTTP_<HEADERNAME>` | Alle Request-Header (Key uppercased, `-` → `_`) |

### CGI-Output-Parsing

- Akzeptiert sowohl `\r\n\r\n` als auch `\n\n` als Trennzeichen zwischen Headers und Body.
- `Status: NNN` → setzt HTTP-Statuscode der Response; alles andere → Response-Header.
- Kein gültiger Trennzeichen gefunden → `502`.
- Fehlender `Status`-Header → Standardmäßig `200 OK`.
- Default `Content-Type: application/octet-stream`, wird durch CGI-eigenen `Content-Type`-Header überschrieben.

## 9. Fehlerbehandlung (`buildErrorResponse`)

1. `ServerConfig._error_pages[code]` vorhanden → versuche zu lesen
2. Schlägt fehl → `default_pages/<code>.html` im `cwd` lesen.
3. Schlägt auch fehl → minimales Inline-HTML mit Statuscode und -text.

Response enthält immer `Content-Type: text/html` und korrekten `Content-Length`.
