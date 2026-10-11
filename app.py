import sys
import socket
import sympy as sp
from sympy.parsing.sympy_parser import (
    convert_xor,
    implicit_multiplication_application,
    parse_expr,
    standard_transformations,
)
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QLineEdit, QPushButton, QComboBox, QMessageBox

TRANSFORMATIONS = standard_transformations + (
    implicit_multiplication_application,
    convert_xor,
)
ALLOWED_FUNCTIONS = {
    "sin": sp.sin,
    "cos": sp.cos,
    "tan": sp.tan,
    "sqrt": sp.sqrt,
    "log": sp.log,
    "exp": sp.exp,
    "abs": sp.Abs,
}

def send_function(expr_text, mode):
    points = []
    step_count = 200
    x_symbol, y_symbol = sp.symbols("x y")
    variables = (x_symbol,) if mode == "2d" else (x_symbol, y_symbol)
    expression = parse_expr(
        expr_text,
        local_dict={"x": x_symbol, "y": y_symbol, **ALLOWED_FUNCTIONS},
        transformations=TRANSFORMATIONS,
    )
    unexpected_symbols = expression.free_symbols.difference(variables)
    if unexpected_symbols:
        names = ", ".join(sorted(str(symbol) for symbol in unexpected_symbols))
        raise ValueError(f"Unknown variable(s): {names}")
    function = sp.lambdify(variables, expression, modules="math")

    if mode == "2d":
        for i in range(step_count):
            x = -5.0 + (10.0 * i / (step_count - 1))
            y = function(x)
            points.append(f"{x},{y},0.0")
    else:
        for i in range(step_count):
            x = -5.0 + (10.0 * i / (step_count - 1))
            for j in range(step_count):
                y = -5.0 + (10.0 * j / (step_count - 1))
                z = function(x, y)
                points.append(f"{x},{y},{z}")
    
    size_header = f"SIZE:{step_count};" if mode == "3d" else ""
    payload = f"MODE:{mode.upper()};{size_header}" + ";".join(points) + ";"

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
    try:
        s.connect(("127.0.0.1", 12345))
        s.sendall(payload.encode("utf-8"))
        print(f"Sent {mode.upper()} graph data to port 12345")
    except OSError as exc:
        print(f"Connection failed: {exc}")
        print("Start the C++ graphics app first so it is listening on port 12345.")
    finally:
        s.close()

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Graph Sender")
        self.resize(400, 150)

        layout = QVBoxLayout()

        self.input_box = QLineEdit("sin(x)")
        layout.addWidget(self.input_box)

        self.mode_box = QComboBox()
        self.mode_box.addItems(["2d", "3d"])
        layout.addWidget(self.mode_box)

        self.button = QPushButton("Send Function")
        self.button.clicked.connect(self.on_send)
        layout.addWidget(self.button)

        self.setLayout(layout)

    def on_send(self):
        expr = self.input_box.text()
        mode = self.mode_box.currentText()
        try:
            send_function(expr, mode)
        except Exception as exc:
            QMessageBox.warning(self, "Invalid function", str(exc))

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())