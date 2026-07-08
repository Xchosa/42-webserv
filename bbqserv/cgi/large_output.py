#!/usr/bin/env python3
# CGI that produces large output
data = "X" * 100000

body = f"<html><body><pre>{data}</pre></body></html>"
print("Content-Type: text/html")
print(f"Content-Length: {len(body)}")
print()
print(body, end="")
