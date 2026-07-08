#!/usr/bin/env python3
import os

env_html = ""
for key, val in sorted(os.environ.items()):
    env_html += f"<tr><td><b>{key}</b></td><td>{val}</td></tr>\n"

body = f"""<!DOCTYPE html>
<html>
<head><title>CGI Environment</title></head>
<body>
<h1>CGI Environment Variables</h1>
<table border="1">
<tr><th>Variable</th><th>Value</th></tr>
{env_html}
</table>
</body>
</html>"""

print("Content-Type: text/html")
print(f"Content-Length: {len(body)}")
print()
print(body, end="")
