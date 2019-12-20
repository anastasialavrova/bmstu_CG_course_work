#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "mesh.h"
#include "light.h"

class Scene
{
public:
    // Конструктор
    Scene();

    // Конструктор копирования
    Scene(const Scene &rhs);

    // Перегрузка оператора присваивания
    Scene& operator=(const Scene& rhs);

    vector<Mesh> theMeshes;     // содержит сетки
    vector<Light> theLights;    // содержит свет

    // Значения окружающего освещения:
    double ambientRedIntensity, ambientGreenIntensity, ambientBlueIntensity;

    // Настройки камеры:
    // Просмотр настроек окна: по умолчанию угол обзора 90 градусов
    double xLow = -1;
    double xHigh = 1;
    double yLow = -1;
    double yHigh = 1;

    double camHither = 1;
    double camYon = 200;

    TransformationMatrix cameraMovement;

    double fogHither = camYon;
    double fogYon = camYon;

    double fogRedIntensity = 0;
    double fogGreenIntensity = 0;
    double fogBlueIntensity = 0;
    unsigned int fogColor = 0xff000000;
    bool isDepthFogged = false;

    unsigned int environmentColor = 0xff000000;  // Цвет фона по умолчанию
    // Настройки трассировки лучей сцены:
    int numRayBounces = 0;          // Количество отскоков по умолчанию при трассировке лучей. По умолчанию = 0 (т.е. без трассировки лучей)
    bool noRayShadows = false;      // Использовать или нет теневые лучи. По умолчанию = false
};

#endif // SCENE_H
