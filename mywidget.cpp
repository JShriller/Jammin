#include "mywidget.h"

void MyWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this); 

    // Filling the background 

    painter.fillRect(rect(), Qt::white); 

    // Lines 
    int w = width(); 
    int h = height(); 
    int gridSpacing = 40; // Step size between lines in pixels 

    // Color for grid lines 
    QPen gridPen(QColor(220, 220, 220), 1, Qt::SolidLine); 
    painter.setPen(gridPen); 

    // Vertical lines 
    for (int x = gridSpacing; x < w; x += gridSpacing) {
        painter.drawLine(x, 0, x, h);
    }

    // Draw horizontal lines 
    for (int y = gridSpacing; y < h; y+=gridSpacing) {
        painter.drawLine(0, y, w, y); 
    }
    
    // Draws darker axes through the center
    QPen axisPen(QColor(60, 60, 60), 2, Qt::SolidLine);
    painter.setPen(axisPen);
    
    int centerX = w / 2;
    int centerY = h / 2; 

    // Horizontal main axis (X axis)
    painter.drawLine(0, centerY, w, centerY);
    // Vertical main axis ( axis) 
    painter.drawLine(centerX, 0, centerX, h);
}