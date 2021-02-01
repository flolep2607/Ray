#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "functions.h"
#include <iostream>
// #include <conio.h>
// #include <future>
// #include <thread>
// #include <chrono>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <linux/input.h>
// #include <iostream>
// #include "keyboard.h"
#include <ncurses.h>
#define PI 3.14159265358979323846
using namespace std;

//screen dimensions

#define WIDTH  1800
#define HEIGHT 900

#define BALLSNUMBER 5

//width and height of each character in pixels
const int dW = 1, dH = 2;

//set cursor at start to avoid flickering
void gotoxy(short x, short y)
{
	#ifdef _WIN32
		COORD coord = {x, y};
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
	#else
		printf("%c[%d;%df", 0x1B, y, x);
	#endif
}
char palette[] = " .\'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$█";

typedef struct Ball
{
	double center[3];
	double radius;
	double color; //on the interval from 0 (dark) to 1 (light)
	double coeff; //on the scale from 0 to 1 how much does the ball reflect light (1 means it's the perfect mirror)
} ball;

class camera
{
public:
	double x, y, z;
	double matrix[16], inv[16];
	camera(double r, double alfa, double beta)
	{
		//alfa is camera's angle along the xy plane.
		//beta is camera's angle along z axis
		//r is the distance from the camera to the origin
		double a = sin(alfa), b = cos(alfa), c = sin(beta), d = cos(beta);
		x = r * b * d;
		y = r * a * d;
		z = r * c;

		//matrix
		matrix[3] = matrix[7] = matrix[11] = 0;
		matrix[15] = 1;
		//x
		matrix[0] = -a;
		matrix[1] = b;
		matrix[2] = 0;
		//y
		matrix[4] = b * c;
		matrix[5] = a * c;
		matrix[6] = -d;
		//z
		matrix[8] = b * d;
		matrix[9] = a * d;
		matrix[10] = c;

		matrix[12] = x;
		matrix[13] = y;
		matrix[14] = z;

		//invert
		invert(inv, matrix);
	}
	double rayTrace(double origin[3], double unit[3], ball balls[], int n, double altitute, double coeff, int limit)
	{
		double color;
		double distanceToPlane = -(origin[2] + altitute) / unit[2]; //this is signed distance
		int index = -1;
		double distance;
		for (int i = 0; i < n; i++) // check chque boule si ca touvhe
		// ! attention ca prends d'en l'ordre et pas la dist
		{
			double diff[3];
			vector(diff, origin, balls[i].center);
			double discriminant = dot(unit, diff) * dot(unit, diff) + balls[i].radius * balls[i].radius - dot(diff, diff);
			if (discriminant < 0)
				continue;
			distance = -dot(unit, diff) - sqrt(discriminant);
			if (distance <= 0)
				continue;
			index = i;
			for (; i < n; i++){
				double diff[3];
				vector(diff, origin, balls[i].center);
				double discriminant = dot(unit, diff) * dot(unit, diff) + balls[i].radius * balls[i].radius - dot(diff, diff);
				if (discriminant < 0)
					continue;
				double distance2 = -dot(unit, diff) - sqrt(discriminant);
				if (distance2 <= 0)
					continue;
				if(distance2>distance){continue;}
				distance=distance2;
				index = i;
			}
			break;
		}
		if (index == -1)
		{
			if (unit[2] > 0)
			{
				return 0; //ray hit the sky
			}
			else //ray hit the groung
			{
				double
					tx = origin[0] + distanceToPlane * unit[0],
					ty = origin[1] + distanceToPlane * unit[1];
				double color = clamp(1 / (1 + distanceToPlane / 10), 0, 1);
				double origin2[3] =
					{
						origin[0] + distanceToPlane * unit[0],
						origin[1] + distanceToPlane * unit[1],
						origin[2] + distanceToPlane * unit[2]};
				double unit2[3] =
					{
						unit[0],
						unit[1],
						-unit[2]};
				if ((int)(floor(tx) + floor(ty)) % 2 == 0)
					return (1 - coeff) * color + coeff * rayTrace(origin2, unit2, balls, n, altitute, coeff, limit - 1);
				else
					return 0;
			}
		}

		if (unit[2] < 0 && distance > distanceToPlane) //ray hit the groung
		{
			double
				tx = origin[0] + distanceToPlane * unit[0],
				ty = origin[1] + distanceToPlane * unit[1];
			return (double)((int)(floor(tx) + floor(ty)) % 2);
		}
		//ray hit a ball
		double origin2[3] =
			{
				origin[0] + unit[0] * distance,
				origin[1] + unit[1] * distance,
				origin[2] + unit[2] * distance};
		double normal[3];
		vector(normal, origin2, balls[index].center);
		normalize(normal);
		double k = 2 * dot(unit, normal);
		scale(normal, k);
		double unit2[3];
		vector(unit2, unit, normal);
		if (limit = 0)
			return balls[index].color;
		return (1 - balls[index].coeff) * balls[index].color + balls[index].coeff * rayTrace(origin2, unit2, balls, n, altitute, coeff, limit - 1);
	}
};


int main()
{

	//ball declaration::
	ball balls[BALLSNUMBER];
	balls[0].center[0] = 0;
	balls[0].center[1] = 0;
	balls[0].center[2] = 0;
	balls[0].radius = 1;
	balls[0].color = 1;
	balls[0].coeff = 0.9;

	balls[1].center[0] = -3;
	balls[1].center[1] = 0;
	balls[1].center[2] = 0;
	balls[1].radius = 0.5;
	balls[1].color = 1;
	balls[1].coeff = 0.7;

	balls[2].center[0] = 0;
	balls[2].center[1] = -3;
	balls[2].center[2] = 0;
	balls[2].radius = 0.5;
	balls[2].color = 1;
	balls[2].coeff = 0.7;

	balls[3].center[0] = 1;
	balls[3].center[1] = 2;
	balls[3].center[2] = 1;
	balls[3].radius = 0.3;
	balls[3].color = 1;
	balls[3].coeff = 0.3;

	balls[4].center[0] = 2;
	balls[4].center[1] = 1;
	balls[4].center[2] = 2;
	balls[4].radius = 1.3;
	balls[4].color = 1;
	balls[4].coeff = 0.99;

	double alfa = 0, beta = PI / 2, r = 1.9;

	//starting screen
	for (int i = 0; i < HEIGHT / dH; i++)
	{
		for (int j = 0; j < WIDTH / dW; j++)
		{
			printf("@");
		}
		printf("\n");
	}
	gotoxy(0, 0);
	char frame[HEIGHT / dH][WIDTH / dW];
	for (int i = 0; i < HEIGHT / dH; i++)
	{
		for (int j = 0; j < WIDTH / dW; j++)
		{
			frame[i][j]=0;
		}
	}
	initscr();
	timeout(-1);
	while (1)
	{
		int c = getch(); 
		// printf ("%d %c\n", c, c); 
		switch(c)
		{
			case '+':
				// cout << "+ was pressed \n";
				r+=0.5;
				flushinp();
				break;
			case '-':
				// cout << "- was pressed \n";
				r-=0.5;
				flushinp();
				break;
			case 'A':
			case 'a':
				// cout << "UP was pressed \n";
				beta+=0.5/r*PI;
				flushinp();
				break;
			case 'B':
			case 'b':
				// cout << "Down was pressed \n";
				beta-=0.5/r*PI;
				flushinp();
				break;
			case 'D':
			case 'd':
			case KEY_LEFT:
				// cout << "Left was pressed \n";
				alfa+=0.5/r*PI;
				flushinp();
				break;
			case 'C':
			case 'c':
				// cout << "Right was pressed \n";
				alfa-=0.5/r*PI;
				flushinp();
				break;
			case 'Q':
			case 'q':
				endwin();
				exit(0);
				break;
		}

		camera cam(r, alfa, beta);
		for (int i = 0; i < HEIGHT / dH; i++)
		{
			for (int j = 0; j < WIDTH / dW; j++)
			{
				double origin[3] =
					{
						cam.x,
						cam.y,
						cam.z};

				double unit[3] =
					{
						-((j - WIDTH / dW / 2) + 0.5) / (WIDTH / dW / 2),
						((i - HEIGHT / dH / 2) + 0.5) / (WIDTH / dH / 2),
						-1};
				transformVector(unit, cam.matrix);
				unit[0] -= cam.x;
				unit[1] -= cam.y;
				unit[2] -= cam.z;
				normalize(unit);
				double luminance = cam.rayTrace(origin, unit, balls, BALLSNUMBER, 2, 0.3, 5);
				int color = (int)((strlen(palette) - 1) * luminance);
				frame[i][j] = palette[color];
			}
		}
		refresh();
		for (int i = 0; i < HEIGHT / dH; i++)
		{
			for (int j = 0; j < WIDTH / dW; j++)
			{
				mvaddch(i,j,frame[i][j]);
			}
		}
	}
}
