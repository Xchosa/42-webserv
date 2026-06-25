# Dispatcher – Rules

This describes how the Dispatcher works.

## TODO

- ...

One function die Pfade auflost 

/maus/a/a/a/a/a/test.txt



root            ./danceserv;

    index           index.html;

    methods         GET POST;

    upload_store    ./uploads;




POST /maus/hund/futter.txt

root: ./danceserv

upload: uploads

pfad: maus/hund/futter.txt

--> ./danceserv/uploads/maus/hund/futter.txt


funktion die pfade aufloest mit lexically_nomal()

(optional upload pfad +) request pfad an den root pfad

./danceserv/uploads/maus/hund/futter.txt


=> pruefung noch in location maus 


POST /maus/../vogel/uploads/pwd.txt

root: ./danceserv

upload: uploads

pfad: maus/hund/futter.txt

--> ./danceserv/uploads/maus/hund/futter.txt



Path traversal: reject request paths containing .. as a path segment.
      - Use 403 Forbidden, not "404 Forbidden".
      - 404 = Not Found.
      - 403 = Forbidden.

// reject traversal

Encoded traversal: after URL decoding, reject %2e%2e, %2E%2E, etc.?? ../ /../

detect this and throw "404 Forbidden"?

where should a absolut path be saved? -> injection, it should only be possible in the root eg. ./danceserv

/danceserv should not be possible? or just handle it as relativ?

printf 'POST /maus/%2e%2e/test2.txt HTTP/1.1\r\nHost: maus\r\nContent-Type: text/plain\r\nContent-Length:11\r\nConnection: close\r\n\r\nhello World' | nc localhost 8081

    - Allow: danceserv, ./danceserv, uploads, ./uploads

    - Reject: /danceserv, /uploads, ../uploads

### Upload Files

POST

#### upload target rule:

* root danceserv;
* upload_store uploads
* POST /maus/test2.txt
* saves to danceserv/uploads/test2.txt

#### Overwrite behavior:

POST request is not a idempotent request like PUT

the server creates a file and subdirectorys for e.g. POST /maus/test2.txt HTTP/1.1\r\nHost: maus\r\nContent-Type: text/plain\r\nContent-Length:11\r\nConnection: close\r\n\r\nhello World

- new file -> 201 Created
- existing file overwritten -> 200 OK

and gives back:  HTTP/ 1.1 201 Created

calling the same request again or changing the content of the text2.txt to e.g. hello Worp would overwrite the file

and gives back: HTTP/ 1.1 200 Ok

binary files are accepted e.g. POST /maus/upload/file.bin HTTP/1.1\r\nHost: maus\r\nContent-Type: application/octet-

  stream\r\nContent-Length: 6\r\nConnection: close\r\n\r\n\x00\xffAB\x0a\x00'

xxd -p danceserv/uploads/file.bin should print in hex: 00ff41420a00

#### Upload Safety Rules

Reject path traversal in request paths:

  /maus/../test2.txt

  /maus/%2e%2e/test2.txt

  Return: 403 Forbidden

  Config paths must stay inside the project-controlled root.

  Allowed:

  root danceserv;

  root ./danceserv;

  upload_store uploads;

  upload_store ./uploads;

  Rejected:

  root /danceserv;

  root ../danceserv;

  upload_store /uploads;

  upload_store ../uploads;

Config safety: decide that root and upload_store must be relative.
