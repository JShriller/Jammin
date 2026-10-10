import sys
import socket
import numpy as np
from sympy import symbols, lambdify, sympify
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, QPushButton, QLabel

class GrapherUI(QWidget):
    def __init__(self):
        super().__init__()
        self.init_ui()

    def init_ui(self):
        self.setWindowTitle("Desmos 3D Controller Panel")
        self.setGeometry(100, 100, 450, 120)
        main_layout = QVBoxLayout()
        input_layout = QHBoxLayout()

        self.label = QLabel("z =")
        self.equation_input = QLineEdit("sin(x) * cos(y)")
        self.plot_button = QPushButton("Update 3D Viewport")

        self.plot_button.clicked.connect(self.process_algebraic_mesh)

        input_layout.addWidget(self.label)
        input_layout.addWidget(self.equation_input)
        main_layout.addLayout(input_layout)
        main_layout.addWidget(self.plot_button)
        self.setLayout(main_layout)

    def process_algebraic_mesh(self):
        user_string = self.equation_input.text()
        try:
            # 1. Parse algebra via SymPy
            x_sym, y_sym = symbols('x y')
            parsed_expr = sympify(user_string)
            math_func = lambdify((x_sym, y_sym), parsed_expr, 'numpy')

            # 2. Build 2D Mesh Evaluation Matrix Grid
            resolution = 50
            x_vals = np.linspace(-5, 5, resolution)
            y_vals = np.linspace(-5, 5, resolution)
            
            # Serialize mathematical coordinate structures cleanly
            data_payload = []
            for i in range(resolution):
                for j in range(resolution):
                    x = float(x_vals[i])
                    y = float(y_vals[j])
                    try:
                        z = float(math_func(x, y))
                    except:
                        z = 0.0 # Gracefully drop undefined infinity bounds
                    data_payload.append(f"{x},{y},{z}")
            
            # Form single clean string packet stream
            packet_string = ";".join(data_payload) + ";"

            # 3. Network Stream Transmission: Push raw bytes to the C++ server loop
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect(("127.0.0.1", 12345))
            client_socket.sendall(packet_string.encode('utf-8'))
            client_socket.close()
            print(f"🚀 [Python UI] Dispatched updated points to C++ for: z = {user_string}")

        except Exception as error:
            print(f"❌ Syntax/Parsing Error: {error}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = GrapherUI()
    ui.show()
    sys.exit(app.exec())