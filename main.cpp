#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cmath>
#include <algorithm>

struct Point3D {
    float x, y, z;
};

std::vector<Point3D> surface_mesh;
int resolution = 50;

// Explicit Mouse Camera Transforms
float rotationX = -45.0f;
float rotationY = 45.0f;
double lastX = 400.0, lastY = 300.0;
bool isDragging = false;
bool is2DMode = false; 
int server_fd = -1;

void setup_networking() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(12345);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
}

void check_for_new_data() {
    int new_socket = accept(server_fd, nullptr, nullptr);
    if (new_socket < 0) return;

    std::string full_data = "";
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = recv(new_socket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_read] = '\0';
        full_data += buffer;
    }
    close(new_socket);
    if (full_data.empty()) return;

    if (full_data == "TOGGLE_2D") { is2DMode = true; return; }
    if (full_data == "TOGGLE_3D") { is2DMode = false; return; }

    std::vector<Point3D> new_mesh;
    std::stringstream ss(full_data);
    std::string token;
    while (std::getline(ss, token, ';')) {
        if (token.empty()) continue;
        std::stringstream coord_ss(token);
        std::string xs, ys, zs;
        if (std::getline(coord_ss, xs, ',') && std::getline(coord_ss, ys, ',') && std::getline(coord_ss, zs, ',')) {
            new_mesh.push_back({std::stof(xs), std::stof(ys), std::stof(zs)});
        }
    }
    if (!new_mesh.empty()) {
        surface_mesh = new_mesh;
        resolution = std::sqrt(surface_mesh.size());
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!isDragging || is2DMode) return;
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    lastX = xpos; lastY = ypos;
    float sensitivity = 0.3f;
    rotationY += xoffset * sensitivity;
    rotationX -= yoffset * sensitivity;
    if (rotationX > 89.0f) rotationX = 89.0f;
    if (rotationX < -89.0f) rotationX = -89.0f;
}

// NEW: Draws a crisp 2D Desmos Grid Sheet Overlay (Axes + Ticks)
void draw_cartesian_grid(float aspect) {
    glDisable(GL_DEPTH_TEST);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-6.0 * aspect, 6.0 * aspect, -6.0, 6.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 1. Draw Subdivision Grid Lines (Light Slate Gray)
    glColor3f(0.2f, 0.22f, 0.28f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float i = -15.0f; i <= 15.0f; i += 1.0f) {
        glVertex2f(i, -10.0f); glVertex2f(i, 10.0f); // Verticals
        glVertex2f(-20.0f, i); glVertex2f(20.0f, i); // Horizontals
    }
    glEnd();

    // 2. Draw Primary Core X and Y Axis Lines (Solid White)
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(-20.0f, 0.0f); glVertex2f(20.0f, 0.0f); // X-Axis
    glVertex2f(0.0f, -10.0f); glVertex2f(0.0f, 10.0f); // Y-Axis
    glEnd();
    glEnable(GL_DEPTH_TEST);
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Desmos Studio Viewport", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glEnable(GL_DEPTH_TEST);

    // Build default starting sheet array
    for(int i=0; i<50; ++i) {
        for(int j=0; j<50; ++j) {
            surface_mesh.push_back({-5.0f+(10.0f*i/50.0f), -5.0f+(10.0f*j/50.0f), 0.0f});
        }
    }
    setup_networking();

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
        check_for_new_data();

        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;

        if (is2DMode) {
            // Draw Cartesian sheets background layer first
            draw_cartesian_grid(aspect);

            // RENDER 2D DESMOS GRAPH CONTOUR DENSITY FILL
            glBegin(GL_QUADS);
            for (int i = 0; i < resolution - 1; ++i) {
                for (int j = 0; j < resolution - 1; ++j) {
                    Point3D p1 = surface_mesh[i * resolution + j];
                    Point3D p2 = surface_mesh[(i + 1) * resolution + j];
                    Point3D p3 = surface_mesh[(i + 1) * resolution + (j + 1)];
                    Point3D p4 = surface_mesh[i * resolution + (j + 1)];

                    // Compute height density color mapping index
                    float n_z = (p1.z + 2.0f) / 4.0f;
                    n_z = std::max(0.0f, std::min(1.0f, n_z));

                    // Shading rule: High areas glow bright, neutral points dim down
                    glColor3f(n_z * 0.9f, 0.2f, 1.0f - n_z);
                    glVertex2f(p1.x, p1.y);
                    glVertex2f(p2.x, p2.y);
                    glVertex2f(p3.x, p3.y);
                    glVertex2f(p4.x, p4.y);
                }
            }
            glEnd();
        } else {
            // 3D VIEWPORT PERSPECTIVE MODALITY PIPELINE
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            float fov = 45.0f, top = 0.1f * std::tan(fov * 0.5f * M_PI / 180.0f), right = top * aspect;
            glFrustum(-right, right, -top, top, 0.1f, 100.0f);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, -15.0f);
            glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
            glRotatef(rotationY, 0.0f, 0.0f, 1.0f);

            glLineWidth(1.5f);
            glBegin(GL_LINES);
            for (int i = 0; i < resolution - 1; ++i) {
                for (int j = 0; j < resolution - 1; ++j) {
                    Point3D p1 = surface_mesh[i * resolution + j];
                    Point3D p2 = surface_mesh[(i + 1) * resolution + j];
                    Point3D p3 = surface_mesh[i * resolution + (j + 1)];

                    float n_z = (p1.z + 2.0f) / 4.0f;
                    n_z = std::max(0.0f, std::min(1.0f, n_z));
                    glColor3f(n_z, 0.7f - (n_z * 0.4f), 1.0f - n_z);

                    glVertex3f(p1.x, p1.y, p1.z); glVertex3f(p2.x, p2.y, p2.z);
                    glVertex3f(p1.x, p1.y, p1.z); glVertex3f(p3.x, p3.y, p3.z);
                }
            }
            glEnd();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    close(server_fd);
    glfwTerminate();
    return 0;
}