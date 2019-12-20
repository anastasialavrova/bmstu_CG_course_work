#ifndef MYRENDERER_H
#define MYRENDERER_H

#include "drawable.h"

#include "polygon.h"
#include "vertex.h"
#include "line.h"
#include "mesh.h"
#include "transformationmatrix.h"
#include "light.h"
#include "scene.h"
#include <limits>

class Renderer{
public:
    // Конструктор
    Renderer(Drawable* newDrawable, int newXRes, int newYRes, int borderWidth);

    // Дескриптор
    ~Renderer();

    // Рисуем прямоугольник. Используется только для настройки цветов фона панели. Игнорирует z-буфер.
    // Предварительное условие: координаты topLeft_ и botRight_ действительны и находятся в пространстве окна пользовательского интерфейса (т. е. (0,0) в левом верхнем углу экрана)
    void drawRectangle(int topLeftX, int topLeftY, int botRightX, int botRightY, unsigned int color);

    // Отрисовка сцену
    void renderScene(Scene theScene);

    void debugLights();

    // Отрисовка линии
    void drawLine(Line theLine, ShadingModel theShadingModel, bool doAmbient, double specularCoefficient, double specularExponent);

private:
    Drawable* drawable; // Drawable объект, используемый для взаимодействия с каркасом QT

    // Растровые настройки:
    int border;         // Ширина границы экрана
    int xRes;           // Расчетное разрешение растра по горизонтали
    int yRes;           // Расчетное вертикальное разрешение растра

    int** ZBuffer;               // Z Глубина буфера
    int maxZVal = std::numeric_limits<int>::max();    // Максимально возможное значение глубины z

    // Рисуются объекты текущей сцены, сетки и многоугольника (используются для доступа к различным переменным рендеринга)
    Scene* currentScene;
    Mesh* currentMesh;
    Polygon* currentPolygon;

    // Матрица преобразования из мира в пространство камеры
    TransformationMatrix worldToCamera;

    // Перспективное преобразование: захватывает объект в пространстве камеры и добавляет перспективу
    TransformationMatrix cameraToPerspective;

    // Матрица преобразования в экранные координаты
    TransformationMatrix perspectiveToScreen;

    // Матрица преобразования из экранных координат
    TransformationMatrix screenToPerspective;

    // Рисуем многоугольник. Вызывает растеризованную вспомогательную функцию Polygon.
    // Предварительное условие: все полигоны находятся в пространстве камеры
    void drawPolygon(Polygon thePolygon, bool isWireframe);

    // Рисуем многоугольник только в каркасном режиме
    void drawPolygonWireframe(Polygon* thePolygon);

    // Рисуем объект сетки
    void drawMesh(Mesh* theMesh);

    // Нарисовать линию скана с учетом Z-буфера.
    // Предварительное условие: начальная и конечная вершины располагаются слева направо
    // Примечание: LERP, если start.color! = End.color. НЕ обновляет экран!
    void drawScanlineIfVisible(Vertex* start, Vertex* end);

    // Рисуем линию развертки с подсветкой на пиксель фонг, с учетом Z-буфера
    void drawPerPxLitScanlineIfVisible(Vertex* start, Vertex* end, bool doAmbient, double specularCoefficient, double specularExponent);

    // Сброс буфера глубины
    void resetDepthBuffer();

    // Изменить форму усеченного конуса
    void transformCamera(TransformationMatrix cameraMovement);

    // Рассчитать результат наложения пикселя
    // Возвращает: целое число без знака, представляющее смешанное значение полупрозрачного пикселя
    unsigned int blendPixelValues(int x, int y, unsigned int color, float opacity);

    // Возвращает: значение цвета без знака int, рассчитанное на основе LERP текущей позиции между двумя предоставленными точками
    unsigned int getPerspCorrectLerpColor(Vertex* p1, Vertex* p2, double ratio) const;

    // Рассчитать значение освещения для данного пикселя на линии между 2 точками
    // Предварительное условие: окружающее освещение уже применено к значениям цвета вершины
    unsigned int getFogPixelValue(Vertex* p1, Vertex* p2, double ratio, double correctZ);

    // Вычисляем интерполированный пиксель и значение глубины
    unsigned int getDistanceFoggedColor(unsigned int pixelColor, double correctZ);

    // Установить пиксель на растре
    // Предварительное условие: точка является действительной координатой на растровом холсте и была предварительно проверена по z-буферу
    void setPixel(int x, int y, double z, unsigned int color);

    // Рисуем многоугольник используя непрозрачность
    // Если вершины Полигона не одного цвета, цвет будет LERP'd
    void rasterizePolygon(Polygon* thePolygon);

    // Осветить полигон, используя плоскую заливку
    // Предварительное условие: все вершины имеют действительную нормаль
    void flatShadePolygon(Polygon* thePolygon);

    // Осветить полигон, используя затенение Гуро
    // Предварительное условие: все вершины имеют действительную нормаль
    void gouraudShadePolygon(Polygon* thePolygon);

    // Осветить заданную точку в пространстве камеры
    unsigned int lightPointInCameraSpace(Vertex* currentPosition, NormalVector* viewVector, bool doAmbient, double specularExponent, double specularCoefficient);

    // Рекурсивно лучевая трассировка освещения точки
    unsigned int recursivelyLightPointInCS(Vertex* currentPosition, NormalVector* viewVector, bool doAmbient, double specularExponent, double specularCoefficient, int bounceRays, bool isEndPoint);

    // Рекурсивная вспомогательная функция для трассировки лучей
    unsigned int recursiveLightHelper(Vertex* currentPosition, NormalVector* viewVector, bool doAmbient, double specularExponent, double specularCoefficient, int bounceRays, bool isEndPoint);

    // Проверяем, находится ли пиксельная координата перед текущей глубиной z-буфера
    bool isVisible(int x, int y, double z);

    // Получить масштабированное значение z-буфера для данного Z
    int getScaledZVal(double correctZ);

    // Определяем, затенена ли текущая позиция каким-либо полигоном в сцене, которая находится между ней и источником света
    bool isShadowed(Vertex currentPosition, NormalVector* lightDirection, double lightDistance);

    // Находим точку пересечения луча и плоскости многоугольника
    // Return: True, если луч пересекается, в противном случае - false. Изменяет результат Vertex, чтобы быть точкой пересечения, в противном случае оставляет его неизменным
    bool getPolyPlaneBackFaceIntersectionPoint(Vertex* currentPosition, NormalVector* currentDirection, Vertex* planePoint, NormalVector* planeNormal, Vertex* result);

    // Находим точку пересечения луча и плоскости многоугольника
    // Return: True, если луч пересекается, в противном случае - false. Изменяет результат Vertex, чтобы быть точкой пересечения, в противном случае оставляет его неизменным
    bool getPolyPlaneFrontFaceIntersectionPoint(Vertex* currentPosition, NormalVector* currentDirection, Vertex* planePoint, NormalVector* planeNormal, Vertex* result);

    // Находим точку пересечения луча и плоскости многоугольника
    // Return: True, если луч пересекается, в противном случае - false. Изменяет результат Vertex, чтобы быть точкой пересечения, в противном случае оставляет его неизменным
    bool getPolyPlaneIntersectionPoint(Vertex* currentPosition, NormalVector* currentDirection, Vertex* planePoint, NormalVector* planeNormal, Vertex* result);

    // Определяем, находится ли точка на плоскости многоугольника внутри многоугольника
    bool pointIsInsidePoly(Polygon* thePolygon, Vertex* intersectionPoint);

    // Рассчитать отражение вектора, направленного в сторону от поверхности
    NormalVector reflectOutVector(NormalVector* faceNormal, NormalVector* outVector);

    // Обновляем точку пересечения трассировки лучей с помощью интерполированных нормалей и значений цвета
    void setInterpolatedIntersectionValues(Vertex* intersectionPoint, Polygon* hitPoly);

    // Проверяем, имеют ли два полигона ребро
    bool haveSharedEdge(Polygon* poly1, Polygon* poly2);

    // Проверяем, больше ли угол между двумя гранями полигона, имеющими общий край, 180 градусов
    bool isFaceReflexAngle(Polygon* currentPoly, Polygon* hitPoly);
};

#endif // MYRENDERER_H
