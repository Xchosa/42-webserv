#!/usr/bin/env python3
import os
import sys

method = os.environ.get("REQUEST_METHOD", "GET")
content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
query = os.environ.get("QUERY_STRING", "")

body_data = ""
if content_length > 0:
    body_data = sys.stdin.read(content_length)

body = f"""<!DOCTYPE html>
<html>
<head><title>Echo CGI</title></head>
<body>
<h1>Echo CGI</h1>
<p><b>Method:</b> {method}</p>
<p><b>Query String:</b> {query}</p>
<p><b>Content-Length:</b> {content_length}</p>
<p><b>Request Body:</b></p>
<pre>{body_data}</pre>
</body>
</html>"""

print("Content-Type: text/html")
print(f"Content-Length: {len(body)}")
print()
print(body, end="")
