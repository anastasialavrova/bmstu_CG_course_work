#ifndef MESH_H
#define MESH_H

#include "polygon.h"
#include <vector>

using std::vector;
class Mesh;

class Mesh
{
public:
    // Конструктор
    Mesh();

    // Конструктор копирования
    Mesh(const Mesh &existingMesh);

    // Перегрузка оператора присваивания
    Mesh& operator=(const Mesh& rhs);

    // Преобразование сетки с помощью матрицы преобразования
    void transform(TransformationMatrix* theMatrix);

    // Преобразование сетки с помощью матрицы преобразования
    void transform(TransformationMatrix* theMatrix, bool doRound);

    // Создать / обновить ограничивающий прямоугольник вокруг граней этой сетки
    // Условие: сетка имеет хотя бы 1 полигон
    void generateBoundingBox();

    // Debug this mesh
    void debug();

    // Mesh attributes:
    vector<Polygon> faces; // Набор граней этой сетки
    vector<Polygon> boundingBoxFaces;    // Набор из 6 граней, образующих ограничивающий прямоугольник вокруг этого многоугольника
    bool isWireframe = false; // Должны ли полигоны этой сетки отображаться в каркасном виде или заполняться
};

#endif // MESH_H
