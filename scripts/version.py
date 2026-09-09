# Inyecta la versión y el commit en el binario. Es una etiqueta de
# conveniencia para saber qué lleva cada placa, NO una prueba de nada:
# un firmware manipulado puede imprimir lo que quiera. La verificación
# de verdad se hace desde fuera (ver SECURITY.md).
Import("env")
import subprocess

def git(*args):
    try:
        return subprocess.check_output(["git"] + list(args),
                                       stderr=subprocess.DEVNULL, text=True).strip()
    except Exception:
        return ""

commit = git("rev-parse", "--short=7", "HEAD") or "nogit"
if git("status", "--porcelain"):
    commit += "*"          # el árbol tenía cambios sin comitear

# La versión se inyecta AQUÍ y no como -D en platformio.ini para que sólo
# exista en un sitio. Estaban las dos cosas a la vez y al publicar la 2.1.1
# se subió custom_version pero no el -D, así que el firmware siguió diciendo
# V2.1.0 en el arranque: el número que se ve venía del que nadie tocaba.
version = env.GetProjectOption("custom_version", "") or "dev"

env.Append(CPPDEFINES=[("SEEDER_VERSION", env.StringifyMacro(version)),
                       ("SEEDER_COMMIT",  env.StringifyMacro(commit))])
print("Version: %s  commit: %s" % (version, commit))
