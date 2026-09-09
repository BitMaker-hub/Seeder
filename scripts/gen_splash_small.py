#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Genera src/Lib/images_splash85.h a partir de src/Lib/images.h.

Los dos bitmaps de la segunda pantalla del splash reducidos al 85%. La
T-Display pequena tiene 135 px de alto y a tamano nativo el grupo ocupaba 93,
que dejaba los creditos pegados al borde; reducidos caben con aire. La S3 usa
los originales, que alli sobra sitio.

Se reduce aqui y no en la placa a proposito: por vecino mas cercano se rompe
la palabra MAKER y se emborrona la linea de uBitcoin, y un reescalador decente
no tiene sitio en un firmware que solo lo usaria una vez al arrancar.

    py scripts/gen_splash_small.py

Necesita Pillow.  Los valores del array son RGB565 tal cual: el firmware llama
a tft.setSwapBytes(true), de modo que el literal de 16 bits ES el color.
"""
import io
import pathlib
import re
import sys

try:
    from PIL import Image
except ImportError:
    sys.exit("hace falta Pillow:  py -m pip install pillow")

PCT = 85
ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = (ROOT / "src" / "Lib" / "images.h").read_text(encoding="utf-8", errors="ignore")
DEST = ROOT / "src" / "Lib" / "images_splash85.h"

COMMENT = re.compile("//[^\n]*")
HEX4 = re.compile(r"0[xX]([0-9a-fA-F]{4})")


def dims(prefix):
    w = int(re.search(r"const uint16_t %sWidth\s*=\s*(\d+)" % prefix, SRC).group(1))
    h = int(re.search(r"const uint16_t %sHeight\s*=\s*(\d+)" % prefix, SRC).group(1))
    return w, h


def pixels(sym, n):
    m = re.search(r"%s\s*\[[^\]]*\]\s*PROGMEM\s*=\s*\{" % re.escape(sym), SRC)
    body = SRC[m.end():]
    body = body[:body.index("};")]
    # Cada linea del array acaba en un comentario que TAMBIEN lleva hex
    # ("// 0x0010 (16)"): sin quitarlo se cuelan como si fueran pixeles y la
    # imagen sale sesgada.
    body = COMMENT.sub("", body)
    vals = [int(x, 16) for x in HEX4.findall(body)]
    if len(vals) < n:
        sys.exit("%s: %d valores, hacen falta %d" % (sym, len(vals), n))
    return vals[:n]


def to_image(vals, w, h):
    im = Image.new("RGB", (w, h))
    im.putdata([(((v >> 11) & 31) * 255 // 31,
                 ((v >> 5) & 63) * 255 // 63,
                 (v & 31) * 255 // 31) for v in vals])
    return im


def to565(px):
    r, g, b = px
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def emit(im, name, wsym, hsym):
    w, h = im.size
    raw = im.convert("RGB").tobytes()          # sin getdata(), que Pillow 12 avisa
    vals = [to565(raw[k:k + 3]) for k in range(0, len(raw), 3)]
    out = ["const uint16_t %s = %d;" % (wsym, w),
           "const uint16_t %s = %d;" % (hsym, h),
           "",
           "const unsigned short %s[0x%X] PROGMEM = {" % (name, len(vals))]
    for k in range(0, len(vals), 16):
        out.append("  " + ", ".join("0x%04X" % v for v in vals[k:k + 16]) + ",")
    out.append("};")
    return "\n".join(out), w, h


def main():
    uw, uh = dims("logouBTC")
    pw, ph = dims("powered")
    ub = to_image(pixels("uBitcoinLogo", uw * uh), uw, uh)
    po = to_image(pixels("powered_logo", pw * ph), pw, ph)

    ub = ub.resize((round(uw * PCT / 100), round(uh * PCT / 100)), Image.LANCZOS)
    po = po.resize((round(pw * PCT / 100), round(ph * PCT / 100)), Image.LANCZOS)

    a, aw, ah = emit(ub, "uBitcoinLogoS", "logouBTCSWidth", "logouBTCSHeight")
    b, bw, bh = emit(po, "powered_logoS", "poweredSWidth", "poweredSHeight")

    DEST.write_text(
        "#pragma once\n"
        "/*******************************************************************\n"
        " GENERADO POR scripts/gen_splash_small.py - no editar a mano.\n\n"
        " Los dos bitmaps del splash al %d%%, reducidos con Lanczos, para la\n"
        " T-Display pequena. La S3 usa los originales de images.h.\n"
        " *******************************************************************/\n\n"
        "/******************* uBitcoin + BitMaker, %dx%d ***************/\n%s\n\n"
        "/******************* Powered, %dx%d ***************/\n%s\n"
        % (PCT, aw, ah, a, bw, bh, b),
        encoding="utf-8", newline="\n")
    print("escrito %s" % DEST)
    print("  logo    %dx%d -> %dx%d" % (uw, uh, aw, ah))
    print("  powered %dx%d -> %dx%d" % (pw, ph, bw, bh))


if __name__ == "__main__":
    main()
