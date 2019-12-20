#ifndef LIGHT_H
#define LIGHT_H

#include "vertex.h"

class Light
{
public:
    // Конструктор по умолчанию
    Light();

    // Конструктор
    Light(double newRedIntensity, double newGreenIntensity, double newBlueIntensity, double newAttA, double newAttB);

    // Перегрузка оператора присваивания
    Light& operator=(const Light& rhs);

    // Рассчитать ослабление этого света в точке, как отношение
    double getAttenuationFactor(Vertex thePoint);

    // Рассчитать ослабление этого света в точке, как отношение
    double getAttenuationFactor(double distance);

    // Атрибуты света
    Vertex position; // Освещает положение, как точку в пространстве

    // Цвет света:
    double redIntensity, greenIntensity, blueIntensity;     // [0, 1]

    double attenuationA, attenuationB;  // Константы затухания

    // Debug this light:
    void debug();

private:

};

#endif // LIGHT_H
