#include "client.h"
#include "renderer.h"

#include <cstdlib>              // Used for the rand() function
#include <ctime>                // Used to seed the rand() function
#include <iostream>
#include <chrono>
#include <string>
#include <QDebug>
#include <windows.h>
#include <QMessageBox>
#include <cmath>

#include <chrono>
#include <thread>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::cout;

double x;
double y;
double z;
double delta = 0.1;
bool flag = 1;
int t = 0;

double x1 = 50, x2 = 60;

#define PI 3.14

// Конструктор по умолчанию
Client::Client(){
    // Ничего не делает
}

// Конструктор
Client::Client(Drawable *drawable){

    this->drawable = drawable;

    clientRenderer = new Renderer(this->drawable, xRes, yRes, PANEL_BORDER_WIDTH);

    std::srand((unsigned int)std::time(0));   // Seed the random number generator

    clientFileInterpreter = FileInterpreter();

    commandLineMode = false;
    filename = "";
}

// Отобразить следующую сцену
void Client::nextPage(double latitude, double xCam, double yCam, double zCam)
{
    static int pageNumber = 0;
    std::cout << "Page #" << pageNumber << std::endl;

    if (pageNumber == 0)
    {
        Scene theScene;
        high_resolution_clock::time_point t1, t2;

        t1 = high_resolution_clock::now();

        theScene = clientFileInterpreter.buildSceneFromFile(0, 0, 0, 1, -4.05);
        clientRenderer->drawRectangle(0, 2, 0.01, 0, 0xff808080);

        t2 = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>( t2 - t1 ).count();
        cout << "File read in:\t" << duration << "ms\n";

        t1 = high_resolution_clock::now();
        clientRenderer->renderScene(theScene);
        t2 = high_resolution_clock::now();
        duration = duration_cast<microseconds>( t2 - t1 ).count();
        cout << "Mesh drawn in:\t" << duration << "ms\n\n";
        pageNumber++;
    }
    else
    {
        if (xCam == 0 && yCam == 0 && zCam == 0)
        {
            xCam = 0;
            yCam = 1;
            zCam = -4.05;
        }
        animation(latitude);
        Scene cmdLineScene;
        std::string no = std::to_string(pageNumber);
        cmdLineScene = clientFileInterpreter.buildSceneFromFile(x, y, xCam, yCam, zCam);
        clientRenderer->renderScene(cmdLineScene);
        drawable->updateScreen();
        pageNumber++;

    }
}

double Client::animation(double latitude)
{
    t++;
    double a = 0.9;
    if (latitude == 90 || latitude == 180)
        latitude = 89.9999;
    // double omega1 = (15*PI/180)*sin((latitude*3.14)/180);
    double omega1 = 0.075;
    //double omega1 = 0.0000727;

    double omega2 = 1;
    x = a * cos(omega2*t) * sin(omega1* t);
    y = a * cos(omega2*t) * cos(omega1* t);

}
