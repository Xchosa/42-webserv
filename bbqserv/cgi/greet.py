#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

method = os.environ.get("REQUEST_METHOD", "GET")

if method == "GET":
    query = os.environ.get("QUERY_STRING", "")
    params = parse_qs(query)
    name = params.get("name", ["Welt"])[0]
else:
    length = int(os.environ.get("CONTENT_LENGTH", "0"))
    body = sys.stdin.read(length)
    params = parse_qs(body)
    name = params.get("name", ["Welt"])[0]

body_html = f"<html><body><h1>Hallo {name}!</h1></body></html>"

print("Content-Type: text/html")
print(f"Content-Length: {len(body_html)}")
print()
print(body_html, end="")
