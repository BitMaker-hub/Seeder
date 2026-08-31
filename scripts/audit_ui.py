#!/usr/bin/env python3
"""
Auditoría de la interfaz, para las dos placas.

La composición se diseñó sobre 240x135 y cada coordenada pasa por SX()/SY(),
así que la placa grande reproduce el mismo diseño escalado. Lo que NO escala
son las fuentes: miden lo que miden en píxeles. Este script comprueba que,
con esa mezcla, ningún texto ni ningún bloque se sale de la pantalla en
ninguna de las dos.

    py scripts/audit_ui.py

Sale con código 1 si encuentra algo fuera de sitio, para poder ponerlo en CI.
"""
import re
import sys
import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
UI_CPP = ROOT / "src" / "ui" / "ui.cpp"

BOARDS = {
    "T-Display    (ESP32)":    dict(w=240, h=135),
    "T-Display-S3 (ESP32-S3)": dict(w=320, h=170),
}

# Avance horizontal por carácter de cada fuente GFX, medido de sus cabeceras
GFX_ADV = {"FMB24": 28, "FMB18": 21, "FMB9": 11, "FM9": 11}
GFX_TOP = {"FMB24": 34, "FMB18": 25, "FMB9": 13, "FM9": 13}   # alto sobre la línea base


def symbols(w, h):
    """Reconstruye las constantes de theme.h para una placa."""
    sx = lambda v: (v * w) // 240
    sy = lambda v: (v * h) // 135
    s = dict(UI_W=w, UI_H=h, SX=sx, SY=sy)
    s["UI_M"] = sx(10)
    s["UI_RAIL_X"] = sx(183)
    s["UI_RAIL_CX"] = (s["UI_RAIL_X"] + w) // 2
    s["UI_MRAIL_X"] = sx(214)
    s["UI_MRAIL_CX"] = (s["UI_MRAIL_X"] + w) // 2
    s["UI_RAIL_TOP_Y"] = sy(22)
    s["UI_RAIL_BOT_Y"] = sy(96)
    s["UI_HEAD_H"] = sy(36)
    s["UI_TINY_W"] = 6
    s["UI_BIG_BODY"] = 2
    s["UI_BIG_LH"] = sy(20)
    s["UI_BIG_CPL"] = (w - 2 * s["UI_M"]) // 12
    s["UI_TINY_CPL"] = (w - 2 * s["UI_M"]) // 6
    # constantes locales de la pantalla de salida
    s.update(bx=sx(126), by=sy(100), bw=sx(106), bh=sy(30))
    return s


def ev(expr, sym):
    try:
        return int(eval(expr.strip(), {"__builtins__": {}}, sym))
    except Exception:
        return None


TINY = re.compile(
    r"tiny\(\s*\"([^\"]*)\"\s*,\s*([^,]+),\s*([^,]+),\s*[^,]+,\s*'(\w)'\s*,\s*(\d+)\s*(?:,\s*(\w+)\s*)?\)"
)
DRAWSTR = re.compile(r'drawString\(\s*"([^"]*)"\s*,\s*([^,]+),\s*([^,]+),')
SETFONT = re.compile(r"setFreeFont\((\w+)\)")


def audit(board, sym):
    src = re.sub(r"//[^\n]*", "", UI_CPP.read_text(encoding="utf-8"))
    problems = []
    checked = 0

    for txt, xs, ys, datum, sp, size in TINY.findall(src):
        x, y = ev(xs, sym), ev(ys, sym)
        if x is None or y is None:
            continue
        checked += 1
        px = sym.get(size, 1) if size else 1
        adv = 6 * px + int(sp)
        wid = len(txt) * adv - int(sp)
        x0 = x - wid // 2 if datum == "C" else (x - wid if datum == "R" else x)
        if x0 < 0 or x0 + wid > sym["UI_W"] or y + 8 * px > sym["UI_H"]:
            problems.append(
                f'texto "{txt}"  x {x0}..{x0+wid}  y {y}..{y+8*px}'
            )

    font = None
    for line in src.splitlines():
        m = SETFONT.search(line)
        if m:
            font = m.group(1)
        d = DRAWSTR.search(line)
        if d and font:
            txt, xs, ys = d.groups()
            x, y = ev(xs, sym), ev(ys, sym)
            if x is None:
                continue
            checked += 1
            wid = len(txt) * GFX_ADV.get(font, 11)
            if x - wid // 2 < 0 or x + wid // 2 > sym["UI_W"] or y > sym["UI_H"]:
                problems.append(f'drawString "{txt}" ({font}) centrado en x={x}')

    # bloques cuya extensión depende del texto, que no escala
    sx, sy = sym["SX"], sym["SY"]
    blocks = [
        ("dado grande de la captura",
         sym["UI_RAIL_X"] - sy(64) - sx(11), sym["UI_RAIL_X"] - sx(11),
         sy(16), sy(16) + sy(64)),
        ("historial de tiradas",
         sym["UI_M"], sym["UI_M"] + 2 * sx(30) + sy(26), sy(78), sy(78) + sy(26)),
        ("barra de progreso",
         sym["UI_M"], sym["UI_M"] + sym["UI_RAIL_X"] - 2 * sym["UI_M"],
         sy(118), sy(118) + sy(4)),
        ("hex de la captura (13 por fila)",
         sym["UI_M"], sym["UI_M"] + 12 * sx(13) + 12, sy(90), sy(90) + sy(11) + 8),
        ("mnemónico, columna izquierda (palabra de 8 letras)",
         sx(6), sx(6) + sx(18) + 8 * 12, sy(30), sy(30) + 5 * sy(17) + 16),
        ("mnemónico, columna derecha (palabra de 8 letras)",
         sx(122), sx(122) + sx(18) + 8 * 12, sy(30), sy(30) + 5 * sy(17) + 16),
        ("entropía, 8 bytes por fila, 4 filas",
         sx(8), sx(8) + 7 * sx(29) + 24, sy(32), sy(32) + 3 * sy(26) + 16),
    ]
    # El QR elige versión según la longitud y el mayor píxel por módulo que
    # quepa, así que hay que comprobar los dos casos por separado.
    for words, mods in (("12 palabras", 41), ("24 palabras", 49)):
        px = 1
        while (px + 1) * mods <= sym["UI_H"] - 12 and px < 6:
            px += 1
        qw = mods * px
        quiet = max(3 * px, 6)
        blocks.append((f"QR, {words} ({mods} módulos a {px}px)",
                       sym["UI_W"] - qw - quiet, sym["UI_W"] - quiet,
                       (sym["UI_H"] - qw) // 2, (sym["UI_H"] - qw) // 2 + qw))
    blocks += [
        ("recuadro HOLD OK",
         sym["bx"], sym["bx"] + sym["bw"], sym["by"], sym["by"] + sym["bh"]),
    ]

    print(f"\n=== {board}   {sym['UI_W']}x{sym['UI_H']} ===")
    print(f"  {checked} textos comprobados: "
          + ("sin desbordes" if not problems else f"{len(problems)} PROBLEMAS"))
    for p in problems:
        print(f"    ! {p}")

    print(f"  caracteres por línea: {sym['UI_TINY_CPL']} normales, "
          f"{sym['UI_BIG_CPL']} a doble tamaño")
    for name, x0, x1, y0, y1 in blocks:
        ok = x0 >= 0 and x1 <= sym["UI_W"] and y0 >= 0 and y1 <= sym["UI_H"]
        if not ok:
            problems.append(f"{name}: x {x0}..{x1} y {y0}..{y1}")
        print(f"    {'ok ' if ok else 'MAL'} {name:52s} x {x0:3d}..{x1:3d}  y {y0:3d}..{y1:3d}")
    return problems


def main():
    bad = 0
    for board, dims in BOARDS.items():
        bad += len(audit(board, symbols(**dims)))
    print()
    if bad:
        print(f"AUDITORÍA FALLIDA: {bad} problemas")
        return 1
    print("Auditoría correcta: la misma composición cabe en las dos placas.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
