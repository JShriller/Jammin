import sys
import socket
import numpy as np
from sympy import symbols, lambdify, sympify
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, QPushButton, QLabel, QSlider
from PyQt6.QtCore import Qt

class GrapherUI(QWidget):
    def __init__(self):
        super().__init__()
        self.current_a_value = 1.0 # Initialize Desmos slider constant variable parameter
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("Desmos Extended Control Room")
        self.setGeometry(100, 100, 500, 240)
        
        main_layout = QVBoxLayout()
        input_layout = QHBoxLayout()
        slider_layout = QHBoxLayout()
        view_layout = QHBoxLayout()

        self.label = QLabel("z =")
        self.equation_input = QLineEdit("sin(a * x) * cos(y)") # Includes parameter token 'a'
        
        self.plot_button = QPushButton("Synchronize Equation Mesh")
        self.plot_button.clicked.connect(self.process_algebraic_mesh)

        # INTERACTIVE SLIDER LAYER Setup
        self.slider_label = QLabel("Variable a = 1.0")
        self.variable_slider = QSlider(Qt.Orientation.Horizontal)
        self.variable_slider.setMinimum(1)
        self.variable_slider.setMaximum(50) # Range 0.1 to 5.0
        self.variable_slider.setValue(10)
        self.variable_slider.valueChanged.connect(self.handle_slider_shift)

        self.btn_2d = QPushButton("Desmos 2D Grid Mode")
        self.btn_3d = QPushButton("Desmos 3D Orbit Mode")
        self.btn_2d.clicked.connect(lambda: self.send_network_command("TOGGLE_2D"))
        self.btn_3d.clicked.connect(lambda: self.send_network_command("TOGGLE_3D"))

        input_layout.addWidget(self.label)
        input_layout.addWidget(self.equation_input)
        
        slider_layout.addWidget(self.slider_label)
        slider_layout.addWidget(self.variable_slider)

        view_layout.addWidget(self.btn_2d)
        view_layout.addWidget(self.btn_3d)

        main_layout.addLayout(input_layout)
        main_layout.addLayout(slider_layout)
        main_layout.addWidget(self.plot_button)
        main_layout.addLayout(view_layout)
        
        self.setLayout(main_layout)

    def handle_slider_shift(self, integer_value):
        # Convert integer slider counts cleanly into float scale steps
        self.current_a_value = integer_value / 10.0
        self.slider_label.setText(f"Variable a = {self.current_a_value:.1f}")
        # Instantly run and re-send data matrix evaluations to C++ viewport
        self.process_algebraic_mesh()

    def send_network_command(self, command_string):
        try:
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(("127.0.0.1", 12345))
            client_socket.sendall(command_string.encode('utf-8'))
            client_socket.close()
        except Exception as error:
            print(f"Pipeline socket notice: {error}")

    def process_algebraic_mesh(self):
        user_string = self.equation_input.text()
        try:
            # Explicitly declare all three active mathematical variables to parsing tree
            x_sym, y_sym, a_sym = symbols('x y a')
            parsed_expr = sympify(user_string)
            
            # Substitute the local dynamic slider state into the symbol evaluation tree
            substituted_expr = parsed_expr.subs(a_sym, self.current_a_value)
            math_func = lambdify((x_sym, y_sym), substituted_expr, 'numpy')

            resolution = 60
            x_vals = np.linspace(-5, 5, resolution)
            y_vals = np.linspace(-5, 5, resolution)
            
            data_payload = []
            for i in range(resolution):
                for j in range(resolution):
                    x = float(x_vals[i])
                    y = float(y_vals[j])
                    try:
                        z = float(math_func(x, y))
                    except:
                        z = 0.0 
                    data_payload.append(f"{x},{y},{z}")
            
            packet_string = ";".join(data_payload) + ";"

            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(("127.0.0.1", 12345))
            client_socket.sendall(packet_string.encode('utf-8'))
            client_socket.close()

        except Exception as error:
            pass # Suppress temporary text composition runtime typing warnings

if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = GrapherUI()
    ui.show()
    sys.exit(app.exec())