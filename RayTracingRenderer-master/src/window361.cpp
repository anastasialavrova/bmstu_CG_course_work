#include <iostream>
#include <QGridLayout>
#include "window361.h"
#include <QTimer>
#include <QMessageBox>
#include <string>

Window361::Window361()
{
    renderArea = new RenderArea361((QWidget *)0);

    nextPageButton = new QPushButton("Запустить");
    connect(nextPageButton, SIGNAL(clicked()), this, SLOT(getLatitude()));

    closeButton = new QPushButton("Остановить программу");
    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeApp()));

    newLine = new QLineEdit();

    xCamera = new QLineEdit();
    yCamera = new QLineEdit();
    zCamera = new QLineEdit();

    latitudeEnter = new QLabel("Введите широту:");

    xLine = new QLabel("X:");
    yLine = new QLabel("Y:");
    zLine = new QLabel("Z:");

    CameraLine = new QLabel("Камера:");



    QGridLayout *layout = new QGridLayout;
    layout->addWidget(renderArea, 1, 0);

    layout->addWidget(CameraLine, 0, 1);

    layout->addWidget(xLine, 0, 2);
    layout->addWidget(yLine, 0, 4);
    layout->addWidget(zLine, 0, 6);

    layout->addWidget(xCamera, 0, 3, 1, 1);
    layout->addWidget(yCamera, 0, 5, 1, 1);
    layout->addWidget(zCamera, 0, 7, 1, 1);

    layout->addWidget(latitudeEnter, 1, 1, 1, 1);
    layout->addWidget(newLine, 1, 2, 1, 4);
    layout->addWidget(nextPageButton, 4, 1, 3, 8);
    layout->addWidget(closeButton, 8, 1, 3, 8);

    setLayout(layout);


    setWindowTitle(tr("Маятник Фуко"));
}

Drawable *Window361::getDrawable() {
    return renderArea;
}
Window361::~Window361() {
    delete renderArea;
    delete nextPageButton;
}

void Window361::setPageTurner(PageTurner *pageTurner) {
    this->pageTurner = pageTurner;
    pageTurner->nextPage(0, 0, 1, -4.05);
}

 void Window361::getLatitude() {
     timer = new QTimer();
     timer->start(1000);
     connect(timer, SIGNAL(timeout()), this, SLOT(slotAlarmTimer()));
     timer->singleShot(170000, this, SLOT(closeApp()));
}

 void Window361::slotAlarmTimer()
 {
     double latitude = newLine->text().toInt(/*0,10*/);
     double x = xCamera->text().toDouble(/*0,10*/);
     double y = yCamera->text().toDouble(/*0,10*/);
     double z = zCamera->text().toDouble(/*0,10*/);


     pageTurner->nextPage(latitude, x, y, z);
     renderArea->repaint();
 }


 void Window361::closeApp()
 {
       this->close();
 }






