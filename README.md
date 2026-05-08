# OpenGL OBJ Viewer

This project is a simple 3D `.obj` viewer implemented in C++ using OpenGL. It supports:

* Scaling around the model center
* Translation via keyboard
* Quaternion-based trackball rotation (mouse drag)
* Perspective / orthographic projection toggle
* Solid, wireframe and point visualization modes

---

## Dependencies

You need the following libraries installed:

* OpenGL (Open Graphics Library)
* GLEW (OpenGL Extension Wrangler Library)
* FreeGLUT (OpenGL Utility Toolkit)
* GLM (OpenGL Mathematics)

### Ubuntu / Debian

Run:

```bash
sudo apt update
sudo apt install build-essential libglew-dev freeglut3-dev libglm-dev
```

---

## Project Structure

```
mesh/
│
├── mesh.cpp
├── obj_reader.hpp
├── obj_reader.cpp
├── shaders/
│   ├── vertex_shader.glsl
│   └── fragment_shader.glsl
├── lib/
│   └── utils.h
│   └── utils.cpp
└── models/
    └── example.obj
    └── bunny.obj
```

---

## Compilation

Use the Makefile. Just type the command:

```bash
make
```

---

## Running the Program

Run the program by passing an `.obj` file:

```bash
./mesh models/bunny.obj
```

---

## Help Menu

You can also run:

```bash
./mesh --help
```

---

## 🎮 Controls

| Input            | Action                            |
| ---------------- | --------------------------------- |
| **Mouse Drag**   | Rotate object (trackball)         |
| **Scroll Wheel** | Zoom (scale)                      |
| **W / S**        | Move forward / backward           |
| **Arrow Keys**   | Move left / right / up / down     |
| **V**            | Toggle wireframe mode             |
| **P**            | Toggle points / triangles         |
| **T**            | Toggle perspective / orthographic |
| **D**            | Toggle debug output               |
| **ESC**          | Exit                              |

---

## Author

Nicolas Barbieri Sousa