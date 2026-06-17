# Server Runtime Behavior

The server uses `epoll` with non-blocking sockets.

- `MAXEVENTS` limits how many events are handled per `epoll_wait`.
- `IDLE_TIME` is used as the `epoll_wait` timeout.
- If no events occur, the server checks clients for timeout.
- Clients idle longer than `KEEP_ALIVE_TIMEOUT` are closed.

# Listening Rules 

Server creates one listening socket per unique host:port

`0.0.0.0:PORT` conflicts with specific hosts on the same port.

# Virtual Hosts

Virtual hosts allow multiple `server` configs to share the same listening socket.

For every unique host:port a listening Server socket gets created 

Example:

    server A: listen 127.0.0.1:8081, server_name site-a
 	server B: listen 127.0.0.1:8081, server_name site-b

Both configs use the same host:port, so the server creates only one listening socekt for 127.0.0.1:8081

Internally, all server configs with the same normalized host:port are stored as candiates for that listening socket. 

When a HTTP request arrives the server reads the HTTP Host header and compares it against each candiate server.s name 

Host: site b 

If Host value matches configured server_name, that server config is selected. 

if no match is found, the default server for that listen socket is used. The default server, is the first of the 'Candidates'

so multiple 127.0.0.1 hosts on the same port create one Server Socket same as 0.0.0.0. and  are grouped together as virtual host candidates.

## Listen Host Rules

empty listen host is normalized to 0.0.0.0 => wildcard bind , it accepts connections on any local interface 

the server does not allow mixing wildcard and specific hosts e.g.localhost/127.0.0.1  the same port

Invalid:

 0.0.0.0:8081
 127.0.0.1:8081

Valid:

  127.0.0.1:8081
  127.0.0.1:8081

  Valid:

  0.0.0.0:8081
  0.0.0.0:8081

# ConfigServer Constants

  This document explains the main server configuration constants used by the webserver.

## `MAXEVENTS`

const MAXEVENTS = 64

Maximum number of events the server asks the  event loop to return at once (from epoll instance)

limits the ready File Descriptors

Time out Values

all Values are in Seconds

## IDLE_TIME

IDLE_TIME = 5 

used as the epoll_wait timeout 

Max Time a connection can stay idle before server considers it inactive.

Every 5 seconds, epoll wait returns 0 if no request arrived (ready Filedescriptors from Kernel)

, and every 5 seconds the loop checks if active Client connections are idle for too long.

Every 5 seconds the loop for inactive connected clients gets checked

## KEEP_ALIVE_TIMEOUT

KEEP_ALIVE_TIMEOUT = 3;

Maximum time the server keeps a Http keep-alive Connection open, while waiting for the next request.

Every Client has a "last Activity Timestemp", if this is above 3 seconds, the connection gets closed.

   Constant              Value    Unit       Purpose

  ━━━━━━━━━━━━━━━━━━━━  ━━━━━━━  ━━━━━━━━━  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

   MAXEVENTS                64    events     Maximum events handled per event loop cycle

  ────────────────────  ───────  ─────────  ─────────────────────────────────────────────

   IDLE_TIME                 5    seconds    Idle connection timeout

  ────────────────────  ───────  ─────────  ─────────────────────────────────────────────

   KEEP_ALIVE_TIMEOUT        3    seconds    Keep-alive wait timeout
