#ifndef PAGETURNER_H
#define PAGETURNER_H

class PageTurner
{
public:
    virtual void nextPage(double latitude, double xCam, double yCam, double zCam) = 0;
};

#endif // PAGETURNER_H
