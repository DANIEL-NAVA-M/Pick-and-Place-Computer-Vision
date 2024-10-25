//VERSION FINAL

#include <stdio.h>
#include <stdlib.h>
#include <libserialport.h>

#define BAUD 9600

#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>

#include <chrono>
#include <thread>

#include <stdio.h>

using namespace cv;
using namespace std;

//MASCARAS DE COLORES

// Rango para detectar el color rojo (en espacio HSV)
int low_H_red = 133, low_S_red = 80, low_V_red = 0;
int high_H_red = 175, high_S_red = 255, high_V_red = 255;

// Rango para detectar el color azul (en espacio HSV)
int low_H_blue = 100, low_S_blue = 0, low_V_blue = 30;
int high_H_blue = 130, high_S_blue = 255, high_V_blue = 255;

// Rango para detectar el color verde (en espacio HSV)
int low_H_green = 40, low_S_green = 50, low_V_green = 50;
int high_H_green = 80, high_S_green = 255, high_V_green = 255;

const String window_capture_name = "Video Capture";
const String window_detection_name = "Object Detection";

Mat maskHSV, resultHSV;

int morph_elem = 0;
int morph_size = 0;
int morph_operator = 1;

int finish = 0;

RNG rng(12345);

int count_red[] = { 0, 0, 0 };
int count_green[] = { 0, 0, 0 };
int count_blue[] = { 0, 0, 0 };

int color[] = { 0, 0, 0 };


string mask = "mask";

Mat location;

Mat location_one;
Mat location_two;
Mat location_three;

int counter = 0;

Mat frame;

// Función para dividir el frame en tres secciones
void sections(Mat frame) {
	location_one = frame(Range(150, 200), Range(140, 200)); // Sección 1
	location_two = frame(Range(150, 200), Range(200, 260)); // Sección 2
	location_three = frame(Range(150, 200), Range(260, 320)); // Sección 3

	imshow("Location One", location_one);
	imshow("Location Two", location_two);
	imshow("Location Three", location_three);
}

// Función genérica para identificar color según rango HSV
void color_identification(Mat location, int low_H, int low_S, int low_V, int high_H, int high_S, int high_V) {
	maskHSV = Mat::zeros(3, 3, CV_8UC1);

	cvtColor(location, location, COLOR_BGR2HSV);
	inRange(location, Scalar(low_H, low_S, low_V), Scalar(high_H, high_S, high_V), maskHSV);

	int operation = morph_operator + 2;
	Mat element = getStructuringElement(morph_elem, Size(20 * morph_size + 10, 20 * morph_size + 10), Point(morph_size, morph_size));
	morphologyEx(maskHSV, maskHSV, operation, element);

	//imshow(mask, maskHSV);
}

// Función para contar los píxeles que pertenecen a la máscara de color
int count_pixels() {
	counter = 0;
	for (int i = 0; i < maskHSV.cols; i++) {
		for (int j = 0; j < maskHSV.rows; j++) {
			if (maskHSV.at<uchar>(j, i) > 0) {
				counter++;  // Incrementa el conteo de píxeles
			}
		}
	}

	if (counter < 20) {
		counter = 0;
	}

	return counter;
}

// Función para determinar el color predominante basado en el conteo
int return_index(int color_m[], int len) {
	int max_index = 0;
	for (int j = 0; j < len; j++) {
		if (color_m[j] > color_m[max_index]) {
			max_index = j;
		}
	}

	if (color_m[max_index] == 0) {
		return -1;  // Return -1 if the maximum value is 0
	}

	return max_index + 1;  // Devuelve el índice del color predominante
}

int findCero(int arr[], int size) {
	for (int i = 0; i < size; ++i) {
		if (arr[i] == 0) {
			return i; // Devuelve el índice donde se encuentra el cero
		}
	}
	return -1; // Devuelve -1 si no hay ceros en el arreglo
}
int main(int argc, char* argv[]) {
	int pos[] = { 0, 0, 0 };

	// Variables para la cámara
	int n = 0;
	char filename[200];
	string window_name = "original";

	// Configurar la cámara
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cout << "Cannot open camera";
		return -1;
	}

	// Variables para la comunicación serial
	struct sp_port* port;
	int err;
	int key = 0;
	int cmd = 0;

	// Variables de procesamiento de imágenes
	Mat temp;

	// Configurar el puerto serial
	if (argc < 2) {
		fprintf(stderr, "Port use\n");
		exit(1);
	}

	err = sp_get_port_by_name(argv[1], &port);
	if (err == SP_OK) err = sp_open(port, SP_MODE_WRITE);
	if (err != SP_OK) {
		fprintf(stderr, "Can't open port %s\n", argv[1]);
		exit(2);
	}

	sp_set_baudrate(port, BAUD);
	sp_set_bits(port, 8);

	// Solicitar al usuario que elija el color a buscar


	// Función principal de bucle
	while (key != 'q') {
		
		for (int i = 0; i < 100; i++) {
			cap >> frame;
			imshow(window_name, frame);
			char key = (char)waitKey(25);

			sections(frame);  // Dividir la imagen en secciones
		}


		int color_choice;
		cout << "Choose a color to detect:\n";
		cout << "1. Red\n";
		cout << "2. Green\n";
		cout << "3. Blue\n";
		cout << "Enter choice: ";
		cin >> color_choice;

		// Identificar el color según la elección del usuario
		switch (color_choice) {
		case 1:
			color[0] = 1;
			cout << "Detecting Red color" << endl;
			color_identification(location_one, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
			count_red[0] = count_pixels();
			color_identification(location_two, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
			count_red[1] = count_pixels();
			color_identification(location_three, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
			count_red[2] = count_pixels();
			break;
		case 2:
			color[1] = 1;

			cout << "Detecting Green color" << endl;
			color_identification(location_one, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
			count_green[0] = count_pixels();
			color_identification(location_two, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
			count_green[1] = count_pixels();
			color_identification(location_three, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
			count_green[2] = count_pixels();
			break;
		case 3:
			color[2] = 1;

			cout << "Detecting Blue color" << endl;
			color_identification(location_one, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
			count_blue[0] = count_pixels();
			color_identification(location_two, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
			count_blue[1] = count_pixels();
			color_identification(location_three, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
			count_blue[2] = count_pixels();
			break;
		default:
			cout << "Invalid choice, exiting..." << endl;

			for (int i = 0; i < 100; i++) {
				cap >> frame;
				imshow(window_name, frame);
				char key = (char)waitKey(25);

				sections(frame);  // Dividir la imagen en secciones
			}

			return -1;
		}

		int color_index = return_index((color_choice == 1) ? count_red : (color_choice == 2) ? count_green : count_blue, 3);

		int another = color_index - 1;

		if (another < 0) {
			cout << "NO BOX DETECTED" << endl;

		}

		else {
			pos[another] = 1;

			cout << "Detected in section: " << color_index << endl;
			cout << "[ " << pos[0] << ", " << pos[1] << ", " << pos[2] << " ]" << endl;

			if (color_choice == 1) { // Red
				cmd = (color_index == 1) ? 12 : (color_index == 2) ? 5 : 8; // Command based on section
			}
			else if (color_choice == 2) { // Green
				cmd = (color_index == 1) ? 3 : (color_index == 2) ? 6 : 10;
			}
			else if (color_choice == 3) { // Blue
				cmd = (color_index == 1) ? 1 : (color_index == 2) ? 4 : 7;
			}

			sp_blocking_write(port, &cmd, 1, 100);
			this_thread::sleep_for(chrono::seconds(15)); //15 para robot, 5 para debug
			sections(frame);  // Dividir la imagen en secciones

		}


		cmd = 0;
		sp_blocking_write(port, &cmd, 1, 100);



		int next_color_choice = (color_choice == 1) ? 2 : (color_choice == 2) ? 3 : 1; // Next color
		
		for (int i = 0; i < 100; i++) {
			cap >> frame;
			imshow(window_name, frame);
			char key = (char)waitKey(25);

			sections(frame);  // Dividir la imagen en secciones
		}

		//first color code ends
		// 
		// 
		//second color code begins
		switch (next_color_choice) {
		case 1:
			switch (color_index) { //previous position
			case 1:

				count_red[0] = 0;
				color_identification(location_two, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[1] = count_pixels();
				color_identification(location_three, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[2] = count_pixels();
				break;


			case 2:

				color_identification(location_one, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[0] = count_pixels();
				count_red[1] = 0;
				color_identification(location_three, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[2] = count_pixels();
				break;
			
			case 3:

				color_identification(location_one, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[0] = count_pixels();
				color_identification(location_two, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[1] = count_pixels();
				count_red[2] = 0;
				break;

			default:
				color_identification(location_one, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[0] = count_pixels();
				color_identification(location_two, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[1] = count_pixels();
				color_identification(location_three, low_H_red, low_S_red, low_V_red, high_H_red, high_S_red, high_V_red);
				count_red[2] = count_pixels();
				break;
			}


			break;

		case 2:
			cout << "Detecting Green color" << endl;
			switch (color_index) {
			case 1:
				count_green[0] = 0;
				color_identification(location_two, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[1] = count_pixels();
				cout << "pixeles en 2: " << count_green[1] << endl;
				color_identification(location_three, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[2] = count_pixels();
				cout << "pixeles en 3: " << count_green[2] << endl;

				break;


			case 2:
				color_identification(location_one, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[0] = count_pixels();
				count_green[1] = 0;
				color_identification(location_three, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[2] = count_pixels();
				break;

			case 3:
				color_identification(location_one, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[0] = count_pixels();
				color_identification(location_two, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[1] = count_pixels();
				count_green[2] = 0;
				break;

			default:
				color_identification(location_one, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[0] = count_pixels();
				color_identification(location_two, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[1] = count_pixels();
				color_identification(location_three, low_H_green, low_S_green, low_V_green, high_H_green, high_S_green, high_V_green);
				count_green[2] = count_pixels();
				break;
			}
		


			break;
		case 3:
			cout << "Detecting Blue color" << endl;
			switch (color_index) {
			case 1:
				count_blue[0] = 0;

				color_identification(location_two, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[1] = count_pixels();
				cout << "pixeles en 2: " << count_blue[1] << endl;

				color_identification(location_three, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[2] = count_pixels();
				cout << "pixeles en 3: " << count_blue[2] << endl;

				break;


			case 2:
				color_identification(location_one, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[0] = count_pixels();
				cout << "pixeles en 1: " << count_blue[0] << endl;

				count_blue[1] = 0;

				color_identification(location_three, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[2] = count_pixels();
				cout << "pixeles en 3: " << count_blue[2] << endl;

				break;

			case 3:
				color_identification(location_one, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[0] = count_pixels();

				color_identification(location_two, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[1] = count_pixels();

				count_blue[2] = 0;
				break;

			default:
				color_identification(location_one, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[0] = count_pixels();

				color_identification(location_two, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[1] = count_pixels();

				color_identification(location_three, low_H_blue, low_S_blue, low_V_blue, high_H_blue, high_S_blue, high_V_blue);
				count_blue[2] = count_pixels();
				break;

			}
			break;


		default:
			cout << "Invalid choice, exiting..." << endl;
			return -1;
		}

		int second_index = return_index((next_color_choice == 1) ? count_red : (next_color_choice == 2) ? count_green : count_blue, 3);

		another = second_index - 1;
		pos[another] = 1;


		cout << "Detected in section: " << second_index << endl;
		cout << "[ " << pos[0] << ", " << pos[1] << ", " << pos[2] << " ]" << endl;


		if (next_color_choice == 1) { // Red
			cmd = (second_index == 1) ? 12 : (second_index == 2) ? 5 : 8; // Command based on section
		}
		else if (next_color_choice == 2) { // Green
			cmd = (second_index == 1) ? 3 : (second_index == 2) ? 6 : 10;
		}
		else if (next_color_choice == 3) { // Blue
			cmd = (second_index == 1) ? 1 : (second_index == 2) ? 4 : 7;
		}

		sp_blocking_write(port, &cmd, 1, 100);
		this_thread::sleep_for(chrono::seconds(15));
		
		cmd = 0;
		sp_blocking_write(port, &cmd, 1, 100);
		//second color code ends
		// 
		// 
		// 
		//third color code begin
		for (int i = 0; i < 100; i++) {
			cap >> frame;
			imshow(window_name, frame);
			char key = (char)waitKey(25);

			sections(frame);  // Dividir la imagen en secciones
		}

		int next_next_color_choice = (next_color_choice == 1) ? 2 : (next_color_choice == 2) ? 3 : 1; // Next color

		//int color_missing = findCero(color, 3);
		int position_missing = findCero(pos, 3);
		cout << "La posicion faltante eeeeesss: > " << position_missing << " < ESTAAAAAA" << endl;

		switch (next_next_color_choice) {
		case 1:
			//red
			switch (position_missing) {
			case 0:
				cmd = 12;
				break;
			case 1:
				cmd = 5;
				break;
			case 2:
				cmd = 8;
				break;
			}

			cout << "red in section " << position_missing;

			break;
		case 2:
			//green
			switch (position_missing) {
			case 0:
				cmd = 3;
				break;
			case 1:
				cmd = 6;
				break;
			case 2:
				cmd = 10;
				break;
			}

			cout << "green in section " << position_missing;

			break;
		case 3:
			//blue
			switch (position_missing) {
			case 0:
				cmd = 1;
				break;
			case 1:
				cmd = 4;
				break;
			case 2:
				cmd = 7;
				break;
			}

			cout << "blue in section " << position_missing + 1;

			break;

		}

		sp_blocking_write(port, &cmd, 1, 100);
		this_thread::sleep_for(chrono::seconds(15));


		//third color code ends
		pos[0] = 0;
		pos[1] = 1;
		pos[2] = 0;

		cmd = 0;
		sp_blocking_write(port, &cmd, 1, 100);


		for (int i = 0; i < 100; i++) {
			cap >> frame;
			imshow(window_name, frame);
			char key = (char)waitKey(25);

			sections(frame);  // Dividir la imagen en secciones
		}

		this_thread::sleep_for(chrono::seconds(45));

		// Guardar imagen si se presiona espacio
		if (key == ' ') {
			sprintf_s(filename, "filename%.3d.jpg", n++);
			imwrite(filename, frame);
			cout << "Saved " << filename << endl;
		}
	}

	// Cerrar el puerto serial
	sp_close(port);
	return 0;
}
