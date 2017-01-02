/*---------------------------------------------------------------------
*
* Copyright © 2016  Minsi Chen
* E-mail: m.chen@derby.ac.uk
*
* The source is written for the Graphics I and II modules. You are free
* to use and extend the functionality. The code provided here is functional
* however the author does not guarantee its performance.
---------------------------------------------------------------------*/
#include <algorithm>
#include <math.h>

#include "Rasterizer.h"

Rasterizer::Rasterizer(void)
{
	mFramebuffer = NULL;
	mScanlineLUT = NULL;
}

void Rasterizer::ClearScanlineLUT()
{
	Scanline *pScanline = mScanlineLUT;

	for (int y = 0; y < mHeight; y++)
	{
		(pScanline + y)->clear();
		(pScanline + y)->shrink_to_fit();
	}
}

unsigned int Rasterizer::ComputeOutCode(const Vector2 & p, const ClipRect& clipRect)
{
	unsigned int CENTRE = 0x0;
	unsigned int LEFT = 0x1;
	unsigned int RIGHT = 0x1 << 1;
	unsigned int BOTTOM = 0x1 << 2;
	unsigned int TOP = 0x1 << 3;
	unsigned int outcode = CENTRE;
	
	if (p[0] < clipRect.left)
		outcode |= LEFT;
	else if (p[0] >= clipRect.right)
		outcode |= RIGHT;

	if (p[1] < clipRect.bottom)
		outcode |= BOTTOM;
	else if (p[1] >= clipRect.top)
		outcode |= TOP;

	return outcode;
}

bool Rasterizer::ClipLine(const Vertex2d & v1, const Vertex2d & v2, const ClipRect& clipRect, Vector2 & outP1, Vector2 & outP2)
{
	//TODO: EXTRA This is not directly prescribed as an assignment exercise. 
	//However, if you want to create an efficient and robust rasteriser, clipping is a usefull addition.
	//The following code is the starting point of the Cohen-Sutherland clipping algorithm.
	//If you complete its implementation, you can test it by calling prior to calling any DrawLine2D .

	const Vector2 p1 = v1.position;
	const Vector2 p2 = v2.position;
	

	outP1 = p1;
	outP2 = p2;
	
	bool draw = false;

	//Cohen-Sutherland Clipping
	while (true) 
	{
		unsigned int outcode1 = ComputeOutCode(p1, clipRect);
		unsigned int outcode2 = ComputeOutCode(p2, clipRect);

		int outcode = outcode1 ? outcode1 : outcode2;
		int x;
		int y;
			

		if (outcode1 || outcode2 == 0) 
		{
			return true;
		}
		else if (outcode1 && outcode2) 
		{
			return false;
		}

		if (outcode & 0x8) 
		{
			y = p1[1];
		}
		else if (outcode & 0x4) 
		{
			y = p2[1];
		}
		else if (outcode & 0x2) 
		{
			x = p2[0];
		}
		else if (outcode & 0x1) 
		{
			x = p1[0];
		}

		if (outcode == outcode1) 
		{
			outP1[0] = x;
			outP1[1] = y;
		}
		else 
		{
			outP2[0] = x;
			outP2[1] = y;
		}
	}

	return true;
}

void Rasterizer::WriteRGBAToFramebuffer(int x, int y, const Colour4 & colour)
{
	if (x >= 0 && x < mWidth - 1 && y >= 0 && y < mHeight - 1) 
	{
		PixelRGBA *pixel = mFramebuffer->GetBuffer();
		pixel[y*mWidth + x] = colour;
	}
	
}

Rasterizer::Rasterizer(int width, int height)
{
	//Initialise the rasterizer to its initial state
	mFramebuffer = new Framebuffer(width, height);
	mScanlineLUT = new Scanline[height];
	mWidth = width;
	mHeight = height;

	mBGColour.SetVector(0.0, 0.0, 0.0, 1.0);	//default bg colour is black
	mFGColour.SetVector(1.0, 1.0, 1.0, 1.0);    //default fg colour is white

	mGeometryMode = LINE;
	mFillMode = UNFILLED;
	mBlendMode = NO_BLEND;

	SetClipRectangle(0, mWidth, 0, mHeight);
}

Rasterizer::~Rasterizer()
{
	delete mFramebuffer;
	delete[] mScanlineLUT;
}

void Rasterizer::Clear(const Colour4& colour)
{
	PixelRGBA *pixel = mFramebuffer->GetBuffer();

	SetBGColour(colour);

	int size = mWidth*mHeight;
	
	for(int i = 0; i < size; i++)
	{
		//fill all pixels in the framebuffer with background colour
		*(pixel + i) = mBGColour;
	}
}

void Rasterizer::DrawPoint2D(const Vector2& pt, int size)
{
	int x = pt[0];
	int y = pt[1];
	
	WriteRGBAToFramebuffer(x, y, mFGColour);
}

void Rasterizer::DrawLine2D(const Vertex2d & v1, const Vertex2d & v2, int thickness)
{
	//The following code is basic Bresenham's line drawing algorithm.
	//The current implementation is only capable of rasterise a line in the first octant, where dy < dx and dy/dx >= 0
	//See if you want to read ahead https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
	
	//TODO:
	//Ex 1.1 Complete the implementation of Rasterizer::DrawLine2D method. 
	//This method currently consists of a partially implemented Bresenham algorithm.
	//You must extend its implementation so that it is capable of drawing 2D lines with arbitrary gradient(slope).
	//Use Test 1 (Press F1) to test your implementation
	
	//Ex 1.2 Extend the implementation of Rasterizer::DrawLine2D so that it is capable of drawing lines based on a given thickness.
	//Note: The thickness is passed as an argument int thickness.
	//Use Test 2 (Press F2) to test your implementation

	//Ex 1.3 Extend the implementation of Rasterizer::DrawLine2D so that it is capable of interpolating colour across a line when each end-point has different colours.
	//Note: The variable mFillMode indicates if the fill mode is set to INTERPOLATED_FILL. 
	//The colour of each point should be linearly interpolated using the colours of v1 and v2.
	//Use Test 2 (Press F2) to test your implementation

	Vector2 pt1 = v1.position;
	Vector2 pt2 = v2.position;

	//Colours for interpolated filling
	Colour4 colourX = v1.colour;
	Colour4 colourY = v2.colour;

	//Case 1: x1 > x2
	bool swap_vertices = pt1[0] > pt2[0];

	//Swaps vector positions as well as colour values
	if (swap_vertices) {
		pt1 = v2.position;
		pt2 = v1.position;
		colourX = v2.colour;
		colourY = v1.colour;
	}

	//Line Drawing variables
	int dx = pt2[0] - pt1[0];
	int dy = pt2[1] - pt1[1];

	int epsilon = 0;

	int x = pt1[0];
	int y = pt1[1];
	int ex = pt2[0];
	int ey = pt2[1];

	//bools for 3 fail cases
	bool negative_slope = pt2[1] - pt1[1] < 0;
	bool swap_xy = abs(dx) < abs(dy);
	bool reflect = false;

	if (swap_xy && negative_slope) {
		reflect = true;
	}

	//Case 3.5: Octants 3 and 7
	//Does both swap xy and negative slope
	if (!reflect)
	{
		if (swap_xy)
		{
			int tempdx = dx;
			int tempx = x;
			x = y;
			y = tempx;
			dx = dy;
			dy = tempdx;
			ex = pt2[1];
			ey = pt2[0];
		}
		if (negative_slope)
		{
			dy = -dy;
			y = -y;
		}
	}
	else 
	{
		//Case 2: negative slope
		//negate y and dy values
		if (negative_slope) 
		{
			dy = -dy;
			y = -y;
		}
		//Case 3: swap x and y
		//swap x and y values of first point and x and y value of second point as well as dx and dy values
		if (swap_xy) 
		{
			int tempdx = dx;
			int tempx = x;
			x = y;
			y = tempx;
			dx = dy;
			dy = tempdx;
			ex = -pt2[1];
			ey = pt2[0];
		}
	}

	//Interpolated colour object
	Colour4 newColour = v1.colour;

	while(x <= ex)
	{
		Vector2 temp(x,y);
		float length = 0;

		//if neg slope and swap xy are true
		if (reflect)
		{
			//draw octants 3 and 7
			int tempx = temp[0];
			int tempy = temp[1];
			temp[1] = x;
			temp[0] = tempy;			
			temp[1] = -temp[1];				
		}
		else 
			//draw other octants
		{
			//negative slope
			if (negative_slope) 
			{		
				temp[1] = -temp[1];				
			}
			//swap x and y
			if (swap_xy) 
			{
				int tempx = temp[0];
				int tempy = temp[1];
				temp[1] = tempx;
				temp[0] = y;
			}
		}

		//Interpolated colour - seperate If statements so colour positions are calculated properly.
		if (reflect) 
		{
			//Interpolated colours for lines in octant 3 and 7
			//Gets length by getting abs value of negative y from first point / y from second point
			length = abs(-pt1[1] - -temp[1]) / (-pt2[1] - -pt1[1]);
		}
		else 
		{
			if (swap_xy) 
			{
				//Interpolated colour for lines with swapped x and y values
				//Gets length by getting abs value of y from first point / y from second point
				length = abs(pt1[1] - temp[1]) / (pt2[1] - pt1[1]);
			}
			else 
			{
				//interpolated colour for lines with negative slope
				//Gets length by getting abs value of x from first point / x from second point
				length = abs(pt1[0] - temp[0]) / (pt2[0] - pt1[0]);
			}
		}


		//interpolated colour - add the colour change to each rgba value of colour.
		//Interpolated colour for lines with swapped vertices/no changes/all other changes
		newColour[0] = length*colourY[0] + (1 - length) * colourX[0];
		newColour[1] = length*colourY[1] + (1 - length) * colourX[1];
		newColour[2] = length*colourY[2] + (1 - length) * colourX[2];
		newColour[3] = length*colourY[3] + (1 - length) * colourX[3];
		SetFGColour(newColour);
		DrawPoint2D(temp);

		//Thickness Code
		int count = 1;		
		//as long as thickness value is greater than 1...
		for (int t = thickness; t > 1; t) 
		{
			//Draw a line above the existing line
			temp[1] += count;
			DrawPoint2D(temp);
			t--;
			count++;
			if (t) 
			{
				//Draw a line below the existing line
				temp[1] -= count;
				DrawPoint2D(temp);
			}
		}

		epsilon += dy;
		
		if ((epsilon << 1) >= dx)
		{
			y ++;
			epsilon -= dx;
		}
		x++;
	}		
}

	


void Rasterizer::DrawUnfilledPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.1 Implement the Rasterizer::DrawUnfilledPolygon2D method so that it is capable of drawing an unfilled polygon, i.e. only the edges of a polygon are rasterised. 
	//Please note, in order to complete this exercise, you must first complete Ex1.1 since DrawLine2D method is reusable here.
	//Note: The edges of a given polygon can be found by conntecting two adjacent vertices in the vertices array.
	//Use Test 3 (Press F3) to test your solution.

	int i = 0;
	Vertex2d v1;
	Vertex2d v2;
	ClipRect clip;
	Vector2 p1;
	Vector2 p2;

	//For each vertex in a polygon...
	while (i < count - 1) 
	{
		//Draw line and begin clipping of any overlapping lines
		ClipLine(vertices[i], vertices[i+1],clip ,p1, p2 );
		DrawLine2D(vertices[i], vertices[i + 1]);
		i++;
	}
	//Draw line which connects with original vertex and clip any lines
	ClipLine(vertices[i], vertices[i + 1], clip, p1, p2);
	DrawLine2D(vertices[count - 1], vertices[0]);

}

//Bool method to store the scanline LUT in X ascending order
bool compare(const ScanlineLUTItem& left, const ScanlineLUTItem& right) 
{
	return left.pos_x < right.pos_x;
}



void Rasterizer::ScanlineFillPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.2 Implement the Rasterizer::ScanlineFillPolygon2D method method so that it is capable of drawing a solidly filled polygon.
	//Note: You can implement floodfill for this exercise however scanline fill is considered a more efficient and robust solution.
	//		You should be able to reuse DrawUnfilledPolygon2D here.
	//
	//Use Test 4 (Press F4) to test your solution, this is a simple test case as all polygons are convex.
	//Use Test 5 (Press F5) to test your solution, this is a complex test case with one non-convex polygon.

	int height = mHeight;
	int width = mWidth;
	ScanlineLUTItem scanItem;

	int LUTSize_at_y;
	int scanY = 0;
	int scanX = 0;

	int i = 0;

	ClipRect clip;
	Vector2 p1;
	Vector2 p2;


	//ScanLine fill algorithm
	//for loop to fill the mScanlineLUT vector with intersecting points
	for (scanY = 0; scanY < height - 1; scanY++, mScanlineLUT[scanY].clear())
	{
		bool fill = true;
		bool running = false;
		//Last vertex in vertices array
		int j = count - 1;

		//For each vertex in a polygon...
		for (i = 0; i < count; i++)
		{
			p1 = vertices[j].position;
			p2 = vertices[i].position;

			//Detecting intersection of points and scanline
			if ((p1[1] >= scanY && scanY > p2[1]) || (p1[1] < scanY && scanY <= p2[1]))
			{
				//Set x coordinate and store in LUT
				scanX = p1[0] + ((scanY - p1[1]) * (p1[0] - p2[0]) / (p1[1] - p2[1]));
				scanItem = { vertices[i].colour, scanX };
				mScanlineLUT[scanY].push_back(scanItem);
				//sort in X ascending order
				std::sort(mScanlineLUT[scanY].begin(), mScanlineLUT[scanY].end(), compare);
				
			}

			LUTSize_at_y = mScanlineLUT[scanY].size();
			int pointCount = 0;
			//for loop to draw inside of polygons by using points from mScanlineLUT
			//as long as Size of LUT is greater than 1 or greater than 0 and running is true....
			for (int loop = LUTSize_at_y; loop > 1 || loop > 0 && running; loop--)
			{
				running = true;
				//Get position of first coordinate from LUT
				int pos_1 = mScanlineLUT[scanY][pointCount].pos_x;
				Colour4 colour1 = mScanlineLUT[scanY][pointCount].colour;
				loop--;
				//Get position of second coordinate from LUT
				int pos_2 = mScanlineLUT[scanY][pointCount + 1].pos_x;
				Colour4 colour2 = mScanlineLUT[scanY][pointCount + 1].colour;
				loop--;

				pointCount += 2;

				//If second coordinate has been claimed...
				if (running)
				{
					//Draw line between 2 coordinates
					Vertex2d start{ colour1, Vector2(pos_1,scanY) };
					Vertex2d end{ colour2, Vector2(pos_2,scanY) };
					DrawLine2D(start, end, 1);
					fill = false;
				}
				else
				{
					fill = true;
				}
			}
			//Reset number of vertices
			j = i;
		}
	}

	//for (std::vector<ScanlineLUTItem>::iterator it = mScanlineLUT[scanY].begin(); it != mScanlineLUT[scanY].end(); it++)


	//Ex 2.3 Extend Rasterizer::ScanlineFillPolygon2D method so that it is capable of alpha blending, i.e. draw translucent polygons.
	//Note: The variable mBlendMode indicates if the blend mode is set to alpha blending.
	//To do alpha blending during filling, the new colour of a point should be combined with the existing colour in the framebuffer using the alpha value.
	//Use Test 6 (Press F6) to test your solution

	
}

void Rasterizer::ScanlineInterpolatedFillPolygon2D(const Vertex2d * vertices, int count)
{
	//TODO:
	//Ex 2.4 Implement Rasterizer::ScanlineInterpolatedFillPolygon2D method so that it is capable of performing interpolated filling.
	//Note: mFillMode is set to INTERPOLATED_FILL
	//		This exercise will be more straightfoward if Ex 1.3 has been implemented in DrawLine2D
	//Use Test 7 to test your solution
	float length = 0;
	int i = 0;
	Colour4 newColour;

	Colour4 colourX = vertices[i].colour;
	Colour4 colourY = vertices[i + 1].colour;

	newColour[0] = length*colourY[0] + (1 - length) * colourX[0];
	newColour[1] = length*colourY[1] + (1 - length) * colourX[1];
	newColour[2] = length*colourY[2] + (1 - length) * colourX[2];
	newColour[3] = length*colourY[3] + (1 - length) * colourX[3];
	SetFGColour(newColour);






}


void Rasterizer::DrawCircle2D(const Circle2D & inCircle, bool filled)
{
	//TODO:
	//Ex 2.5 Implement Rasterizer::DrawCircle2D method so that it can draw a filled circle.
	//Note: For a simple solution, you can first attempt to draw an unfilled circle in the same way as drawing an unfilled polygon.
	//Use Test 8 to test your solution
	//Circle2D initialPoint = inCircle.centre;
	
	//Variables for circle drawing
	Vector2 origin = inCircle.centre;
	float radius = inCircle.radius;
	Colour4 colour = inCircle.colour;


	//variables for parametric equation
	const float PI = 3.14159265;
	
	int cX = origin[0];
	int cY = origin[1];
	int segment = 16;
	float theta = 0;
	float dt = 2 * PI / segment;
		
	//Circle Drawing - Parametric Equation - Unfilled only
/*	while(theta < 2*PI)
	{
		int x = radius*cos(theta) + cX;
		int y = radius*sin(theta) + cY;

		int x2 = radius*cos(theta - dt) + cX;
		int y2 = radius*sin(theta - dt) + cY;

		Vertex2d temp1;
		temp1.position[0] = x;
		temp1.position[1] = y;
		temp1.colour = colour;

		Vertex2d temp2;
		temp2.position[0] = x2;
		temp2.position[1] = y2;
		temp2.colour = colour;

		DrawLine2D(temp2,temp1);
		
		theta += dt;
	}*/

	
	//Circle Algorithm Midpoint circle algoritm - Includes filled
	int t = 0;
	int err = 0;
	SetFGColour(colour);

	while (radius >= t)
	{
		if (!filled)
		{
			//Draws unfilled circles with points rather than lines
			DrawPoint2D(Vector2(cX + radius, cY + t), 1);
			DrawPoint2D(Vector2(cX + t, cY + radius), 1);
			DrawPoint2D(Vector2(cX - t, cY + radius), 1);
			DrawPoint2D(Vector2(cX - radius, cY + t), 1);
			DrawPoint2D(Vector2(cX - radius, cY - t), 1);
			DrawPoint2D(Vector2(cX - t, cY - radius), 1);
			DrawPoint2D(Vector2(cX + t, cY - radius), 1);
			DrawPoint2D(Vector2(cX + radius, cY - t), 1);
		}
		else
		{
			//Same as normal midpoint algorithm except it stores and draws each respective line colour
			//and fills shapes with lines - similar to scanline except it draws from side to side
			Vertex2d point1, point2;

			Vector2 temp(cX + radius, cY + t);
			Vector2 temp2(cX - radius, cY + t);

			point1.colour = colour;
			point1.position = temp;

			point2.colour = colour;
			point2.position = temp2;
			DrawLine2D(point1, point2, 1);

			temp.SetVector(cX - radius, cY - t);
			temp2.SetVector(cX + radius, cY - t);

			point1.position = temp;
			point2.position = temp2;
			DrawLine2D(point1, point2, 1);

			temp.SetVector(cX + t, cY + radius);
			temp2.SetVector(cX - t, cY + radius);

			point1.position = temp;
			point2.position = temp2;
			DrawLine2D(point1, point2, 1);

			temp.SetVector(cX - t, cY - radius);
			temp2.SetVector(cX + t, cY - radius);

			point1.position = temp;
			point2.position = temp2;
			DrawLine2D(point1, point2, 1);
		}

		t += 1;
		err += 1 + 2 * t;
		if (2 * (err - radius) + 1 > 0)
		{
			radius -= 1;
			err += 1 - 2 * radius;
		}
	}
}

Framebuffer *Rasterizer::GetFrameBuffer() const
{
	return mFramebuffer;
}