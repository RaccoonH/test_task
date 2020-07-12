#include <iostream>
#include <fstream>

#include "bmp_header.h"

using namespace std;

int main(int argc, char* argv[])
{
	//Initializing for BMP file
	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER bmpInfoHeader;

	if (argc < 6)
	{
		cout << "Too few arguments" << endl;
		return -1;
	}

	char* fileBMP = argv[1];
	char* inputFilename = argv[2];
	char* outputFilename = argv[3];
	int width = atoi(argv[4]);
	int height = atoi(argv[5]);

	//Open BMP file and convert RGB to YUV
	ifstream fileStream;
	fileStream.open(fileBMP, ifstream::binary);
	if (!fileStream)
	{
		cout << "Error opening file" << endl;
		return -1;
	}
	if (readHeader(fileStream, bmpHeader, bmpInfoHeader) < 1)
	{
		cout << "Error with image file" << endl;
		return -1;
	}

	RGB **rgb = new RGB*[bmpInfoHeader.biHeight];
	for (unsigned int i = 0; i < bmpInfoHeader.biHeight; i++)
	{
		rgb[i] = new RGB[bmpInfoHeader.biWidth];
	}
	readRGB(fileStream, bmpInfoHeader, rgb);

	YUV **yuv = new YUV*[bmpInfoHeader.biWidth];
	for (unsigned int i = 0; i < bmpInfoHeader.biHeight; i++)
	{
		yuv[i] = new YUV[bmpInfoHeader.biWidth];
	}
	convertRGBtoYUV(rgb, yuv, bmpInfoHeader.biHeight, bmpInfoHeader.biWidth);

	//Open video files
	ifstream inputFile;
	ofstream outputFile;
	int frameSize = (width * height + (width * height / 2));
	uint8_t *rawData = NULL;
	inputFile.open(inputFilename, ios::binary);
	rawData = (uint8_t *)malloc(frameSize);
	outputFile.open(outputFilename, ios::binary);

	while (!inputFile.eof())
	{
		inputFile.read((char*)rawData, frameSize);
		for (int i = 0; i < bmpInfoHeader.biHeight; i++)
		{
			//Y
			for (int j = 0; j < bmpInfoHeader.biWidth; j++)
			{
				int index = i * width;
				int indexY = bmpInfoHeader.biHeight - i - 1;
				rawData[index + j] = yuv[bmpInfoHeader.biHeight - i - 1][j].yuvY;
			}
		}

		for (int i = 0; i < bmpInfoHeader.biHeight / 2; i++)
		{
			//U
			for (int j = 0; j < bmpInfoHeader.biWidth / 2; j++)
			{
				int index = (width * height) + i * (width/2);
				int indexU = bmpInfoHeader.biHeight - (i * 2) - 1;
				rawData[index + j] = yuv[indexU][j * 2].yuvU;
			}
		}

		for (int i = 0; i < bmpInfoHeader.biHeight / 2; i++)
		{
			//V
			for (int j = 0; j < bmpInfoHeader.biWidth / 2; j++)
			{
				int index = (width * height) + (width * height / 4) + i * (width / 2);
				int indexV = bmpInfoHeader.biHeight - (i * 2) - 1;
				rawData[index + j] = yuv[indexV][j * 2].yuvV;
			}
		}

		outputFile.write((char *)rawData, frameSize);
	}

	inputFile.close();
	outputFile.close();
	free(rawData);

	for (unsigned int i = 0; i < bmpInfoHeader.biHeight; i++)
	{
		free(rgb[i]);
	}

	for (unsigned int i = 0; i < bmpInfoHeader.biHeight; i++)
	{
		free(yuv[i]);
	}
	free(yuv);
	free(rgb);
}
