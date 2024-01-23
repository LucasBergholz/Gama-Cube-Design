#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <opencv2/imgproc.hpp>
#include <windows.h>

using namespace std;
using namespace cv;


int main() {
    // Recebendo entrada de qual tratamento sera realizado
    int scene;
    cout << "Escolha uma cena: (1 ou 2)\n";
    cin >> scene;

    // Criar diretorio que vai armazenar o video
    string path = "camera_video";
    if (CreateDirectory(path.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
        printf("Pressione qualquer tecla para iniciar a gravação...\n");
        waitKey(0);
    }
    else {
        std::cerr << "Failed to create directory: " << path << std::endl;
        return -1;
    }

    // Criar VideoCapture
    VideoCapture capture(0);

    //Checando se abriu o video
    if (!capture.isOpened()) {
        printf("Erro no VideoCapture.");
        return -1;
    }

    int numberOfFrames = 0;
    // Loop para pegar cada frame
    while (true) {
        // Ler o frame
        Mat frame;
        capture >> frame;

        imshow("Webcam", frame);
        // Grava o frame no folder designado
        imwrite("camera_video\\frame_" + to_string(numberOfFrames++) + ".png", frame);
        // Escolhe o tempo de distancia entre os frames
        if (waitKey(100) >= 0) break;
    }

    // Vetores que guardarao as maiores areas e o index de cada maior contorno, por frame
    vector<double> biggestAreas(numberOfFrames, 0);
    vector<int> indexOfContours(numberOfFrames, 0);

    int countOfFrames = 0;
    while (numberOfFrames--) {
        Mat greyFrame = imread("camera_video\\frame_" + to_string(countOfFrames++) + ".png", 0);
        GaussianBlur(greyFrame, greyFrame, Size(5, 5), 0);
        Mat threshFrame;

        threshold(greyFrame, threshFrame, 200, 255, THRESH_BINARY);

        vector<Vec4i> hierarchy;
        vector<vector<Point>> contours;

        Mat output = Mat::zeros(greyFrame.size(), CV_8UC3);
        findContours(threshFrame, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE);

        int count2 = 0; // Contador para percorrer os Contornos
        int maxContour = 0; // Qtdade de pontos do contorno
        int biggestContour = 0; //Index do maior raio
        // Pegar indice do maior raio
        for (auto i : contours) {
            int contourSize = contours[count2].size();
            if (contourSize >= maxContour) { maxContour = contourSize; biggestContour = count2; }
            count2++;
        }
        drawContours(output, contours, -1, Scalar(0, 255, 0), 3);

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
        for (size_t i = 0; i < contours.size(); ++i) {
            // Calcular os moments
            Moments mu = moments(contours[i]);
            // Calcular coordenadas dos centroides
            c.centroids[i].x = mu.m10 / mu.m00;
            c.centroids[i].y = mu.m01 / mu.m00;
            // Draw a circle at the centroid
            //circle(output, c.centroids[printed], 3, Scalar(0, 0, 255), -1);
            //circle(greyFrame, c.centroids[printed], 3, Scalar(0, 0, 255), -1);

            // Print das coordenadas
            //cout << "Centroid of Contour " << i << ": (" << c.centroids[i].x << ", " << c.centroids[i].y << ")" << endl;
            // Print das coordenadas e valor da area
            //cout << "Contour " << i << ": " << contourArea(contours[i]) << endl;
        }

        if (scene == 1) {

            //Discriminar centroides
            vector<Point2f> finalCentroids(contours.size()); //Vetor que guarda os centroids apos discriminacao
            for (size_t i = 0; i < c.centroids.size(); i++) {
                int biggestPoint = i;
                if (!c.visited[i]) { // Se o centroid nao tiver sido visitado ainda, compare ele com os demais
                    for (size_t j = i + 1; j < c.centroids.size(); j++) {
                        if (c.visited[j]) continue; // Se o centroid ja foi visitado, pula pro proximo
                        cout << abs(c.centroids[i].x - c.centroids[j].x) << "  " << abs(c.centroids[i].y - c.centroids[j].y) << endl;
                        if (abs(c.centroids[i].x - c.centroids[j].x) <= 30 && abs(c.centroids[i].y - c.centroids[j].y) <= 30) { // abs serve para achar o modulo, se o modulo da diferenca for menor/igual a 20 entra
                            if (contours[biggestPoint].size() < contours[j].size()) biggestPoint = j;
                            c.visited[j] = 1;
                        }
                    }
                    finalCentroids[i] = c.centroids[biggestPoint];
                }
                c.visited[i] = 1;
            }

            //Desenhar circulos nos centroides
            for (size_t i = 0; i < finalCentroids.size(); ++i) {
                if ((finalCentroids[i].x + finalCentroids[i].y == 0)) continue;
                circle(output, finalCentroids[i], 3, Scalar(0, 0, 255), -1);
                circle(greyFrame, finalCentroids[i], 3, Scalar(0, 0, 255), -1);
            }


        }
        else {
            /*
            //Somar area
            double totalArea = contourArea(contours[biggestContour]);

            //Loop que checara quais contornos estao a uma distancia de 30px do maior contorno de todos, levando em conta o centroide
            for (size_t j = 0; j < c.centroids.size(); j++) {
                if (j == biggestContour) continue; // Se o contorno for o maior, pula, pois ele é a referencia
                if (abs(c.centroids[biggestContour].x - c.centroids[j].x) <= 30 && abs(c.centroids[biggestContour].y - c.centroids[j].y) <= 30) { // abs serve para achar o modulo, se o modulo da diferenca for menor/igual a 20 entra
                    totalArea += contourArea(contours[j]);
                    printf("%d ", j);
                }
            }

            /*Isso esta levando em conta o maior raio como parametro, e nao a maior soma de raios, existe um mundo onde
            4 raios pequenos tem area maior q um raio grande, devo mudar este codigo*/
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

        imshow("frame cinza", greyFrame);
        imshow("oie", output);

        waitKey(0);
        string filename = "C:\\Users\\Lucas Bergholz\\Documents\\GAMA CUBE DESIGN\\CameraInput\\camera_video\\frame_" + to_string(countOfFrames - 1) + ".png";
        //remove(filename.c_str());

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
            printf("%f\n", biggestAreas[i]);
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
        findContours(frameBiggestContour, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //Contornos encontrados
        drawContours(output, contours, indexOfContours[indexOfFrameOfBiggestContour], Scalar(0, 255, 0), FILLED);
        // Calcular os moments
        Moments mu = moments(contours[indexOfContours[indexOfFrameOfBiggestContour]]);
        // Calcular coordenadas dos centroides
        Point2f centroide;
        centroide.x = mu.m10 / mu.m00;
        centroide.y = mu.m01 / mu.m00;
        circle(output, centroide, 3, Scalar(0, 0, 255), -1);
        //imshow("Testando1", frameBiggestContour);
        //imshow("Testando", output);
        //printf("\n Area = %f \n", contourArea(contours[indexOfContours[indexOfFrameOfBiggestContour]]));
        //cout << centroide << endl;

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
            findContours(nextFrame, nextContours, nextHierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //Contornos encontrados
            if (!contours.size()) {
                cout << "Acabou aqui" << endl;
                break;
            }
            drawContours(output, nextContours, -1, Scalar(255 - (duration * 10), (duration * 10), (duration * 5)), 3);
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
                    circle(output, centroide, 3, Scalar(0, 0, 255), -1);
                    duration++;
                    cout << isInside << " e frame numero: " << nextIndex << " e centroide: " << centroide << endl;
                    break;
                }
            }
            imshow("Testando", output);
            waitKey(200);
            // Quando chegar no frame pra frente que o raio acaba, inverte o count pra comecar a procurar quando ele comeca
            if (beforeCheckDuration == duration) {
                if (count == -1) break; //Se o count = -1 e pq ja checou o raio pra frente, se entrou nesse if denovo e pq ja checou o raio pra tras
                nextIndex = indexOfFrameOfBiggestContour;
                //Retornar o centroide para a posicao de inicio
                // Calcular os moments
                mu = moments(contours[indexOfContours[indexOfFrameOfBiggestContour]]);
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