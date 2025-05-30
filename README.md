# offscreen

Multi-platform OpenGL offscreen tester.

## Build & run

```
git clone https://github.com/kintel/offscreen.git
cd offscreen
git submodule update --init
```

```
cmake .
make
./offscreen -h
```


## Platforms

* macOS
   * Defaults to non-wrangled OpenGL
   * Supports: NSOpenGL, CGL
   * OpenGL 2, OpenGL 4
* Linux
   * Supports: EGL
   * Supports multi-GPU
   * OpenGL 2-4
   * GLES2-3
* Raspberry Pi
   * Only OpenGL 2 for now
* Windows
   * Supports: WGL
   * OpenGL2-4

## TODO

* Use EGL by default, and fall back to GLX. This is apparently needed in some places. Google it and look into it.
* Qt-backed GL context
* macOS: Look into "warning gl.h and gl3.h are both included"
* Support forward-compatible contexts
* Windows: Support GLES
* WASM

## Prerequisites

* cmake
* glfw3
* OpenGL, GLU
* libgbm (Linux specific)

**Linux (apt)**

```bash
sudo apt install cmake libglfw3-dev libgl1-mesa-dev libegl1-mesa-dev libgbm-dev libglu1-mesa-dev
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
./offscreen --width 640 --height 480 --context cgl --opengl 2 --mode modern -o out.png
./offscreen --width 640 --height 480 --context nsopengl --opengl 2 --mode immediate -o out.png
```

**OpenGL 3+**

```bash
./offscreen --width 640 --height 480 --context cgl --opengl 3.2 --mode modern -o out.png
./offscreen --width 640 --height 480 --context nsopengl --opengl 3.2 --mode modern -o out.png
```

### Linux OpenGL 3 compatibility mode

```bash
./offscreen --context egl --opengl 3 --profile compatibility --mode immediate -o out.png
```

### Linux OpenGL 4 core

```bash
./offscreen --context egl --opengl 4 --profile core --mode modern -o out.png
```
### Linux Choose GPU

```bash
./offscreen --context egl --gpu /dev/dri/renderD128 -o out.png
```

```bash
./offscreen --context egl --gpu /dev/dri/renderD129 -o out.png
```

### GLES

```bash
./offscreen --gles 2
```

## Running Tests

First, ensure you have built the project as described in the 'Build & run' section above.

Tests are run from the build directory using `ctest -C <BUILD_TYPE>`. For example, to run tests for the Release build type:

```bash
ctest -C Release
```


## Context Notes

### Linux

#### GPU Permissions

Sometimes, users don't have access to GPUs and thus cannot render. A common cause for this is too strict permission on /dev/dri/renderD128. Usually, this node would have 660 permissions with RW permissions given to a special group (e.g. render or video). Make sure that users are added to this group to allow using the GPU.

This may cause a fallback to a software renderer.

Can be validating by checking if running under sudo changes the behavior.

#### Default EGL Device

The default EGL display generally only works when executing in a native desktop session.
Running on ssh or using screen sharing (unless it's a VNC-style literal sharing of an active session), the default display may not return a valid EGL display. Symptom: eglInitialize() generates EGL_NOT_INITIALIZED.

In these cases, we need to search for a proper display using eglQueryDevicesEXT() and eglGetPlatformDisplayEXT().

#### Choosing a custom GPU

TODO: How to query DRM nodes.
