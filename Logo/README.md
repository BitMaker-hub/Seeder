# First-splash artwork

`seeder-splash.png` is the lossless 200x88 RGB565 master for the first splash
screen. Regenerate the firmware array and its legacy copy with:

```sh
python3 -m pip install pillow
python3 scripts/gen_splash_logo.py
python3 scripts/gen_splash_logo.py --check
```

The generator updates only `seeder_logo` in `src/Lib/images.h` and
`BLACK_small` in `Logo/BLACK_small.c`. It preserves their existing dimensions,
pixel order and array formatting. The separate credit bitmaps and
`scripts/gen_splash_small.py` are unrelated to this first-splash asset.

The master comes from the original `BLACK_small` RGB565 pixels at upstream
commit `ba6e9ebe226222cbb04415a635a934eeae372dab`. That bitmap was generated
from `BLACK_small.jpg` by ImageConverter 565 v2.1 in 2022 and copied into
`images.h` as `seeder_logo`.

The spelling correction from `SOVEREING` to `SOVEREIGN` reuses the original
N and G glyph pixels. Within the half-open rectangle `(167, 44, 189, 56)`,
the original 10px N, 1px gap and 11px G are reordered as G, gap, N. No glyph
is rescaled or redrawn; every pixel outside that rectangle is unchanged.
The original green palette, logo and attribution artwork are preserved.

The older JPEG artwork and `LOGO SEEDER.c` remain historical references;
the PNG is the canonical source for the firmware splash.
