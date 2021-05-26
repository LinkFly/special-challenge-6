// special-challenge-6.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <string>
#include <utility>
#include <array>
#include <windows.h>

using namespace std;

struct Display {
	HWND myconsole{};
	HDC mydc{};
	uint32_t width{}, height{};
	int scale = 5;

	Display() {
		//Get a console handle
		myconsole = GetConsoleWindow();
		//Get a handle to device context
		mydc = GetDC(myconsole);
	}	

	Display(uint32_t width, uint32_t height) : Display() {
		this->width = width;
		this->height = height;
	}

	~Display() {
		ReleaseDC(myconsole, mydc);
	}

	void writePixel(array<uint8_t, 4>& color, int x, int y) {
		COLORREF COLOR = RGB(color[2], color[1], color[0]);
		x *= scale;
		y *= scale;
		auto curHeight = height * scale;
		y = curHeight - y;
		//Draw pixels
		for (int i = 0; i < scale; ++i)
			for(int j = 0; j < scale; ++j)
				SetPixel(mydc, x + i, y + j, COLOR);
	}
};

struct DisplayStub {
	uint32_t width{}, height{};
	DisplayStub(uint32_t width, uint32_t height): width{width}, height{height} {

	}
};

void error(string errMsg, const string extraMsg = "") {
	cerr << errMsg << extraMsg << endl;
	exit(-1);
}

int main(int argc, char** argv)
{
	//// Get file name
	if (argc < 2) {
		cout << "Using: <program> <file.bmp>";
		exit(0);
	}
	string file = argv[1];
	//// Get file name

	//// Open file
    ifstream fin(file, ios::binary);
    if (!fin) {
		error("Failed open of file: ", file);
    }
	//// end Open file

	///////// Helpers functions /////////
	auto chRead = [&fin, &file]() {
		if (!fin) {
			error("Failed read of file: ", file);
		}
	};

	auto setPos = [&fin, &file](size_t pos) {
		if (!fin.seekg(pos, ios::beg)) {
			error("Failed set position of file: ", file);
		}
	};

	auto binRead = [&fin, &chRead](char* buf, size_t bytesCount) {
		fin.read(buf, bytesCount);
		chRead();
	};

	auto readNum = [&binRead, &setPos](size_t& curPos, bool bReverse = false) {
		setPos(curPos);
		uint32_t res;
		binRead(reinterpret_cast<char*>(&res), 4);
		curPos += 4;
		return res;
	};

	auto readShort = [&binRead, &setPos](size_t& curPos, bool bReverse = false) {
		setPos(curPos);
		uint16_t res;
		binRead(reinterpret_cast<char*>(&res), 2);
		curPos += 2;
		return res;
	};

	auto readPixel = [&binRead, &setPos](size_t& curPos, bool bReverse = false) {
		setPos(curPos);
		uint8_t res[3];
		binRead(reinterpret_cast<char*>(&res), 3);
		curPos += 3;

		array<uint8_t, 4> resAr{};
		resAr[2] = res[2];
		resAr[1] = res[1];
		resAr[0] = res[0];
		return resAr;
	};

	auto printNewLine = []() {
		cout << endl;
	};

	auto isBfType = [](uint16_t word) {
		return (((word >> 8) == 'B')
			|| (word & 0xff) == 'M');
	};
	///////// end Helpers functions /////////

	///////// Reading bytes /////////
	size_t curPos = 0;
	if (isBfType(readShort(curPos))) {
		error("It isn't a bmp-file");
	}

	uint32_t fsize;
	binRead((char*)&fsize, 4);

	curPos = 10;
	uint32_t offset = readNum(curPos);
	uint32_t biSize = readNum(curPos);
	uint32_t biWidth = readNum(curPos);
	uint32_t biHeight = readNum(curPos);
	uint16_t biPlanes = readShort(curPos);
	uint16_t biBitCount = readShort(curPos);
	uint8_t bytespp = biBitCount / 8;
	uint32_t biCompression = readNum(curPos);
	if (biPlanes != 1 || biCompression != 0) {
		error("biPlanes != 1 or biCompression != 0 don't supported");
	}
	uint32_t biSizeImage = biWidth * biHeight * (biBitCount / 8);

	Display display{ biWidth, biHeight };

	auto printPixel = [&display](array<uint8_t, 4>& pixel, int x, int y) {
		display.writePixel(pixel, x, y);
	};

	//// Read pixels
	const uint8_t alignDiv = 4;
	const uint8_t align = alignDiv - ((biWidth * bytespp) % alignDiv);
	bool bFixAlign = align % alignDiv;
	curPos = offset;
	for (size_t j = 0; j < biHeight; ++j) {


		for (size_t i = 0; i < biWidth; ++i)
		{
			auto pixel = readPixel(curPos);
			printPixel(pixel, i, j);
			// align position
			if (bFixAlign) {
				if ((i + 1) == biWidth) {
					curPos += align;
				}
			}
		}
	}
	///////// end Reading bytes /////////

	fin.close();
	cin.ignore();
}

