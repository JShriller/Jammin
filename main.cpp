#include <GLFW/glfw3.h>

#include <arpa/inet.h>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

struct Point2D {
    float x;
    float y;
};

struct Point3D {
    float x;
    float y;
    float z;
};

constexpr int kPort = 12345;

int server_fd = -1;
bool is2DMode = true;
std::vector<Point2D> graph_points;
std::vector<Point3D> surface_mesh;
std::size_t new_grid_size = 0;
float rotationX = 0.0f;
float rotationY = 0.0f;
std::size_t surface_grid_size = 0;

void setup_socket() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket() failed: " << std::strerror(errno) << '\n';
        return;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt() failed: " << std::strerror(errno) << '\n';
        close(server_fd);
            server_fd = -1;
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(kPort);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "bind() failed: " << std::strerror(errno) << '\n';
        close(server_fd);
            server_fd = -1;
        return;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "listen() failed: " << std::strerror(errno) << '\n';
        close(server_fd);
            server_fd = -1;
        return;
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    }
        std::cout << "Listening on port " << kPort << '\n';
}

void receive_points() {
    if (server_fd < 0) {
        return;
    }

    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "accept() failed: " << std::strerror(errno) << '\n';
        }
        return;
    }

    std::string data;
    char buffer[1024];

    while (true) {
        ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            data += buffer;
        } else {
            break;
        }
    }

    close(client_fd);

    if (data.empty()) {
        return;
    }

    std::string mode = is2DMode ? "2D" : "3D";
    size_t mode_pos = data.find("MODE:");
    if (mode_pos != std::string::npos) {
        size_t end = data.find(';', mode_pos);
        if (end != std::string::npos) {
            mode = data.substr(mode_pos + 5, end - mode_pos - 5);
            is2DMode = (mode == "2D" || mode == "2d");
        }
    }

    std::vector<Point2D> new_2d_points;
    std::vector<Point3D> new_3d_points;
    std::size_t new_grid_size = 0;
    std::stringstream ss(data);
    std::string token;

    while (std::getline(ss, token, ';')) {
        if (token.empty() || token.rfind("MODE:", 0) == 0) {
            continue;
        }

        if (token.rfind("SIZE:", 0) == 0) {
            new_grid_size = static_cast<std::size_t>(
                std::stoul(token.substr(5))
            );
            continue;
        }

        std::stringstream coord_ss(token);
        std::string sx, sy, sz;
        if (std::getline(coord_ss, sx, ',') &&
            std::getline(coord_ss, sy, ',') &&
            std::getline(coord_ss, sz, ',')) {
            float x = std::stof(sx);
            float y = std::stof(sy);
            float z = std::stof(sz);

            if (is2DMode) {
                new_2d_points.push_back({x, y});
            } else {
                new_3d_points.push_back({x, y, z});
            }
        }
    }

    if (is2DMode && !new_2d_points.empty()) {
        graph_points = new_2d_points;
    } else if (!is2DMode &&
               new_grid_size > 1 &&
               new_3d_points.size() == new_grid_size * new_grid_size) {
        surface_mesh = new_3d_points;
        surface_grid_size = new_grid_size;
    }
}

int main() {
    if (!glfwInit()) {
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Graph", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);
    setup_socket();

    while (!glfwWindowShouldClose(window)) {
        receive_points();

        glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        if (is2DMode) {
            std::cout << "Received 2D graph with " << graph_points.size() << " points\n";
            glOrtho(-10.0, 10.0, -10.0, 10.0, -1.0, 1.0);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            std::cout << "Received 3D surface: " << surface_grid_size << " x "
                      << surface_grid_size << " points\n";
            glColor3f(0.2f, 0.8f, 1.0f);
            glBegin(GL_LINE_STRIP);
            for (const auto& p : graph_points) {
                glVertex2f(p.x, p.y);
            }
            glEnd();
        } else {
            float aspect = 800.0f / 600.0f;
            float fov = 45.0f;
            float top = 0.1f * std::tan(fov * 0.5f * M_PI / 180.0f);
            float right = top * aspect;
            glFrustum(-right, right, -top, top, 0.1f, 100.0f);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(0.0f, 0.0f, -15.0f);
            glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
            glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        if (server_fd < 0) {
            glfwDestroyWindow(window);
            glfwTerminate();
            return 1;
        }

            glColor3f(0.8f, 0.9f, 1.0f);
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            for (const auto& p : surface_mesh) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            glEnd();

            const std::size_t grid_size = surface_grid_size;

            if (grid_size > 1 &&
                surface_mesh.size() == grid_size * grid_size) {
                glBegin(GL_TRIANGLES);

                for (std::size_t i = 0; i + 1 < grid_size; ++i) {
                    for (std::size_t j = 0; j + 1 < grid_size; ++j) {
                        const Point3D& p00 = surface_mesh[i * grid_size + j];
                        const Point3D& p01 = surface_mesh[i * grid_size + j + 1];
                        const Point3D& p10 = surface_mesh[(i + 1) * grid_size + j];
                        const Point3D& p11 = surface_mesh[(i + 1) * grid_size + j + 1];

                        glVertex3f(p00.x, p00.y, p00.z);
                        glVertex3f(p01.x, p01.y, p01.z);
                        glVertex3f(p10.x, p10.y, p10.z);

                        glVertex3f(p01.x, p01.y, p01.z);
                        glVertex3f(p11.x, p11.y, p11.z);
                        glVertex3f(p10.x, p10.y, p10.z);
                    }
                }

                glEnd();
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (server_fd >= 0) {
        close(server_fd);
    }

    glfwTerminate();
    return 0;
}
