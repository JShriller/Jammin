#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

// My Widget class 
class MyWidget : public QWidget {
    Q_OBJECT
    
    public: 
        explicit MyWidget(QWidget *parent = nullptr) : QWidget(parent) {}

    protected: 
        // Declare the paintEvent override under the protected key word 
        void paintEvent(QPaintEvent *event) override; 
};

#endif