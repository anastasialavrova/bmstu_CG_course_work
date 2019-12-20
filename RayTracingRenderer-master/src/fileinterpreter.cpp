#include "fileinterpreter.h"
#include <fstream>
#include <iostream>
#include <list>
#include <iterator>
#include <string>
#include "mesh.h"
#include "transformationmatrix.h"
#include <stack>
#include <vector>
#include "renderutilities.h"
#include "normalvector.h"
#include "light.h"
#include "polygon.h"
#include <QDebug>

using std::ifstream;
using std::cout;
using std::list;
using std::stack;
using std::vector;

// Конструктор по умолчанию
FileInterpreter::FileInterpreter(){
    // Ничего не делает
}

// Чтение файла и сборка сетки с учетом имени файла
// Возвращает: объект сетки, созданный из описаний файлов .simp
Scene FileInterpreter::buildSceneFromFile(double x, double z, double xCam, double yCam, double zCam){

    // Сбор получившихся полигонов в сцену и ее возврат
    Scene theScene;
    currentScene = &theScene;

    theScene.theMeshes = getMeshHelper2(x, z, xCam, yCam, zCam, false, false, false, false, 0xffffffff, phong , 0.3, 8, 0.5); // Set the default values to start

    // Генерируем ограничивающие рамки:
    for (auto &currentMesh : theScene.theMeshes){
        currentMesh.generateBoundingBox();
    }

    currentScene = nullptr; // Удалить ссылку на локальный объект для безопасности

    return theScene;
}

// интерпретировать прочитанную строку
// Возвращает: список разделенных и очищенных токенов
list<string> FileInterpreter::interpretTokenLine(string currentLine){

    // Удаляем пробелы в начале строки
    unsigned int start = 0;
    while (start < currentLine.length() && (currentLine[start] == ' ' || currentLine[start] == '\t' || currentLine[start] == '\n' || currentLine[start] == '\r' ))
        start++;
    if (start != 0)
        currentLine = currentLine.substr(start, string::npos);

    // Обработка пустых строк или комментариев:
    if (currentLine.empty() || currentLine[0] == '#')
        return list<string>(); // Nothing to insert: Return an empty list

    // Разбиваем строку, затем очищаем каждый токен от ненужных символов форматирования:
    list<string> tokens = cleanseTokens( split(currentLine) );

    return tokens;
}

// Разделить строку
// Возвращает: список <string>, содержащий каждый токен как отдельный элемент, в порядке чтения файла
list<string> FileInterpreter::split(string line){
    list<string> tokens;

    unsigned int start = 0;
    while (line[start] == ' ' || line[start] == '\t' || line[start] == '\n' || line[start] == '\r')
        start++;
    line = line.substr(start, string::npos);

    if (line.length() == 1 && (line[0] == '{' || line[0] == '}')){
        tokens.push_back(line);
        return tokens;
    }

    start = 0;
    unsigned int indexIterator = 1;
    while (indexIterator < line.length() ){

        if (line[indexIterator] == ' '){
            while ((indexIterator + 1) < line.length() && line[indexIterator + 1] == ' ')
                indexIterator++;

            string newToken = line.substr(start, indexIterator - start);
            tokens.push_back(newToken);

            start = indexIterator + 1;
            indexIterator+= 2;
        }

        else if (line[indexIterator] == '/'){
            string newToken = line.substr(start, (indexIterator - start));
            tokens.push_back(newToken);
            tokens.push_back("/");

            indexIterator++;

            if (line[indexIterator] == '/'){
                tokens.push_back("/");
                indexIterator++;
            }
            start = indexIterator;

        } else
            indexIterator++;

    }

    if (start <= line.length() - 1)
        tokens.push_back(line.substr(start, string::npos));


    return tokens;
}

// Очистить список токенов любых символов форматирования. Используется после того, как split извлек токены из строки
// Возвращает: список <string>, содержащий токены, очищенные от символов форматирования
list<string> FileInterpreter::cleanseTokens(list<string> tokens){
    list<string>::iterator theIterator = tokens.begin();
    while (theIterator != tokens.end()){
        string currentLine = *theIterator;

        unsigned int start = 0;
        while ( start < currentLine.length() && ( currentLine[start] == ' ' || currentLine[start] == '(' || currentLine[start] == '\t' || currentLine[start] == '\"' ) ){
            start++;
        }

        int end = (int)currentLine.length() - 1;
        while ( end >= 0 && ( currentLine[ end ] == ' ' || currentLine[ end ] == ',' || currentLine[ end ] == ')' || currentLine[ end ] == '\t' || currentLine[ end ] == '\"' || currentLine[ end ] == '\n' || currentLine[ end ] == '\r') ) {
            end--;
        }

        if ( end < (int)start){
            list<string>::iterator temp = theIterator;
            theIterator++;
            tokens.erase(temp);
        } else{
            *theIterator = currentLine.substr(start, end - start + 1);
            theIterator++;
        }
    }

    return tokens;
}

// Чтение файла obj
// Возвращает: вектор <Polygon>, содержащий все грани, описанные объектом
vector<Polygon> FileInterpreter::getPolysFromObj(string filename){

    vector<Polygon> theFaces;
    vector<Vertex> theVertices;
    theVertices.emplace_back( Vertex() );

    vector<NormalVector> theNormals;
    theNormals.emplace_back( NormalVector() );

    ifstream* input = new ifstream();
    input->open(filename);
    if (input->is_open() ){
        while(!input->eof()){
            string currentLine;
            getline(*input, currentLine);
            list<string>currentLineTokens = interpretTokenLine(currentLine);

            list<string>::iterator theIterator = currentLineTokens.begin();
            while (theIterator != currentLineTokens.end()){

                if (theIterator->compare("v") == 0){
                    theIterator++;

                    Vertex v1;
                    double x = stod(*theIterator++);
                    double y = stod(*theIterator++);
                    double z = stod(*theIterator++);
                    v1.x = x;
                    v1.y = y;
                    v1.z = z;


                    if (currentLineTokens.size() == 5){
                        double w = stod(*theIterator++);
                        v1.setW(w);
                    }

                    else if (currentLineTokens.size() == 7){
                        double red = stod(*theIterator++);
                        double green = stod(*theIterator++);
                        double blue = stod(*theIterator++);
                        v1.color = combineColorChannels( red, green, blue );
                    }
                    else if (currentLineTokens.size() == 8){
                        double w = stod(*theIterator++);
                        v1.setW(w);

                        double red = stod(*theIterator++);
                        double green = stod(*theIterator++);
                        double blue = stod(*theIterator++);
                        v1.color = combineColorChannels( red, green, blue );
                    }

                    v1.z *= -1;

                    theVertices.emplace_back(v1);
                }

                else if(theIterator->compare("vn") == 0){
                    theIterator++;

                    NormalVector newNormal;
                    newNormal.xn = stod(*theIterator++);
                    newNormal.yn = stod(*theIterator++);
                    newNormal.zn = stod(*theIterator++);

                    newNormal.zn *= -1;

                    theNormals.emplace_back(newNormal);
                }

                else if(theIterator->compare("vt") == 0){
                    theIterator++;

                }

                else if(theIterator->compare("f") == 0){
                    theIterator++;

                    Polygon newFace;

                    while (theIterator != currentLineTokens.end() ){

                        Vertex newVertex;

                        int index = stoi( *theIterator++ );
                        if (index < 1)
                            newVertex = theVertices[ theVertices.size() + index];
                        else
                            newVertex = theVertices[ index ];

                        if (theIterator != currentLineTokens.end() && theIterator->compare("/") == 0){
                            theIterator++;

                            if (theIterator != currentLineTokens.end() && !theIterator->compare("/") == 0){
                                theIterator++;

                                if (theIterator != currentLineTokens.end() && theIterator->compare("/") == 0){
                                    theIterator++;
                                    NormalVector newNormal;
                                    index = stoi( *theIterator++ );
                                    if (index < 1)
                                        newNormal = theNormals[ theNormals.size() + index];
                                    else
                                        newNormal = theNormals[ index ];

                                    newVertex.normal = newNormal;
                                }
                            }
                            else {
                                theIterator++;

                                NormalVector newNormal;
                                index = stoi( *theIterator++ );
                                if (index < 1)
                                    newNormal = theNormals[ theNormals.size() + index];
                                else
                                    newNormal = theNormals[ index ];

                                newVertex.normal = newNormal;
                            }
                        }

                        newFace.addVertex(newVertex);
                    }


                    NormalVector faceNormal = newFace.getFaceNormal();
                    for (int i = 0; i < newFace.getVertexCount(); i++){
                        if (newFace.vertices[i].normal.isZero() ) {
                            newFace.vertices[i].normal = faceNormal; // Set 0,0,0 normals to be the face normal
                        }
                    }

                    if (newFace.getVertexCount() > 3){
                        vector<Polygon>* triangulatedFaces = newFace.getTriangulatedFaces();
                        for (unsigned int i = 0; i < triangulatedFaces->size(); i++){
                            theFaces.emplace_back(triangulatedFaces->at(i) );
                        }
                        delete triangulatedFaces;

                    } else
                        theFaces.emplace_back(newFace);

                }
                else
                    theIterator++;

            }

        }

    }
    else
        cout << "ERROR - File " << filename << " not found!!! Please place it the working directory.\n";

    return theFaces;
}

// Рекурсивная вспомогательная функция: извлекает многоугольники
vector<Mesh> FileInterpreter::getMeshHelper2(double sphere_x, double sphere_z, double xCam, double yCam, double zCam, bool currentIsWireframe, bool currentisDepthFogged, bool currentAmbientLighting, bool currentUseSurfaceColor, unsigned int currentSurfaceColor, ShadingModel currentShadingModel, double currentSpecCoef, double currentSpecExponent, double currentReflectivity){

    // Флаги сетки:
    bool isWireframe = currentIsWireframe;                // Каркас / заполненный
    bool isDepthFogged = currentisDepthFogged;            // Флаг глубины

    // Флаги полигонов:
    bool usesAmbientLighting = currentAmbientLighting;  // Флаг внешнего освещения
    bool usesSurfaceColor = currentUseSurfaceColor;
    unsigned int theSurfaceColor = currentSurfaceColor;

    ShadingModel theShadingModel = currentShadingModel;
    double theSpecCoefficient = currentSpecCoef;
    double theSpecExponent = currentSpecExponent;
    double theReflectivity = currentReflectivity;

    TransformationMatrix CTM;
    stack<TransformationMatrix> theCTMStack;

    vector<Polygon> currentFaces;               // Рабочий вектор граней полигона
    vector<Mesh> extractedMeshes;               // Коллекция собранных сеток

    currentScene->numRayBounces = 0;

    for (int i = 1; i <= 7; i++)
    {
        theCTMStack.push(CTM);
        CTM = TransformationMatrix();

        switch(i)
        {
        case 1:
        {
            TransformationMatrix currentTransformation;
            double transX = xCam;
            double transY = yCam;
            double transZ = zCam;
            currentTransformation.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation;

            currentScene->xLow = -1;
            currentScene->yLow = -1;
            currentScene->xHigh = 1;
            currentScene->yHigh = 1;
            currentScene->camHither = 1;
            currentScene->camYon = 200;
            currentScene->cameraMovement = CTM;

            CTM = theCTMStack.top();
            theCTMStack.pop();
            // Применяем текущий CTM к граням из блока
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            // Вставляем обработанные грани в конечный объект сетки:
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
                }

            break;
            }


        case 2:
        {
            TransformationMatrix currentTransformation;
            double transX = 0;
            double transY = 125;
            double transZ = 2000;
            currentTransformation.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation;

            double redIntensity = 2;
            double greenIntensity = 1.7;
            double blueIntensity = 1.3;
            double attenuationA = 0.001;
            double attenuationB = 0.02;
            // Создать источник света, преобразовать его с помощью CTM и передать его в средство визуализации
            Light newLight(redIntensity, greenIntensity, blueIntensity, attenuationA, attenuationB);
            newLight.position.transform( &CTM );
            currentScene->theLights.emplace_back(newLight);

            CTM = theCTMStack.top();
            theCTMStack.pop();
            // Применяем текущий CTM к граням из блока
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            // Вставляем обработанные грани в конечный объект сетки:
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
                }

            break;
        }

        case 3:
        {
            TransformationMatrix currentTransformation;
            double transX = 100;
            double transY = 100;
            double transZ = -200;
            currentTransformation.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation;

            double redIntensity = 2;
            double greenIntensity = 1.7;
            double blueIntensity = 1.3;
            double attenuationA = 0.01;
            double attenuationB = 0.01;
            Light newLight(redIntensity, greenIntensity, blueIntensity, attenuationA, attenuationB);
            newLight.position.transform( &CTM );
            currentScene->theLights.emplace_back(newLight);

            CTM = theCTMStack.top();
            theCTMStack.pop();
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
                }

            break;
        }


        case 4:
        {
            TransformationMatrix currentTransformation;
            double transX = -75;
            double transY = 50;
            double transZ = -200;
            currentTransformation.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation;

            double redIntensity = 1.7;
            double greenIntensity = 1.5;
            double blueIntensity = 1.5;
            double attenuationA = 0.05;
            double attenuationB = 0.05;
            Light newLight(redIntensity, greenIntensity, blueIntensity, attenuationA, attenuationB);
            newLight.position.transform( &CTM );
            currentScene->theLights.emplace_back(newLight);

            CTM = theCTMStack.top();
            theCTMStack.pop();
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
                }

            break;
        }


        case 5:
        {
            TransformationMatrix currentTransformation1;
            double transX = -0.5;
            double transY = 2;
            double transZ = 0;
            currentTransformation1.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation1;

            TransformationMatrix currentTransformation2;
            double scaleX = 1.0;
            double scaleY = 1.0;
            double scaleZ = 1.0;
            currentTransformation2.addNonUniformScale(scaleX, scaleY, scaleZ);
            CTM *= currentTransformation2;

            usesSurfaceColor = true;
            double red = 0.6;
            double green = 0.6;
            double blue = 0.6;
            theSurfaceColor = combineColorChannels(red, green, blue);

            theSpecCoefficient = 0.5;
            theSpecExponent = 2.5;

            theReflectivity = 0.5;

            theShadingModel = flat;

            vector<Polygon> objContents = getPolysFromObj("./unitCube.obj");
            // Обработка полученных полигонов:
            for (unsigned int i = 0; i < objContents.size(); i++){
                objContents[i].transform(&CTM);
                // Устанавливаем инструкции цвета поверхности:
                if (usesSurfaceColor)
                    objContents[i].setSurfaceColor(theSurfaceColor);
                objContents[i].setSpecularCoefficient(theSpecCoefficient);
                objContents[i].setSpecularExponent(theSpecExponent);
                objContents[i].setShadingModel(theShadingModel);
                objContents[i].setReflectivity(theReflectivity);
                // Установить окружающее освещение:
                objContents[i].setAffectedByAmbientLight(usesAmbientLighting);
                // Рассчитать нормаль грани многоугольника:
                objContents[i].faceNormal = objContents[i].getFaceNormal();
            }
            currentFaces.insert(currentFaces.end(), objContents.begin(), objContents.end() );

            CTM = theCTMStack.top();
            theCTMStack.pop();
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
            }
            if (!currentUseSurfaceColor) // Обработка рекурсивных случаев, когда мы унаследовали цвет, и его необходимо применить к загруженному файлу
                usesSurfaceColor = false;

            break;
        }


        case 6:
        {
            TransformationMatrix currentTransformation1;
            double transX = 0;
            double transY = -0.75;
            double transZ = 0;
            currentTransformation1.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation1;

            TransformationMatrix currentTransformation2;
            double scaleX = 4;
            double scaleY = 4;
            double scaleZ = 4;
            currentTransformation2.addNonUniformScale(scaleX, scaleY, scaleZ);
            CTM *= currentTransformation2;

            usesSurfaceColor = true;
            double red = 0.6;
            double green = 0.6;
            double blue = 0.6;
            theSurfaceColor = combineColorChannels(red, green, blue);

            theSpecCoefficient = 0.5;
            theSpecExponent = 2.5;

            theReflectivity = 0.5;

            theShadingModel = flat;

            vector<Polygon> objContents = getPolysFromObj("./unitPlane.obj");
            for (unsigned int i = 0; i < objContents.size(); i++){
                objContents[i].transform(&CTM);
                if (usesSurfaceColor)
                    objContents[i].setSurfaceColor(theSurfaceColor);
                objContents[i].setSpecularCoefficient(theSpecCoefficient);
                objContents[i].setSpecularExponent(theSpecExponent);
                objContents[i].setShadingModel(theShadingModel);
                objContents[i].setReflectivity(theReflectivity);
                objContents[i].setAffectedByAmbientLight(usesAmbientLighting);
                objContents[i].faceNormal = objContents[i].getFaceNormal();
            }
            currentFaces.insert(currentFaces.end(), objContents.begin(), objContents.end() );

            CTM = theCTMStack.top();
            theCTMStack.pop();
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
            }
            if (!currentUseSurfaceColor)
                usesSurfaceColor = false;

            break;
        }


        case 7:
        {
            TransformationMatrix currentTransformation1;
            double transX = sphere_x;
            double transY = 0;
            double transZ = sphere_z;
            currentTransformation1.addTranslation(transX, transY, transZ);
            CTM *= currentTransformation1;

            TransformationMatrix currentTransformation2;
            double scaleX = 1;
            double scaleY = 1;
            double scaleZ = 1;
            currentTransformation2.addNonUniformScale(scaleX, scaleY, scaleZ);
            CTM *= currentTransformation2;

            usesSurfaceColor = true;
            double red = 0.6;
            double green = 0.6;
            double blue = 0.6;
            theSurfaceColor = combineColorChannels(red, green, blue);

            theSpecCoefficient = 0.5;
            theSpecExponent = 2.5;

            theReflectivity = 0.5;

            theShadingModel = gouraud;

            vector<Polygon> objContents = getPolysFromObj("./unitSphere_20.obj");
            for (unsigned int i = 0; i < objContents.size(); i++){
                objContents[i].transform(&CTM);
                if (usesSurfaceColor)
                    objContents[i].setSurfaceColor(theSurfaceColor);
                objContents[i].setSpecularCoefficient(theSpecCoefficient);
                objContents[i].setSpecularExponent(theSpecExponent);
                objContents[i].setShadingModel(theShadingModel);
                objContents[i].setReflectivity(theReflectivity);
                objContents[i].setAffectedByAmbientLight(usesAmbientLighting);
                objContents[i].faceNormal = objContents[i].getFaceNormal();
            }
            currentFaces.insert(currentFaces.end(), objContents.begin(), objContents.end() );

            CTM = theCTMStack.top();
            theCTMStack.pop();
            for (unsigned int i = 0; i < currentFaces.size(); i++){
                currentFaces[i].transform(&CTM);
            }
            if (currentFaces.size() > 0 ){
                Mesh newMesh;
                newMesh.faces = currentFaces;
                currentFaces.clear();
                newMesh.isWireframe = isWireframe;
                extractedMeshes.emplace_back( newMesh );
            }
            if (!currentUseSurfaceColor)
                usesSurfaceColor = false;

            break;
        }

        }
    }

    if (currentFaces.size() > 0){

        Mesh newMesh;
        newMesh.faces = currentFaces;
        newMesh.isWireframe = isWireframe;

        extractedMeshes.emplace_back( newMesh );
    }

    return extractedMeshes;
}



















