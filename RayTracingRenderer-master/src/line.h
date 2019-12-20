#ifndef LINE_H
#define LINE_H

#include "vertex.h"

class Line
{
public:
    // Конструктор
    Line(Vertex p1, Vertex p2);

    // Определите, имеет ли эта линия те же цвета вершин
    bool hasSameVertexColors();

    // Свойства линии:
    Vertex p1;
    Vertex p2;

private:
    bool sameColors;
};

#endif // LINE_H
