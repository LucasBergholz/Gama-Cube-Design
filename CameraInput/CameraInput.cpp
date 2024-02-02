#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <opencv2/imgproc.hpp>
#include <thread>

using namespace std;
using namespace cv;

// Variável para indicar se o loop deve ser interrompido
bool stopLoop = false;

// Função que verifica a entrada do teclado
void checkInput() {
    while (true) {
        if (cin.get() == 'q') {
            ::stopLoop = true;
            break;
        }
    }
}


int main() {
    // Recebendo entrada de qual tratamento sera realizado
    int scene;
    std::cout << "Escolha uma cena: (1 ou 2)\n";
    cin >> scene;

    // Criar VideoCapture
    VideoCapture capture(0);

    //Checando se abriu o video
    if (!capture.isOpened()) {
        printf("Erro no VideoCapture.");
        return -1;
    }

    int numberOfFrames = 0;
    thread inputThread(checkInput);
    // Loop para pegar cada frame
    while (!::stopLoop) {
        // Ler o frame
        Mat frame;
        capture >> frame;
        // Grava o frame no folder designado
        imwrite("camera_video\\frame_" + to_string(numberOfFrames++) + ".png", frame);
        // Escolhe o tempo de distancia entre os frames
        // comando de espera + comando scanf
        //if (waitKey(100) >= 0) break;
        
        // Escolhe o tempo de distancia entre os frames
        // Adiciona um pequeno delay
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    inputThread.join();

    // Vetores que guardarao as maiores areas por frame e quais centroides ja foram visitados
    vector<double> biggestAreas(numberOfFrames, 0);
    vector<vector<short int>> matrixVisited(numberOfFrames);
    vector<vector<Point2f>> finalCentroidsReal(numberOfFrames);

    int countOfFrames = 1;
    numberOfFrames--;
    while (numberOfFrames--) {
        Mat greyFrame = imread("camera_video\\frame_" + to_string(countOfFrames++) + ".png", 0);
        GaussianBlur(greyFrame, greyFrame, Size(5, 5), 0);
        Mat threshFrame;

        threshold(greyFrame, threshFrame, 200, 255, THRESH_BINARY);

        vector<Vec4i> hierarchy;
        vector<vector<Point>> contours;

        Mat output = Mat::zeros(greyFrame.size(), CV_8UC3);
        cv::findContours(threshFrame, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);        

        //Achar os centroides
        struct centroides { // Struct que guardara os centroids
            vector<Point2f> centroids; //Vetor que guarda todos os centroids encontrados
            vector<int> visited; //Vetor que guarda se o centroid foi visitado ou nao
        };

        struct centroides c; // Inicializacao da struct
        c.centroids.resize(contours.size()); // Inicializacao do vetor centroids com o tamanho do numero de contours q existem
        c.visited.resize(contours.size(), 0); // Inicializando com valor 0 todos os membros do vetor visited

        // Print do frame
        printf("\n%d:\n\n", countOfFrames);

        //Loop para pegar todos os centroides
        for (size_t i = 0; i < contours.size(); ++i) {
            // Calcular os moments
            Moments mu = moments(contours[i]);
            // Calcular coordenadas dos centroides
            c.centroids[i].x = mu.m10 / mu.m00;
            c.centroids[i].y = mu.m01 / mu.m00;
        }

        if (scene == 1) {

            //Discriminar centroides
            vector<Point2f> finalCentroids(contours.size()); //Vetor que guarda os centroids apos discriminacao
            for (size_t i = 0; i < c.centroids.size(); i++) {
                int biggestPoint = i;
                if (!c.visited[i]) { // Se o centroid nao tiver sido visitado ainda, compare ele com os demais
                    for (size_t j = i + 1; j < c.centroids.size(); j++) {
                        if (c.visited[j]) continue; // Se o centroid ja foi visitado, pula pro proximo
                        if (abs(c.centroids[i].x - c.centroids[j].x) <= 30 && abs(c.centroids[i].y - c.centroids[j].y) <= 30) { // abs serve para achar o modulo, se o modulo da diferenca for menor/igual a 20 entra
                            if (contours[biggestPoint].size() < contours[j].size()) biggestPoint = j;
                            c.visited[j] = 1;
                        }
                    }
                    finalCentroids[i] = c.centroids[biggestPoint];
                }
                c.visited[i] = 1;
            }
            std::cout << finalCentroids << endl;
            matrixVisited[countOfFrames-1].resize(finalCentroids.size(), 0);
            finalCentroidsReal[countOfFrames-1] = finalCentroids;

        }
        else {
            //Metodo correto considerando descrepancias de areas
            double totalArea = 0;
            vector<double> areas(contours.size(), 0); //Vetor que guarda as areas apos discriminacao
            for (size_t i = 0; i < c.centroids.size(); i++) {
                if (!c.visited[i]) { // Se o centroid nao tiver sido visitado ainda, compare ele com os demais
                    areas[i] += contourArea(contours[i]);
                    for (size_t j = i + 1; j < c.centroids.size(); j++) {
                        if (c.visited[j]) continue; // Se o centroid ja foi visitado, pula pro proximo
                        if (abs(c.centroids[i].x - c.centroids[j].x) <= 30 && abs(c.centroids[i].y - c.centroids[j].y) <= 30) { // abs serve para achar o modulo, se o modulo da diferenca for menor/igual a 20 entra
                            areas[i] += contourArea(contours[j]);
                            c.visited[j] = 1;
                        }
                    }
                }
                c.visited[i] = 1;
            }

            for (size_t i = 0; i < c.centroids.size(); i++) {
                if (areas[i] >= totalArea) {
                    totalArea = areas[i];
                }
            }
            biggestAreas[countOfFrames - 1] = totalArea;
        }

    }

    if (scene == 1) {
        for (size_t k = 1; k < countOfFrames - 1; k++) {
            std::cout << "TO NO FRAME = " << k << endl;
            //Calcular centroide desse raio para realizar pointPolygonTest
            //Realizando tratamento da imagem pra achar novamente o contorno


            for (size_t j = 0; j < finalCentroidsReal[k].size(); j++) {
                Point2f zero(0,0);
                if (matrixVisited[k][j] == 1 || finalCentroidsReal[k][j] != finalCentroidsReal[k][j] || finalCentroidsReal[k][j] == zero) continue;
                std::cout << "TO NO CONTORNO = " << j << endl;
                int duration = 1;
                Point2f centroide = finalCentroidsReal[k][j];
                matrixVisited[k][j] = 1;
                int nextIndex = k + 1;
                while (true)
                {
                    //Agora achar duracao de cada centroide do frame atual
                    Mat nextFrame = imread("camera_video\\frame_" + to_string(nextIndex) + ".png", 0);
                    cv::GaussianBlur(nextFrame, nextFrame, Size(5, 5), 0);
                    cv::threshold(nextFrame, nextFrame, 200, 255, THRESH_BINARY);
                    vector<Vec4i> nextHierarchy;
                    vector<vector<Point>> nextContours;
                    cv::findContours(nextFrame, nextContours, nextHierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //Contornos encontrados
                    //Se o valor do beforeCheckDuration nao mudar, e pq nenhum raio passou no pointPolygonTest
                    int beforeCheckDuration = duration;
                    for (size_t i = 0; i < nextContours.size(); i++)
                    {
                        if (matrixVisited[nextIndex][i] == 1) continue;
                        int isInside = pointPolygonTest(nextContours[i], centroide, true);
                        if (isInside >= -10) {
                            //Se estiver dentro de 10px de distancia do centroide atual, atualizar o centroide
                            // Calcular os moments
                            Moments mu = moments(nextContours[i]);
                            // Calcular coordenadas dos centroides
                            centroide.x = mu.m10 / mu.m00;
                            centroide.y = mu.m01 / mu.m00;
                            duration++;
                            std::cout << isInside << " e frame numero: " << nextIndex << " e centroide: " << centroide << endl;
                            //Colocar como visitado
                            matrixVisited[nextIndex][i] = 1;
                            break;
                        }
                    }
                    // Quando chegar no frame pra frente que o raio acaba, inverte o count pra comecar a procurar quando ele comeca
                    if (beforeCheckDuration == duration) {
                        //Calculo do centroide do Rocha
                        //Salvamento da duracao e do valor do centroide
                        std::cout << "Duracao = " << duration << endl;
                        break;
                    }
                    nextIndex++;
                }

            }
        }
    }

    if (scene == 2) {
        // Achar qual o frame de maior area
        int indexOfFrameOfBiggestContour = 0;
        double biggestContourFrames = 0;
        for (size_t i = 0; i < biggestAreas.size(); i++) {
            if (biggestContourFrames < biggestAreas[i]) {
                indexOfFrameOfBiggestContour = i;
                biggestContourFrames = biggestAreas[i];
            }
        }

        printf("Index aqui: %d\n Maior Area aqui: %d\n", indexOfFrameOfBiggestContour, biggestAreas[indexOfFrameOfBiggestContour]);

        //Calcular centroide desse raio para realizar pointPolygonTest
        //Realizando tratamento da imagem pra achar novamente o contorno
        Mat frameBiggestContour = imread("camera_video\\frame_" + to_string(indexOfFrameOfBiggestContour) + ".png", 0);
        GaussianBlur(frameBiggestContour, frameBiggestContour, Size(5, 5), 0);
        threshold(frameBiggestContour, frameBiggestContour, 200, 255, THRESH_BINARY);
        vector<Vec4i> hierarchy;
        vector<vector<Point>> contours;

        Mat output = Mat::zeros(frameBiggestContour.size(), CV_8UC3);
        cv::findContours(frameBiggestContour, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //Contornos encontrados
        int count2 = 0; // Contador para percorrer os Contornos
        int maxContour = 0; // Qtdade de pontos do contorno
        int biggestContour = 0; //Index do maior raio
        // Pegar indice do maior raio
        for (auto i : contours) {
            int contourSize = contours[count2].size();
            if (contourSize >= maxContour) { maxContour = contourSize; biggestContour = count2; }
            count2++;
        }
        // Calcular os moments
        Moments mu = moments(contours[biggestContour]);
        // Calcular coordenadas dos centroides
        Point2f centroide;
        centroide.x = mu.m10 / mu.m00;
        centroide.y = mu.m01 / mu.m00;

        // Rastrear quantos frames esse raio existe, primeiro pra frente e depois pra trás
        int duration = 1;
        int count = 1;
        int nextIndex = indexOfFrameOfBiggestContour + count;
        while (true)
        {
            if (nextIndex == countOfFrames || nextIndex < 0) break;
            //Hora de achar os contornos do frame da frente
            Mat nextFrame = imread("camera_video\\frame_" + to_string(nextIndex) + ".png", 0);
            GaussianBlur(nextFrame, nextFrame, Size(5, 5), 0);
            threshold(nextFrame, nextFrame, 200, 255, THRESH_BINARY);
            vector<Vec4i> nextHierarchy;
            vector<vector<Point>> nextContours;
            cv::findContours(nextFrame, nextContours, nextHierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //Contornos encontrados
            if (!contours.size()) {
                std::cout << "Acabou aqui" << endl;
                break;
            }
            //Se o valor do beforeCheckDuration nao mudar, e pq nenhum raio passou no pointPolygonTest
            int beforeCheckDuration = duration;
            for (size_t i = 0; i < nextContours.size(); i++)
            {
                int isInside = pointPolygonTest(nextContours[i], centroide, true);
                if (isInside >= -10) {
                    //Se estiver dentro de 10px de distancia do centroide atual, atualizar o centroide
                    // Calcular os moments
                    mu = moments(nextContours[i]);
                    // Calcular coordenadas dos centroides
                    centroide.x = mu.m10 / mu.m00;
                    centroide.y = mu.m01 / mu.m00;
                    duration++;
                    std::cout << isInside << " e frame numero: " << nextIndex << " e centroide: " << centroide << endl;
                    break;
                }
            }
            // Quando chegar no frame pra frente que o raio acaba, inverte o count pra comecar a procurar quando ele comeca
            if (beforeCheckDuration == duration) {
                if (count == -1) break; //Se o count = -1 e pq ja checou o raio pra frente, se entrou nesse if denovo e pq ja checou o raio pra tras
                nextIndex = indexOfFrameOfBiggestContour;
                //Retornar o centroide para a posicao de inicio
                // Calcular os moments
                mu = moments(contours[biggestContour]);
                // Calcular coordenadas dos centroides
                centroide.x = mu.m10 / mu.m00;
                centroide.y = mu.m01 / mu.m00;
                count = -1;
            }
            nextIndex += count;
        }
        printf("Duracao = %d\n", duration);
    }

    waitKey(0);

    return 0;
}