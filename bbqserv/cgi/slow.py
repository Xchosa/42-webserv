#!/usr/bin/env python3
import time

time.sleep(40)

body = "<html><body><h1>Slow CGI - should timeout</h1></body></html>"
print("Content-Type: text/html")
print(f"Content-Length: {len(body)}")
print()
print(body, end="")
