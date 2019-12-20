#ifndef NORMALVECTOR_H
#define NORMALVECTOR_H

#include "transformationmatrix.h"

class NormalVector
{
public:
    // Конструктор по умолчанию
    NormalVector();

    // XYZ Конструктор
    NormalVector(double newX, double newY, double newZ);

    // Конструктор копирования
    NormalVector(const NormalVector& rhs);

    // Интерполяционный конструктор: создает интерполированный нормальный вектор на основе текущего между началом и концом
    NormalVector(const NormalVector& lhs, double lhsZ, const NormalVector& rhs, double rhsZ, double current, double start, double end);

    // Перегрузка оператора присваивания
    NormalVector& operator=(const NormalVector& rhs);

    // Нормализация вектора
    void normalize();

    // Получить длину этого нормализованного вектора
    double length();

    // Получить векторное произведение двух векторов
    NormalVector crossProduct(const NormalVector& rhs);

    // Получите скалярное произведение двух векторов
    double dotProduct(const NormalVector& rhs);

    // Перегрузка оператора умножения
    NormalVector& operator*=(const double rhs);

    // Перегрузка оператора умножения
    NormalVector operator*(double scalar);

    // Перегрузка оператора вычитания
    NormalVector operator-(const NormalVector& rhs);

    // Перегрузка оператора вычитания
    NormalVector& operator-=(const NormalVector& rhs);

    // Детерминировать, если нормальный вектор (0,0,0)
    bool isZero();

    // Преобразовать нормальный вектор с помощью матрицы преобразования
    void transform(TransformationMatrix* theMatrix);

    // Изменить направление вектора
    void reverse();


    // Атрибуты нормального вектора:

    double xn = 0;
    double yn = 0;
    double zn = 0;


    void debug();
};

#endif // NORMALVECTOR_H
