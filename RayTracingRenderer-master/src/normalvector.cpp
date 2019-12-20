#include "normalvector.h"
#include "renderutilities.h"
#include "math.h"
#include <iostream>

using std::cout;

// Конструктор по умолчанию
NormalVector::NormalVector(){
    xn = 0;
    yn = 0;
    zn = 0;
}

// XYZ конструктор
NormalVector::NormalVector(double newX, double newY, double newZ){
    xn = newX;
    yn = newY;
    zn = newZ;
}

// Скопировать конструктор
NormalVector::NormalVector(const NormalVector& rhs){
    this->xn = rhs.xn;
    this->yn = rhs.yn;
    this->zn = rhs.zn;
}

// Интерполяционный конструктор: Создает интерполированный вектор нормали на основе текущей позиции между начальной и конечной позициями
NormalVector::NormalVector(const NormalVector& lhs, double lhsZ, const NormalVector& rhs, double rhsZ, double current, double start, double end){

    double ratio;
    if (end - start == 0)
        ratio = 0;
    else
        ratio = (current - start) / (double)(end - start);

    this->xn = getPerspCorrectLerpValue(lhs.xn, lhsZ, rhs.xn, rhsZ, ratio);
    this->yn = getPerspCorrectLerpValue(lhs.yn, lhsZ, rhs.yn, rhsZ, ratio);
    this->zn = getPerspCorrectLerpValue(lhs.zn, lhsZ, rhs.zn, rhsZ, ratio);

    this->normalize();

}

// перегружен оператор присваивания
NormalVector& NormalVector::operator=(const NormalVector& rhs){
    if (this == &rhs)
        return *this;

    this->xn = rhs.xn;
    this->yn = rhs.yn;
    this->zn = rhs.zn;

    return *this;
}

// Нормализуем вектор
void NormalVector::normalize(){
    // Рассчитать обратную длину вектора
    double inverseLength = 1 / length();

    // Normalize:
    xn *= inverseLength;
    yn *= inverseLength;
    zn *= inverseLength;
}

// Получить длину этого нормального вектора
double NormalVector::length(){
    return sqrt( (xn * xn) + (yn * yn) + (zn * zn));
}

// Получить перекрестное произведение этого и другого вектора
NormalVector NormalVector::crossProduct(const NormalVector& rhs){
    return NormalVector( // Примечание: здесь мы переворачиваем «стандартный» порядок перекрестных произведений, чтобы инвертировать результирующий вектор, чтобы он работал с нашей левой системой координат
                ( (this->zn * rhs.yn) - (this->yn * rhs.zn) ),
                ( (this->xn * rhs.zn) - (this->zn * rhs.xn) ),
                ( (this->yn * rhs.xn) - (this->xn * rhs.yn) )
                );
}

// Получить скалярное произведение двух векторов
double NormalVector::dotProduct(const NormalVector& rhs){
    double result = 0;
    result += this->xn * rhs.xn;
    result += this->yn * rhs.yn;
    result += this->zn * rhs.zn;

    return result;
}

// перегружен скалярный оператор умножения
NormalVector& NormalVector::operator*=(const double rhs){

    this->xn *= rhs;
    this->yn *= rhs;
    this->zn *= rhs;

    return *this;
}

// перегружен скалярный оператор умножения
NormalVector NormalVector::operator*(double scalar){
    NormalVector result(*this);

    result.xn = this->xn * scalar;
    result.yn = this->yn * scalar;
    result.zn = this->zn * scalar;

    return result;
}

// перегружен оператор вычитания
NormalVector NormalVector::operator-(const NormalVector& rhs){
    NormalVector newNormal(*this);
    newNormal.xn -= rhs.xn;
    newNormal.yn -= rhs.yn;
    newNormal.zn -= rhs.zn;

    return newNormal;
}

// перегружен -= оператор
NormalVector& NormalVector::operator-=(const NormalVector& rhs){
    this->xn -= rhs.xn;
    this->yn -= rhs.yn;
    this->zn -= rhs.zn;

    return *this;
}

// Определяем, является ли это нормальное значение (0, 0, 0)
bool NormalVector::isZero(){
    return (xn == 0 && yn == 0 && zn == 0);
}

// Преобразуем нормаль с помощью матрицы преобразования
void NormalVector::transform(TransformationMatrix* theMatrix){

    // Копируем нашу координату в массив, чтобы упростить наши вычисления
    double coords[4];
    coords[0] = xn;
    coords[1] = yn;
    coords[2] = zn;

    // Создать массив для хранения наших результатов
    double result[3];
    for (int i = 0; i < 3; i++)
        result[i] = 0;

    // Умножаем матрицу преобразования и координатный 4-векторный массив
    for (int row = 0; row < 3; row++){
        for (int col = 0; col < 3; col++){
            result[row] += (theMatrix->arrayVal(row, col) * coords[col]); // Умножение: [xform]*[x, y, z]
        }
    }

    xn = result[0];
    yn = result[1];
    zn = result[2];

    // Повторная нормализация
    normalize();
}

// Обратное направление этого вектора
void NormalVector::reverse(){
    xn *= -1;
    yn *= -1;
    zn *= -1;
}

void NormalVector::debug(){
    cout << "normal: (" << xn << ", " << yn << ", " << zn << ")\n";
}
