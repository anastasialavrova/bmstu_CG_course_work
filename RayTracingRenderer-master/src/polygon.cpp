#include <iostream>
#include "polygon.h"
#include "renderutilities.h"
#include <vector>
#include "normalvector.h"
#include <cmath>

using std::vector;
using std::cout;

// Конструктор
Polygon::Polygon(){ //
    vertexArraySize = 3; // Выделяем 3 вершины (для треугольника)

    vertices = new Vertex[vertexArraySize];
    currentVertices = 0;

    isAmbientLit = false;

    // Установить модель затенения только для ambient, по умолчанию:
    theShadingModel = ambientOnly;
}

// Конструктор треугольника
Polygon::Polygon(Vertex p0, Vertex p1, Vertex p2){
    vertexArraySize = 3; // Выделяем 3 вершины (для треугольника)

    vertices = new Vertex[vertexArraySize];
    vertices[0] = Vertex{p0.x, p0.y, p0.z, p0.color};
    vertices[1] = Vertex{p1.x, p1.y, p1.z, p1.color};
    vertices[2] = Vertex{p2.x, p2.y, p2.z, p2.color};

    for (unsigned int i = 0; i < vertexArraySize; i++) // Записать номера вершин
        vertices[i].vertexNumber = i;

    currentVertices = 3;

    isAmbientLit = false;

    // Установить модель затенения только для ambient, по умолчанию:
    theShadingModel = ambientOnly;

    faceNormal = this->getFaceNormal();
}

// Копировать конструктор
Polygon::Polygon(const Polygon& currentPoly){
    this->vertexArraySize = currentPoly.vertexArraySize;
    this->currentVertices = currentPoly.currentVertices;

    this->isAmbientLit = currentPoly.isAmbientLit;

    this->theShadingModel = currentPoly.theShadingModel;
    this->specularCoefficient = currentPoly.specularCoefficient;
    this->specularExponent = currentPoly.specularExponent;
    this->reflectivity = currentPoly.reflectivity;

    this->vertices = new Vertex[vertexArraySize];
    for (unsigned int i = 0; i < vertexArraySize; i++){
        this->vertices[i] = currentPoly.vertices[i];
    }

    this->faceNormal = currentPoly.faceNormal;
}

// перегружен оператор присваивания
Polygon& Polygon::operator=(const Polygon& rhs){
    if (this == &rhs)
        return *this;

    this->vertexArraySize = rhs.vertexArraySize;
    this->currentVertices = rhs.currentVertices;

    this->isAmbientLit = rhs.isAmbientLit;

    this->theShadingModel = rhs.theShadingModel;
    this->specularCoefficient = rhs.specularCoefficient;
    this->specularExponent = rhs.specularExponent;
    this->reflectivity = rhs.reflectivity;

    if (vertices != nullptr)
        delete[] vertices;

    this->vertices = new Vertex[vertexArraySize];
    for (unsigned int i = 0; i < currentVertices; i++)
        this->vertices[i] = rhs.vertices[i];

    this->faceNormal = rhs.faceNormal;

    return *this;
}

Polygon::~Polygon(){
    if (vertices == nullptr)
        return;

    delete[] vertices;
}

// Удалить все вершины из массива вершин этого многоугольника
void Polygon::clearVertices(){
    if (vertices == nullptr)
        return;

    delete[] vertices;

    vertexArraySize = 3;

    vertices = new Vertex[vertexArraySize];
    currentVertices = 0;
}

// Добавить вершину к многоугольнику.
// Предварительное условие: вершины всегда добавляются в порядке против часовой стрелки (вершины [i + 1] = CCW, вершины [i-1] = CW)
void Polygon::addVertex(Vertex newPoint){
    // Если в массиве вершин еще осталось место:
    if (currentVertices < vertexArraySize){

        vertices[currentVertices] = Vertex(newPoint.x, newPoint.y, newPoint.z, newPoint.color, currentVertices);
        vertices[currentVertices].normal = newPoint.normal;

        currentVertices++;

        return;
    }
    else{
        Vertex* newVertices = new Vertex[vertexArraySize + 1];
        for (unsigned int i = 0; i < vertexArraySize; i++){
            newVertices[i] = vertices[i];
        }
        delete [] vertices;

        newPoint.vertexNumber = vertexArraySize;
        newVertices[vertexArraySize] = Vertex(newPoint);

        vertexArraySize++;
        currentVertices++;

        vertices = newVertices;
    }

    return;
}

// Получить вершину с наибольшим значением y. Используется средством визуализации для рисования этого многоугольника.
// Возвращает: самая высокая вершина в этом многоугольнике, с точки зрения координат y
// Предварительное условие: предполагается, что этот многоугольник имеет как минимум 3 действительные точки.
Vertex* Polygon::getHighest(){
    double highestY = vertices[0].y;
    int highestVertex = 0;
    for (unsigned int i = 1; i < currentVertices; i++){
        if (vertices[i].y > highestY){
            highestY = vertices[i].y;
            highestVertex = i;
        }
    }

    return &vertices[highestVertex];
}

// Получить вершину с наименьшим значением y. Используется средством визуализации для рисования этого многоугольника.
// Возвращает: самая низкая вершина, с точки зрения координат y
// Предварительное условие: предполагается, что этот многоугольник имеет как минимум 3 действительные точки.
Vertex* Polygon::getLowest(){
    double lowestY = vertices[0].y;
    int lowestVertex = 0;
    for (unsigned int i = 0; i < currentVertices; i++)
        if (vertices[i].y < lowestY){
            lowestY = vertices[i].y;
            lowestVertex = i;
        }
    return &vertices[lowestVertex];
}

// Получить следующую вершину в массиве вершин
Vertex* Polygon::getNext(unsigned int currentVertex){
    if (currentVertex >=  vertexArraySize - 1)
        return &vertices[0];
    else
        return &vertices[currentVertex + 1];
}

// Получить предыдущую вершину в массиве вершин
Vertex* Polygon::getPrev(unsigned int currentVertex){
    if (currentVertex <= 0)
        return &vertices[vertexArraySize - 1];
    else
        return &vertices[currentVertex - 1];
}

// Получить последнюю вершину в массиве вершин
Vertex* Polygon::getLast(){
    return &vertices[currentVertices - 1];
}

// Устанавливаем все вершины в этом многоугольнике в один цвет
void Polygon::setSurfaceColor(unsigned int solidColor){
    for (unsigned int i = 0; i < currentVertices; i++)
            vertices[i].color = solidColor;
}

// Определяем, содержат ли вершины этого многоугольника более одного цвета
// Возвращает true, если все вершины имеют одинаковый цвет (или если многоугольник имеет 0 вершин)
bool Polygon::isSolidColor(){
    if (currentVertices > 0){
        for (unsigned int i = 1; i < currentVertices; i++){
            if (vertices[i].color != vertices[0].color){
                return false;
            }
        }
    }
    return true;
}

// Определяем, правильно ли инициализирован этот полигон.
// Возвращает true, если у Polygon есть хотя бы 2 вершины, иначе false
bool Polygon::isValid(){
    return currentVertices >= 2;
}

// Определяем, является ли этот полигон прямой (т. е. имеет ровно 2 вершины)
bool Polygon::isLine(){
    return (currentVertices == 2);
}

// Определяем, находится ли этот многоугольник между hither и yon
bool Polygon::isInDepth(double hither, double yon){
    bool beforeHither = false;
    bool afterYon = false;

    for (unsigned int i = 0; i < currentVertices; i++){
        if (vertices[i].z >= hither && vertices[i].z <= yon){
            return true;
        }
        if (vertices[i].z < hither)
            beforeHither = true;
        if (vertices[i].z > yon)
            afterYon = true;
    }

    return beforeHither && afterYon;
}

// Отбор усеченного полигона: проверка, находится ли этот многоугольник в границах усеченного вида
bool Polygon::isInFrustum(double xLow, double xHigh, double yLow, double yHigh){

    bool belowXLow = true;
    bool aboveXHigh = true;
    bool belowYLow = true;
    bool aboveYHigh = true;
    for (unsigned int i = 0; i < currentVertices; i++){
        if (vertices[i].x > xLow){
            belowXLow = false;
        }
        if (vertices[i].x < xHigh){
            aboveXHigh = false;
        }
        if (vertices[i].y > yLow){
            belowYLow = false;
        }
        if (vertices[i].y < yHigh){
            aboveYHigh = false;
        }
    }

    return (!(belowXLow || aboveXHigh || belowYLow || aboveYHigh));
}


// Обрезать многоугольник на ближнюю / дальнюю плоскости
void Polygon::clipHitherYon(double hither, double yon){
    Vertex hitherPlane(0, 0, hither);
    NormalVector hitherNormal(0, 0, 1);

    *this = clipHelper(*this, hitherPlane, hitherNormal, false);

    Vertex yonPlane(0, 0, yon);
    NormalVector yonNormal(0, 0, -1);

    if (this->currentVertices > 1)
        *this = clipHelper(*this, yonPlane, yonNormal, false);

    return;
}

// Прикрепить многоугольник к усеченному виду
void Polygon::clipToScreen(double xLow, double xHigh, double yLow, double yHigh){

    Vertex boundaryPlane[4];
    boundaryPlane[0] = Vertex (xLow, yHigh, 0);     // topLeft
    boundaryPlane[1] = Vertex (xLow, yLow, 0);      // botLeft
    boundaryPlane[2] = Vertex (xHigh, yLow, 0);     // botRight
    boundaryPlane[3] = Vertex (xHigh, yHigh, 0);    // topRight

    for (int i = 0; i < 3; i++){
        if (this->currentVertices > 2)
            *this = clipHelper(*this, boundaryPlane[i], NormalVector(boundaryPlane[i].y - boundaryPlane[i + 1].y, boundaryPlane[i + 1].x - boundaryPlane[i].x, 0 ), true );

    }
    if (this->currentVertices > 2)
        *this = clipHelper(*this, boundaryPlane[3], NormalVector(boundaryPlane[3].y - boundaryPlane[0].y, boundaryPlane[0].x - boundaryPlane[3].x, 0 ), true );
}

// Вспомогательная функция: обрезает полигоны, используя алгоритм отсечения Сазерленда-Ходжмана 2D
Polygon Polygon::clipHelper(Polygon source, Vertex planePoint, NormalVector planeNormal, bool doPerspectiveCorrect){

    Polygon result;
    result.isAmbientLit = source.isAmbientLit;

    result.theShadingModel = source.theShadingModel;
    result.specularCoefficient = source.specularCoefficient;
    result.specularExponent = source.specularExponent;
    result.reflectivity = source.reflectivity;

    result.faceNormal = source.faceNormal;

    bool dontAddLast = false;
    int last = source.getVertexCount() - 1;

    Vertex D = *source.getLast();
    bool Din = inside(D, planePoint, planeNormal);
    if (Din){
        result.addVertex(D);

        dontAddLast = true;
    }

    for (int i = 0; i < source.getVertexCount(); i++){
        Vertex C = D;
        bool Cin = Din;

        D = source.vertices[i];
        Din = inside(D, planePoint, planeNormal);

        if (Din != Cin)
            result.addVertex(intersection(C, D, planePoint, planeNormal, doPerspectiveCorrect) );

        if (Din && (i != last || dontAddLast == false)){
            result.addVertex(D);
        }
    }

    return result;
}

// Проверяем, находится ли вершина в положительном полупространстве плоскости. Используется для обрезки полигонов.
bool Polygon::inside(Vertex theVertex, Vertex thePlane, NormalVector planeNormal){
    return (theVertex - thePlane).dot(planeNormal) >= 0; // Compare vector pointing from plane towards point against the face normal
}

// Рассчитать векторное пересечение с плоскостью. Используется для обрезки полигонов.
Vertex Polygon::intersection(Vertex prevVertex, Vertex currentVertex, Vertex planePoint, NormalVector planeNormal, bool doPerspectiveCorrect){

    Vertex distance = currentVertex - prevVertex;

    double ratio = (planePoint - prevVertex).dot(planeNormal)/(double)((distance).dot(planeNormal));

    Vertex intersectionPoint = prevVertex + (distance * ratio);

    if (doPerspectiveCorrect)
        intersectionPoint.z = getPerspCorrectLerpValue(prevVertex.z, prevVertex.z, currentVertex.z, currentVertex.z, ratio);

    intersectionPoint.color = addColors( multiplyColorChannels(prevVertex.color, 1 - ratio), multiplyColorChannels(currentVertex.color, ratio) );

    intersectionPoint.normal.xn = (prevVertex.normal.xn * (1 - ratio)) + (currentVertex.normal.xn * ratio);
    intersectionPoint.normal.yn = (prevVertex.normal.yn * (1 - ratio)) + (currentVertex.normal.yn * ratio);
    intersectionPoint.normal.zn = (prevVertex.normal.zn * (1 - ratio)) + (currentVertex.normal.zn * ratio);

    intersectionPoint.normal.normalize();

    return intersectionPoint;
}

// Проверяем обмотку вершины: определяем, смотрим ли мы на переднюю или заднюю часть многоугольника
bool Polygon::isFacingCamera(){

    double sum = 0;
    for (unsigned int i = 0; i < currentVertices - 1; i++){
        sum += (vertices[i + 1].x - vertices[i].x) * (vertices[i + 1].y + vertices[i].y);
    }
    sum += (vertices[0].x - vertices[currentVertices - 1].x) * (vertices[0].y + vertices[currentVertices - 1].y);

    if (sum > 0){
        return false;
    }

    return true;
}

// Преобразовать этот многоугольник с помощью матрицы преобразования
void Polygon::transform(TransformationMatrix* theMatrix){
    transform(theMatrix, false);
}

// Преобразуем этот многоугольник с помощью матрицы преобразования, округляя его значения
void Polygon::transform(TransformationMatrix* theMatrix, bool doRound){
    for (unsigned int i = 0; i < currentVertices; i++){
        vertices[i].transform(theMatrix, doRound);
    }

    if (!doRound)
        faceNormal.transform(theMatrix);
}

// Получить количество вершин, содержащихся в этом многоугольнике
int Polygon::getVertexCount(){
    return currentVertices;
}

// Триангуляция этого многоугольника
// Предварительное условие: полигон имеет> = 4 вершины
// Возвращает: сетка, содержащая только треугольные грани. Каждый треугольник будет содержать первую вершину
vector<Polygon>* Polygon::getTriangulatedFaces(){
    vector<Polygon>* result = new vector<Polygon>();

    if (currentVertices < 4){
        result->emplace_back( *this );
        return result;
    }

    Vertex v1 = vertices[0];

    int index = 1;
    int lastIndex = currentVertices - 1;
    while (index < lastIndex){
        Polygon newFace(*this);
        newFace.clearVertices();

        newFace.addVertex(v1);

        newFace.addVertex(vertices[ index ]);
        newFace.addVertex(vertices[ index + 1 ]);
        index++;

        result->emplace_back(newFace);
    }

    return result;
}

// Проверяем, влияет ли окружающий свет на этот полигон
bool Polygon::isAffectedByAmbientLight(){
    return this->isAmbientLit;
}

// Обновляем цвета вершин этого многоугольника в зависимости от интенсивности окружающего освещения
void Polygon::lightAmbiently(double redIntensity, double greenIntensity, double blueIntensity){
    for (unsigned int i = 0; i < currentVertices; i++){
        vertices[i].color = multiplyColorChannels(vertices[i].color, 1, redIntensity, greenIntensity, blueIntensity);
    }
}

// Устанавливаем этот полигон под влиянием окружающего освещения
void Polygon::setAffectedByAmbientLight(bool newAmbientLit){
    isAmbientLit = newAmbientLit;
}

// Получить модель затенения этого многоугольника
ShadingModel Polygon::getShadingModel(){
    return theShadingModel;
}

// Установить модель затенения этого многоугольника
void Polygon::setShadingModel(ShadingModel newShadingModel){
    theShadingModel = newShadingModel;
}

// Получить зеркальный коэффициент этого многоугольника
double Polygon::getSpecularCoefficient(){
    return specularCoefficient;
}

// Установить зеркальный коэффициент этого многоугольника
void Polygon::setSpecularCoefficient(double newSpecCoefficient){
    specularCoefficient = newSpecCoefficient;
}

// Получить зеркальный показатель этого многоугольника
double Polygon::getSpecularExponent(){
    return specularExponent;
}

// Установить зеркальный показатель этого многоугольника
void Polygon::setSpecularExponent(double newSpecExponent){
    specularExponent = newSpecExponent;
}

// Получить отражательную способность этого многоугольника
double Polygon::getReflectivity(){
    return reflectivity;
}

// Установить отражательную способность этого многоугольника
void Polygon::setReflectivity(double newReflectivity){
    reflectivity = newReflectivity;
}

// Получить центр этого многоугольника как вершину
Vertex Polygon::getFaceCenter(){
    double xTotal = 0;
    double yTotal = 0;
    double zTotal = 0;
    for (unsigned int i = 0; i < currentVertices; i++){
        xTotal += vertices[i].x;
        yTotal += vertices[i].y;
        zTotal += vertices[i].z;
    }
    return Vertex(xTotal/currentVertices, yTotal/currentVertices, zTotal/currentVertices);
}

// Получить (нормализованную) нормаль поверхности этого многоугольника
// Предварительное условие: многоугольник - это треугольник (т.е. vertex_i! = Vertex_j)
NormalVector Polygon::getFaceNormal(){

    NormalVector lhs(vertices[1].x - vertices[0].x, vertices[1].y - vertices[0].y, vertices[1].z - vertices[0].z);  // 0 to 1
    NormalVector rhs(vertices[2].x - vertices[0].x, vertices[2].y - vertices[0].y, vertices[2].z - vertices[0].z);  // 0 to 2

    lhs = lhs.crossProduct(rhs);
    lhs.normalize();

    return lhs;
}

// Получить среднее значение нормали этого многоугольника
NormalVector Polygon::getNormalAverage(){
    NormalVector average;

    for (unsigned int i = 0; i < currentVertices; i++){
        average.xn += vertices[i].normal.xn;
        average.yn += vertices[i].normal.yn;
        average.zn += vertices[i].normal.zn;
    }

    average.xn /= currentVertices;
    average.yn /= currentVertices;
    average.zn /= currentVertices;

    average.normalize();

    return average;
}

// Debug this polygon
void Polygon::debug(){

    cout << "Polygon:\n";
    for (unsigned int i = 0; i < currentVertices; i++){
        vertices[i].debug();
    }
    cout << "Face ";
    faceNormal.debug();
    cout << "End Polygon.\n\n";
}

