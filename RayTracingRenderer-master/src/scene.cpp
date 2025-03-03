#include "scene.h"

Scene::Scene()
{
}

Scene::Scene(const Scene &rhs){
    this->theMeshes = rhs.theMeshes;
    this->theLights = rhs.theLights;

    this->ambientRedIntensity = rhs.ambientRedIntensity;
    this->ambientGreenIntensity = rhs.ambientGreenIntensity;
    this->ambientBlueIntensity = rhs.ambientBlueIntensity;

    this->xLow = rhs.xLow;
    this->xHigh = rhs.xHigh;
    this->yLow = rhs.yLow;
    this->yHigh = rhs.yHigh;

    this->camHither = rhs.camHither;
    this->camYon = rhs.camYon;

    this->cameraMovement = rhs.cameraMovement;

    this->fogHither = rhs.fogHither;
    this->fogYon = rhs.fogYon;

    this->fogRedIntensity = rhs.fogRedIntensity;
    this->fogGreenIntensity = rhs.fogGreenIntensity;
    this->fogBlueIntensity = rhs.fogBlueIntensity;
    this->fogColor = rhs.fogColor;

    this->isDepthFogged = rhs.isDepthFogged;

    this->environmentColor = rhs.environmentColor;

    this->numRayBounces = rhs.numRayBounces;
    this->noRayShadows = rhs.noRayShadows;
}

// перегружен оператор присваивания
Scene& Scene::operator=(const Scene& rhs){
    if (this == &rhs)
        return *this;

    this->theMeshes = rhs.theMeshes;
    this->theLights = rhs.theLights;

    this->ambientRedIntensity = rhs.ambientRedIntensity;
    this->ambientGreenIntensity = rhs.ambientGreenIntensity;
    this->ambientBlueIntensity = rhs.ambientBlueIntensity;

    this->xLow = rhs.xLow;
    this->xHigh = rhs.xHigh;
    this->yLow = rhs.yLow;
    this->yHigh = rhs.yHigh;

    this->camHither = rhs.camHither;
    this->camYon = rhs.camYon;

    this->cameraMovement = rhs.cameraMovement;

    this->fogHither = rhs.fogHither;
    this->fogYon = rhs.fogYon;

    this->fogRedIntensity = rhs.fogRedIntensity;
    this->fogGreenIntensity = rhs.fogGreenIntensity;
    this->fogBlueIntensity = rhs.fogBlueIntensity;
    this->fogColor = rhs.fogColor;

    this->isDepthFogged = rhs.isDepthFogged;

    this->environmentColor = rhs.environmentColor;

    this->numRayBounces = rhs.numRayBounces;
    this->noRayShadows = rhs.noRayShadows;

    return *this;
}
