// Line object
// By Adam Badke

#include "line.h"
#include "vertex.h"

Line::Line(Vertex p1, Vertex p2)
{
    // Сохраняем точки в порядке слева направо
    if (p1.x <= p1.y){
        this->p1 = p1;
        this->p2 = p2;
    }
    else{
        this->p1 = p2;
        this->p2 = p1;
    }

    // Проверяем, совпадают ли цвета или нет
    if (this->p1.color == this->p2.color)
        sameColors = true;
    else
        sameColors = false;
}

// Определяем, имеет ли эта линия одинаковые цвета вершин
bool Line::hasSameVertexColors(){
    return sameColors;
}
