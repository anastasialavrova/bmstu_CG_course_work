#include "light.h"
#include <iostream>

using std::cout;

Light::Light()
{
    redIntensity = 0;
    greenIntensity = 0;
    blueIntensity = 0;
    attenuationA = 0.5;
    attenuationB = 0.5;
}


// 5-аргументный конструктор
Light::Light(double newRedIntensity, double newGreenIntensity, double newBlueIntensity, double newAttA, double newAttB){
    redIntensity = newRedIntensity;
    greenIntensity = newGreenIntensity;
    blueIntensity = newBlueIntensity;
    attenuationA = newAttA;
    attenuationB = newAttB;
}

// перегрузка оператора присваивания
Light& Light::operator=(const Light& rhs){
    this->redIntensity = rhs.redIntensity;
    this->greenIntensity = rhs.greenIntensity;
    this->blueIntensity = rhs.blueIntensity;

    this->attenuationA = rhs.attenuationA;
    this->attenuationB = rhs.attenuationB;

    this->position = rhs.position;

    return *this;
}

// Рассчитать ослабление этого источника света в точке как отношение (перегружено)
double Light::getAttenuationFactor(Vertex thePoint){
    NormalVector distance(position.x - thePoint.x, position.y - thePoint.y, position.z - thePoint.z);

    return (1.0 /((double) (attenuationA + (attenuationB * distance.length() ) ) ));
}

// Рассчитать ослабление этого источника света в точке как отношение (перегружено)
double Light::getAttenuationFactor(double distance){
    return (1.0 /((double) (attenuationA + (attenuationB * distance) ) ));
}

// Debug this light:
void Light::debug(){
    cout << "\nLight: ";
    cout << "R: " << redIntensity << " G: " << greenIntensity << " B: " << blueIntensity << " att_A: " << attenuationA << " att_B " << attenuationB << "\n";
    cout << "Position: x: " << position.x << " y: " << position.y << " z: " << position.z << "\n\n";

}
