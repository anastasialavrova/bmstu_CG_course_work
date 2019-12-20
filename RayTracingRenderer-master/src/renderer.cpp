#include "renderer.h"
#include "renderutilities.h"

#include <cmath>
#include <iostream>
#include "math.h"

#include <QDebug>

using std::round;
using std::cout;

// Конструктор
Renderer::Renderer(Drawable* newDrawable, int newXRes, int newYRes, int borderWidth){
    this->drawable = newDrawable;

    border = borderWidth;
    xRes = newXRes;
    yRes = newYRes;

    ZBuffer = new int*[yRes];
    for (int row = 0; row < yRes; row++){
        ZBuffer[row] = new int[xRes];
        for (int col = 0; col < xRes; col++){
            ZBuffer[row][col] = maxZVal;
        }
    }

    // Create a perspective transformation matrix:
    cameraToPerspective.arrayVal(3, 3) = 0; // Removes w component
    cameraToPerspective.arrayVal(3, 2) = 1; // Replaces w component with a copy of the z component
}

// Деструктор
Renderer::~Renderer(){
    for (int x = 0; x < xRes; x++){
        for (int y = 0; y < yRes; y++){
            delete ZBuffer[x];
        }
    }
    delete ZBuffer;
}

// Рисуем прямоугольник. Используется только для настройки цветов фона панели. Игнорирует z-буфер.
// Предварительное условие: координаты topLeft_ и botRight_ действительны и находятся в пространстве окна пользовательского интерфейса (т. е. (0,0) в левом верхнем углу экрана!)
void Renderer::drawRectangle(int topLeftX, int topLeftY, int botRightX, int botRightY, unsigned int color){
    for (int x = topLeftX; x <= botRightX; x++){
        for (int y = topLeftY; y <= botRightY; y++){
            drawable->setPixel(x, y, color);
        }
    }
    drawable->updateScreen();
}

// Рисуем линию
// Если theLine's p1.color! = P2.color, цвет линии будет LERP'd. Обновляет Z-Buffer.
void Renderer::drawLine(Line theLine, ShadingModel theShadingModel, bool doAmbient, double specularCoefficient, double specularExponent){

    // Обработка вертикальных линий:
    if (theLine.p1.x == theLine.p2.x){
        int y, y_max;
        double z;
        double z_slope = (theLine.p2.z - theLine.p1.z)/(double)(theLine.p2.y - theLine.p1.y); // How much we're moving in the z-axis for every unit along the y-axis

        if (theLine.p1.y > theLine.p2.y){ // Всегда рисуем снизу вверх
            Vertex temp = theLine.p1;
            theLine.p1 = theLine.p2;
            theLine.p2 = temp;
        }
        y = (int)theLine.p1.y;
        y_max = (int)theLine.p2.y;
        z = theLine.p1.z;

       // Нарисовать линию:
        while (y < y_max){

            double ratio = (y - theLine.p1.y)/(double)(theLine.p2.y - theLine.p1.y); // Get our current position as a ratio

            double correctZ = getPerspCorrectLerpValue(theLine.p1.z, theLine.p1.z, theLine.p2.z, theLine.p2.z, ratio);

            // Рисуем, только если мы перед текущей глубиной z-буфера
            if ( isVisible( (int)round(theLine.p1.x), y, correctZ) ){

                if (theShadingModel == phong){
                    // Вычисляем текущую позицию пикселя, как вершину в пространстве камеры:
                    Vertex currentPosition(theLine.p1.x, y, correctZ);        // Создаем вершину, представляющую текущую точку на линии сканирования

                    currentPosition.transform(&screenToPerspective); // Преобразование обратно в перспективное пространство

                    // Исправить трансформацию перспективы: трансформировать точку обратно в пространство камеры
                    currentPosition.x *= correctZ;
                    currentPosition.y *= correctZ;

                    // Установить правильную перспективу нормали
                    currentPosition.normal.xn = getPerspCorrectLerpValue(theLine.p1.normal.xn, theLine.p1.z, theLine.p2.normal.xn, theLine.p2.z, ratio);
                    currentPosition.normal.yn = getPerspCorrectLerpValue(theLine.p1.normal.yn, theLine.p1.z, theLine.p2.normal.yn, theLine.p2.z, ratio);
                    currentPosition.normal.zn = getPerspCorrectLerpValue(theLine.p1.normal.zn, theLine.p1.z, theLine.p2.normal.zn, theLine.p2.z, ratio);
                    currentPosition.normal.normalize(); // Нормализовать

                    currentPosition.color = getPerspCorrectLerpColor(&theLine.p1, &theLine.p2, ratio); // Получить базовый цвет (в перспективе)

                    // Создаем вектор вида: указывает от поверхности к камере
                    NormalVector viewVector(-currentPosition.x, -currentPosition.y, -currentPosition.z);
                    viewVector.normalize();

                   // Вычисляем значение освещенного пикселя, применяем расстояние, затем пытаемся установить его:
                    currentPosition.color = lightPointInCameraSpace(&currentPosition, &viewVector, doAmbient, specularExponent, specularCoefficient);

                    if (currentScene->isDepthFogged)
                        setPixel((int)theLine.p1.x, y, correctZ, getDistanceFoggedColor( currentPosition.color, correctZ ) );
                    else
                        setPixel((int)theLine.p1.x, y, correctZ, currentPosition.color );
                }
                else{
                    if (currentScene->isDepthFogged)
                        setPixel((int)theLine.p1.x, (int)y, correctZ, getFogPixelValue(&theLine.p1, &theLine.p2, ratio, correctZ) );
                    else
                        setPixel((int)theLine.p1.x, (int)y, correctZ, getPerspCorrectLerpColor(&theLine.p1, &theLine.p2, ratio) );
                }

            } // Конец z-проверки

            y++;
            z += z_slope;
        }

    } // Обработка не вертикальных линий:
    else {
        // Если конечные точки не слева (p1) направо (p2), меняем их местами:
        if (theLine.p1.x > theLine.p2.x){
            Vertex temp = theLine.p1;
            theLine.p1 = theLine.p2;
            theLine.p2 = temp;
        }

        // Рассчитать наклон:
        double slope = (theLine.p2.y - theLine.p1.y)/(double)(theLine.p2.x - theLine.p1.x);
        bool steep = false;
        if (slope < -1 || slope > 1){ // Если линия в октанте II, III, VI, VII, пометить ее и инвертировать наклон
            steep = true;
            slope = 1/slope;
        }

        // Рисуем крутую линию:
        if (steep){ // Обработка рисунка в октантах II, III, VI, VII:
            int y, y_min, y_max;
            double x, z;
            double z_slope = (theLine.p2.z - theLine.p1.z)/(double)(theLine.p2.y - theLine.p1.y); // Как много мы движемся по оси Z для каждой единицы по оси X
            Vertex lowest, highest; // Поскольку мы не знаем, рисуем ли мы сверху вниз или снизу вверх, мы создаем копию для использования при выполнении интерполяции

            if (theLine.p1.y < theLine.p2.y){
                x = (double)theLine.p1.x;
                y = (int)theLine.p1.y;
                y_min = (int)theLine.p1.y; // Здесь мы используем целые числа, потому что точки уже были округлены как часть преобразования в экранное пространство
                y_max = (int)theLine.p2.y;
                z = theLine.p1.z;
                lowest = theLine.p1;
                highest = theLine.p2;
            } else {
                x = theLine.p2.x;
                y = (int)theLine.p2.y;
                y_min = (int)theLine.p2.y;
                y_max = (int)theLine.p1.y;
                z = theLine.p2.z;
                lowest = theLine.p2;
                highest = theLine.p1;
            }

            while (y < y_max){
                int round_x = (int)round(x);

                double ratio = (y - y_min)/(double)(y_max - y_min); // Получить нашу текущую позицию как отношение

                double correctZ = getPerspCorrectLerpValue(lowest.z, lowest.z, highest.z, highest.z, ratio);

                // Рисуем, только если мы перед текущей z-глубиной
                if ( isVisible(round_x, y, correctZ) ){

                    if (theShadingModel == phong){

                        // Вычисляем текущую позицию пикселя, как вершину в пространстве камеры:
                        Vertex currentPosition(x, y, correctZ);        // Создаем вершину, представляющую текущую точку на линии сканирования
                        currentPosition.transform(&screenToPerspective); // Преобразование обратно в перспективное пространство

                        // Исправить трансформацию перспективы: трансформировать точку обратно в пространство камеры
                        currentPosition.x *= correctZ;
                        currentPosition.y *= correctZ;

                        // Установить правильную перспективу нормали
                        currentPosition.normal.xn = getPerspCorrectLerpValue(lowest.normal.xn, lowest.z, highest.normal.xn, highest.z, ratio);
                        currentPosition.normal.yn = getPerspCorrectLerpValue(lowest.normal.yn, lowest.z, highest.normal.yn, highest.z, ratio);
                        currentPosition.normal.zn = getPerspCorrectLerpValue(lowest.normal.zn, lowest.z, highest.normal.zn, highest.z, ratio);
                        currentPosition.normal.normalize(); // Нормализовать

                        currentPosition.color = getPerspCorrectLerpColor(&theLine.p1, &theLine.p2, ratio); // Получить базовый цвет (в перспективе)

                        // Создаем вектор вида: указывает от лица к камере
                        NormalVector viewVector(-currentPosition.x, -currentPosition.y, -currentPosition.z);
                        viewVector.normalize();

                        currentPosition.color = lightPointInCameraSpace(&currentPosition, &viewVector, doAmbient, specularExponent, specularCoefficient);

                        if (currentScene->isDepthFogged) // Вычисляем значение освещенного пикселя, применяем расстояние, затем пытаемся установить его:
                            setPixel(round_x, y, correctZ, getDistanceFoggedColor( currentPosition.color, correctZ ) );
                        else
                            setPixel(round_x, y, correctZ, currentPosition.color );
                    }
                    else{
                        if (currentScene->isDepthFogged)
                            setPixel(round_x, y, correctZ, getFogPixelValue(&theLine.p1, &theLine.p2, ratio, correctZ) );
                        else
                            setPixel(round_x, y, correctZ, getPerspCorrectLerpColor(&theLine.p1, &theLine.p2, ratio) );
                    }

                }

                y++;
                x += slope;
                z += z_slope;
            }

        } // Конец, если крутой

        else { // Обработка октантов I, IV, V, VIII:
            double y = theLine.p1.y;
            double z = theLine.p1.z;
            double z_slope = (theLine.p2.z - theLine.p1.z)/(double)(theLine.p2.x - theLine.p1.x); // Как много мы движемся по оси Z для каждой единицы по оси X

            for (int x = (int)theLine.p1.x; x <= theLine.p2.x; x++){
                int round_y = (int)round(y);

                double ratio = (x - theLine.p1.x)/(double)(theLine.p2.x - theLine.p1.x); // Get our current position as a ratio
                double correctZ = getPerspCorrectLerpValue(theLine.p1.z, theLine.p1.z, theLine.p2.z, theLine.p2.z, ratio);

                // Рисуем, только если мы перед текущей z-глубиной
                if ( isVisible(x, round_y, correctZ) ){

                    if (theShadingModel == phong){

                        // Вычисляем текущую позицию пикселя, как вершину в пространстве камеры:
                        Vertex currentPosition(x, y, correctZ);        // Создаем вершину, представляющую текущую точку на линии сканирования
                        currentPosition.transform(&screenToPerspective); // Преобразование обратно в перспективное пространство

                        // Исправить трансформацию перспективы: трансформировать точку обратно в пространство камеры
                        currentPosition.x *= correctZ;
                        currentPosition.y *= correctZ;

                        // Установить правильную перспективу нормально
                        currentPosition.normal.xn = getPerspCorrectLerpValue(theLine.p1.normal.xn, theLine.p1.z, theLine.p2.normal.xn, theLine.p2.z, ratio);
                        currentPosition.normal.yn = getPerspCorrectLerpValue(theLine.p1.normal.yn, theLine.p1.z, theLine.p2.normal.yn, theLine.p2.z, ratio);
                        currentPosition.normal.zn = getPerspCorrectLerpValue(theLine.p1.normal.zn, theLine.p1.z, theLine.p2.normal.zn, theLine.p2.z, ratio);
                        currentPosition.normal.normalize(); // Normalize

                        currentPosition.color = getPerspCorrectLerpColor(&theLine.p1, &theLine.p2, ratio); // Get the (perspective correct) base color

                       // Создаем вектор вида: указывает от лица к камере
                        NormalVector viewVector(-currentPosition.x, -currentPosition.y, -currentPosition.z);
                        viewVector.normalize();

                        // Вычисляем значение освещенного пикселя, применяем расстояние, затем пытаемся установить его:
                        currentPosition.color = lightPointInCameraSpace(&currentPosition, &viewVector, doAmbient, specularExponent, specularCoefficient);

                        if (currentScene->isDepthFogged)
                            setPixel(x, round_y, correctZ, getDistanceFoggedColor( currentPosition.color, correctZ ) );
                        else
                            setPixel(x, round_y, correctZ, currentPosition.color );
                    }
                    else{
                        if (currentScene->isDepthFogged)
                            setPixel(x, round_y, correctZ, getFogPixelValue(&theLine.p1, &theLine.p2, ratio, correctZ) );
                        else
                            setPixel(x, round_y, correctZ, getPerspCorrectLerpColor(&theLine.p1, &theLine.p2, ratio) );
                    }
                }

                y += slope;
                z += z_slope;
            }
        }
    } // Конец не вертикальной линии

    // Обновляем экран:
    drawable->updateScreen();
}

// Рисуем многоугольник. Вызывает растеризованную вспомогательную функцию Polygon.
// Если вершины Полигона не одного цвета, цвет будет LERP'd
// Предварительное условие: все полигоны находятся в пространстве камеры
void Renderer::drawPolygon(Polygon thePolygon, bool isWireframe){

    if (!thePolygon.isInDepth(currentScene->camHither, currentScene->camYon)){
        return;
    }

    thePolygon.clipHitherYon(currentScene->camHither, currentScene->camYon);

    if(!thePolygon.isValid())
        return;

    if (thePolygon.isLine() && thePolygon.isAffectedByAmbientLight() ){
        thePolygon.lightAmbiently( currentScene->ambientRedIntensity, currentScene->ambientGreenIntensity, currentScene->ambientBlueIntensity);
    }

    else if (thePolygon.getShadingModel() == flat && !isWireframe && !thePolygon.isLine() ){ // Only light the polygon if it's not wireframe or a line
        flatShadePolygon( &thePolygon );
    }
    else if (thePolygon.getShadingModel() == gouraud && !isWireframe && !thePolygon.isLine()){ // Only light the polygon if it's not wireframe or a line
        gouraudShadePolygon( &thePolygon );
    }

    thePolygon.transform( &cameraToPerspective );

    if (!thePolygon.isFacingCamera() && !isWireframe && !thePolygon.isLine()){
      return;
    }

    if (!thePolygon.isInFrustum(currentScene->xLow, currentScene->xHigh, currentScene->yLow, currentScene->yHigh)){
        return;
    }

    thePolygon.clipToScreen(currentScene->xLow, currentScene->xHigh, currentScene->yLow, currentScene->yHigh);

    if(!thePolygon.isValid())
        return;

    thePolygon.transform(&perspectiveToScreen, true);

    if(thePolygon.isLine()){
        drawLine(Line(*(thePolygon.getLast()), *(thePolygon.getPrev(thePolygon.getLast()->vertexNumber) )), ambientOnly, true, 0, 0);
        return;
    }

    // Триангуляция
    vector<Polygon>* theFaces = thePolygon.getTriangulatedFaces();  

    for (unsigned int i = 0; i < theFaces->size(); i++){

        if (!isWireframe) {
            rasterizePolygon( &theFaces->at(i) );
        }
        else{
            drawPolygonWireframe( &theFaces->at(i) );
        }
    }

    delete theFaces;
}

// Рисуем многоугольник используя непрозрачность
// Если вершины Полигона не одного цвета, цвет будет LERP'd
void Renderer::rasterizePolygon(Polygon* thePolygon){


    Vertex* topLeftVertex = thePolygon->getHighest();
    Vertex* topRightVertex = topLeftVertex;

    Vertex* botLeftVertex = thePolygon->getNext(topLeftVertex->vertexNumber);
    Vertex* botRightVertex = thePolygon->getPrev(topRightVertex->vertexNumber);


    int count = thePolygon->getVertexCount();
    while (topLeftVertex->y == botLeftVertex->y && count > 0){
        count--;
        topLeftVertex = botLeftVertex;
        botLeftVertex = thePolygon->getNext(botLeftVertex->vertexNumber);
    }
    count = thePolygon->getVertexCount();
    while (topRightVertex->y == botRightVertex->y && count > 0){
        count--;
        topRightVertex = botRightVertex;
        botRightVertex = thePolygon->getPrev(botRightVertex->vertexNumber);
    }


    int y = (int)(topLeftVertex->y);
    int yMin = (int)( thePolygon->getLowest()->y );


    double xLeft = topLeftVertex->x;
    double xRight = topRightVertex->x;


    double DYLeft = topLeftVertex->y - botLeftVertex->y;
    double DYRight = topRightVertex->y - botRightVertex->y;
    double zLeft = topLeftVertex->z;
    double zRight = topRightVertex->z;


    double xLeftSlope, xRightSlope, zLeftSlope, zRightSlope, leftRatioDiff, rightRatioDiff;
    if (DYLeft == 0){
        xLeftSlope = 0;
        zLeftSlope = 0;
        leftRatioDiff = 0;
    }
    else{
        xLeftSlope = (topLeftVertex->x - botLeftVertex->x)/(double)DYLeft;
        zLeftSlope = (topLeftVertex->z - botLeftVertex->z)/(double)DYLeft;
        leftRatioDiff = 1.0 /(double) DYLeft;
    }
    if (DYRight == 0){
        xRightSlope = 0;
        zRightSlope = 0;
        rightRatioDiff = 0;
    }
    else {
        xRightSlope = (topRightVertex->x - botRightVertex->x)/(double)DYRight;
        zRightSlope = (topRightVertex->z - botRightVertex->z)/(double)DYRight;
        rightRatioDiff = 1.0 /(double) DYRight;
    }


    double leftRatio = 0;
    double rightRatio = 0;

    while (y >= yMin){

        double leftCorrectZ = getPerspCorrectLerpValue(topLeftVertex->z, topLeftVertex->z, botLeftVertex->z, botLeftVertex->z, leftRatio );
        double rightCorrectZ = getPerspCorrectLerpValue(topRightVertex->z, topRightVertex->z, botRightVertex->z, botRightVertex->z, rightRatio );

        double xLeft_rounded = round(xLeft);
        double xRight_rounded = round(xRight);

        if (thePolygon->getShadingModel() == phong){

            Vertex lhs(xLeft_rounded, (double)y, leftCorrectZ, getPerspCorrectLerpColor(topLeftVertex, botLeftVertex, leftRatio));
            lhs.normal = NormalVector(topLeftVertex->normal, topLeftVertex->z, botLeftVertex->normal, botLeftVertex->z, y, topLeftVertex->y, botLeftVertex->y);

            Vertex rhs(xRight_rounded, (double)y, rightCorrectZ, getPerspCorrectLerpColor(topRightVertex, botRightVertex, rightRatio));
            rhs.normal = NormalVector(topRightVertex->normal, topRightVertex->z, botRightVertex->normal, botRightVertex->z, y, topRightVertex->y, botRightVertex->y);

            drawPerPxLitScanlineIfVisible( &lhs, &rhs, thePolygon->isAffectedByAmbientLight(), thePolygon->getSpecularCoefficient(), thePolygon->getSpecularExponent());
        }
        else
        {
            Vertex* start = new Vertex(xLeft_rounded, (double)y, leftCorrectZ, getPerspCorrectLerpColor(topLeftVertex, botLeftVertex, leftRatio));
            Vertex* end = new Vertex(xRight_rounded, (double)y, rightCorrectZ, getPerspCorrectLerpColor(topRightVertex, botRightVertex, rightRatio));
            drawScanlineIfVisible( start, end);
        }

        y--;

        if (y < botLeftVertex->y){
            topLeftVertex = botLeftVertex;
            botLeftVertex = thePolygon->getNext(botLeftVertex->vertexNumber);
            DYLeft = topLeftVertex->y - botLeftVertex->y;

            if (DYLeft == 0){
                xLeftSlope = 0;
                zLeftSlope = 0;
                leftRatioDiff = 0;
            }
            else{
                xLeftSlope = (topLeftVertex->x - botLeftVertex->x) / DYLeft;
                zLeftSlope = (topLeftVertex->z - botLeftVertex->z) / DYLeft;
                leftRatioDiff = 1.0 /(double) DYLeft;
            }

            zLeft = (double)topLeftVertex->z - zLeftSlope;
            xLeft = topLeftVertex->x - xLeftSlope;
            leftRatio = leftRatioDiff;
        }
        else {
            xLeft -= xLeftSlope;
            zLeft -= zLeftSlope;
            leftRatio += leftRatioDiff;
        }


        if (y < botRightVertex->y){
            topRightVertex = botRightVertex;
            botRightVertex = thePolygon->getPrev(botRightVertex->vertexNumber);
            DYRight = topRightVertex->y - botRightVertex->y;

            if (DYRight == 0){
                xRightSlope = 0;
                zRightSlope = 0;
                rightRatioDiff = 0;
            }
            else {
                xRightSlope = (topRightVertex->x - botRightVertex->x) / DYRight;
                zRightSlope = (topRightVertex->z - botRightVertex->z) / DYRight;
                rightRatioDiff = 1.0 /(double) DYRight;
            }

            zRight = (double)topRightVertex->z - zRightSlope;
            xRight = topRightVertex->x - xRightSlope;
            rightRatio = rightRatioDiff;
        }
        else {
            xRight -= xRightSlope;
            zRight -= zRightSlope;
            rightRatio += rightRatioDiff;
        }

    }


    drawable->updateScreen();
}

// Осветить полигон, используя плоскую заливку
// Предварительное условие: все вершины имеют действительную нормаль
void Renderer::flatShadePolygon(Polygon* thePolygon){

    unsigned int* ambientValues = new unsigned int[ thePolygon->getVertexCount() ];

    double* redDiffuseTotals = new double[ thePolygon->getVertexCount() ];
    double* greenDiffuseTotals = new double[ thePolygon->getVertexCount() ];
    double* blueDiffuseTotals = new double[ thePolygon->getVertexCount() ];

    double* redSpecTotals = new double[ thePolygon->getVertexCount() ];
    double* greenSpecTotals = new double[ thePolygon->getVertexCount() ];
    double* blueSpecTotals = new double[ thePolygon->getVertexCount() ];

    for (int i = 0; i < thePolygon->getVertexCount(); i++){
        ambientValues[i] = 0;

        redDiffuseTotals[i] = 0;
        greenDiffuseTotals[i] = 0;
        blueDiffuseTotals[i] = 0;

        redSpecTotals[i] = 0;
        greenSpecTotals[i] = 0;
        blueSpecTotals[i] = 0;

        if (thePolygon->isAffectedByAmbientLight() ){
            ambientValues[i] = multiplyColorChannels(thePolygon->vertices[i].color, 1.0, currentScene->ambientRedIntensity, currentScene->ambientGreenIntensity, currentScene->ambientBlueIntensity );
        }
    }

    Vertex faceCenter = thePolygon->getFaceCenter();


    NormalVector faceNormal = thePolygon->getNormalAverage();


    for (unsigned int i = 0; i < currentScene->theLights.size(); i++){


        NormalVector lightDirection(currentScene->theLights[i].position.x - faceCenter.x, currentScene->theLights[i].position.y - faceCenter.y, currentScene->theLights[i].position.z - faceCenter.z);
        lightDirection.normalize();


        double faceNormalDotLightDirection = faceNormal.dotProduct(lightDirection);

        if (faceNormalDotLightDirection > 0){


            double attenuationFactor = currentScene->theLights[i].getAttenuationFactor(faceCenter);


            double redDiffuseIntensity = currentScene->theLights[i].redIntensity * attenuationFactor;
            double greenDiffuseIntensity = currentScene->theLights[i].greenIntensity * attenuationFactor;
            double blueDiffuseIntensity = currentScene->theLights[i].blueIntensity * attenuationFactor;


            redDiffuseIntensity *= faceNormalDotLightDirection;
            greenDiffuseIntensity *= faceNormalDotLightDirection;
            blueDiffuseIntensity *= faceNormalDotLightDirection;


            double redSpecIntensity = thePolygon->getSpecularCoefficient();
            double greenSpecIntensity = thePolygon->getSpecularCoefficient();
            double blueSpecIntensity = thePolygon->getSpecularCoefficient();


            NormalVector viewVector(-faceCenter.x, -faceCenter.y, -faceCenter.z);
            viewVector.normalize();


            NormalVector reflectionVector = reflectOutVector(&faceNormal, &lightDirection);


            double viewDotReflection = viewVector.dotProduct(reflectionVector);
            if (viewDotReflection < 0)
                viewDotReflection = 0;

            viewDotReflection = pow(viewDotReflection, thePolygon->getSpecularExponent() );

            redSpecIntensity *= (currentScene->theLights[i].redIntensity * attenuationFactor * viewDotReflection);
            greenSpecIntensity *= (currentScene->theLights[i].greenIntensity * attenuationFactor * viewDotReflection);
            blueSpecIntensity *= (currentScene->theLights[i].blueIntensity * attenuationFactor * viewDotReflection);


            for (int i = 0; i < thePolygon->getVertexCount(); i++){

                redDiffuseTotals[i] += redDiffuseIntensity;
                greenDiffuseTotals[i] += greenDiffuseIntensity;
                blueDiffuseTotals[i] += blueDiffuseIntensity;

                redSpecTotals[i] += redSpecIntensity;
                greenSpecTotals[i] += greenSpecIntensity;
                blueSpecTotals[i] += blueSpecIntensity;

            }

        }
    }


    for (int i = 0; i < thePolygon->getVertexCount(); i++){
        thePolygon->vertices[i].color = addColors( ambientValues[i],
                                                  addColors(
                                                       multiplyColorChannels(thePolygon->vertices[i].color, 1.0, redDiffuseTotals[i], greenDiffuseTotals[i], blueDiffuseTotals[i]),
                                                       combineColorChannels( redSpecTotals[i], greenSpecTotals[i], blueSpecTotals[i] )
                                                       )
                                                  );
    }


    delete [] ambientValues;

    delete [] redDiffuseTotals;
    delete [] greenDiffuseTotals;
    delete [] blueDiffuseTotals;

    delete [] redSpecTotals;
    delete [] greenSpecTotals;
    delete [] blueSpecTotals;
}

// Осветить полигон, используя затенение Гуро
// Предварительное условие: все вершины имеют действительную нормаль
void Renderer::gouraudShadePolygon(Polygon* thePolygon){

    for (int i = 0; i < thePolygon->getVertexCount(); i++){

        NormalVector viewVector(-thePolygon->vertices[i].x, -thePolygon->vertices[i].y, -thePolygon->vertices[i].z);
        viewVector.normalize();

        thePolygon->vertices[i].color = lightPointInCameraSpace(&thePolygon->vertices[i], &viewVector, thePolygon->isAffectedByAmbientLight(), thePolygon->getSpecularExponent(), thePolygon->getSpecularCoefficient());
    }
}

 // Рисуем многоугольник только в каркасном режиме
void Renderer::drawPolygonWireframe(Polygon* thePolygon){
    if (thePolygon->isValid() ){ // Only draw if our polygon has at least 3 vertices

        Vertex* theTop = thePolygon->getHighest();
        Vertex* v1 = theTop;
        Vertex* v2 = thePolygon->getNext(v1->vertexNumber);

        do {
            drawLine(Line(*v1, *v2), ambientOnly, thePolygon->isAffectedByAmbientLight(), 0, 0);
            v1 = v2;
            v2 = thePolygon->getNext(v2->vertexNumber);
        } while (v1 != theTop);
    }
}

// Рисуем каркас (сетку)
void Renderer::drawMesh(Mesh* theMesh){
    for (unsigned int i = 0; i < theMesh->faces.size(); i++){
        currentPolygon = &theMesh->faces[i];
        drawPolygon(theMesh->faces[i], theMesh->isWireframe);
    }

    currentPolygon = nullptr;
}

// Рендерим сцену
void Renderer::renderScene(Scene theScene){
    currentScene = &theScene;

    drawRectangle(0, 0, xRes - 1, yRes - 1, currentScene->fogColor);

    transformCamera(theScene.cameraMovement);

    for (auto &currentLight : theScene.theLights){
        currentLight.position.transform(&worldToCamera);

    }


    for(auto &processingMesh : theScene.theMeshes){
        processingMesh.transform(&worldToCamera);
    }


    for (auto &renderMesh : theScene.theMeshes){
        currentMesh = &renderMesh; // Update the currentMesh pointer to the current mesh being drawn
        drawMesh(&renderMesh);

//        // UNCOMMENT TO VISIBLY DEBUG BOUNDING BOXES:
//        for (int i = 0; i < renderMesh.boundingBoxFaces.size(); i++){
//            drawPolygon(renderMesh.boundingBoxFaces[i], true);
//        }
    }


    currentScene = nullptr;
    currentMesh = nullptr;
}

// Нарисовать линию скана с учетом Z-буфера.
// Предварительное условие: начальная и конечная вершины располагаются слева направо
// Примечание: LERP, если start.color! = End.color. НЕ обновляет экран!
void Renderer::drawScanlineIfVisible(Vertex* start, Vertex* end){

    double z = start->z;
    double z_slope; // Make sure we're setting a valid z-slope value
    if (end->x - start->x == 0)
        z_slope = 0;
    else
        z_slope = (end->z - start->z)/(double)(end->x - start->x);

    int x_start = (int)start->x;
    int x_end = (int)end->x;
    int y_rounded = (int)start->y;

    double ratio = 0;
    double ratioDiff;
    if (x_end - x_start == 0)
        ratioDiff = 0;
    else
        ratioDiff = 1/(double)(x_end - x_start);

    for (int x = x_start; x <= x_end; x++){

        double correctZ = getPerspCorrectLerpValue(start->z, start->z, end->z, end->z, ratio);

        if (isVisible(x, y_rounded, correctZ) ){
            if (currentScene->isDepthFogged)
                setPixel(x, y_rounded, correctZ, getFogPixelValue(start, end, ratio, correctZ) );
            else
                setPixel(x, y_rounded, correctZ, getPerspCorrectLerpColor(start, end, ratio) );
        }

        z += z_slope;
        ratio += ratioDiff;
    }
}

// Рисуем линию развертки с подсветкой на пиксель (Фонг), с учетом Z-буфера
void Renderer::drawPerPxLitScanlineIfVisible(Vertex* start, Vertex* end, bool doAmbient, double specularCoefficient, double specularExponent){

    double zCameraSpace = start->z;
    double z_slope;
    if (end->x - start->x == 0)
        z_slope = 0;
    else
        z_slope = (end->z - start->z)/(double)(end->x - start->x);

    int x_start = (int)start->x;
    int x_end = (int)end->x;

    int y_rounded = (int)start->y;

    double ratio = 0;
    double ratioDiff;
    if (x_end - x_start == 0)
        ratioDiff = 0;
    else
        ratioDiff = 1/(double)(x_end - x_start);

    // Draw:
    for (int x = x_start; x <= x_end; x++){

        double correctZ = getPerspCorrectLerpValue(start->z, start->z, end->z, end->z, ratio); // Calculate the perspective correct Z for the current pixel


        if ( isVisible(x, y_rounded, correctZ) ){


            Vertex currentPosition(x, y_rounded, correctZ);

            currentPosition.transform(&screenToPerspective);


            currentPosition.x *= correctZ;
            currentPosition.y *= correctZ;


            currentPosition.normal.xn = getPerspCorrectLerpValue(start->normal.xn, start->z, end->normal.xn, end->z, ratio);
            currentPosition.normal.yn = getPerspCorrectLerpValue(start->normal.yn, start->z, end->normal.yn, end->z, ratio);
            currentPosition.normal.zn = getPerspCorrectLerpValue(start->normal.zn, start->z, end->normal.zn, end->z, ratio);
            currentPosition.normal.normalize();

            currentPosition.color = getPerspCorrectLerpColor(start, end, ratio);


            NormalVector viewVector(-currentPosition.x, -currentPosition.y, -currentPosition.z);
            viewVector.normalize();


            currentPosition.color = recursivelyLightPointInCS(&currentPosition, &viewVector, doAmbient, specularExponent, specularCoefficient, currentScene->numRayBounces, x == x_start || x == x_end);


            setPixel(x, y_rounded, correctZ, currentPosition.color);
        }

        ratio += ratioDiff;

        zCameraSpace += z_slope;
    }
}

// Рекурсивно лучевая трассировка освещения точки
unsigned int Renderer::recursivelyLightPointInCS(Vertex* currentPosition, NormalVector* viewVector, bool doAmbient, double specularExponent, double specularCoefficient, int bounceRays, bool isEndPoint){

    unsigned int initialColor = lightPointInCameraSpace(currentPosition, viewVector, doAmbient, specularExponent, specularCoefficient);


    if (bounceRays > 0){

        NormalVector bounceDirection = reflectOutVector(&(currentPosition->normal), viewVector);


        return addColors(   initialColor,
                            multiplyColorChannels(
                                    recursiveLightHelper(currentPosition, &bounceDirection, doAmbient, specularExponent, specularCoefficient, bounceRays - 1, isEndPoint),
                                    1.0, currentPolygon->getReflectivity(), currentPolygon->getReflectivity(), currentPolygon->getReflectivity()
                            )
                         );
    }
    else
        return initialColor;
}

// Рекурсивная вспомогательная функция для трассировки лучей
unsigned int Renderer::recursiveLightHelper(Vertex* currentPosition, NormalVector* inBounceDirection, bool doAmbient, double specularExponent, double specularCoefficient, int bounceRays, bool isEndPoint){

    Vertex* intersectionResult;
    Polygon* hitPoly;
    double hitDistance;

    intersectionResult = new Vertex();

    hitPoly = nullptr;
    hitDistance = std::numeric_limits<double>::max();
    Vertex closestIntersection;

    for (auto &currentVisibleMesh : currentScene->theMeshes){

        for (int i = 0; i < currentVisibleMesh.boundingBoxFaces.size(); i++){


            if ( getPolyPlaneIntersectionPoint(currentPosition, inBounceDirection, &currentVisibleMesh.boundingBoxFaces[i].vertices[0], &currentVisibleMesh.boundingBoxFaces[i].faceNormal, intersectionResult ) ){


                if ( pointIsInsidePoly( &currentVisibleMesh.boundingBoxFaces[i], intersectionResult ) || currentMesh == &currentVisibleMesh ){


                    for (int j = 0; j < currentVisibleMesh.faces.size(); j++){


                        if ( &currentVisibleMesh.faces[j] == currentPolygon )
                            continue;


                        if ( getPolyPlaneFrontFaceIntersectionPoint(currentPosition, inBounceDirection, &currentVisibleMesh.faces[j].vertices[0], &currentVisibleMesh.faces[j].faceNormal, intersectionResult ) ){


                            if( pointIsInsidePoly( &currentVisibleMesh.faces[j], intersectionResult )

                                    && (currentMesh !=  &currentVisibleMesh   || !isEndPoint || !haveSharedEdge(currentPolygon, &currentVisibleMesh.faces[j]) || !isFaceReflexAngle(currentPolygon, &currentVisibleMesh.faces[j]) )
                              )
                            {

                                double currentHitDistance = (*intersectionResult - *currentPosition).length();

                                if (currentHitDistance < hitDistance){
                                    hitDistance = currentHitDistance;
                                    hitPoly = &currentVisibleMesh.faces[j];
                                    closestIntersection = *intersectionResult;
                                }
                            }
                        }
                    }

                    break;
                }
            }
        }
    }


    delete intersectionResult;


    if (hitPoly != nullptr){

        setInterpolatedIntersectionValues(&closestIntersection, hitPoly);


        inBounceDirection->reverse();


        closestIntersection.color = lightPointInCameraSpace(&closestIntersection, inBounceDirection, hitPoly->isAffectedByAmbientLight(), hitPoly->getSpecularExponent(), hitPoly->getSpecularCoefficient());


        if (bounceRays > 0 && hitPoly->getReflectivity() > 0){


            NormalVector nextBounceDirection = reflectOutVector(&closestIntersection.normal, inBounceDirection);

            return addColors(  closestIntersection.color,
                                   multiplyColorChannels(   recursiveLightHelper(&closestIntersection, &nextBounceDirection, doAmbient, specularExponent, specularCoefficient, bounceRays - 1, false),
                                                            1.0, hitPoly->getReflectivity(), hitPoly->getReflectivity(), hitPoly->getReflectivity() )
                             );

        }

        else
            return closestIntersection.color;

    }


    return currentScene->environmentColor;
}

// Осветить заданную точку в пространстве камеры
unsigned int Renderer::lightPointInCameraSpace(Vertex* currentPosition, NormalVector* viewVector, bool doAmbient, double specularExponent, double specularCoefficient) {


    unsigned int ambientValue = 0;
    double redTotalDiffuseIntensity, greenTotalDiffuseIntensity, blueTotalDiffuseIntensity, redTotalSpecIntensity, greenTotalSpecIntensity, blueTotalSpecIntensity;
    redTotalDiffuseIntensity = greenTotalDiffuseIntensity = blueTotalDiffuseIntensity = redTotalSpecIntensity = greenTotalSpecIntensity = blueTotalSpecIntensity = 0;


    if ( doAmbient ){
        ambientValue = multiplyColorChannels(currentPosition->color, 1.0, currentScene->ambientRedIntensity, currentScene->ambientGreenIntensity, currentScene->ambientBlueIntensity );
    }


    for (unsigned int i = 0; i < currentScene->theLights.size(); i++){


        NormalVector lightDirection(currentScene->theLights[i].position.x - currentPosition->x, currentScene->theLights[i].position.y - currentPosition->y, currentScene->theLights[i].position.z - currentPosition->z);
        lightDirection.normalize();


        double currentNormalDotLightDirection = currentPosition->normal.dotProduct(lightDirection);


        if (currentNormalDotLightDirection > 0) {

            double lightDistance = NormalVector(currentScene->theLights[i].position.x - currentPosition->x, currentScene->theLights[i].position.y - currentPosition->y, currentScene->theLights[i].position.z - currentPosition->z).length();


            if (currentScene->noRayShadows || !isShadowed(*currentPosition, &lightDirection, lightDistance) ){

                double attenuationFactor = currentScene->theLights[i].getAttenuationFactor(lightDistance);


                double redDiffuseIntensity = currentScene->theLights[i].redIntensity * attenuationFactor;
                double greenDiffuseIntensity = currentScene->theLights[i].greenIntensity * attenuationFactor;
                double blueDiffuseIntensity = currentScene->theLights[i].blueIntensity * attenuationFactor;


                redDiffuseIntensity *= currentNormalDotLightDirection;
                greenDiffuseIntensity *= currentNormalDotLightDirection;
                blueDiffuseIntensity *= currentNormalDotLightDirection;


                redTotalDiffuseIntensity += redDiffuseIntensity;
                greenTotalDiffuseIntensity += greenDiffuseIntensity;
                blueTotalDiffuseIntensity += blueDiffuseIntensity;


                NormalVector reflectionVector = reflectOutVector(&(currentPosition->normal), &lightDirection);


                double viewDotReflection = viewVector->dotProduct(reflectionVector);

                if (viewDotReflection > 0){

                    double redSpecIntensity = specularCoefficient;
                    double greenSpecIntensity = specularCoefficient;
                    double blueSpecIntensity = specularCoefficient;

                    viewDotReflection = pow(viewDotReflection, specularExponent );

                    redSpecIntensity *= (currentScene->theLights[i].redIntensity * attenuationFactor * viewDotReflection);
                    greenSpecIntensity *= (currentScene->theLights[i].greenIntensity * attenuationFactor * viewDotReflection);
                    blueSpecIntensity *= (currentScene->theLights[i].blueIntensity * attenuationFactor * viewDotReflection);

                    redTotalSpecIntensity += redSpecIntensity;
                    greenTotalSpecIntensity += greenSpecIntensity;
                    blueTotalSpecIntensity += blueSpecIntensity;
                }

            }
        }
    }

    if (currentScene->isDepthFogged && currentPolygon->getShadingModel() == phong){
        return getDistanceFoggedColor( addColors(     ambientValue,
                                                  addColors(
                                                      multiplyColorChannels( currentPosition->color, 1.0, redTotalDiffuseIntensity, greenTotalDiffuseIntensity, blueTotalDiffuseIntensity ),
                                                      combineColorChannels( redTotalSpecIntensity, greenTotalSpecIntensity, blueTotalSpecIntensity ) )
                                                  ),
                                   currentPosition->z);
    }
    else
        return addColors(     ambientValue,
                              addColors(
                                  multiplyColorChannels( currentPosition->color, 1.0, redTotalDiffuseIntensity, greenTotalDiffuseIntensity, blueTotalDiffuseIntensity ),
                                  combineColorChannels( redTotalSpecIntensity, greenTotalSpecIntensity, blueTotalSpecIntensity ) )
                        );
}

// Определяем, затенена ли текущая позиция каким-либо полигоном в сцене, которая находится между ней и источником света
bool Renderer::isShadowed(Vertex currentPosition, NormalVector* lightDirection, double lightDistance){

    currentPosition += (currentPosition.normal * 0.1);


    Vertex* intersectionResult = new Vertex();


    for (auto &currentVisibleMesh : currentScene->theMeshes){


        for (int i = 0; i < currentVisibleMesh.boundingBoxFaces.size(); i++){


            if (    ( getPolyPlaneIntersectionPoint(&currentPosition, lightDirection, &currentVisibleMesh.boundingBoxFaces[i].vertices[0], &currentVisibleMesh.boundingBoxFaces[i].faceNormal, intersectionResult ) )


                 && ( (*intersectionResult - currentPosition).length() < lightDistance )


                 && ( pointIsInsidePoly( &currentVisibleMesh.boundingBoxFaces[i], intersectionResult ) )
                ) {

                        for (int j = 0; j < currentVisibleMesh.faces.size(); j++){


                            if ( &currentVisibleMesh.faces[j] == currentPolygon )
                                continue;


                            if ( ( getPolyPlaneBackFaceIntersectionPoint(&currentPosition, lightDirection, &currentVisibleMesh.faces[j].vertices[0], &currentVisibleMesh.faces[j].faceNormal, intersectionResult ) )


                                && ( (*intersectionResult - currentPosition).length() < lightDistance )


                                && ( pointIsInsidePoly( &currentVisibleMesh.faces[j], intersectionResult ) )

                                 ){


                                    delete intersectionResult;

                                    return true;

                            }
                        }

                        break;

                    }

        }
    }


    delete intersectionResult;

    return false;
}

// Находим точку пересечения луча и плоскости многоугольника
// Return: True, если луч пересекается, в противном случае - false. Изменяет результат Vertex, чтобы быть точкой пересечения, в противном случае оставляет его неизменным
bool Renderer::getPolyPlaneBackFaceIntersectionPoint(Vertex* currentPosition, NormalVector* currentDirection, Vertex* planePoint, NormalVector* planeNormal, Vertex* intersectionResult){

    double currentDirectionDotPlaneNormal = currentDirection->dotProduct(*planeNormal);


    if (currentDirectionDotPlaneNormal <= 0)
        return false;

    double distance = (*planePoint - *currentPosition).dot(*planeNormal)/(double)currentDirectionDotPlaneNormal;

    if (distance > 0.06){
        *intersectionResult = (*currentPosition + (*currentDirection * distance));
        return true;
    }

    return false;
}


// Находим точку пересечения луча и плоскости многоугольника
// Return: True, если луч пересекается, в противном случае - false. Изменяет результат Vertex, чтобы быть точкой пересечения, в противном случае оставляет его неизменным
bool Renderer::getPolyPlaneFrontFaceIntersectionPoint(Vertex* currentPosition, NormalVector* currentDirection, Vertex* planePoint, NormalVector* planeNormal, Vertex* intersectionResult){

    double currentDirectionDotPlaneNormal = currentDirection->dotProduct(*planeNormal);


    if (currentDirectionDotPlaneNormal >= 0)
        return false;

    double distance = (*planePoint - *currentPosition).dot(*planeNormal)/(double)currentDirectionDotPlaneNormal;


    if (distance > 0){
        *intersectionResult = (*currentPosition + (*currentDirection * distance));

        return true;
    }

    return false;
}

// Находим точку пересечения луча и плоскости многоугольника
// Return: True, если луч пересекается, в противном случае - false. Изменяет результат Vertex, чтобы быть точкой пересечения, в противном случае оставляет его неизменным
bool Renderer::getPolyPlaneIntersectionPoint(Vertex* currentPosition, NormalVector* currentDirection, Vertex* planePoint, NormalVector* planeNormal, Vertex* intersectionResult){

    double currentDirectionDotPlaneNormal = currentDirection->dotProduct(*planeNormal);


    if (currentDirectionDotPlaneNormal == 0)
        return false;

    double distance = (*planePoint - *currentPosition).dot(*planeNormal)/(double)currentDirectionDotPlaneNormal;


    if (distance > 0){
        *intersectionResult = (*currentPosition + (*currentDirection * distance));
        return true;
    }

    return false;
}

// Определяем, находится ли точка на плоскости многоугольника внутри многоугольника
bool Renderer::pointIsInsidePoly(Polygon* thePolygon, Vertex* intersectionPoint){

    bool isInside = true;
    for (int i = 0; i < thePolygon->getVertexCount() - 1; i++){

        NormalVector innerNormal(thePolygon->vertices[i].x - thePolygon->vertices[i+1].x, thePolygon->vertices[i].y - thePolygon->vertices[i+1].y, thePolygon->vertices[i].z - thePolygon->vertices[i+1].z);

        innerNormal = innerNormal.crossProduct(thePolygon->faceNormal);

        if ((*intersectionPoint - thePolygon->vertices[i]).dot(innerNormal) < 0){
            isInside = false;
            break;
        }
    }

    if (isInside){
        NormalVector innerNormal(thePolygon->vertices[thePolygon->getVertexCount() - 1].x - thePolygon->vertices[0].x, thePolygon->vertices[thePolygon->getVertexCount() - 1].y - thePolygon->vertices[0].y, thePolygon->vertices[thePolygon->getVertexCount() - 1].z - thePolygon->vertices[0].z);

        innerNormal = innerNormal.crossProduct(thePolygon->faceNormal);

        if ((*intersectionPoint - thePolygon->vertices[thePolygon->getVertexCount() - 1]).dot(innerNormal) < 0){
            isInside = false;
        }
    }

    return isInside;
}

// Рассчитать результат наложения пикселя
// Возвращает: целое число без знака, представляющее смешанное значение полупрозрачного пикселя
unsigned int Renderer::blendPixelValues(int x, int y, unsigned int color, float opacity){
    unsigned int currentColor = drawable->getPixel(x, y); // Sample the existing color

    return addColors (multiplyColorChannels(color, opacity), multiplyColorChannels(currentColor, (1 - opacity) ) ); // Calculate the blended colors
}

// Возвращает: значение цвета без знака int, рассчитанное на основе LERP текущей позиции между двумя предоставленными точками
unsigned int Renderer::getPerspCorrectLerpColor(Vertex* p1, Vertex* p2, double ratio) const {
    if (p1->color == p2->color || ratio <= 0)
        return p1->color;

    if (ratio >= 1)
        return p2->color;

    double red = getPerspCorrectLerpValue(extractColorChannel(p1->color, 1), p1->z, extractColorChannel(p2->color, 1), p2->z, ratio);
    double green = getPerspCorrectLerpValue(extractColorChannel(p1->color, 2), p1->z, extractColorChannel(p2->color, 2), p2->z, ratio);
    double blue = getPerspCorrectLerpValue(extractColorChannel(p1->color, 3), p1->z, extractColorChannel(p2->color, 3), p2->z, ratio);

    return combineColorChannels(red, green, blue);
}

// Рассчитать значение освещения для данного пикселя на линии между 2 точками
// Предварительное условие: окружающее освещение уже применено к значениям цвета вершины
unsigned int Renderer::getFogPixelValue(Vertex* p1, Vertex* p2, double ratio, double correctZ) {

    return getDistanceFoggedColor( getPerspCorrectLerpColor(p1, p2, ratio) , correctZ );
}

// Вычисляем интерполированный пиксель и значение глубины
unsigned int Renderer::getDistanceFoggedColor(unsigned int pixelColor, double correctZ){

    if (correctZ <= currentScene->fogHither)
        return pixelColor;

    if (correctZ >= currentScene->fogYon)
        return currentScene->fogColor;

    double ratio = (correctZ - currentScene->fogHither) / (currentScene->fogYon - currentScene->fogHither);

    return addColors( multiplyColorChannels(pixelColor, (1 - ratio) ), multiplyColorChannels(currentScene->fogColor, ratio) );
}

// Сброс буфера глубины
void Renderer::resetDepthBuffer(){
    for (int x = 0; x < xRes; x++){
        for (int y = 0; y < yRes; y++){
            ZBuffer[x][y] = maxZVal;
        }
    }
}

// Установить пиксель на растре
// Предварительное условие: точка является действительной координатой на растровом холсте и была предварительно проверена по z-буферу
void Renderer::setPixel(int x, int y, double z, unsigned int color){

    y = yRes - y;

    drawable->setPixel(x, y, color);

    ZBuffer[x][y] = getScaledZVal( z );
}

// Проверяем, находится ли пиксельная координата перед текущей глубиной z-буфера
bool Renderer::isVisible(int x, int y, double z){
    return ( getScaledZVal( z ) < ZBuffer[x][yRes - y]);
}

// Получить масштабированное значение z-буфера для данного Z
int Renderer::getScaledZVal(double correctZ){
    return (int)( (correctZ - currentScene->camHither)/(double)(currentScene->camYon - currentScene->camHither) * std::numeric_limits<int>::max() );
}

// Изменить форму усеченного конуса
void Renderer::transformCamera(TransformationMatrix cameraMovement){

    resetDepthBuffer();

    worldToCamera = cameraMovement.getInverse();

    perspectiveToScreen = TransformationMatrix();

    int highestResolution;
    if (xRes > yRes)
        highestResolution = xRes;
    else
        highestResolution = yRes;

    double highestXYDelta;
    if ((currentScene->xHigh - currentScene->xLow) > (currentScene->yHigh - currentScene->yLow))
        highestXYDelta = currentScene->xHigh - currentScene->xLow;
    else
        highestXYDelta = currentScene->yHigh - currentScene->yLow;


    perspectiveToScreen.addTranslation(xRes/2, yRes/2, 0);

    perspectiveToScreen.addNonUniformScale( (highestResolution - (2 * border)) / (highestXYDelta), ( (highestResolution - (2 * border)) / (highestXYDelta) ), 1 ); // Scale

    perspectiveToScreen.addTranslation(-(currentScene->xHigh + currentScene->xLow)/2.0, -(currentScene->yHigh + currentScene->yLow)/2.0, 0);

    screenToPerspective = TransformationMatrix();
    screenToPerspective *= perspectiveToScreen;
    screenToPerspective = screenToPerspective.getInverse();
}

// Рассчитать отражение вектора, направленного в сторону от поверхности
NormalVector Renderer::reflectOutVector(NormalVector* faceNormal, NormalVector* outVector){

    NormalVector bounceDirection( *faceNormal );
    bounceDirection *= 2 * (faceNormal->dotProduct( *outVector ) );
    bounceDirection -= *outVector;
    bounceDirection.normalize();

    return bounceDirection;
}

// Обновляем точку пересечения трассировки лучей с помощью интерполированных нормалей и значений цвета
void Renderer::setInterpolatedIntersectionValues(Vertex* intersectionPoint, Polygon* hitPoly){

    Vertex* topLeftVertex = hitPoly->getHighest();
    Vertex* topRightVertex = topLeftVertex;

    Vertex* botLeftVertex = hitPoly->getNext(topLeftVertex->vertexNumber);
    Vertex* botRightVertex = hitPoly->getPrev(topRightVertex->vertexNumber);


    int count = hitPoly->getVertexCount();
    while (topLeftVertex->y == botLeftVertex->y && count > 0){
        count--;
        topLeftVertex = botLeftVertex;
        botLeftVertex = hitPoly->getNext(botLeftVertex->vertexNumber);
    }
    count = hitPoly->getVertexCount();
    while (topRightVertex->y == botRightVertex->y && count > 0){
        count--;
        topRightVertex = botRightVertex;
        botRightVertex = hitPoly->getPrev(botRightVertex->vertexNumber);
    }

    count = hitPoly->getVertexCount();
    while (!(topLeftVertex->y >= intersectionPoint->y && botLeftVertex->y <= intersectionPoint->y) && count > 0){
        count--;
        topLeftVertex = botLeftVertex;
        botLeftVertex = hitPoly->getNext(botLeftVertex->vertexNumber);
    }
    count = hitPoly->getVertexCount();
    while (!(topRightVertex->y >= intersectionPoint->y && botRightVertex->y <= intersectionPoint->y) && count > 0){
        count--;
        topRightVertex = botRightVertex;
        botRightVertex = hitPoly->getPrev(botRightVertex->vertexNumber);
    }


    double leftRatio = (intersectionPoint->y - topLeftVertex->y)/(double)(botLeftVertex->y - topLeftVertex->y); // How far from top to bot we are
    double rightRatio = (intersectionPoint->y - topRightVertex->y)/(double)(botRightVertex->y - topRightVertex->y);

    NormalVector leftNml;
    leftNml.xn = ((1 - leftRatio) * topLeftVertex->normal.xn) + (leftRatio * botLeftVertex->normal.xn);
    leftNml.yn = ((1 - leftRatio) * topLeftVertex->normal.yn) + (leftRatio * botLeftVertex->normal.yn);
    leftNml.zn = ((1 - leftRatio) * topLeftVertex->normal.zn) + (leftRatio * botLeftVertex->normal.zn);
    leftNml.normalize();

    NormalVector rightNml;
    rightNml.xn = ((1 - rightRatio) * topRightVertex->normal.xn) + (rightRatio * botRightVertex->normal.xn);
    rightNml.yn = ((1 - rightRatio) * topRightVertex->normal.yn) + (rightRatio * botRightVertex->normal.yn);
    rightNml.zn = ((1 - rightRatio) * topRightVertex->normal.zn) + (rightRatio * botRightVertex->normal.zn);
    rightNml.normalize();

    unsigned int leftColor = addColors(
                                        multiplyColorChannels(topLeftVertex->color, 1 - leftRatio ),
                                        multiplyColorChannels(botLeftVertex->color, leftRatio)
                );

    unsigned int rightColor = addColors(
                                        multiplyColorChannels(topRightVertex->color, 1 - rightRatio ),
                                        multiplyColorChannels(botRightVertex->color, rightRatio)
                );


    double leftX, rightX;
    double leftYDiff = botLeftVertex->y - topLeftVertex->y;
    double rightYDiff = botRightVertex->y - topRightVertex->y;

    if (leftYDiff != 0){
        leftX = ((intersectionPoint->y - topLeftVertex->y) * ((botLeftVertex->x - topLeftVertex->x)/leftYDiff) + topLeftVertex->x);
    }
    else
        leftX = botLeftVertex->x;

    if (rightYDiff != 0){
        rightX = ((intersectionPoint->y - topRightVertex->y) * ((botRightVertex->x - topRightVertex->x)/rightYDiff) + topRightVertex->x);
    }
    else
        rightX = botRightVertex->x;

    double pointRatio;
    double xDiff = rightX - leftX;
    if (xDiff != 0)
        pointRatio = (intersectionPoint->x - leftX)/(double)(xDiff);
    else
        pointRatio = 0;


    intersectionPoint->normal.xn = ((1 - pointRatio) * leftNml.xn) + (pointRatio * rightNml.xn);
    intersectionPoint->normal.yn = ((1 - pointRatio) * leftNml.yn) + (pointRatio * rightNml.yn);
    intersectionPoint->normal.zn = ((1 - pointRatio) * leftNml.zn) + (pointRatio * rightNml.zn);
    intersectionPoint->normal.normalize();

    intersectionPoint->color = addColors(
                                        multiplyColorChannels(leftColor, 1 - pointRatio ),
                                        multiplyColorChannels(rightColor, pointRatio )
    );

}

// Проверяем, имеют ли два полигона ребро
bool Renderer::haveSharedEdge(Polygon* poly1, Polygon* poly2){
    int count = 0;
    for (int i = 0; i < poly1->getVertexCount(); i++){
        for (int j = 0; j < poly2->getVertexCount(); j++){
            if (poly1->vertices[i] == poly2->vertices[j]){
                count++;
                if (count >= 2)
                    return true;

                break;
            }
        }
    }
    return false;
}

// Проверяем, больше ли угол между двумя гранями полигона, имеющими общий край, 180 градусов
bool Renderer::isFaceReflexAngle(Polygon* currentPoly, Polygon* hitPoly){
    Vertex* notCommon = nullptr;
    for (int i = 0; i < hitPoly->getVertexCount(); i++){

        bool foundCommon = false;

        for (int j = 0; j < currentPoly->getVertexCount(); j++){

            if (hitPoly->vertices[i] == currentPoly->vertices[j]){
                foundCommon = true;

                break;
            }
        }


        if (!foundCommon){
            notCommon = &hitPoly->vertices[i];
            break;
        }
    }

    if (notCommon == nullptr)
        return false;


    NormalVector faceTangent(hitPoly->getNext(notCommon->vertexNumber)->x - notCommon->x, hitPoly->getNext(notCommon->vertexNumber)->y - notCommon->y, hitPoly->getNext(notCommon->vertexNumber)->z - notCommon->z);
    faceTangent.normalize();


    if (currentPoly->faceNormal.dotProduct(faceTangent) > 0)
        return true;
    else
        return false;
}


void Renderer::debugLights(){

    for (unsigned int i = 0; i < currentScene->theLights.size(); i++){
        Light debug = currentScene->theLights[i];
        debug.position.transform(&cameraToPerspective);
        debug.position.transform(&perspectiveToScreen);
        drawLine( Line(Vertex(debug.position.x - 15, debug.position.y, debug.position.z, 0xffff0000), Vertex(debug.position.x + 15, debug.position.y, debug.position.z, 0xffff0000) ), ambientOnly, true, 0, 0);
        drawLine( Line(Vertex(debug.position.x, debug.position.y - 15, debug.position.z, 0xff00ff00), Vertex(debug.position.x, debug.position.y + 15, debug.position.z, 0xff00ff00) ), ambientOnly, true, 0, 0);
        drawLine( Line(Vertex(debug.position.x, debug.position.y, debug.position.z - 15, 0xffff00ff), Vertex(debug.position.x, debug.position.y, debug.position.z + 15, 0xffff00ff) ), ambientOnly, true, 0, 0);

        debug.debug();
    }
}

