# Dispatcher – Rules

This describes how the Dispatcher works.

## TODO

- ...



// do we allow traveling?? ../ /../ 

detect this and throw "404 Forbidden"? 

where should a absolut path be saved? -> injection, it should only be possible in the root eg. ./danceserv

/danceserv should not be possible? or just handle it as relativ? 


and traversal request encoded 

printf 'POST /maus/%2e%2e/test2.txt HTTP/1.1\r\nHost: maus\r\nContent-Type: text/plain\r\nContent-Length:11\r\nConnection: close\r\n\r\nhello World' | nc localhost 8081


### Upload Files

POST 

POST request is not a idempotent request like PUT

the server creates a file and subdirectorys for e.g. POST /maus/test2.txt HTTP/1.1\r\nHost: maus\r\nContent-Type: text/plain\r\nContent-Length:11\r\nConnection: close\r\n\r\nhello World

and gives back:  HTTP/ 1.1 Created 201

calling the same request again or changing the content of the text2.txt to e.g. hello Worp would overwrite the file

and gives back: HTTP/ 1.1 OK 200


binary files are accepted e.g. POST /maus/upload/file.bin HTTP/1.1\r\nHost: maus\r\nContent-Type: application/octet-

  stream\r\nContent-Length: 6\r\nConnection: close\r\n\r\n\x00\xffAB\x0a\x00'

xxd -p danceserv/uploads/file.bin should print in hex: 00ff41420a00
