# OpenGL OBJ Viewer

This project is a simple 3D `.obj` viewer implemented in C++ using OpenGL.

The original viewer (`mesh`) supports:

* Scaling around the model center
* Translation via keyboard
* Quaternion-based trackball rotation (mouse drag)
* Perspective / orthographic projection toggle
* Solid, wireframe and point visualization modes

The extended viewer (`mesh2`) additionally supports:

* Texture mapping
* Orthographic, cylindrical and spherical texture-coordinate generation
* Phong lighting toggle

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

```text
mesh/
│
├── mesh.cpp
├── mesh2.cpp
├── file_reader.hpp
├── file_reader.cpp
├── shaders/
│   ├── fragment_shader.glsl
│   ├── vertex_shader.glsl
│   ├── vertex_orthographic.glsl
│   ├── vertex_cylindrical.glsl
│   ├── vertex_spherical.glsl
│   └── fragment.glsl
├── lib/
│   ├── utils.h
│   ├── utils.cpp
│   └── stb_image.h
└── models/
    ├── cow.obj
    ├── bunny.obj
    └── ...
```

---

## Compilation

Use the Makefile. Just type:

```bash
make
```

---

## Running the Program

### mesh

Run the program by passing an `.obj` file:

```bash
./mesh models/bunny.obj
```

### mesh2

Run the program by passing an `.obj` file and a texture file:

```bash
./mesh2 models/bunny.obj textures/wall.jpg
```

---

## Help Menu

You can also run:

```bash
./mesh --help
```

---

## Controls

### mesh

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

### mesh2-only commands

| Input | Action                          |
| ----- | ------------------------------- |
| **1** | Toggle Phong lighting           |
| **2** | Orthographic texture projection |
| **3** | Cylindrical texture projection  |
| **4** | Spherical texture projection    |

---

## Author

Nicolas Barbieri Sousa
