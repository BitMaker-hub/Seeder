# -*- coding: utf-8 -*-
"""
SEEDER — carcasa tactica  (LilyGO TTGO T-Display, ESP32)
--------------------------------------------------------
Piezas: CUERPO (frontal) + TAPA (trasera, clips) + BOTONES x2.
Cotas de placa tomadas del STEP oficial "T-Display.step".
Impresion: Bambu Lab P1S/P2S/H2S, PLA o PETG, boquilla 0.4.

Sistema de coordenadas:
  z = 0  cara frontal exterior;  +z hacia atras (hacia la tapa).
  y = 0  centro;  -y extremo del USB;  +y extremo del colgador.
"""
import json
import os
from build123d import *

SCRATCH = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(SCRATCH, "components.json")) as _f:
    COMPS = json.load(_f)          # cajas de componentes (coords carcasa, DZ=5.2)

OUT = r"C:\Users\bitma\Desktop\Dev\Seeder\3d files\Carcasa_Tactica_v1"
os.makedirs(OUT, exist_ok=True)

# ═══════════════ PARAMETROS PRINCIPALES ═══════════════
W, L, T = 30.6, 66.0, 17.4          # exterior (ancho, largo, grosor)
CH_S, CH_N = 4.6, 7.2               # corte de esquinas sur (USB) / norte
WALL = 2.4                          # pared lateral
PLATE = 2.0                         # espesor cara frontal
CEIL = 2.2                          # camara frontal (vidrio + pulsadores)
LID_T = 1.8                         # espesor tapa
REBATE = 1.2                        # apoyo perimetral de la tapa
CLR = 0.25                          # holgura tapa
FACE_CH = 2.0                       # chaflan cara frontal
BACK_CH = 1.6                       # chaflan cara trasera

PCB_T = 1.0
BACK_CLR = 3.8                      # espacio componentes traseros (ESP32 3.1)

# --- USB: verificar con la placa real y ajustar si hace falta ---
USB_W = 12.4                        # ancho de la abertura
USB_Z0, USB_Z1 = 4.5, 8.9           # tramo z (micro-USB/USB-C montado detras)

# --- Bateria (bolsillo en la tapa) ---
BAT_W, BAT_L, BAT_H = 21.0, 40.0, 6.6

# --- Clips de cierre ---
CLIP_Y = (-24.0, 17.5)              # centros de las 4 ventanas
CLIP_LEN = 7.0                      # longitud de cada ventana
CZ0, CZ1 = 8.5, 11.2                # tramo z de la ventana

# ═══════════════ DERIVADAS ═══════════════
Z_PCB_F = PLATE + CEIL              # 4.2  cara frontal PCB
Z_PCB_B = Z_PCB_F + PCB_T           # 5.2  cara trasera PCB
Z_BAT = Z_PCB_B + BACK_CLR          # 9.0  suelo del bolsillo de bateria
Z_LID = T - LID_T                   # 15.6 cara interior de la tapa

hw, hl = W / 2, L / 2
XI = hw - WALL                      # 12.9  media anchura interior
YI_S, YI_N = -hl + WALL, 24.0       # interior (banda norte maciza: colgador)
LEDGE = 2.0                         # repisa de apoyo de la PCB

YB0 = YI_S + CLR                    # y de case del borde USB de la placa


def bx(x):
    """STEP(placa) -> case, eje X."""
    return x + 12.445


def by(y):
    """STEP(placa) -> case, eje Y."""
    return y + YB0


# posiciones tomadas del STEP
DISP = (bx(-21.44), bx(-3.84), by(10.65), by(42.65))   # modulo LCD 17.6x32.0
BTN_A = (bx(-20.66), by(3.44))
BTN_B = (bx(-4.17), by(3.29))
RESET = (bx(-23.52), by(11.87))
JST = (bx(-13.33), by(8.45))        # conector bateria (cara trasera)

# ═══════════════ CONTORNO ═══════════════
outline = Polygon(
    (-hw + CH_S, -hl), (hw - CH_S, -hl), (hw, -hl + CH_S),
    (hw, hl - CH_N), (hw - CH_N, hl), (-hw + CH_N, hl),
    (-hw, hl - CH_N), (-hw, -hl + CH_S), align=None)

body = extrude(outline, T)
body = chamfer(body.edges().group_by(Axis.Z)[0], FACE_CH)
body = chamfer(body.edges().group_by(Axis.Z)[-1], BACK_CH)

# ═══════════════ VENTANA DE PANTALLA ═══════════════
LIP = 0.75                                     # solape sobre el borde del modulo
WX0, WX1 = DISP[0] + LIP, DISP[1] - LIP
WY0, WY1 = DISP[2] + LIP, DISP[3] - LIP
wc = ((WX0 + WX1) / 2, (WY0 + WY1) / 2)
body -= Pos(wc[0], wc[1], PLATE / 2) * Box(WX1 - WX0, WY1 - WY0, PLATE + 2)
body = chamfer([e for e in body.edges()
                if abs(e.center().Z) < 1e-6
                and WX0 - 1 < e.center().X < WX1 + 1
                and WY0 - 1 < e.center().Y < WY1 + 1], 1.6)

# ═══════════════ COLGADOR ═══════════════
LAN_Y, LAN_W, LAN_H = 27.6, 12.0, 4.4
slot = Pos(0, LAN_Y) * SlotOverall(LAN_W, LAN_H)
body -= extrude(slot, T + 1)
wedge = extrude(offset(slot, 0.7, kind=Kind.ARC), 0.71, taper=45)
body -= Pos(0, 0, -0.01) * wedge
body -= Pos(0, 0, T + 0.01) * mirror(wedge, Plane.XY)

# rayado junto al colgador
for sx in (-1, 1):
    for k in range(3):
        body -= Pos(sx * (9.0 + 1.7 * k), LAN_Y - 1.1 * k, 0.28) * \
            Rot(0, 0, 45 * sx) * Box(1.1, 7.4 - 2.0 * k, 0.58)

# ═══════════════ CAVIDADES INTERIORES ═══════════════
# camara frontal (delimitada por la repisa)
body -= Pos(0, (YI_S + YI_N) / 2, (PLATE + Z_PCB_F) / 2) * \
    Box(2 * (XI - LEDGE), (YI_N - LEDGE) - (YI_S + LEDGE), CEIL + 0.01)
# hueco principal (placa + componentes + bateria)
body -= Pos(0, (YI_S + YI_N) / 2, (Z_PCB_F + T + 1) / 2) * \
    Box(2 * XI, YI_N - YI_S, T + 1 - Z_PCB_F)
# --- holgura automatica: descuenta la envolvente de cada componente frontal ---
INF_XY, INF_Z = 0.35, 0.25
carved = 0
for x0, y0, z0, x1, y1, z1 in COMPS["front"]:
    a0, b0 = x0 - INF_XY, y0 - INF_XY
    a1, b1 = x1 + INF_XY, y1 + INF_XY
    # solo interesa si toca la repisa perimetral o la cara interna de la placa frontal
    if not (a1 > XI - LEDGE or a0 < -(XI - LEDGE)
            or b0 < YI_S + LEDGE or b1 > YI_N - LEDGE or z0 - INF_Z < PLATE):
        continue
    c0, c1 = min(z0 - INF_Z, PLATE), Z_PCB_F + 0.05
    body -= Pos((a0 + a1) / 2, (b0 + b1) / 2, (c0 + c1) / 2) * \
        Box(a1 - a0, b1 - b0, c1 - c0)
    carved += 1
print(f"holguras frontales descontadas: {carved} de {len(COMPS['front'])}")
# (el conector USB va en la cara trasera de la PCB: la repisa sur no estorba)

# ═══════════════ USB: abertura + surco ═══════════════
body -= Pos(0, -hl + WALL / 2, (USB_Z0 + USB_Z1) / 2) * \
    Box(USB_W, WALL + 4, USB_Z1 - USB_Z0)
sw, sh, sc = 18.0, 8.6, 2.4
scallop = Polygon(
    (-sw / 2 + sc, USB_Z0 - 2.1), (sw / 2 - sc, USB_Z0 - 2.1),
    (sw / 2, USB_Z0 - 2.1 + sc), (sw / 2, USB_Z0 - 2.1 + sh - sc),
    (sw / 2 - sc, USB_Z0 - 2.1 + sh), (-sw / 2 + sc, USB_Z0 - 2.1 + sh),
    (-sw / 2, USB_Z0 - 2.1 + sh - sc), (-sw / 2, USB_Z0 - 2.1 + sc), align=None)
body -= extrude(Plane((0, -hl + 2.4, 0), x_dir=(1, 0, 0), z_dir=(0, -1, 0)) *
                scallop, 4.5)

# ═══════════════ BOTONES + RESET ═══════════════
CAP_HOLE = 5.8                      # paso del vastago en la placa frontal
CAP_POCK = 7.2                      # hueco de la pestana (retencion)
POCK_Z0, POCK_Z1 = 1.1, 2.4
Z_BTN = min(b[2] for b in COMPS["front"]
            if b[0] > -10.5 and b[4] < -24.0)     # cara superior del pulsador SMD
for cx, cy in (BTN_A, BTN_B):
    body -= extrude(Pos(cx, cy) * RectangleRounded(CAP_HOLE, CAP_HOLE, 0.8), PLATE + 1)
    body -= extrude(Pos(cx, cy, POCK_Z0) * RectangleRounded(CAP_POCK, CAP_POCK, 1.0),
                    POCK_Z1 - POCK_Z0)
# rayado entre los dos botones
for k in (-1, 0, 1):
    body -= Pos((BTN_A[0] + BTN_B[0]) / 2 + k * 2.4, BTN_A[1], 0.28) * \
        Rot(0, 0, 45) * Box(1.1, 6.6, 0.58)
# reset (pinhole avellanado)
body -= Pos(RESET[0], RESET[1], PLATE / 2) * Cylinder(1.5, PLATE + 2)
body -= Pos(RESET[0], RESET[1], 0.35) * Cone(1.95, 1.5, 0.75)

# ═══════════════ SEEDER + BROTE ═══════════════
FONTS = [r"C:\Windows\Fonts\ariblk.ttf", r"C:\Windows\Fonts\arialbd.ttf"]
TXT_Y = 17.6                        # banda norte, entre pantalla y colgador
txt = None
for fp in FONTS:
    if os.path.exists(fp):
        try:
            txt = Text("SEEDER", font_size=4.0, font_path=fp,
                       align=(Align.CENTER, Align.CENTER))
            break
        except Exception as e:
            print("aviso fuente:", fp, e)
if txt is not None:
    body -= extrude(Pos(2.4, TXT_Y) * txt, 0.6)


def sprout(scale=1.0):
    """Brote del logo SEEDER: dos hojas, tallo y semilla."""
    def vesica(r, d):
        return Circle(r) & Pos(d, 0) * Circle(r)
    s = scale
    return (Pos(-1.3 * s, 1.15 * s) * Rot(0, 0, 40) * vesica(1.4 * s, 1.6 * s)
            + Pos(1.15 * s, 1.6 * s) * Rot(0, 0, 130) * vesica(1.15 * s, 1.35 * s)
            + Pos(0.1 * s, 0.1 * s) * Rot(0, 0, 6) * Rectangle(0.55 * s, 3.5 * s)
            + Pos(-0.25 * s, -1.95 * s) * Rot(0, 0, -35) * Ellipse(1.45 * s, 1.0 * s))


body -= extrude(Pos(-9.4, TXT_Y + 0.2) * sprout(0.78), 0.6)

# ═══════════════ NERVIOS LATERALES (agarre) ═══════════════
# franjas diagonales grabadas 0.7 mm en las dos paredes laterales
ribs = None
for sx in (-1, 1):
    yy = -22.5
    while yy < 22.0:
        if all(abs(yy - cy) > CLIP_LEN / 2 + 2.0 for cy in CLIP_Y):
            g = Pos(sx * (hw + 0.9), yy, T / 2) * Rot(0, 0, 45 * sx) * \
                Box(1.6, 3.0, T - 5.6)
            ribs = g if ribs is None else ribs + g
        yy += 5.2
if ribs is not None:
    body -= ribs

# nervios diagonales en el frontal, flanqueando la pantalla
for sx in (-1, 1):
    yy = -13.0
    while yy < 10.5:
        body -= Pos(sx * 11.3, yy, 0.4) * Rot(0, 0, 45 * sx) * Box(1.5, 8.6, 0.8)
        yy += 5.8

# ═══════════════ VENTANAS DE CLIP ═══════════════
for yc in CLIP_Y:
    for sx in (-1, 1):
        # rebaje exterior: enmarca el clip y da agarre a la una
        body -= Pos(sx * (hw + 0.3), yc, (CZ0 + CZ1) / 2) * \
            Box(1.4, CLIP_LEN + 3.4, CZ1 - CZ0 + 2.6)
        body -= Pos(sx * (hw - WALL / 2 + 0.6), yc, (CZ0 + CZ1) / 2) * \
            Box(WALL + 1.4, CLIP_LEN, CZ1 - CZ0)

# ═══════════════ MUESCA DE APERTURA ═══════════════
pry = Pos(0, -hl + 0.8, T - 0.9) * Box(10.0, 1.6, 3.0)
body -= pry

# ═══════════════ REBAJE PERIMETRAL DE LA TAPA ═══════════════
rebate_sk = offset(outline, -REBATE, kind=Kind.INTERSECTION) & \
    Pos(0, -8.0) * Rectangle(80.0, 64.0)
body -= extrude(Pos(0, 0, Z_LID) * rebate_sk, LID_T + 1)

# ══════════════════════════ TAPA ══════════════════════════
lid_sk = offset(rebate_sk, -CLR, kind=Kind.INTERSECTION)
lid = extrude(Pos(0, 0, Z_LID + 0.05) * lid_sk, LID_T - 0.05)
lid = chamfer(lid.edges().group_by(Axis.Z)[-1], 0.7)


def clip(yc, sx):
    """Brazo flexible con gancho; retencion 0.8 dentro de la ventana."""
    ax0, ax1 = 11.3, 12.6           # brazo
    hx1 = 13.65                     # punta del gancho
    arm = Pos(sx * (ax0 + ax1) / 2, yc, (CZ0 + Z_LID + 0.1) / 2) * \
        Box(ax1 - ax0, CLIP_LEN - 0.6, Z_LID + 0.1 - CZ0)
    hook = Pos(sx * (ax1 + hx1) / 2, yc, (CZ0 + 0.15 + CZ1 - 0.15) / 2) * \
        Box(hx1 - ax1, CLIP_LEN - 0.6, (CZ1 - 0.15) - (CZ0 + 0.15))
    # rampa de insercion (cara inferior exterior)
    hook -= Pos(sx * (hx1 + 0.2), yc, CZ0 + 0.9) * Rot(0, 45, 0) * \
        Box(1.9, CLIP_LEN, 1.9)
    return arm + hook


for yc in CLIP_Y:
    for sx in (-1, 1):
        lid += clip(yc, sx)

# --- bolsillo de bateria ---
BW = BAT_W / 2
BY0, BY1 = -18.6, 21.0
bat_wall = 1.5
h_bat = Z_LID + 0.1 - Z_BAT
for sx in (-1, 1):                                   # laterales
    lid += Pos(sx * (BW + bat_wall / 2), (BY0 + BY1) / 2, (Z_BAT + Z_LID + 0.1) / 2) * \
        Box(bat_wall, BY1 - BY0, h_bat)
lid += Pos(0, BY1 + bat_wall / 2, (Z_BAT + Z_LID + 0.1) / 2) * \
    Box(2 * BW + 2 * bat_wall, bat_wall, h_bat)      # tope norte
for sx in (-1, 1):                                   # tope sur partido (paso de cable)
    lid += Pos(sx * (BW - 3.0), BY0 - bat_wall / 2, (Z_BAT + Z_LID + 0.1) / 2) * \
        Box(6.0, bat_wall, h_bat)

lid -= pry
# la tapa no debe invadir el conector de bateria ni el USB
lid -= Pos(JST[0], JST[1], (Z_PCB_B + Z_BAT) / 2) * Box(9.0, 7.0, 12.0)

# --- patron de brotes en la cara trasera ---
def y_glyph(s=1.0):
    arm = Pos(0, 1.2 * s) * Rectangle(1.0 * s, 2.4 * s)
    return arm + Rot(0, 0, 120) * arm + Rot(0, 0, 240) * arm


pat = None
i = 0
yy = -27.0
while yy < 21.5:
    xs = (-8.0, 0.0, 8.0) if i % 2 == 0 else (-4.0, 4.0)
    for xx in xs:
        ang = (i * 67 + int(xx * 11)) % 360
        s = 0.9 + ((i * 37 + int(xx * 17)) % 45) / 100.0
        jx = ((i * 23 + int(abs(xx) * 5)) % 5) * 0.4 - 0.8
        g = Pos(xx + jx, yy) * Rot(0, 0, ang) * y_glyph(s)
        pat = g if pat is None else pat + g
    yy += 6.0
    i += 1
lid -= extrude(Pos(0, 0, T) * pat, -0.6)

# ══════════════════════════ BOTONES ══════════════════════════
PROUD = 0.6                         # cuanto sobresale el boton de la carcasa


def check_icon():
    """Palomita de validar, dentro de una caja de 3.5 x 2.7 mm."""
    def bar(p, q, w):
        import math
        dx, dy = q[0] - p[0], q[1] - p[1]
        ln = math.hypot(dx, dy)
        return Pos((p[0] + q[0]) / 2, (p[1] + q[1]) / 2) * \
            Rot(0, 0, math.degrees(math.atan2(dy, dx))) * Rectangle(ln, w)
    knee = (-0.5, -1.2)
    return bar((-1.7, 0.1), knee, 0.95) + bar(knee, (1.8, 1.5), 0.95) + \
        Pos(*knee) * Circle(0.47)


def cap(icon, icon_rot=0):
    """Se imprime boca abajo: z=0 es la cara vista, +z hacia dentro."""
    stem_h = POCK_Z0 + PROUD                       # atraviesa la placa frontal
    flg_h = 0.55
    post_top = Z_BTN + PROUD - 0.05                # 0.05 de precarga sobre el SMD
    c = extrude(RectangleRounded(CAP_HOLE - 0.35, CAP_HOLE - 0.35, 0.7), stem_h)
    c += extrude(Pos(0, 0, stem_h) *
                 RectangleRounded(CAP_POCK - 0.35, CAP_POCK - 0.35, 0.9), flg_h)
    c += Pos(0, 0, stem_h + flg_h) * \
        Cylinder(1.6, post_top - stem_h - flg_h,
                 align=(Align.CENTER, Align.CENTER, Align.MIN))
    c = chamfer(c.edges().group_by(Axis.Z)[0], 0.4)
    ic = (Rectangle(3.4, 1.15) + Rectangle(1.15, 3.4)) if icon == "plus" \
        else check_icon()
    c -= extrude(Rot(0, 0, icon_rot) * ic, 0.5)
    return c


# La UI va en landscape (setRotation(1)): el aparato se lee girado 90 grados,
# asi que los iconos se graban girados. Se exportan las dos orientaciones.
# MOVE = cambiar pantalla (+)   ·   SELECT = aceptar (check)
caps_a = cap("plus", -90) + Pos(11, 0, 0) * cap("check", -90)
caps_b = cap("plus", 90) + Pos(11, 0, 0) * cap("check", 90)

# ══════════════════════════ EXPORT ══════════════════════════
export_stl(body, os.path.join(OUT, "Seeder_CUERPO.stl"))
export_stl(lid, os.path.join(OUT, "Seeder_TAPA.stl"))
export_stl(caps_a, os.path.join(OUT, "Seeder_BOTONES_x2.stl"))
export_stl(caps_b, os.path.join(OUT, "Seeder_BOTONES_x2_giro_opuesto.stl"))
bb, lb = body.bounding_box(), lid.bounding_box()
print(f"cuerpo : {bb.size.X:.1f} x {bb.size.Y:.1f} x {bb.size.Z:.1f} mm")
print(f"tapa   : {lb.size.X:.1f} x {lb.size.Y:.1f} x {lb.size.Z:.1f} mm")
print(f"bateria: {BAT_W:.0f} x {BAT_L:.0f} x {BAT_H:.1f} mm max")
print("OK ->", OUT)
