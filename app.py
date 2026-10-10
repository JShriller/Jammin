import sys
import os
from sympy import symbols, lambdify, sympify 


# Directing python to locate to the C++ library module inside the build folder
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), 'build')))
try: 
    import cpp_backend
except ImportError as e:
    print("Error: Could not find 'cpp_backend.so'. Make user to hit 'Build' (F7) in VScode")
    print(e)
    sys.exit(1)
    
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, QPushButton, QLabel

class GrapherUI(QWidget):
    # pulled over constructor from parent class and called a function to initalize the UI
    def __init__(self):
        super().__init__()
        self.init_ui()
    # Creating the ui elements and features with funcitons 
    def init_ui(self):
        self.setWindowTitle("3D Graphing Calculator")
        self.setGeometry(100, 100, 450, 150)
        
        # Variables store layout strucuters
    
        main_layout = QVBoxLayout()
        input_layout = QHBoxLayout()
    
        # UI Contorl elements 
        self.label = QLabel("z =")
        self.equation_input = QLineEdit("sin(x) * cos(y)") # Default sample string
        self.plot_button = QPushButton("Calculate Mesh")
        
        # Binds the button to the mathematical evalutaion logic 
        self.plot_button.clicked.connect(self.process_algebraic_mesh)
        
        # Assembles the layout architecture
        input_layout.addWidget(self.label)
        input_layout.addWidget(self.equation_input)
        main_layout.addLayout(input_layout)
        main_layout.addWidget(self.plot_button)
        
        self.setLayout(main_layout)
    
    # This function takes the algebraic equations and passes them to my C++ module
    def process_algebraic_mesh(self):
        user_string = self.equation_input.text()
        print(f"\n[Python UI] Processing expression string: '{user_string}'")
        
        try:
            # Safely tokenize and parse the text via SymPy functions and class methods
            x_sym, y_sym = symbols('x y')
            parsed_expr = sympify(user_string)
            
            # convert the SymPy expression into an ultra-fast compiled math function target
            math_func = lambdify((x_sym, y_sym), parsed_expr, 'math')
            
            print("[Python UI] Code successfully parsed. Passing grid computation to next phase...")
            
            # The custom C++ module logic is called using the pybind11 library 
            # There is also a resultion parameter for how dense the 3D grid layout can be
            resolution = 50 
            cpp_calculated_points = cpp_backend.generate_surface_mesh(resolution)
            
            print(f"[C++ Core] Success! Returned {len(cpp_calculated_points)} coordinate back to Python smoothly")
            print(f"Sample 3D Point Coordinates: {cpp_calculated_points[0]}")
            
        except Exception as error:
            print(f"Algebraic parsing error occurred: {error}")
            
if __name__ == "__main__":
    app = QApplication(sys.argv)
    ui = GrapherUI()
    ui.show()
    sys.exit(app.exec())