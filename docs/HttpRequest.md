# HTTP Request Parser – Rules

This describes the syntax and validation rules applied when receiving a HTTP request.

## TODO

- Implement BODY_CHUNKED and add it to the docs
- `_client_server_config` must be set before reaching `BODY_CONTENT_LEN`/`BODY_CHUNKED`, otherwise null-pointer access
- bug: whitespaces only nach dem :, leert value auf "" danach zugriff mit size_t -1 geht ins negative, HttpParser.cpp:117-120
- transfer encoding wird case sensitiv verglichen, wenn val "Chunked" wird nicht erkannt, nur key wird lowercased
- header laenge begrenzen sonst kann der client unendlich viele daten senden solange keine leerzeile kommt

## 1. States

The parser works incrementally over `_raw_buffer` (filled via `feedBuffer`) and goes through the following states (`ParseState`):

```
REQUEST_LINE → HEADERS → (BODY_CHUNKED | BODY_CONTENT_LEN | -) → DONE
```

- As long as a complete line (terminated by `\r\n`) is missing, the status stays `INCOMPLETE`
  and the parser waits for more data.
- Return values (ParseStatus): `INCOMPLETE`, `HEADER_COMPLETE`, `COMPLETE`, `ERROR_400`, `ERROR_413`.

## 2. Request line

- Must match exactly the following pattern:
  `^(GET|POST|DELETE) [\x21-\x7E]{1,2048} HTTP/1\.1$`
  - Only the methods `GET`, `POST`, `DELETE` are allowed.
  - The path may contain 1–2048 printable ASCII characters (no whitespace).
  - Only `HTTP/1.1` is accepted.
- If not matched → `ERROR_400`.
- The path is split at the first `?` into `_path` and `_query` (the query string is
  not further validated/decoded).

## 3. Headers

- Read line by line until the empty line (`\r\n\r\n`).
- A header line without `:` or without a value after the `:` is skipped (no error).
- The header key is lowercased; the header value is trimmed of leading/trailing spaces.
- Duplicate headers overwrite the previous value ("last wins").
- Special validation:
  - `host`: must not contain characters from `FORBIDDEN_HOST_CHARS` (`*?{}();\n\t#"' \/`) → otherwise `ERROR_400`.
  - `content-length`: must be a plain, non-negative integer (no `+`/`-` sign,
    no non-digit characters, no overflow) → otherwise `ERROR_400`.
- After headers are complete (empty line):
  - If `transfer-encoding: chunked` is set → state `BODY_CHUNKED` (takes precedence over content-length).
  - If `content-length` is set → state `BODY_CONTENT_LEN`
    - If the length is `0` (or less) → directly `DONE` (no body).
  - Otherwise → directly `DONE` (no body expected).

## 4. Body – `Content-Length`

- As long as `_raw_buffer` is shorter than `_content_len_expected` → `INCOMPLETE` (wait for more data).
- If `_content_len_expected` is greater than `_client_server_config->_client_max_body_size`
  → `ERROR_413` (Payload Too Large).
- Otherwise exactly `_content_len_expected` bytes are taken as `_body`, the rest
  stays in `_raw_buffer`.

## 5. Body – `Transfer-Encoding: chunked`

- Chunked body is parsed incrementally from _raw_buffer
- Accepted format only :
- HexSize\r\n Body_Bytes\r\n Hez_size\r\n Body_bytes \r\n 0 \r\n\r\n

  5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n

- Chunked Body arrives always in this format:

  - hexValue specfic states how many bytes the next text block has until next \r\n comes (Cariage return + new line)
  - each text block ends with a \r\n
  - tha http request only ends with a 0\r\n\r\n . ONLY this is valid.
    Example:
  - hexNbr\r\n TEXTBODY \r\nhexNbr\r\nMORE TEXT\r\n0\r\n\r\n

#### Parsing Rules

Each chunk starts with a hexadecimal size line ending in \r\n.

  - The hex size must contain only hex digits (0-9, a-f, A-F).

  - Chunk extensions are not accepted.

    - Invalid: 5;abc=1\r\nHello\r\n

 After the size line, exactly that many bytes are copied into _request._body.

- Every chunk body must be followed by \r\n.
- The request body ends only with the final chunk: 0\r\n\r\n
- Trailer headers after the final chunk are not accepted.
- If a full chunk has not arrived yet, parser returns INCOMPLETE.
- If the chunk syntax is wrong, parser returns ERROR_400.
- If the built body becomes larger than _client_server_config->_client_max_body_size, parser returns ERROR_413.

#### Priority: chunked or Content-Length

Transfer-Encoding: chunked takes priority over Content-Length. If both headers exist, the parser enters BODY_CHUNKED.

## 6. Final validation (`validateRequest`)

- Reached as soon as the state is `DONE`.
- Mandatory: the `host` header must be present → otherwise `ERROR_400`.
- Otherwise `COMPLETE`.

## 7. Other

- `reset()` fully resets the parser, so the same `HttpParser` instance can be reused for the
  next request on the same connection (keep-alive connections).
- `setServerConfig()` must have been called before reaching `BODY_CONTENT_LEN`/`BODY_CHUNKED`,
  since the pointer is directly dereferenced there (`_client_server_config->_client_max_body_size`).
