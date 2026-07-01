# Dispatcher – Rules

This describes the routing and processing rules of the Dispatcher.

## TODO
- handleDelete
- autoindex only lists directories, not files

## 1. Overview

`dispatch()` is the central entry point. It receives a fully parsed `HttpRequest`
and the matching `ServerConfig*` and returns either `DP_DONE` (response ready) or
`DP_CGI_PENDING` (CGI process still running).

## 2. Location matching (`findLocation`)

- Searches all locations in `ServerConfig._locations` for the longest prefix match.
- No match → `404`.

## 3. Dispatch order

| Order | Condition | Handler |
|---|---|---|
| 1 | Location has `_redirect_code` | `handleRedirect` |
| 2 | Method not in `_methods` | `405` |
| 3 | Method is `DELETE` | `handleDelete` |
| 4 | `_cgi_map` not empty → extension matched | `handleCgi` |
| 4a | CGI no match, POST + `_upload_store` | `handleUpload` |
| 4b | CGI no match, GET | `handleStatic` |
| 5 | POST + `_upload_store` (no CGI map) | `handleUpload` |
| 6 | GET (no CGI map, no upload) | `handleStatic` |
| 7 | Anything else | `405` |

## 4. `handleRedirect`

- `3xx` → `Location` header with `_redirect_url`, `Content-Length: 0`, no body.
- `2xx`/`4xx`/`5xx` with optional `_redirect_url` → value as plaintext body.
- No `_redirect_url` → default error HTML as body.

## 5. `handleStatic`

### Path resolution

- Root starts with `/` → absolute filesystem path, otherwise relative to the executable.

### File logic

| `stat()` result | Behaviour |
|---|---|
| Error | `404` |
| Regular file | Read content, Content-Type by file extension |
| Directory with `_index` | Read index file |
| Directory + index fails + `_autoindex` | Autoindex HTML |
| Directory + no index + no `_autoindex` | `403` |
| Directory + no index + `_autoindex` | Autoindex HTML |
| Other file type | `502` |

### Autoindex

- Generates an HTML directory listing with clickable links.

## 6. `handleUpload`

- Requires `_upload_store` in `LocationConfig`.
- Upload base: `getFullRootPath(lc) + "/" + upload_store`.
- Upload target path: upload base + `request._path`.
- Missing directories are created automatically.
- File is written in binary mode.
- Response:
  - File already existed → `200 OK`
  - New file → `201 Created`

## 7. `handleDelete`

- TODO @PAUL

## 8. `handleCgi`

### Extension detection

- No `.` in request path → handle as static page.
- Check last extension against `_cgi_map`.
- No entry in `_cgi_map` → handle as static page.
- `PATH_INFO`: if a `/...` follows after the extension, that part is set as the
  `PATH_INFO` environment variable.

### Process start

- `fork()` + two pipes:
  - `in_pipe`: parent writes request body → CGI `stdin`.
  - `out_pipe`: CGI `stdout` → parent reads response.
- Child: `dup2` for stdin/stdout, close all remaining pipe ends,
  `chdir` into location root, then `execve`.
- Parent: close unused pipe ends; `out_pipe[0]` always non-blocking.
  - No request body → close `in_pipe[1]` immediately (`stdin_fd = -1`).
  - With body → `in_pipe[1]` non-blocking, body stored in `CgiSession._body`.

### CGI environment variables

| Variable | Source |
|---|---|
| `GATEWAY_INTERFACE` | `CGI/1.1` (fixed) |
| `SERVER_PROTOCOL` | `request._version` |
| `REQUEST_METHOD` | `request._method` |
| `SCRIPT_NAME` | Request path (without PATH_INFO) |
| `SCRIPT_FILENAME` | Absolute file path of the script |
| `SERVER_PORT` | `sc->_listen_port` |
| `QUERY_STRING` | `request._query` |
| `SERVER_NAME` | `sc->_listen_host` (only if set) |
| `CONTENT_LENGTH` | Length of `request._body` (only if present) |
| `CONTENT_TYPE` | `content-type` header (only if present) |
| `REDIRECT_STATUS=200` | PHP only (extension `.php`) |
| `HTTP_<HEADERNAME>` | All request headers (key uppercased, `-` → `_`) |

### CGI output parsing

- Accepts both `\r\n\r\n` and `\n\n` as separator between headers and body.
- `Status: NNN` → sets HTTP status code of the response; everything else → response headers.
- No valid separator found → `502`.
- Missing `Status` header → defaults to `200 OK`.
- Default `Content-Type: application/octet-stream`, overridden by CGI-provided `Content-Type` header.

## 9. Error handling (`buildErrorResponse`)

1. `ServerConfig._error_pages[code]` present → attempt to read it.
2. Fails → read `default_pages/<code>.html` from `cwd`.
3. Also fails → minimal inline HTML with status code and text.

Response always contains `Content-Type: text/html` and a correct `Content-Length`.
