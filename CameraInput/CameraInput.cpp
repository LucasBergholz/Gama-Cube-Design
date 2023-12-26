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
        drawContours(output, contours, biggestContour, Scalar(0, 255, 0), 3);

        int x = 0;
        int y = 0;
        count2 = 0;

        for (auto i : contours) {
            for (auto j : contours[biggestContour]) {
                if (contours[biggestContour].size()) {
                    x = x + contours[biggestContour][count2].x;
                    y = y + contours[biggestContour][count2].y;
                }
                count2++;
            }
            break;
        }

        if (count2 > 0) { printf("%d e %d\n", x / count2, y / count2); }
        if (x != 0 && y != 0) {
            Point p(x / count2, y / count2);
            circle(output, p, 3, Scalar(0, 0, 255), -1);
        }

        // coordinates of centroid
        printf("%d:\n\n\n\n", countOfFrames);

        imshow("frame cinza", greyFrame);
        imshow("oie", output);

        string filename = "C:\\Users\\Lucas Bergholz\\Documents\\GAMA CUBE DESIGN\\CameraInput\\camera_video\\frame_" + to_string(countOfFrames-1) + ".png";
        remove(filename.c_str());
        waitKey(0);

    }

    waitKey(0);

    return 0;
}