"""Builds 'Instincts - The BeastMaster <version>.zip' from the parts of this repo.

Inputs (all must exist; the script stops otherwise):
  - merged plugin  : MERGED_ESP (built by BeastMaster/esp Spriggit YAML -> spriggit deserialize)
  - base archive   : BeastMaster/assets/Instincts - The BeastMaster.bsa  (Kheree's original BSA, renamed to match the plugin)
  - loose scripts  : Scripts/ from the Instincts - Fixes build (compiled .pex + .psc sources) and BeastHarmony_*.pex
  - DLL + licences : Beast-Harmony build output
"""
import os, sys, zipfile, hashlib

VERSION = "0.2.1"
NAME = "Instincts - The BeastMaster"
REPO = r"E:\WorkSpace\Instincts-2026"
OUT = os.path.join(REPO, "release", f"{NAME} {VERSION}.zip")
MERGED_ESP = r"E:\WorkSpace\_merge\out\Instincts - The BeastMaster.esp"
BSA = os.path.join(REPO, "BeastMaster", "assets", f"{NAME}.bsa")
DLL = r"E:\WorkSpace\ng-build\build\release\out\BeastHarmony\BeastHarmony.dll"
PREV = os.path.join(REPO, "release", "superseded", "_unpacked")  # unpacked 0.1.0 parts (scripts, sounds, icon)
DOCS = os.path.join(REPO, "BeastMaster", "docs")

entries = {}  # zip path -> source path


def add(zpath, src):
    if not os.path.isfile(src):
        sys.exit("MISSING " + src)
    if zpath in entries:
        sys.exit("DUPLICATE " + zpath)
    entries[zpath] = src


add(f"{NAME}.esp", MERGED_ESP)
add(f"{NAME}.bsa", BSA)
add("SKSE/Plugins/BeastHarmony.dll", DLL)

# scripts, sounds, icon, licences carried over unchanged from the 0.1.0 parts
for sub in ("Scripts", "Sound", "Interface"):
    for dp, dn, fn in os.walk(os.path.join(PREV, sub)):
        for f in sorted(fn):
            src = os.path.join(dp, f)
            add(os.path.relpath(src, PREV).replace("\\", "/"), src)
add("SKSE/Plugins/BeastHarmony-LICENSE.txt", os.path.join(PREV, "SKSE", "Plugins", "BeastHarmony-LICENSE.txt"))
add("SKSE/Plugins/BeastHarmony-THIRD-PARTY.txt", os.path.join(PREV, "SKSE", "Plugins", "BeastHarmony-THIRD-PARTY.txt"))

for f in ("README.txt", "NOTICE.txt", "CHANGELOG.txt"):
    add(f, os.path.join(DOCS, f))

os.makedirs(os.path.dirname(OUT), exist_ok=True)
if os.path.exists(OUT):
    os.remove(OUT)
with zipfile.ZipFile(OUT, "w", zipfile.ZIP_DEFLATED) as z:
    for zp, src in entries.items():
        z.write(src, zp)
print(len(entries), "files ->", OUT)
print("sha256", hashlib.sha256(open(OUT, "rb").read()).hexdigest())
