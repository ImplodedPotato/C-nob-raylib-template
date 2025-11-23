# C-nob-raylib-template

A simple template for [raylib](https://www.raylib.com/), using the [nob.h](https://github.com/tsoding/nob.h) build system.

## To Build:

### Simple: 

```sh
cc nob.c -o nob
./nob
```

### All Build Options:
- `r` / `run`: Runs to your code after building it.
    - When compined with `web`, it will run `python3 -m https.server` in the build folder.
- `web`: Build for the web
- `b` / `build`: Does nothing, as it always builds
- `clean`: Cleans up the `build/` folder
- `cleanall`: The same as `clean`, but also running `make clean` for raylib
    - This is necesary when compiling to a diffrent target than the last compile
- `h` / `help`: Show this help/usage message
