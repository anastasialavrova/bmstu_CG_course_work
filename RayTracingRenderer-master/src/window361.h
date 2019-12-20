#ifndef WINDOW_H
#define WINDOW_H

#include <QDebug>
#include <QLineEdit>
#include <QLabel>

#include <QWidget>
#include <QPushButton>
#include "pageturner.h"
#include "drawable.h"
#include "renderarea361.h"
#include <QTimer>
#include <string>


namespace Ui {
class Window361;
}

class Window361 : public QWidget
{
    Q_OBJECT

public:
    Window361();
    ~Window361();
    Drawable *getDrawable();
    void setPageTurner(PageTurner *pageTurner);

private:
    RenderArea361 *renderArea;
    QPushButton *nextPageButton;
    PageTurner *pageTurner;
    PageTurner *animation;
    QLineEdit *newLine;
    QTimer *timer;
    QPushButton *closeButton;
    QLabel *latitudeEnter;

    QLineEdit *xCamera;
    QLineEdit *yCamera;
    QLineEdit *zCamera;

    QLabel *xLine;
    QLabel *yLine;
    QLabel *zLine;

    QLabel *CameraLine;


private slots:
    void nextPageClicked();
    void getLatitude();
    void slotAlarmTimer();
    void closeApp();
};

#endif // WINDOW_H
