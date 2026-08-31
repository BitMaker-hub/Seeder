# -*- coding: utf-8 -*-
"""Comprueba que la placa real (STEP) encaja dentro de la carcasa generada."""
import os

import numpy as np
import trimesh
from build123d import import_step, export_stl, Compound

STEP = r"C:\Users\bitma\Desktop\Dev\Seeder\3d files\T-Display.step"
OUT = r"C:\Users\bitma\Desktop\Dev\Seeder\3d files\Carcasa_Tactica_v1"
SCRATCH = os.path.dirname(os.path.abspath(__file__))
BOARD_STL = os.path.join(SCRATCH, "board_in_case.stl")

DX, DZ = 12.445, 5.2
DY = -33.0 + 2.4 + 0.25          # YB0 = -30.35

if not os.path.exists(BOARD_STL):
    asm = import_step(STEP)
    keep = []
    dropped = 0
    for s in asm.solids():
        b = s.bounding_box()
        if abs(b.min.X) > 1e4:
            continue
        # descarta solo las tiras de pines (no van soldadas)
        if (b.min.Z + 5.2) < 1.9 and (b.max.Z - b.min.Z) > 6.0:
            dropped += 1
            continue
        keep.append(s)
    print(f"solidos usados: {len(keep)}  (descartados pines: {dropped})")
    comp = Compound(children=keep)
    export_stl(comp, BOARD_STL, tolerance=0.05)

board = trimesh.load(BOARD_STL)
board.apply_translation([DX, DY, DZ])
print("placa en coords carcasa: bounds", board.bounds.round(2).tolist())

body = trimesh.load(os.path.join(OUT, "Seeder_CUERPO.stl"))
lid = trimesh.load(os.path.join(OUT, "Seeder_TAPA.stl"))

# 1) la placa debe quedar dentro del volumen exterior
print("\n--- comprobaciones ---")
bb, cb = board.bounds, body.bounds
inside = (bb[0] >= cb[0] - 1e-6).all() and (bb[1] <= cb[1] + 1e-6).all()
print(f"placa dentro del volumen exterior: {inside}")

# 2) interferencias por muestreo de puntos de la placa dentro del solido de la carcasa
pts = board.sample(60000)
for name, part in (("CUERPO", body), ("TAPA", lid)):
    hit = part.contains(pts)
    n = int(hit.sum())
    print(f"puntos de la placa dentro de {name}: {n} / {len(pts)}")
    if n:
        h = pts[hit]
        print(f"   zona X[{h[:,0].min():.2f},{h[:,0].max():.2f}] "
              f"Y[{h[:,1].min():.2f},{h[:,1].max():.2f}] "
              f"Z[{h[:,2].min():.2f},{h[:,2].max():.2f}]")

# 3) holguras clave
disp = board.bounds
print(f"\ncara frontal placa z={disp[0][2]:.2f} (esperado 2.62 = vidrio)")
print(f"cara trasera placa z={disp[1][2]:.2f}")
print(f"ancho placa X[{disp[0][0]:.2f},{disp[1][0]:.2f}]  interior +-12.90")
print(f"largo placa Y[{disp[0][1]:.2f},{disp[1][1]:.2f}]  interior -30.60..24.00")
