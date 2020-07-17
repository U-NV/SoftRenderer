#include "tgaimage.h"
#include "myVector.h"
#include "myGL.h"
#include <vector>
#include <limits>
#include <cmath>
#include <iostream>


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

void printTGAColor(TGAColor color) {
	printf("r:%d g:%d b:%d\n", color.r, color.g, color.b);
}

const int width = 800;
const int height = 800;

int main(int argc, char** argv) {
	TGAImage frame(width, height, TGAImage::RGB);

	std::vector<Vector2i> vertexBuffer =
	{
		Vector2i(0, 0),
		Vector2i(0, 99),
		Vector2i(99, 0),
		Vector2i(99, 0),
		Vector2i(0, 99),
		Vector2i(99,99)
	};
	
	std::vector<TGAColor> colorBuffer =
	{
		TGAColor(255, 0, 0,255),
		TGAColor(0, 255, 0,255),
		TGAColor(0, 0, 255,255),

		TGAColor(0, 0, 255,255),
		TGAColor(0, 255, 0,255),
		TGAColor(255, 0, 0,255)
		
	};

	drawTriangle2D(vertexBuffer, colorBuffer, frame);

	//image.set(52, 41, red);
	frame.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	frame.write_tga_file("output.tga");
	return 0;
}

