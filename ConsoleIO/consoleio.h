#pragma once

/*
	Author: Alwyn Dippenaar (alwyn.j.dippenaar@gmail.com)
	Represents a standard and uniform way, to communicate with the console.
	And as a linked library, allows for easily changing the underlying mechanism
	it uses to communicat with the console.
*/

#ifndef _CONSOLEIO_H
#define _CONSOLEIO_H


#include <windowComponent_64.h>
#pragma comment(lib, "WindowComponent_64.lib")

#include <iostream>
#include <stdio.h>
#include <conio.h>

using namespace std;

//========================== Fill Characters =======================================================.
const char TOP_LEFT_CORNER		= 214;		//Top left corner.
const char HORIZON_LINE			= 205;		//Straight double horizontal line.
const char SINGLE_HOR_LINE		= 196;		//Straight single horizontal line.
const char VERT_LINE			= 186;		//Straight double vertical line.
const char SINGLE_VERT_LINE		= 179;		//Straight single vertical line.
const char TOP_RIGHT_CORNER		= 191;		//Top right corner.
const char BOTTOM_LEFT_CORNER	= 200;		//Bottom left corner.
const char BOTTOM_RIGHT_CORNER	= 190;		//Bottom right corner.
//========================== Fill Characters =======================================================.

///----------------------------------------------------------------------
/// Represents a point.
/// This structure will be used to represent location, and dimension
/// depending on the requirement.
///
///----------------------------------------------------------------------
struct Point2D
{
	int x;		//The x coordinate/ row dimension.
	int y;		//The y coordinate / column dimension.

	Point2D()	{ ZeroMemory(this, sizeof(Point2D)); }			//Constructor.
	~Point2D()	{ ZeroMemory(this, sizeof(Point2D)); }			//Destructor.
};



///----------------------------------------------------------------------
/// Represents a rectangle.
/// Used to specify any rectangular type object, or bounds
/// where location and dimension need to be supplied, for example
/// to draw a rectangle on the console UI.
///----------------------------------------------------------------------
struct AlRectangle
{
	Point2D	topleft;			//The top left corner.
	Point2D	dimension;			//The width, height dimensions.

	AlRectangle()	{ ZeroMemory(this, sizeof(AlRectangle)); }
	~AlRectangle()	{ ZeroMemory(this, sizeof(AlRectangle)); }

};



///====================================================================================
/// This class enables standard Console IO.
/// This class enables retrieving a console, creating a primary and secondary 
/// console buffer, using specified dimensions, or the dimensions of the 
/// current console window will be used if bufferSize 
/// (x/y, respectively) is not specified.
///
///====================================================================================
class DLLEXPORT AlConsoleIO
{
public:
	AlConsoleIO(AlCLogger* pLogger);				//Constructor.
	~AlConsoleIO();									//Destructor.

	HANDLE			hConsole;						//A handle to the console.
	HANDLE			hBackBuffer;					//The console back buffer will be written to, then  "flipped" when ready to eb presented.

	Point2D		consoleDimensions;					//The console dimensions. (x = rows, y = columns)
	Point2D		consoleCursorPos;					//The current console cursor position, updated with every call to retrieveConsole. (x = row, y = column)
	Point2D		consoleBufferSize;					//The size to make the console buffer. (bigger than the actual buffer, should cause window to scroll.)
	bool		bExtendedKey;						//Was an extended key pressed.
	char		keyPressed;							//The character pressed.

	void	clearScreen();								//Clears the console screen.
	void	initializeConsole();						//Gets a handle to the console, including it's dimensions, creates primary and back buffers.
	void	drawConsoleRect(AlRectangle rect, HANDLE pBuffer = NULL);			//Draws a rectangle on the console.
	void	conouts(int x, int y, char* text, HANDLE pBuffer=NULL);			//Draws the string, at the location.
	void	conoutc(int x, int y, char text, HANDLE pBuffer = NULL);			//Draws a single char, at the location.
	void	conouts(Point2D* iopos, char* text, HANDLE pBuffer = NULL);			//Draws the string, at the location, updates the position.
	void	conoutc(Point2D* iopos, char text, HANDLE pBuffer = NULL);			//Draws a single char, at the location, updates the cursor position.
	void	conoutfill(Point2D* iopos, char text, int width, HANDLE pBuffer = NULL);//Draws a single char for n width, at the location, updates the cursor position.

	int		checkConsoleInput(bool bWaitForInput = true);	//Checks if there is console input, and returns the ascii key press, else 0 for no key pressed.
															//If bWaitForInput is true, the function will block until keyboard input is received.
	void	resetConsoleInput();					//Resets user input.

	void	swapBuffers();							//Presents the back buffer, swops the active buffer to the new backbuffer.
	void	setConsoleCursor(Point2D pos);			//Sets the console cursor position.
	void	adjustCursor(Point2D *pos);				//ensures the cursor is in bounds.

private:
	AlCLogger*		logger;							//Logger for debugging.

	void	clearScreen(HANDLE buffer);					//Clears the console screen.
	void	debug(char* msg);						//Allows for debugging.
	void	zeroMem();								//Clears memory for this instance.
	char	tbuf[2048];								//Temporary character buffer.

};


#endif