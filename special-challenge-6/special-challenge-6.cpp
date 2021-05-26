// special-challenge-6.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <string>
#include <utility>

using namespace std;

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
		if (res[0] == 0 && res[1] == 0 && res[2] == 0)
			return 0;
		else if (res[0] == 255 && res[1] == 255 && res[2] == 255)
			return 1;
		else return 2;
	};

	auto printPixel = [](int pixel) {
		switch (pixel) {
		case 0: cout << ' '; break;
		case 1: cout << '\xDB'; break;
		// Bad symbol (not black and not white)
		case 2: cout << '#'; break;
		}
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
	// Commented because sometimes it's zeros
	//uint32_t biSizeImage = readNum(curPos); 
	uint32_t biSizeImage = biWidth * biHeight * (biBitCount / 8);

	//// Read pixels
	const uint8_t alignDiv = 4;
	const uint8_t align = alignDiv - ((biWidth * bytespp) % alignDiv);
	bool bFixAlign = align % alignDiv;
	for (size_t j = biHeight; j > 0; --j) {
		curPos = offset + (j - 1) * biHeight * bytespp;
		if (bFixAlign)
			curPos += (j - 1) * align;
		setPos(curPos);
		for (size_t i = 0; i < biWidth; ++i)
		{
			printPixel(readPixel(curPos));
			// align position
			if (bFixAlign) {
				if ((i + 1) == biWidth) {
					curPos += align;
					printNewLine();
				}
			}
		}
	}
	///////// end Reading bytes /////////

	fin.close();
}
