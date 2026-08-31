# -*- coding: utf-8 -*-
"""
Extrae del STEP las cajas envolventes de los componentes de la placa,
ya en coordenadas de la carcasa, y las guarda en components.json.
La carcasa las descuenta para garantizar que nada roza.
"""
import json
import os

from build123d import import_step

STEP = r"C:\Users\bitma\Desktop\Dev\Seeder\3d files\T-Display.step"
SCRATCH = os.path.dirname(os.path.abspath(__file__))
DX, DY, DZ = 12.445, -30.35, 5.2

asm = import_step(STEP)
front, back, pcb = [], [], None
headers = 0

for s in asm.solids():
    b = s.bounding_box()
    if abs(b.min.X) > 1e4:
        continue
    x0, y0, z0 = b.min.X + DX, b.min.Y + DY, b.min.Z + DZ
    x1, y1, z1 = b.max.X + DX, b.max.Y + DY, b.max.Z + DZ
    box = [round(v, 3) for v in (x0, y0, z0, x1, y1, z1)]
    # tiras de pines: no van soldadas
    if z0 < 1.9 and (z1 - z0) > 6.0:
        headers += 1
        continue
    if z1 <= 4.25:                      # cara frontal (pantalla, pulsadores, SMD)
        front.append(box)
    elif z0 >= 5.15:                    # cara trasera
        back.append(box)
    else:
        pcb = box if pcb is None else [
            min(pcb[0], x0), min(pcb[1], y0), min(pcb[2], z0),
            max(pcb[3], x1), max(pcb[4], y1), max(pcb[5], z1)]

data = {"front": front, "back": back, "pcb": pcb, "headers_ignored": headers}
with open(os.path.join(SCRATCH, "components.json"), "w") as f:
    json.dump(data, f, indent=1)

print(f"frontales: {len(front)}   traseros: {len(back)}   pines ignorados: {headers}")
print("pcb:", pcb)
fz = max(b[5] for b in front)
bz = max(b[5] for b in back)
print(f"altura max componente frontal z={fz:.2f} (camara 2.00-4.20)")
print(f"altura max componente trasero z={bz:.2f} (suelo bateria 9.00)")
# los mas altos por detras
back.sort(key=lambda b: -b[5])
for b in back[:6]:
    print(f"   trasero z={b[5]:.2f}  X[{b[0]:.1f},{b[3]:.1f}] Y[{b[1]:.1f},{b[4]:.1f}]")
front.sort(key=lambda b: b[2])
for b in front[:6]:
    print(f"   frontal z0={b[2]:.2f}  X[{b[0]:.1f},{b[3]:.1f}] Y[{b[1]:.1f},{b[4]:.1f}]")
