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


COMMENT = re.compile("//[^\n]*")


def img_dims():
    """Tamanos de los bitmaps del splash, leidos de las cabeceras.

    Copiarlos aqui como numeros sueltos hacia que el script se diese la razon
    a si mismo: reexportar un logo movia la composicion del firmware y no la
    de la auditoria, y CI seguia en verde."""
    d = {}
    for f in ("images.h", "images_splash85.h"):
        p = ROOT / "src" / "Lib" / f
        if not p.exists():
            continue
        txt = COMMENT.sub("", p.read_text(encoding="utf-8", errors="ignore"))
        for name, val in re.findall(r"const uint16_t (\w+)\s*=\s*(\d+)\s*;", txt):
            d[name] = int(val)
    falta = [k for k in ("logouBTCHeight", "poweredWidth", "poweredHeight",
                         "logouBTCSHeight", "poweredSWidth", "poweredSHeight") if k not in d]
    if falta:
        sys.exit("no encuentro en src/Lib/: " + ", ".join(falta))
    return d


IMG = img_dims()

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
    s["UI_TINY_H"] = 8
    s["UI_BIG_BODY"] = 2
    s["UI_BIG_LH"] = sy(20)
    s["UI_BIG_CPL"] = (w - 2 * s["UI_M"]) // 12
    s["UI_TINY_CPL"] = (w - 2 * s["UI_M"]) // 6
    # constantes locales de la pantalla de salida
    s.update(bx=sx(126), by=sy(100), bw=sx(106), bh=sy(30))
    # y las de los creditos del splash
    s["cr2"] = h - sy(9) - s["UI_TINY_H"]
    s["cr1"] = s["cr2"] - sy(12)
    # cada placa usa su bitmap: la S3 el nativo, la pequena el del 85%
    big = w >= 320
    s["SPL_H"]  = IMG["logouBTCHeight" if big else "logouBTCSHeight"]
    s["SPL_PW"] = IMG["poweredWidth"   if big else "poweredSWidth"]
    s["SPL_PH"] = IMG["poweredHeight"  if big else "poweredSHeight"]
    s["top"] = (s["cr1"] - (s["SPL_H"] + sy(6) + s["SPL_PH"])) // 2
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
# Las paginas de QR reciben cinco rotulos literales (el primero con espaciado
# 1, el resto con 0) y un sexto valor, que es texto fijo en la de la semilla y
# una variable -la huella, 8 hex- en la de solo lectura.
QRPAGE = re.compile(r'qrPage\(' + r'\s*"([^"]*)"\s*,' * 5 + r'\s*(?:"([^"]*)"|(\w+))')
SETFONT = re.compile(r"setFreeFont\((\w+)\)")


def audit(board, sym):
    src = re.sub(r"//[^\n]*", "", UI_CPP.read_text(encoding="utf-8"))
    problems = []
    checked = 0

    creditos = {"L": [], "R": []}
    for txt, xs, ys, datum, sp, size in TINY.findall(src):
        x, y = ev(xs, sym), ev(ys, sym)
        if x is None or y is None:
            continue
        checked += 1
        px = sym.get(size, 1) if size else 1
        adv = 6 * px + int(sp)
        wid = len(txt) * adv - int(sp)
        x0 = x - wid // 2 if datum == "C" else (x - wid if datum == "R" else x)
        # Los creditos del splash son los textos que caen en sus dos lineas.
        # Se recogen del propio ui.cpp, con el texto y el espaciado reales:
        # escribirlos aqui a mano hacia que la comprobacion se diera la razon
        # a si misma y no saltara al alargar un nombre.
        if y in (sym["cr1"], sym["cr2"]) and datum in "LR":
            creditos[datum].append((txt, x0, x0 + wid))
        if x0 < 0 or x0 + wid > sym["UI_W"] or y + sym["UI_TINY_H"] * px > sym["UI_H"]:
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
    # El QR elige versión según la longitud, y luego el mayor píxel por módulo
    # que deje 2 módulos de margen, ensanchando el margen hasta 4 con lo que
    # sobre. Se comprueban los PEORES casos: 12 palabras de 8 letras son 107
    # caracteres (versión 6, 41 módulos) y 24 de 8 son 215 (versión 9, 53).
    # El bloque incluye la zona tranquila, que también se pinta.
    for words, mods in (("12 palabras", 41), ("24 palabras", 53)):
        px = 1
        while (mods + 4) * (px + 1) <= sym["UI_H"] and px < 6:
            px += 1
        quiet = min((sym["UI_H"] - mods * px) // (2 * px), 4)
        qw, b = mods * px, quiet * px
        x1 = sym["UI_W"] - sx(2)
        blocks.append((f"QR, {words} ({mods} módulos a {px}px, margen {quiet})",
                       x1 - qw - 2 * b, x1,
                       (sym["UI_H"] - qw) // 2 - b, (sym["UI_H"] - qw) // 2 + qw + b))
    blocks += [
        ("splash: logotipos + Powered by uBitcoin",
         (sym["UI_W"]-sym["SPL_PW"])//2, (sym["UI_W"]-sym["SPL_PW"])//2 + sym["SPL_PW"],
         sym["top"], sym["top"] + sym["SPL_H"] + sy(6) + sym["SPL_PH"]),
        ("recuadro HOLD OK",
         sym["bx"], sym["bx"] + sym["bw"], sym["by"], sym["by"] + sym["bh"]),
    ]

    # El aviso de "mantener OK" tapa el area de trabajo pero NO puede invadir
    # el rail, asi que se mide contra UI_RAIL_X y no contra el ancho total.
    tw = 10 * (6 * sym["UI_BIG_BODY"] + 1) - 1       # "START OVER" a doble tamano
    sw = 17 * (6 + 1) - 1                            # "RELEASE TO CANCEL"
    # tiny() con datum 'C' hace px = x - w/2 y avanza w: en anchos impares el
    # borde derecho cae 1 px mas alla de x + w//2, asi que se calcula igual.
    cen = lambda x, w: (x - w//2, x - w//2 + w)
    c = sym["UI_RAIL_X"] // 2
    work = [
        ("aviso: titulo START OVER",
         *cen(c, tw), sy(38), sy(38) + 8 * sym["UI_BIG_BODY"]),
        ("aviso: barra del mantenido",
         sym["UI_M"], sym["UI_RAIL_X"] - sym["UI_M"], sy(68), sy(68) + sy(10)),
        ("aviso: RELEASE TO CANCEL",
         *cen(c, sw), sy(92), sy(92) + 8),
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

    for name, x0, x1, y0, y1 in work:
        ok = x0 >= 0 and x1 <= sym["UI_RAIL_X"] and y0 >= 0 and y1 <= sym["UI_H"]
        if not ok:
            problems.append(f"{name}: x {x0}..{x1} y {y0}..{y1} (limite {sym['UI_RAIL_X']})")
        print(f"    {'ok ' if ok else 'MAL'} {name:52s} x {x0:3d}..{x1:3d}  y {y0:3d}..{y1:3d}")

    # El QR crece hacia la izquierda segun la version; el texto de esa pantalla
    # vive a su izquierda. Que no se pisen es lo unico que los separa.
    # El rotulo mas largo sale del propio ui.cpp: ponerlo aqui a mano era
    # repetir el error de que el script se de la razon a si mismo.
    anchos = []
    for tag, l1, l2, l3, extra, vlit, vvar in QRPAGE.findall(src):
        anchos.append((tag, len(tag) * (sym["UI_TINY_W"] + 1) - 1))
        anchos += [(t, len(t) * sym["UI_TINY_W"]) for t in (l1, l2, l3, extra) if t]
        if vlit:                  # el valor es texto fijo: se mide tal cual
            anchos.append((vlit, len(vlit) * sym["UI_TINY_W"]))
        elif vvar:                # es una variable: la huella, 8 hex
            anchos.append(("<" + vvar + ">", 8 * sym["UI_TINY_W"]))
    if not anchos:
        problems.append("no encuentro los rotulos de las paginas de QR en ui.cpp")
        anchos = [("?", 0)]
    largo, ancho = max(anchos, key=lambda a: a[1])
    texto = sym["UI_M"] + ancho
    for caso, mods in (("zpub / 12 palabras", 41), ("descriptor", 45), ("24 palabras", 53)):
        px = 1
        while (mods + 4) * (px + 1) <= sym["UI_H"] and px < 6:
            px += 1
        quiet = min((sym["UI_H"] - mods * px) // (2 * px), 4)
        qleft = sym["UI_W"] - sx(2) - mods * px - 2 * quiet * px
        ok = texto < qleft
        if not ok:
            problems.append(f"QR {caso}: el codigo pisa el rotulo \"{largo}\" ({qleft} < {texto})")
        print(f"    {'ok ' if ok else 'MAL'} {'QR ' + caso + ': rotulo | codigo':52s} "
              f"x ..{texto:3d} | {qleft:3d}..  ({mods} mod, {px}px)")

    # --- creditos del splash -------------------------------------------
    # El bounding box solo, por como se calcula top, no puede fallar nunca:
    # hay que comprobar las dos cosas que este diseno si arriesga.
    if creditos["L"] and creditos["R"]:
        izq = max(e for _, _, e in creditos["L"])       # borde derecho de la columna izquierda
        der = min(s for _, s, _ in creditos["R"])       # borde izquierdo de la derecha
        ok = izq < der
        if not ok:
            choca = "/".join(t for t, _, _ in creditos["L"] + creditos["R"])
            problems.append(f"splash: las columnas de creditos chocan en x {der}..{izq} ({choca})")
        print(f"    {'ok ' if ok else 'MAL'} {'splash: columnas de creditos, izq | der':52s} "
              f"x ..{izq:3d} | {der:3d}..   ({len(creditos['L'])}+{len(creditos['R'])} textos)")

    hueco = sym["cr1"] - (sym["top"] + sym["SPL_H"] + sy(6) + sym["SPL_PH"])
    ok = hueco > 0
    if not ok:
        problems.append(f"splash: el grupo pisa los creditos (hueco {hueco})")
    print(f"    {'ok ' if ok else 'MAL'} {'splash: hueco grupo -> creditos':52s} "
          f"{hueco:3d} px")

    # El aviso reescribe el rail entero y vuelve a poner OK/HOLD y el caret.
    hx0, hx1 = cen(sym["UI_RAIL_CX"], 4 * 6)          # "HOLD", sin espaciado
    ok = hx0 > sym["UI_RAIL_X"] and hx1 <= sym["UI_W"] and sy(106) + 8 <= sy(118)
    if not ok:
        problems.append(f"aviso: OK/HOLD del rail mal colocado x {hx0}..{hx1}")
    print(f"    {'ok ' if ok else 'MAL'} {'aviso: OK/HOLD en el rail':52s} "
          f"x {hx0:3d}..{hx1:3d}  y {sy(96):3d}..{sy(106)+8:3d}")
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
