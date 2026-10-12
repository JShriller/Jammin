#include <QApplication>
#include "mywidget.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv); 

    MyWidget window; 
    window.resize(800, 600);
    window.setWindowTitle("Graphing Calculator");
    window.show(); 

    return app.exec();
}