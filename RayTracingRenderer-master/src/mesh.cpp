#include "mesh.h"
#include "polygon.h"
#include <iostream>

using std::cout;

// Конструктор
Mesh::Mesh(){
    isWireframe = true; // По умолчанию заполнено

    faces.reserve(3);
    boundingBoxFaces.reserve(6);
}

// Копировать конструктор
Mesh::Mesh(const Mesh &existingMesh){
    faces = existingMesh.faces;
    isWireframe = existingMesh.isWireframe;

    boundingBoxFaces = existingMesh.boundingBoxFaces;
}

// Перегруженный оператор присваивания
Mesh& Mesh::operator=(const Mesh& rhs){
    this->faces = rhs.faces;
    this->isWireframe = rhs.isWireframe;

    this->boundingBoxFaces = rhs.boundingBoxFaces;

    return *this;
}

// Преобразовать этот многоугольник с помощью матрицы преобразования
void Mesh::transform(TransformationMatrix* theMatrix){
    transform(theMatrix, false);
}

// Преобразовать этот многоугольник с помощью матрицы преобразования
void Mesh::transform(TransformationMatrix* theMatrix, bool doRound){

    // Преобразуем грани видимой сетки:
    for(unsigned int i = 0; i < faces.size(); i++){
        faces[i].transform(theMatrix, doRound);
    }

    // Преобразование граней ограничительной рамки:
    for (unsigned int i = 0; i < boundingBoxFaces.size(); i++){
        boundingBoxFaces[i].transform(theMatrix, doRound);
    }
}

// Создать / обновить ограничивающий прямоугольник вокруг граней этой сетки
// Условие: сетка имеет хотя бы 1 полигон
void Mesh::generateBoundingBox(){

    double xMin = faces[0].vertices[0].x;
    double xMax = faces[0].vertices[0].x;
    double yMin = faces[0].vertices[0].y;
    double yMax = faces[0].vertices[0].y;
    double zMin = faces[0].vertices[0].z;
    double zMax = faces[0].vertices[0].z;

    for (int i = 0; i < faces.size(); i++){

        // Получить количество вершин для текущего многоугольника один раз:
        int numVertices = faces[i].getVertexCount();

        // Перебираем каждую вершину в многоугольнике
        for (int j = 0; j < numVertices; j++){

            if (faces[i].vertices[j].x < xMin)
                xMin = faces[i].vertices[j].x;

            if (faces[i].vertices[j].x > xMax)
                xMax = faces[i].vertices[j].x;

            if (faces[i].vertices[j].y < yMin)
                yMin = faces[i].vertices[j].y;

            if (faces[i].vertices[j].y > yMax)
                yMax = faces[i].vertices[j].y;

            if (faces[i].vertices[j].z < zMin)
                zMin = faces[i].vertices[j].z;

            if (faces[i].vertices[j].z > zMax)
                zMax = faces[i].vertices[j].z;
        }
    }

    double swell = 0.001;

    // Убедитесь, что ограничивающая рамка не плоская:
    if (xMin == xMax){
        xMin -= swell;
        xMax += swell;
    }

    if (yMin == yMax){
        yMin -= swell;
        yMax += swell;
    }

    if (zMin == zMax){
        zMin -= swell;
        zMax += swell;
    }

    // Собираем 6 сторон ограничивающего куба, используя наши минимальные / максимальные координаты
    Polygon left, right, front, back, top, bottom;

    Vertex frontLeftTop(xMin, yMax, zMin);
    Vertex frontLeftBot(xMin, yMin, zMin); // Mins
    Vertex frontRightTop(xMax, yMax, zMin);
    Vertex frontRightBot(xMax, yMin, zMin);

    Vertex backLeftTop(xMin, yMax, zMax);
    Vertex backLeftBot(xMin, yMin, zMax);
    Vertex backRightTop(xMax, yMax, zMax); // Maxs
    Vertex backRightBot(xMax, yMin, zMax);

    left.addVertex( backLeftTop );
    left.addVertex( backLeftBot);
    left.addVertex( frontLeftBot );
    left.addVertex( frontLeftTop );

    right.addVertex( frontRightTop );
    right.addVertex( frontRightBot );
    right.addVertex( backRightBot );
    right.addVertex( backRightTop );

    top.addVertex( backLeftTop );
    top.addVertex( frontLeftTop );
    top.addVertex( frontRightTop );
    top.addVertex( backRightTop );

    bottom.addVertex( frontLeftBot );
    bottom.addVertex( backLeftBot );
    bottom.addVertex( backRightBot );
    bottom.addVertex( frontRightBot );

    front.addVertex( frontLeftTop );
    front.addVertex( frontLeftBot );
    front.addVertex( frontRightBot );
    front.addVertex( frontRightTop );

    back.addVertex( backRightTop );
    back.addVertex( backRightBot );
    back.addVertex( backLeftBot );
    back.addVertex( backLeftTop );

    // Генерируем нормали поверхности для каждой грани ограничительной рамки:
    left.faceNormal = left.getFaceNormal();
    right.faceNormal = right.getFaceNormal();
    front.faceNormal = front.getFaceNormal();
    back.faceNormal = back.getFaceNormal();
    top.faceNormal = top.getFaceNormal();
    bottom.faceNormal = bottom.getFaceNormal();

    // Добавить грани в ограничительную рамку:
    boundingBoxFaces.emplace_back(left);
    boundingBoxFaces.emplace_back(right);
    boundingBoxFaces.emplace_back(top);
    boundingBoxFaces.emplace_back(bottom);
    boundingBoxFaces.emplace_back(front);
    boundingBoxFaces.emplace_back(back);

}

