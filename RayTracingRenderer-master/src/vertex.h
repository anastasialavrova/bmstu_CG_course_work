#ifndef VERTEX_H
#define VERTEX_H

#include "transformationmatrix.h"
#include "math.h"
#include "normalvector.h"

class Vertex{
public:
    // Конструктор
    Vertex();

    // Устанавливает x, y, z и использует цвет по умолчанию (отладка)
    Vertex(double newX, double newY, double newZ);

    // Устанавливает x, y, z и цвет
    Vertex(double newX, double newY, double newZ, unsigned int newColor);

    // Устанавливает x, y, z, w
    Vertex(double newX, double newY, double newZ, double newW);

    // Устанавливает x, y, z, цвет и номер вершины. Для использования объектом многоугольника при увеличении количества вершин
    Vertex(double newX, double newY, double newZ, unsigned int newColor, int vertexNumber);

    // Конструктор копирования
    Vertex(const Vertex &currentVertex);

    // Перегрузка оператора присваивания
    Vertex& operator=(const Vertex& rhs);

    // Проверяет вершины на пространственное равенство
    // Возвращает true, если эти вершины занимают одну и ту же относительную позицию в трехмерном пространстве, иначе false
    bool operator==(Vertex &otherVertex);

    // Проверяет вершины на наличие специального неравенства
    // Возвращает true, если эти вершины НЕ занимают одинаковую относительную позицию в трехмерном пространстве, иначе false
    bool operator!=(Vertex &otherVertex);

    // Перегрузка оператора вычитания двух вершин
    Vertex operator-(const Vertex& rhs) const;

    // Перегрузка оператора сложения вершины и нормали
    Vertex& operator+=(const NormalVector& rhs);

    // Перегрузка оператора сложения двух вершин
    Vertex operator+(const Vertex& rhs) const;

    // Перегрузка оператора сложения двух вершин
    Vertex operator+(const NormalVector& rhs) const;

    // Перегрузка оператора умножения (скалярно)
    Vertex operator*(double scale);

    // Выполнить скалярное произведение: Вершинный вектор, скалярное произведение, нормальный вектор.
    double dot(NormalVector theNormal);

    //Получить длину вектора этой вершины (т.е. ее расстояние от начала координат)
    double length();

    // Установить номер вершины
    void setVertexNumber(int newVertexNumber);

    // Преобразовать вершину с помощью матрицы преобразования
    void transform(TransformationMatrix* theMatrix);

    // Преобразовать вершину с помощью матрицы преобразования и округлить компоненты x / y.
    // Используется для преобразования в экранное пространство для предотвращения ошибок округления
    void transform(TransformationMatrix* theMatrix, bool doRound);

    // Обновите компонент W этой вершины
    void setW(double newW);

    void debug();

    // Свойства вершины:
    double x;
    double y;
    double z;
    unsigned int color = 0xffff00ff; // Назначаем для вершины цвет по умолчанию ярко-розовый (переопределяется цветами вершин и / или командами поверхности)
    unsigned int vertexNumber; // Индекс этой вершины в содержащем его массиве вершин многоугольника

    // Нормаль вершины:
    NormalVector normal;

private:
    static const unsigned int DEFAULT_COLOR = 0xffffffff; // Цвет по умолчанию для всех вершин, если ни одна не назначена
    double w; // Общий делитель

    // Делит вектор на координату W. Сбрасывает общий знаменатель как 1. Используется при добавлении перспективных преобразований
    void divideByW();
};


#endif // VERTEX_H
