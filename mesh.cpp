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
#include "file_reader.hpp"

/* Debug mode. */
bool debug_mode = false;

/* Perspective toggle. */
bool perspective = false;

/* Globals */
/** Window width. */
int win_width  = 800;
/** Window height. */
int win_height = 600;

glm::quat currentRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Global rotation quaternion

/** Program variable. */
int program;
/** Vertex array and buffer objects. */
unsigned int VAO, VBO, EBO;

/* Object content */
objContent obj;
float bg_color[3] = {0.2, 0.3, 0.3};

/* Wireframe toggle. */
bool wireframe = false;

/* Primitive type. */
int type_primitive = GL_TRIANGLES;

/** Uniform locations. */
GLuint scaleLoc;
GLuint projectionLoc;
GLuint viewLoc;
GLuint modelCenterLoc;
GLuint userOffsetLoc;
GLuint rotationLoc;

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
    }
    if (debug_mode) {
        printf("Offset: (%f, %f, %f)\n", userOffset[0], userOffset[1], userOffset[2]);
    }
    glUniform3f(userOffsetLoc, userOffset[0], userOffset[1], userOffset[2]);
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
    glUniform3f(userOffsetLoc, userOffset[0], userOffset[1], userOffset[2]);
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

    glUniform1f(scaleLoc, scale);
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
    glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, &rotationMatrix[0][0]);

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

    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    glClearColor(bg_color[0], bg_color[1], bg_color[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAO);
    // trocar o gl_triangles pra fazer o mesh view
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    glDrawElements(type_primitive, obj.n_faceElements, GL_UNSIGNED_INT, 0);

    glutSwapBuffers();
}

void updateScale() {
    scale = 2.0f / maxExtent;
    scale *= scaling_factor;
    glUniform1f(scaleLoc, scale);
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
 * Init vertex data.
 *
 * Defines the coordinates for vertices, creates the arrays for OpenGL.
 */
void initData(const objContent obj)
{
    // Vertex array.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, obj.n_vertices * sizeof(float), obj.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, obj.n_faceElements * sizeof(int), obj.faceElements, GL_STATIC_DRAW);

    // Set attributes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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
    const char *vertex_code = read_file_into_string("shaders/vertex_shader.glsl");
    if (!vertex_code) { 
        fprintf(stderr, "Failed to load vertex shader\n"); 
        exit(1); 
    }

    const char *fragment_code = read_file_into_string("shaders/fragment_shader.glsl");
    if (!fragment_code) { 
        fprintf(stderr, "Failed to load fragment shader\n"); 
        exit(1); 
    }

    program = createShaderProgram(vertex_code, fragment_code);
    free((void*)vertex_code);
    free((void*)fragment_code);
    scaleLoc = glGetUniformLocation(program, "scale");
    projectionLoc = glGetUniformLocation(program, "uProjection");
    viewLoc = glGetUniformLocation(program, "uView");
    modelCenterLoc = glGetUniformLocation(program, "modelCenter");
    userOffsetLoc = glGetUniformLocation(program, "userOffset");
    rotationLoc = glGetUniformLocation(program, "uRotation");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: %s <obj_file>\n", argv[0]);
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

    glm::vec3 modelCenter;

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

	initData(obj);

	// Create shaders.
	initShaders();
	glUseProgram(program);
	updateScale();

    glUniform3f(modelCenterLoc, modelCenter.x, modelCenter.y, modelCenter.z);
    glUniform3f(userOffsetLoc, 0.0f, 0.0f, 0.0f);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(keyboardSpecial);
    glutMainLoop();

    return 0;
}