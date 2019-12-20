#ifndef FILEINTERPRETER_H
#define FILEINTERPRETER_H

#include <string>
#include <list>
#include "mesh.h"
#include "polygon.h"
#include "scene.h"

#include <vector>


using std::string;
using std::list;
using std::vector;

class FileInterpreter
{
public:

    // Конструктор
    FileInterpreter();

    // Чтение файла и сборка сетки с учетом имени файла. Вызывает рекурсивную вспомогательную функцию
    // Return: объект сетки, созданный из описаний файлов .simp
    Scene buildSceneFromFile(double x, double z, double xCam, double yCam, double zCam);

private:
    Scene* currentScene; // Объект Scene: используется для вставки значений во время построения

    // Рекурсивная вспомогательная функция: извлекает многоугольники
    vector<Mesh> getMeshHelper2(double sphere_x, double sphere_z, double xCam, double yCam, double zCam, bool currentIsWireframe, bool currentisDepthFogged, bool currentAmbientLighting, bool currentUseSurfaceColor, unsigned int currentSurfaceColor, ShadingModel currentShadingModel, double currentSpecCoef, double currentSpecExponent, double currentReflectivity);

    // Чтение файла obj
    // Return: вектор <Polygon>, содержащий все грани, описанные объектом
    vector<Polygon> getPolysFromObj(string filename);

    // интерпретировать прочитанную строку
    // Return: список разделенных и очищенных токенов
    list<string> interpretTokenLine(string newString);

    // Разделить строку
    // Return: список <string>, содержащий каждый токен как отдельный элемент, в порядке чтения файла
    list<string> split(string line);

    // Очистить список токенов любых символов форматирования. Используется после того, как split извлек токены из строки
    // Return: список <string>, содержащий токены, очищенные от символов форматирования
    list<string> cleanseTokens(list<string> tokens);
};

#endif // FILEINTERPRETER_H
