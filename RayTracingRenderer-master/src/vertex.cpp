#include "vertex.h"
#include "transformationmatrix.h"
#include "renderutilities.h"
#include "normalvector.h"

#include <iostream>

using std::cout;

// Конструктор
Vertex::Vertex(){
    x = 0;
    y = 0;
    z = 0;
    color = DEFAULT_COLOR;
    w = 1;
}


Vertex::Vertex(double newX, double newY, double newZ){
    x = newX;
    y = newY;
    z = newZ;
    color = DEFAULT_COLOR;

    w = 1;
}


Vertex::Vertex(double newX, double newY, double newZ, unsigned int newColor){
    x = newX;
    y = newY;
    z = newZ;
    this->color = newColor;

    w = 1;
}


Vertex::Vertex(double newX, double newY, double newZ, double newW){
    x = newX;
    y = newY;
    z = newZ;
    w = newW;
    color = DEFAULT_COLOR;

    if (w != 0) // Handle vertices initialized with w != 0
        divideByW();
}


Vertex::Vertex(double newX, double newY, double newZ, unsigned int newColor, int newVertexNumber){
    x = newX;
    y = newY;
    z = newZ;
    color = newColor;
    vertexNumber = newVertexNumber;

    w = 1;
}


Vertex::Vertex(const Vertex &currentVertex){
    this->x = currentVertex.x;
    this->y = currentVertex.y;
    this->z = currentVertex.z;
    this->color = currentVertex.color;
    this->vertexNumber = currentVertex.vertexNumber;
    this->w = currentVertex.w;


    this->normal = currentVertex.normal;
}


Vertex& Vertex::operator=(const Vertex& rhs){
    if (this == &rhs)
        return *this;

    this->x = rhs.x;
    this->y = rhs.y;
    this->z = rhs.z;
    this->color = rhs.color;
    this->vertexNumber = rhs.vertexNumber;
    this->w = rhs.w;


    this->normal = rhs.normal;

    return *this;
}


bool Vertex::operator==(Vertex &otherVertex){
    return (   this->x/this->w == otherVertex.x/otherVertex.w
            && this->y/this->w == otherVertex.y/otherVertex.w
            && this->z/this->w == otherVertex.z/otherVertex.w
            );
}


bool Vertex::operator!=(Vertex &otherVertex){
    return !(  this->x/this->w == otherVertex.x/otherVertex.w
            && this->y/this->w == otherVertex.y/otherVertex.w
            && this->z/this->w == otherVertex.z/otherVertex.w
            );
}


Vertex Vertex::operator-(const Vertex& rhs) const {

    Vertex result(*this);
    result.x -= rhs.x;
    result.y -= rhs.y;
    result.z -= rhs.z;

    return result;
}


Vertex& Vertex::operator+=(const NormalVector& rhs){
    this->x += rhs.xn;
    this->y += rhs.yn;
    this->z += rhs.zn;

    return *this;
}


Vertex Vertex::operator+(const Vertex& rhs) const{
    Vertex result(*this);
    result.x += rhs.x;
    result.y += rhs.y;
    result.z += rhs.z;

    return result;
}


Vertex Vertex::operator+(const NormalVector& rhs) const{
    Vertex result = (*this);
    result.x += rhs.xn;
    result.y += rhs.yn;
    result.z += rhs.zn;

    return result;
}


Vertex Vertex::operator*(double scale){
    Vertex result(*this);

    result.x *= scale;
    result.y *= scale;
    result.z *= scale;

    return result;
}


double Vertex::dot(NormalVector theNormal){
    double result = 0;
    result += this->x * theNormal.xn;
    result += this->y * theNormal.yn;
    result += this->z * theNormal.zn;

    return result;
}


double Vertex::length(){
    return sqrt( (x * x) + (y * y) + (z * z) );
}


void Vertex::transform(TransformationMatrix* theMatrix){
    transform(theMatrix, false);
}


void Vertex::transform(TransformationMatrix* theMatrix, bool doRound){


    double coords[4];
    coords[0] = x;
    coords[1] = y;
    coords[2] = z;
    coords[3] = w;


    double result[4];
    for (int i = 0; i < 4; i++)
        result[i] = 0;


    for (int row = 0; row < theMatrix->size(); row++){
        for (int col = 0; col < theMatrix->size(); col++){
            result[row] += (theMatrix->arrayVal(row, col) * coords[col]);
        }
    }

    if (doRound){
        x = round(result[0]);
        y = round(result[1]);
    }
    else{
        x = result[0];
        y = result[1];
    }
    z = result[2];
    w = result[3];


    if (w != 1)
        divideByW();


    if (!doRound){

        normal.transform(theMatrix);
    }
}


void Vertex::divideByW(){
    if (w != 1 && w != 0){
        x = x/w;
        y = y/w;
    }
    w = 1;
}


void Vertex::setW(double newW){
    w = newW;
    if (w != 1)
        divideByW();
}

void Vertex::debug(){
    cout << "Vertex: (" << x << ", " << y << ", " << z << ", " << w << ") color: " << std::hex << color << std::dec << " R: " << extractColorChannel(color, 1) << " G: " << extractColorChannel(color, 2) << " B: " << extractColorChannel(color, 3) << "\n";
    normal.debug();
}
