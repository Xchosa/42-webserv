#!/usr/bin/env python3

import os
from urllib.parse import parse_qs


NAMED_COLORS = {
    "black": "#000000",
    "white": "#ffffff",
    "red": "#ff0000",
    "green": "#008000",
    "blue": "#0000ff",
    "yellow": "#ffff00",
    "cyan": "#00ffff",
    "magenta": "#ff00ff",
    "orange": "#ffa500",
    "purple": "#800080",
    "pink": "#ffc0cb",
    "brown": "#a52a2a",
    "gray": "#808080",
    "grey": "#808080",
}

#inverting RGB values 

def normalize_color(value):
    value = value.strip().lower()

    if value in NAMED_COLORS:
        return NAMED_COLORS[value]

    if value.startswith("#"):
        value = value[1:]

    if len(value) == 3:
        value = "".join(ch * 2 for ch in value)

    if len(value) == 6 and all(ch in "0123456789abcdef" for ch in value):
        return "#" + value

    return None


def complementary_color(hex_color):
    red = int(hex_color[1:3], 16)
    green = int(hex_color[3:5], 16)
    blue = int(hex_color[5:7], 16)

    return "#{:02x}{:02x}{:02x}".format(255 - red, 255 - green, 255 - blue)


query = os.environ.get("QUERY_STRING", "")
params = parse_qs(query)
name = params.get("name", ["blue"])[0]

base_color = normalize_color(name)

if base_color is None:
    status = "400 Bad Request"
    body = (
        "Unknown color.\n"
        "Try: blue, red, green, yellow, #3498db, or 3498db\n"
    )
else:
    status = "200 OK"
    complement = complementary_color(base_color)
    body = (
        "<!doctype html>\n"
        "<html>\n"
        "<body>\n"
        "<h1>Complementary color</h1>\n"
        f"<p>Input: {name}</p>\n"
        f"<p>Base: {base_color}</p>\n"
        f"<p>Complement: {complement}</p>\n"
        f"<div style='width:120px;height:60px;background:{base_color}'></div>\n"
        f"<div style='width:120px;height:60px;background:{complement}'></div>\n"
        "</body>\n"
        "</html>\n"
    )

body_bytes = body.encode("utf-8")

print(f"Status: {status}")
print("Content-Type: text/html; charset=utf-8")
print(f"Content-Length: {len(body_bytes)}")
print()
print(body, end="")
