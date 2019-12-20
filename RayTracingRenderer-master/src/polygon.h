#ifndef POLYGON_H
#define POLYGON_H

#include "vertex.h"
#include "transformationmatrix.h"
#include <vector>
#include "normalvector.h"

using std::vector;

// перечислитель модели затенения: используется для выбора / управления моделью затенения объекта
enum ShadingModel{
    ambientOnly = 0,
    flat = 1,
    gouraud = 2,
    phong = 3
};

class Polygon{
public:
    // Конструктор
    Polygon();

    // Конструктор
    Polygon(Vertex p0, Vertex p1, Vertex p2);

    // Конструктор копирования
    Polygon(const Polygon &existingPolygon);

    // Перегрузка операторов присваивания
    Polygon& operator=(const Polygon& rhs);

    // Деструктор;
    ~Polygon();

    // Удалить все вершины из массива вершин этого многоугольника
    void clearVertices();

    // Добавить вершину к многоугольнику.
    // Предварительное условие: вершины всегда добавляются в порядке против часовой стрелки (вершины [i + 1] = CCW, вершины [i-1] = CW
    void addVertex(Vertex newPoint);

    // Получить вершину с наибольшим значением y. Используется средством визуализации для отрисовки этого многоугольника.
    // Возвращает: самая высокая вершина в этом многоугольнике, с точки зрения координат y
    Vertex* getHighest();

    // Получить вершину с наименьшим значением y. Используется средством визуализации для отрисовки этого многоугольника.
    // Возвращает: указатель на объект Vertex
    Vertex* getLowest();

    // Получить следующую вершину в массиве вершин
    Vertex* getNext(unsigned int currentVertex);

    // Получить следующую вершину в массиве вершин
    Vertex* getPrev(unsigned int currentVertex);

    // Получить следующую вершину в массиве вершин
    Vertex* getLast();

    // Устанавливаем все вершины в этом многоугольнике в один цвет
    void setSurfaceColor(unsigned int solidColor);

    // Определяем, содержат ли вершины этого многоугольника более одного цвета
    // Возвращает true, если все вершины имеют одинаковый цвет (или если многоугольник имеет 0 вершин)
    bool isSolidColor();

    // Определяем, правильно ли инициализирован этот полигон.
    // Возвращает true, если у Polygon есть хотя бы 2 вершины, иначе false
    bool isValid();

    // Определяем, является ли этот полигон прямой (т. е. имеет ровно 2 вершины)
    bool isLine();

    // Отбор усеченного полигона: проверка, находится ли этот многоугольник в границах усеченного вида
    bool isInFrustum(double xLow, double xHigh, double yLow, double yHigh);

    // Обрезать многоугольник по краям плоскости вида
    void clipToScreen(double xLow, double xHigh, double yLow, double yHigh);

    // Обрезать этот многоугольник в ближних / дальних плоскостях
    void clipHitherYon(double hither, double yon);

    // Определяем, находится ли этот многоугольник между hither и yon
    bool isInDepth(double hither, double yon);

    // Триангуляция этого многоугольника
    // Предварительное условие: полигон имеет >= 4 вершины
    // Возвращает: сетка, содержащая только треугольные грани. Каждый треугольник будет содержать первую вершину
    vector<Polygon>* getTriangulatedFaces();

    // Проверяем поворот вершины: определяем, смотрим ли мы на переднюю или заднюю часть многоугольника
    bool isFacingCamera();

    // Преобразовать этот многоугольник с помощью матрицы преобразования
    void transform(TransformationMatrix* theMatrix);

    // Преобразуем этот многоугольник с помощью матрицы преобразования, округляя его значения
    void transform(TransformationMatrix* theMatrix, bool doRound);

    // Получить количество вершин, содержащихся в этом многоугольнике
    int getVertexCount();

    // Проверяем, влияет ли окружающий свет на этот полигон
    bool isAffectedByAmbientLight();

    // Устанавливаем этот полигон под влиянием окружающего освещения
    void setAffectedByAmbientLight(bool newAmbientLit);

    // Обновляем цвета вершин этого многоугольника в зависимости от интенсивности окружающего освещения
    void lightAmbiently(double redIntensity, double greenIntensity, double blueIntensity);

    // Получить модель затенения этого многоугольника
    ShadingModel getShadingModel();

    // Установить модель затенения этого многоугольника
    void setShadingModel(ShadingModel newShadingModel);

    // Получить коэффициент отражения этого многоугольника
    double getSpecularCoefficient();

    // Установить коэффициент отражения этого многоугольника
    void setSpecularCoefficient(double newSpecCoefficient);

    // Получить зеркальный показатель этого многоугольника
    double getSpecularExponent();

    // Установить зеркальный показатель этого многоугольника
    void setSpecularExponent(double newSpecExponent);

    // Получить отражательную способность этого многоугольника
    double getReflectivity();

    // Установить отражательную способность этого многоугольника
    void setReflectivity(double newReflectivity);

    // Получить центр этого многоугольника как вершину
    Vertex getFaceCenter();

    // Получить нормаль поверхности этого многоугольника
    NormalVector getFaceNormal();

    // Получить среднее значение нормалей этого многоугольника
    NormalVector getNormalAverage();

    void debug();

    // Public polygon attributes:

    Vertex* vertices = nullptr; // Массив точек, которые описывают этот многоугольник

    NormalVector faceNormal;    // Предварительно вычисленная нормаль поверхности этого многоугольника

private:
    unsigned int vertexArraySize; // Размер массива вершин в этом многоугольнике
    unsigned int currentVertices; // Количество вершин, добавленных к этому многоугольнику

    bool isAmbientLit; // Окружающее освещение

    ShadingModel theShadingModel; // Модель закраски, которая будет использоваться для этого многоугольника
    double specularCoefficient = 0.3;
    double specularExponent = 8;
    double reflectivity = 0.5;


    // Проверяем, находится ли вершина в положительном полупространстве плоскости. Используется для обрезки полигонов.
    bool inside(Vertex V, Vertex P, NormalVector n);

    // Рассчитать векторное пересечение с плоскостью. Используется для обрезки полигонов.
    Vertex intersection(Vertex C, Vertex D, Vertex P, NormalVector n, bool doPerspectiveCorrect);

    // Вспомогательная функция: обрезает полигоны, используя алгоритм отсечения Сазерленда-Ходжмана
    Polygon clipHelper(Polygon source, Vertex P, NormalVector n, bool doPerspectiveCorrect);
};

#endif // POLYGON_H
