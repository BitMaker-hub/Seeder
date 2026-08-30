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

env.Append(CPPDEFINES=[("SEEDER_COMMIT", env.StringifyMacro(commit))])
print("Version: %s  commit: %s" % (env.GetProjectOption("custom_version", "?"), commit))
