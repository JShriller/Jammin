#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

struct Point3D {
    float x, y, z;
};

// Global states
std::vector<Point3D> surface_mesh;
int resolution = 50;
float rotationX = -45.0f, rotationY = 45.0f;
double lastX, lastY;
bool firstMouse = true;
int server_fd = -1;

// Sets up a non-blocking local networking socket to listen for Python data
void setup_networking() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Set socket to non-blocking so it doesn't freeze our 60FPS render loop
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(12345); // Listen on local port 12345

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
}

// Checks if Python has sent over a new batch of 3D math coordinates
void check_for_new_data() {
    int new_socket = accept(server_fd, nullptr, nullptr);
    if (new_socket < 0) return; // No new data available right now

    std::string full_data = "";
    char buffer[4096];
    ssize_t bytes_read;
    
    // Read the incoming coordinate stream completely
    while ((bytes_read = read(new_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        full_data += buffer;
    }
    close(new_socket);

    if (full_data.empty()) return;

    // Parse the data stream: format "x,y,z;x,y,z;..."
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
        resolution = static_cast<int>(std::sqrt(static_cast<double>(surface_mesh.size())));
        std::cout << "Received " << surface_mesh.size() << " mesh points\n";
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Calculator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    setup_networking();

    while (!glfwWindowShouldClose(window)) {
        check_for_new_data();

        glClearColor(0.08f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-10.0, 10.0, -10.0, 10.0, -50.0, 50.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

        glColor3f(0.2f, 0.85f, 0.75f);
        glBegin(GL_LINES);
        for (int i = 0; i < resolution; ++i) {
            for (int j = 0; j < resolution; ++j) {
                const std::size_t index = static_cast<std::size_t>(i * resolution + j);
                if (index >= surface_mesh.size()) continue;

                const Point3D& point = surface_mesh[index];
                if (j + 1 < resolution && index + 1 < surface_mesh.size()) {
                    const Point3D& right = surface_mesh[index + 1];
                    glVertex3f(point.x, point.y, point.z);
                    glVertex3f(right.x, right.y, right.z);
                }
                if (i + 1 < resolution && index + resolution < surface_mesh.size()) {
                    const Point3D& next_row = surface_mesh[index + resolution];
                    glVertex3f(point.x, point.y, point.z);
                    glVertex3f(next_row.x, next_row.y, next_row.z);
                }
            }
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    close(server_fd);
    glfwTerminate();
    return 0;
}