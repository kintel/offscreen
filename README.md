# offscreen

Multi-platform OpenGL offscreen tester.

## TODO

* Use EGL by default, and fall back to GLX. This is apparently needed in some places. Google it and look into it.
* Qt-backed GL context
* Pure GLES mode?
* Look into "warning gl.h and gl3.h are both included"
* Windows support


## Prerequisites

* cmake
* glfw3
* OpenGL

**Linux (apt)**

```bash
sudo apt install cmake libglfw3-dev libgl1-mesa-dev libegl1-mesa-dev
```

## Examples

### macOS

macOS is different: Apple doesn't provide a compatibility profile for OpenGL 3+, so we have to request an OpenGL 2 context in order to use immediate-mode GL functions.

Furthermore, macOS uses weak linking for its OpenGL library, which essentially eliminates the need for any _getprocaddress_ aka. OpenGL extension wrangling. OpenGL 3+ functions are declared in the `OpenGL/gl3.h` header. For cross-platform compatibility, it is, however, possible to use dlopen()/dlsym() to lookup OpenGL functions. This should yield the same function pointers as when using the regular, weakly linked, library.

**GLFW**

```bash
./offscreen --width 640 --height 480 --context glfw --opengl 3.2 --mode modern
./offscreen --width 640 --height 480 --context glfw --invisible --opengl 3.2 --mode modern
```

**OpenGL 2**

```bash
./offscreen --width 640 --height 480 --context cgl --opengl 2 --mode modern
./offscreen --width 640 --height 480 --context nsopengl --opengl 2 --mode immediate
```

**OpenGL 3+**

```bash
./offscreen --width 640 --height 480 --context cgl --opengl 3.2 --mode modern
./offscreen --width 640 --height 480 --context nsopengl --opengl 3.2 --mode modern
```

### Linux OpenGL 3 compatibility mode

```bash
./offscreen --context egl --opengl 3 --profile compatibility --mode immediate
```

### Linux OpenGL 4 core

```bash
./offscreen --context egl --opengl 4 --profile core --mode modern
```
### Linux Choose GPU

```bash
./offscreen --context egl --gpu /dev/dri/renderD128
```

```bash
./offscreen --context egl --gpu /dev/dri/renderD129
```
