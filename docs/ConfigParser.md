# Config Parser – Rules

This describes the syntax and validation rules applied when reading a `.conf` file.

## TODO
- Validate the config at the end (mandatory settings, etc.)
	
	- root/index in LocationConfig

## 1. Lexer (Tokenizer)

- Recognizes 5 token types: `WORD`, `SEMICOLON`, `LBRACE`, `RBRACE`, `END_OF_FILE`.
- Comments start with `#` and run until the end of the line.
- Whitespace (including newlines) is skipped.
- A `WORD` is any sequence of characters without whitespace and without the special characters `;`, `{`, `}`, `\n`.

## 2. Basic structure

- A config file consists of 0..n `server { ... }` blocks (directly one after another, no wrapper).
- Every server-setting line and every location-setting line must end with `;`.
- Inside a server block, every line is either:
  - `location <path> { ... }` (its own block, no `;`)
  - `<setting_key> <value...>;`
- If the parser encounters a token that doesn't fit this pattern (e.g. a missing second value), it throws an error.
- All settings follow the "last wins" principle.

## 3. Server settings (`pss`)

| Key | Rule |
|---|---|
| `listen [host:]port` | Optional `host:` is validated as a valid IPv4 via `inet_pton`. `port` must be a number between 1 and 65535. |
| `server_name <name> [name...]` | Any number of values until `;`. Each value must not contain characters from `FORBIDDEN_INDEX_CHARS` (`*?[]{}():;\n#"' \/`). |
| `root <path>` | Must not contain characters from `FORBIDDEN_PATH_CHARS` (`*?[]{}():;\n#"' \`). Initially only stored as `_tmp_root` (default for locations). |
| `index <name>` | Must not contain characters from `FORBIDDEN_INDEX_CHARS`. Stored as `_tmp_index` (default for locations). |
| `client_max_body_size <n><suffix>` | `0` = unlimited. Otherwise: number + suffix `k`/`m`/`g` (case-insensitive) as multiplier (1024 / 1024² / 1024³). Invalid suffix or no number → error. |
| `error_page <code> <path>` | `code` must be a number in `[300, 600)`. `path` must be at least 3 characters long and start with `/`. Stored in `_error_pages[code]`. |
| `is_default_server <bool>` | Accepts `true`/`1` or `false`/`0`. |

Unknown setting key in a server block → error.

## 4. Location block

- Syntax: `location <path> { ... }`, directly inside a server block.
- `<path>` must start with `/` and must not contain characters from `FORBIDDEN_PATH_CHARS`.
- Duplicate location path within the same server → `"Doubled location '...'"`.
- After the server block, missing `root`/`index` in all locations are filled in with the
  server defaults (`_tmp_root` / `_tmp_index`) if they are empty
  (`setLocationDefaultSettings`).

## 5. Location settings (`pls`)

| Key | Rule |
|---|---|
| `root <path>` | Same as server `root` (same `FORBIDDEN_PATH_CHARS` check), overrides the default for this location. |
| `index <name>` | Same as server `index` (same `FORBIDDEN_INDEX_CHARS` check). |
| `methods <m1> ...` | Only `GET`, `POST`, `DELETE` allowed; any number of values until `;`. |
| `autoindex <on\|off>` | Only `on` or `off`. |
| `return <code> [url/path]` | `code` is a number in `[200, 600)`. For `3xx` the URL/path is **mandatory** and must start with `/`, `http://` or `https://`; for other codes the second value is optional free text. |
| `upload_store <path>` | Must not contain characters from `FORBIDDEN_PATH_CHARS`. |
| `cgi_ext <.ext> <path>` | Extension: at least 2 characters, starts with `.`, followed only by alphanumeric characters. `path` must not contain `FORBIDDEN_PATH_CHARS`. Stored in `_cgi_map[ext]`. |

Unknown setting key in a location block → error.

## 6. Error format

All errors are thrown as `std::runtime_error` with the prefix `"[<config_file>:<line>] <message>"`.

## 7. Forbidden characters (constants)

```cpp
FORBIDDEN_PATH_CHARS  = "*?[]{}():;\n#\"' \\"     // for root, upload_store, cgi-path, location-path
FORBIDDEN_INDEX_CHARS = "*?[]{}():;\n#\"' \\/"    // for index, server_name (additionally forbids '/')
```
