#define GLM_ENABLE_EXPERIMENTAL

#include <stdio.h>
#include <cstring>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "lib/utils.h"
#include <cfloat>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "file_reader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

/* Debug mode. */
bool debug_mode = false;

/* Perspective toggle. */
bool perspective = false;

/* Globals */
/** Window width. */
int win_width  = 1200;
/** Window height. */
int win_height = 800;

glm::quat currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Global rotation quaternion
glm::vec3 modelCenter;

bool phong = 1;

/** Program variable. */
int program, program_orthographic, program_cylindrical, program_spherical;

/** Vertex array and buffer objects. */
unsigned int VAO, VBO, NBO, EBO;

/** Texture information */
unsigned int texture;
int width, height, nrChannels;

/* Object content */
objContent obj;
float bg_color[3] = {0.2, 0.3, 0.3};

/* Wireframe toggle. */
bool wireframe = false;

/* Primitive type. */
int type_primitive = GL_TRIANGLES;

bool dragging = false;
glm::vec3 lastPoint;

/* User offset for translation. */
float userOffset[3] = {0.0f, 0.0f, 0.0f};
float scale;
float scaling_factor = 0.5f;
float minX, maxX, minY, maxY, minZ, maxZ, maxExtent;

/* Functions. */
void display(void);
void reshape(int, int);
void keyboard(unsigned char, int, int);
void keyboardSpecial(int, int, int);
void mouse(int, int, int, int);
void initData(const objContent obj);
void initShaders(void);
void updateScale(void);
glm::vec2 screenToNDC(int x, int y);
glm::vec3 projectToSphere(float x, float y);

void motion(int x, int y) {
    if (!dragging) return;

    glm::vec2 ndc = screenToNDC(x, y);
    glm::vec3 currentPoint = projectToSphere(ndc.x, ndc.y);

    glm::vec3 axis = glm::cross(lastPoint, currentPoint);
    float dot = glm::clamp(
        glm::dot(lastPoint, currentPoint),
        -1.0f,
        1.0f
    );

    float angle = acos(dot);

    if (glm::length(axis) > 1e-5) {
        glm::quat delta = glm::angleAxis(angle, glm::normalize(axis));
        currentRotation = delta * currentRotation;
    }

    lastPoint = currentPoint;
    glutPostRedisplay();
}

glm::vec2 screenToNDC(int x, int y) {
    return glm::vec2(
        (2.0f * x - win_width) / win_width,
        (win_height - 2.0f * y) / win_height
    );
}

glm::vec3 projectToSphere(float x, float y) {
    float z2 = 1.0f - x*x - y*y;
    float z = z2 > 0 ? sqrt(z2) : 0.0f;
    return glm::normalize(glm::vec3(x, y, z));
}

void generate_uniforms()
{
    glUseProgram(program);
    
    glUniform2f(glGetUniformLocation(program, "x_lim"), minX, maxX);
    glUniform2f(glGetUniformLocation(program, "y_lim"), minY, maxY);
    glUniform3f(glGetUniformLocation(program, "modelCenter"), modelCenter.x, modelCenter.y, modelCenter.z);
    glUniform3f(glGetUniformLocation(program, "userOffset"), userOffset[0], userOffset[1], userOffset[2]);
    glUniform1f(glGetUniformLocation(program, "scale"), scale);
    glUniform1i(glGetUniformLocation(program, "ourTexture"), 0);
    glUniform1i(glGetUniformLocation(program, "phong"), phong);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 27: // ESC
            exit(0);
            break;
        // Visualizar wireframe
        case 'v':
        case 'V':
            wireframe = !wireframe;
            break;
        // Visualizar pontos
        case 'p':
        case 'P':
            if (type_primitive == GL_TRIANGLES) {
                type_primitive = GL_POINTS;
            } else {
                type_primitive = GL_TRIANGLES;
            }
            break;
        // Translação em Z
        case 'w':
        case 'W':
            userOffset[2] -= 0.03f; // Move forward
            break;
        case 's':
        case 'S':
            userOffset[2] += 0.03f; // Move backward
            break;
        case 'd':
        case 'D':
            debug_mode = !debug_mode;
            break;
        // Toggle perspective
        case 't':
        case 'T':
            perspective = !perspective;
            break;
        case '1':
            phong = !phong;
            generate_uniforms();
            break;
        case '2':
            program = program_orthographic;
            generate_uniforms();
            break;
        case '3':
            program = program_cylindrical;
            generate_uniforms();
            break;
        case '4':
            program = program_spherical;
            generate_uniforms();
            break;
    }
    if (debug_mode) {
        printf("Offset: (%f, %f, %f)\n", userOffset[0], userOffset[1], userOffset[2]);
    }
    glUniform3f(glGetUniformLocation(program, "userOffset"), userOffset[0], userOffset[1], userOffset[2]);
    glutPostRedisplay();
}

void keyboardSpecial(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            userOffset[1] += 0.01f; // Move up
            break;
        case GLUT_KEY_DOWN:
            userOffset[1] -= 0.01f; // Move down
            break;
        case GLUT_KEY_LEFT:
            userOffset[0] -= 0.01f; // Move left
            break;
        case GLUT_KEY_RIGHT:
            userOffset[0] += 0.01f; // Move right
            break;
    }
    if (debug_mode) {
        printf("Offset: (%f, %f, %f)\n", userOffset[0], userOffset[1], userOffset[2]);
    }
    glUniform3f(glGetUniformLocation(program, "userOffset"), userOffset[0], userOffset[1], userOffset[2]);
    glutPostRedisplay();
}

// Trackball
void mouse(int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN) {
                dragging = true;
                glm::vec2 ndc = screenToNDC(x, y);
                lastPoint = projectToSphere(ndc.x, ndc.y);
            } else {
                dragging = false;
            }
            break;
        case GLUT_RIGHT_BUTTON:
            break;
        case 3: // Scroll up
            scale *= 1.06f; // Scale down to 80% of the window funciona
            break;
        case 4: // Scroll down
            scale *= 0.93f; // Scale down to 80% of the window funciona
            break;
    }

    if (debug_mode) {
        printf("Scaling factor: %f\n", scaling_factor);
    }

    glUniform1f(glGetUniformLocation(program, "scale"), scale);
    glutPostRedisplay();
}

/** 
 * Drawing function.
 *
 * Draws primitive.
 */
void display()
{
    glUseProgram(program);
    glm::mat4 rotationMatrix = glm::toMat4(currentRotation);
    glUniformMatrix4fv(glGetUniformLocation(program, "uRotation"), 1, GL_FALSE, &rotationMatrix[0][0]);

    glm::mat4 projection;
    
    // Set the projection and view matrices
    if (perspective) {
    projection = glm::perspective(glm::radians(45.0f), (float)win_width / (float)win_height, 0.1f, 100.0f);
    } else {
        float aspect = (float)win_width / (float)win_height;
        projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
    }
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), // Camera position
                                 glm::vec3(0.0f, 0.0f, 0.0f), // Look at point
                                 glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector

    glUniformMatrix4fv(glGetUniformLocation(program, "uProjection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(program, "uView"), 1, GL_FALSE, &view[0][0]);

    // initializing model matrix for color as identity matrix
    glm::mat4 model = glm::mat4(1.0);
    glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // Object color.
    glUniform3f(glGetUniformLocation(program, "objectColor"), 0.5, 0.1, 0.1);
    // Light color.
    glUniform3f(glGetUniformLocation(program, "lightColor"), 1.0, 1.0, 1.0);
    // Light position.
    glUniform3f(glGetUniformLocation(program, "lightPosition"), 1.0, 0.0, 2.0);
    // Camera position.
    glUniform3f(glGetUniformLocation(program, "cameraPosition"), 0.0, 0.0, 3.0);

    glClearColor(bg_color[0], bg_color[1], bg_color[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);
    // trocar o gl_triangles pra fazer o mesh view
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(type_primitive, obj.n_faceElements, GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

void updateScale() {
    scale = 2.0f / maxExtent;
    scale *= scaling_factor;
    glUniform1f(glGetUniformLocation(program, "scale"), scale);
}

/**
 * Reshape function.
 *
 * Called when window is resized.
 *
 * @param width New window width.
 * @param height New window height.
 */
void reshape(int width, int height)
{
    win_width = width;
    win_height = height;
    glViewport(0, 0, width, height);
    updateScale();
    glutPostRedisplay();
}

/** 
 * Load texture data.
 *
 * Creates the OpenGL object and unloads the texture.
 */
unsigned int loadTex(const char* path_to_img)
{
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path_to_img, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return texture;
}

/**
 * Init vertex data.
 *
 * Defines the coordinates for vertices, creates the arrays for OpenGL.
 */
void initData(const objContent obj)
{
    // Vertex array.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Texture
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);

    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, obj.n_vertices * sizeof(float), obj.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &NBO);
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, obj.n_vertexNormals * sizeof(float), obj.vertexNormals, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.n_faceElements * sizeof(int), obj.faceElements, GL_STATIC_DRAW);

    // Set vertex attribute
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set normal attribute (one normal per vertex)
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    // Unbind Vertex Array Object.
    glBindVertexArray(0);
}

/** Create program (shaders).
 * 
 * Compile shaders and create the program.
 */
void initShaders()
{
    // Request a program and shader slots from GPU
    const char *vertex_orthographic_code = read_file_into_string("shaders/vertex_orthographic.glsl");
    if (!vertex_orthographic_code) { 
        fprintf(stderr, "Failed to load vertex shader\n"); 
        exit(1); 
    }

    const char *vertex_cylindrical_code = read_file_into_string("shaders/vertex_cylindrical.glsl");
    if (!vertex_cylindrical_code) { 
        fprintf(stderr, "Failed to load vertex shader\n"); 
        exit(1); 
    }

    const char *vertex_spherical_code = read_file_into_string("shaders/vertex_spherical.glsl");
    if (!vertex_spherical_code) { 
        fprintf(stderr, "Failed to load vertex shader\n"); 
        exit(1); 
    }

    const char *fragment_code = read_file_into_string("shaders/fragment.glsl");
    if (!fragment_code) { 
        fprintf(stderr, "Failed to load fragment shader\n"); 
        exit(1); 
    }

    program_orthographic = createShaderProgram(vertex_orthographic_code, fragment_code);
    program_cylindrical = createShaderProgram(vertex_cylindrical_code, fragment_code);
    program_spherical = createShaderProgram(vertex_spherical_code, fragment_code);

    program = program_orthographic;

    free((void*)vertex_orthographic_code);
    free((void*)vertex_cylindrical_code);
    free((void*)vertex_spherical_code);
    free((void*)fragment_code);
    generate_uniforms();
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <obj_file> <texture_file>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        printf("Usage: %s <obj_file>\n", argv[0]);
        printf("Controls:\n");
        printf("  W/S: Move object forward/backward\n");
        printf("  Arrow Keys: Move object left/right/up/down\n");
        printf("  Mouse Drag: Rotate model\n");
        printf("  Scroll: Scale object\n");
        printf("  V: Toggle wireframe/solid mode\n");
        printf("  P: Toggle points/triangles mode\n");
        printf("  T: Toggle perspective/orthographic view\n");
        printf("  D: Toggle debug mode (prints offsets and scaling factor)\n");
        printf("  ESC: Exit\n");

        return 0;
    }

    obj = readfile(argv[1]);

    // Compute bounding box
    minX = minY = minZ = FLT_MAX;
    maxX = maxY = maxZ = -FLT_MAX;
    for (int i = 0; i < obj.n_vertices; i += 3) {
        float x = obj.vertices[i], y = obj.vertices[i+1], z = obj.vertices[i+2];
        minX = fmin(minX, x); maxX = fmax(maxX, x);
        minY = fmin(minY, y); maxY = fmax(maxY, y);
        minZ = fmin(minZ, z); maxZ = fmax(maxZ, z);
    }
    maxExtent = fmax(maxX - minX, fmax(maxY - minY, maxZ - minZ));
    modelCenter = glm::vec3((minX+maxX)/2, (minY+maxY)/2, (minZ+maxZ)/2);

    glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(win_width,win_height);
	glutCreateWindow(argv[0]);
	glewExperimental = GL_TRUE;
	glewInit();
    glEnable(GL_DEPTH_TEST);

    texture = loadTex(argv[2]);

	initData(obj);

	// Create shaders.
	initShaders();
	glUseProgram(program);
	updateScale();

    generate_uniforms();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(keyboardSpecial);
    glutMainLoop();

    return 0;
}
