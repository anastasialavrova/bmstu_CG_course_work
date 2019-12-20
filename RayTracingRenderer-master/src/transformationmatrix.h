#ifndef TRANSFORMATIONMATRIX_H
#define TRANSFORMATIONMATRIX_H

// Перечислитель оси
enum Axis {
    X = 0,
    Y = 1,
    Z = 2
};

class TransformationMatrix
{
public:
    // Конструктор
    TransformationMatrix();

    // Конструктор копирования
    TransformationMatrix(const TransformationMatrix& existingMatrix);

    // Перегрузка оператора присваивания
    TransformationMatrix& operator=(const TransformationMatrix& rhs);

    // Деструктор
    ~TransformationMatrix();

    // Умножение матрицы (скалярное)
    void addScaleUniform(double scalar);

    // Добавить неравномерную шкалу к этой матрице
    void addNonUniformScale(double x, double y, double z);

    // Добавить перенос к этой матрице в (x, y, z)
    void addTranslation(double x, double y, double z);

    // Добавить поворот к этой матрице
    void addRotation(Axis theAxis, double angle);

    // Перегрузка оператора умножения
    TransformationMatrix& operator*=(const TransformationMatrix& rhs);

    // Получить размер этой матрицы
    int size();

    // Доступ к значениям массива
    double& arrayVal(int x, int y);

    // Получить обратное: вычислить обратное этой матрицы и вернуть int
    TransformationMatrix getInverse();


    void debug();

private:
    // Свойства матрицы
    static const int DIMENSION = 4;

    double** CTM;// Матрица преобразования


    // Вычисляем определитель этой матрицы: вызывает рекурсивную вспомогательную функцию
    double getDeterminant();

    // Рекурсивная вспомогательная функция: получить определитель матрицы
    double getDeterminantRecursive(double** theMatrix, int theDimension);

    // Сделать второстепенную матрицу из текущей матрицы
    double** makeMinor(double** theMatrix, int currentDimension, int row, int col);
};

TransformationMatrix operator*(TransformationMatrix& lhs, TransformationMatrix& rhs);

#endif // TRANSFORMATIONMATRIX_H
