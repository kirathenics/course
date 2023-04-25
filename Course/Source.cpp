#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <mpi.h>
#include <iostream>
#include <ctime>

using namespace std;
using namespace cv;

#define white_color 255
#define image_size 3000

void open_image(Mat& image, string path) 
{
	image = imread(path, IMREAD_GRAYSCALE);
	if (!image.data)
	{
		cout << "Не удалось открыть изображение!" << endl;
		exit(-1);
	}
	return;
}

unsigned char** create_buffer(Mat& image)
{
	unsigned char** buffer = new unsigned char* [image.rows];
	for (int i = 0; i < image.rows; i++)
	{
		buffer[i] = new unsigned char[image.cols];
	}

	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			buffer[i][j] = image.at<unsigned char>(i, j);
		}
	}
	return buffer;
}

void process_image(Mat& image, int scale)
{
	unsigned char** buffer = nullptr;
	// уменьшение изображения
	threshold(image, image, 0, 255, THRESH_OTSU | THRESH_BINARY);
	for (int number_of_times = 0; number_of_times < scale; number_of_times++)
	{
		buffer = create_buffer(image);
		for (int i = 0; i < image.rows; i++)
		{
			for (int j = 0; j < image.cols; j++)
			{
				if (buffer[i][j] == white_color)
				{
					for (int x = i - 1; x <= i + 1; x++)
					{
						for (int y = j - 1; y <= j + 1; y++)
						{
							if (x != -1 && y != -1 && x != image.rows && y != image.cols)
							{
								image.at<unsigned char>(x, y) = white_color;
							}
						}
					}
				}
			}
		}
	}

	// дифференциальный оператор Робертса
	buffer = create_buffer(image);
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			image.at<unsigned char>(i, j) = i != image.rows - 1 ? (unsigned char)sqrt(pow(buffer[i][j] - buffer[i + 1][j + 1], 2) + pow(buffer[i][j + 1] - buffer[i + 1][j], 2)) : (unsigned char)abs(buffer[i][j] - buffer[i][j + 1]);
			image.at<unsigned char>(i, j) = image.at<unsigned char>(i, j) > 15 ? 255 : 0;
		}
	}

	for (int i = 0; i < image.rows; i++)
	{
		delete[] buffer[i];
	}
	delete[] buffer;

	return;
}

void save_image(Mat& image, string path)
{
	if (!imwrite(path, image)) 
	{
		cout << "Не удалось сохранить изображение!" << endl;
		exit(-1);
	}
	return;
}

int main(int argc, char* argv[])
{
	setlocale(0, "");
	utils::logging::setLogLevel(utils::logging::LOG_LEVEL_SILENT);

	MPI_Init(&argc, &argv);
	int rank, size, scale;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	string source_image_path = "Source.bmp", output_image_path = "Result.bmp";
	Mat image, section;

	if (rank == 0)
	{
		cout << "Введите масштаб уменьшения изображения: ";
		cin >> scale;

		chrono::steady_clock::time_point start = chrono::steady_clock::now();

		open_image(image, source_image_path);

		if (size == 1)
		{
			process_image(image, scale);
		}

		for (int i = 1; i < size; i++)
		{
			section = Mat(image, Rect(0, (i - 1) * image_size / (size - 1), image_size, image_size / (size - 1)));
			MPI_Send(section.data, image_size / (size - 1) * image_size, MPI_UNSIGNED_CHAR, i, i, MPI_COMM_WORLD);
			MPI_Send(&scale, 1, MPI_INT, i, i + size, MPI_COMM_WORLD);
		}

		for (int i = 1; i < size; i++)
		{
			section = Mat(image_size / (size - 1), image_size, CV_8U);
			MPI_Recv(section.data, image_size / (size - 1) * image_size, MPI_UNSIGNED_CHAR, i, i, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			if (i == 1)
			{
				image = section;
				continue;
			}
			vconcat(image, section, image);
		}
		save_image(image, output_image_path);

		chrono::steady_clock::time_point finish = chrono::steady_clock::now();
		float Duration = chrono::duration_cast<chrono::milliseconds>(finish - start).count();

		cout << "Длительность в секундах: " << Duration / 1000 << endl;
	}
	else 
	{
		section = Mat(image_size / (size - 1), image_size, CV_8U);
		MPI_Recv(section.data, image_size / (size - 1) * image_size, MPI_UNSIGNED_CHAR, 0, rank, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		MPI_Recv(&scale, 1, MPI_INT, 0, rank + size, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

		process_image(section, scale);

		MPI_Send(section.data, image_size / (size - 1) * image_size, MPI_UNSIGNED_CHAR, 0, rank, MPI_COMM_WORLD);
	}
	MPI_Finalize();

	return 0;
}
