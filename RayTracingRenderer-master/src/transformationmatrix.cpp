#include "transformationmatrix.h"
#include <iostream>
#include <QDebug>

#define _USE_MATH_DEFINES       // Allow use of M_PI (3.14159265358979323846)
#include "math.h"

using std::cout;


TransformationMatrix::TransformationMatrix(){
    CTM = new double*[DIMENSION];

    for (int row = 0; row < DIMENSION; row++){
        CTM[row] = new double[DIMENSION];
        for (int col = 0; col < DIMENSION; col++){
            if (row == col)
                CTM[row][col] = 1;
            else
                CTM[row][col] = 0;
        }
    }
}


TransformationMatrix::TransformationMatrix(const TransformationMatrix& existingMatrix){
    CTM = new double*[DIMENSION];

    for (int row = 0; row < DIMENSION; row++){
        CTM[row] = new double[DIMENSION];
        for (int col = 0; col < DIMENSION; col++){
            this->CTM[row][col] = existingMatrix.CTM[row][col];
        }
    }
}


TransformationMatrix& TransformationMatrix::operator=(const TransformationMatrix& rhs){
    for (int row = 0; row < DIMENSION; row++){
        for (int col = 0; col < DIMENSION; col++){
            this->CTM[row][col] = rhs.CTM[row][col];
        }
    }
    return *this;
}


TransformationMatrix::~TransformationMatrix(){
    if (CTM != nullptr){
        for (int i = 0; i < DIMENSION; i++){
                delete [] CTM[i];
        }
    }
    delete [] CTM;
}


void TransformationMatrix::addScaleUniform(double scalar){

    TransformationMatrix scale;
    scale.arrayVal(0, 0) = scalar; // X
    scale.arrayVal(1, 1) = scalar; // Y
    scale.arrayVal(2, 2) = scalar; // Z


    *this *= scale;
}


void TransformationMatrix::addNonUniformScale(double x, double y, double z){
    TransformationMatrix scale;
    scale.arrayVal(0, 0) = x;
    scale.arrayVal(1, 1) = y;
    scale.arrayVal(2, 2) = z;


    *this *= scale;
}


void TransformationMatrix::addTranslation(double x, double y, double z){
    TransformationMatrix translate;
    translate.arrayVal(0, 3) = x;
    translate.arrayVal(1, 3) = y;
    translate.arrayVal(2, 3) = z;
    *this *= translate;
}


void TransformationMatrix::addRotation(Axis theAxis, double angle){

    angle = angle * M_PI/180;

    switch(theAxis){
    case 0:{
        TransformationMatrix rotateX;
        rotateX.arrayVal(1, 1) = cos(angle);
        rotateX.arrayVal(1, 2) = sin(angle);
        rotateX.arrayVal(2, 1) = -sin(angle);
        rotateX.arrayVal(2, 2) = cos(angle);

        *this *= rotateX;
    }
        break;

    case 1:{ // Y Axis
        TransformationMatrix rotateY;
        rotateY.arrayVal(0, 0) = cos(angle);
        rotateY.arrayVal(0, 2) = -sin(angle);
        rotateY.arrayVal(2, 0) = sin(angle);
        rotateY.arrayVal(2, 2) = cos(angle);

        *this *= rotateY;
    }
        break;

    case 2:{ // Z Axis
        TransformationMatrix rotateZ;
        rotateZ.arrayVal(0, 0) = cos(angle);
        rotateZ.arrayVal(0, 1) = sin(angle);
        rotateZ.arrayVal(1, 0) = -sin(angle);
        rotateZ.arrayVal(1, 1) = cos(angle);

        *this *= rotateZ;
    }
        break;

    default:
        cout << "ERROR! Invalid transformation axis!\n";
        break;
    }
}


int TransformationMatrix::size(){
    return DIMENSION;
}


double& TransformationMatrix::arrayVal(int x, int y) {
    return CTM[x][y];
}


TransformationMatrix& TransformationMatrix::operator*=(const TransformationMatrix& rhs){

    TransformationMatrix result;
    for (int i = 0; i < DIMENSION; i++){ // 0 out the identity matrix
        result.CTM[i][i] = 0;
    }


    for (int row = 0; row < DIMENSION; row++){
        for (int col = 0; col < DIMENSION; col++){
            for (int pos = 0; pos < DIMENSION; pos++){
                result.CTM[row][col] += this->CTM[row][pos] * rhs.CTM[pos][col]; //[lhs]*[rhs]
            }
        }
    }


    for (int row = 0; row < DIMENSION; row++){
        for (int col = 0; col < DIMENSION; col++){
            this->CTM[row][col] = result.CTM[row][col] ;
        }
    }
    return *this;
}


TransformationMatrix operator*(TransformationMatrix& lhs, TransformationMatrix& rhs){
    const int DIMENSION = 4;

    TransformationMatrix result;
    for (int row = 0; row < DIMENSION; row++){
        for (int col = 0; col < DIMENSION; col++){
            result.arrayVal(row, col) = 0;


            for (int pos = 0; pos < DIMENSION; pos++){
                result.arrayVal(row, col) += (lhs.arrayVal(row, pos) * rhs.arrayVal(pos, col));
            }
        }
    }
    return result;
}


TransformationMatrix TransformationMatrix::getInverse(){


    double** theCofactors = new double*[DIMENSION];
    for (int i = 0; i < DIMENSION; i++){
        theCofactors[i] = new double[DIMENSION];
        for (int j = 0; j < DIMENSION; j++){


            double** currentMinor = makeMinor(CTM, DIMENSION, i, j);
            int minorDimension = DIMENSION - 1;


            theCofactors[i][j] = getDeterminantRecursive(currentMinor, minorDimension);


            if ((i + j) % 2 != 0){
                theCofactors[i][j] *= -1;
            }


            for (int i = 0; i < minorDimension; i++){
                delete [] currentMinor[i];
            }
            delete [] currentMinor;
        }
    }


    double inverseDeterminant = 1 / this->getDeterminant();


    TransformationMatrix result;


    for (int i = 0; i < DIMENSION; i++){
        for (int j = 0; j < DIMENSION; j++){
            result.CTM[i][j] = inverseDeterminant * theCofactors[j][i];
        }
    }


    for (int i = 0; i < DIMENSION; i++){
        delete [] theCofactors[i];
    }
    delete theCofactors;

    return result;
}


double TransformationMatrix::getDeterminant(){
    return getDeterminantRecursive(CTM, DIMENSION);
}


double TransformationMatrix::getDeterminantRecursive(double** theMatrix, int theDimension){
    if (theDimension == 2){
        return ((theMatrix[0][0] * theMatrix[1][1]) - (theMatrix[0][1] * theMatrix[1][0]));
    }


    double determinant = 0;
    double** theMinor;
    for (int coefficient = 0; coefficient < theDimension; coefficient++){
        theMinor = makeMinor(theMatrix, theDimension, 0, coefficient);;
        if (coefficient % 2 == 0){
            determinant += theMatrix[0][coefficient] * getDeterminantRecursive( theMinor, theDimension - 1);
        }
        else {
            determinant -= theMatrix[0][coefficient] * getDeterminantRecursive( theMinor, theDimension - 1);
        }

        for (int i = 0; i < theDimension - 1; i++){
            delete theMinor[i];
        }
        delete theMinor;
    }
    return determinant;
}


double** TransformationMatrix::makeMinor(double** theMajor, int currentDimension, int row, int col){

    int minorDimension = currentDimension - 1;
    double** theMinor = new double*[minorDimension];
    for (int i = 0; i < minorDimension; i++){
        theMinor[i] = new double[minorDimension];
        for (int j = 0; j < minorDimension; j++){
            theMinor[i][j] = 0;
        }
    }


    for (int minorRow = 0, majorRow = 0; minorRow < minorDimension; minorRow++, majorRow++){
        for (int minorCol = 0, majorCol = 0; minorCol < minorDimension; minorCol++, majorCol++){

            if (majorRow == row)
                majorRow++;
            if (majorCol == col)
                majorCol++;

            theMinor[minorRow][minorCol] = theMajor[majorRow][majorCol];
        }
    }
    return theMinor;
}


void TransformationMatrix::debug(){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++)
            cout << CTM[i][j] << " ";
        cout << "\n";
    }
    cout << "\n";
}
