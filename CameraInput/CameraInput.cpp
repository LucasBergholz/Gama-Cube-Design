#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <opencv2/imgproc.hpp>
#include <windows.h>

using namespace std;
using namespace cv;


int main() {
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
            cout << "Centroid of Contour " << i << ": (" << c.centroids[i].x << ", " << c.centroids[i].y << ")" << endl;
        }

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

        for (size_t i = 0; i < finalCentroids.size(); ++i) {
            if ((finalCentroids[i].x + finalCentroids[i].y == 0)) continue;
            circle(output, finalCentroids[i], 3, Scalar(0, 0, 255), -1);
            circle(greyFrame, finalCentroids[i], 3, Scalar(0, 0, 255), -1);
        }

        printf("%d:\n\n\n\n", countOfFrames);

        imshow("frame cinza", greyFrame);
        imshow("oie", output);

        waitKey(0);
        string filename = "C:\\Users\\Lucas Bergholz\\Documents\\GAMA CUBE DESIGN\\CameraInput\\camera_video\\frame_" + to_string(countOfFrames - 1) + ".png";
        remove(filename.c_str());

    }
    waitKey(0);

    return 0;
}