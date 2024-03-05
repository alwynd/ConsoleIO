#include "consoleio.h"

///============================================ ALCONSOLE IO impl =================================================================================

//------------------------------------------------------------------------------------------------------------------------
/// Constructs a new instance of this.
///
/// param: pLogger A pointer to a logger, to log debug information.
///
//------------------------------------------------------------------------------------------------------------------------
AlConsoleIO::AlConsoleIO(AlCLogger* pLogger)
{
	zeroMem();
	logger = pLogger;
	debug("AlConsoleIO::AlConsoleIO:-- START\n");
}


AlConsoleIO::~AlConsoleIO()
{
	debug("AlConsoleIO::~AlConsoleIO:-- START\n");
	if (hConsole)
	{
		CloseHandle(hConsole);
	}
	if (hBackBuffer)
	{
		CloseHandle(hBackBuffer);
	}
	zeroMem();
}

//------------------------------------------------------------------------------------------------------------------------
/// This function fills the console with a specified character, for a specified width, to the console back buffer.
///
/// param: text  The character to fill with.
/// param: width The width to fill by (columns).
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::conoutfill(Point2D* iopos, char text, int width, HANDLE pBuffer)
{
	if (!pBuffer)
	{
		pBuffer = hBackBuffer;
	}

	ZeroMemory(tbuf, sizeof(char) * 2048);
	FillMemory(tbuf, width, text);
	conouts(iopos, tbuf, pBuffer);
}


//------------------------------------------------------------------------------------------------------------------------
/// This function draws a 3D rectanlge, that has a drop shadow, to the console back buffer.
///
/// param: rect The rectangle to draw, using the rect.topleft and rect.dimensions.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::drawConsoleRect(AlRectangle rect, HANDLE pBuffer)
{

	if (!pBuffer)
	{
		pBuffer = hBackBuffer;
	}

	Point2D cp;
	cp.x = rect.topleft.x;	
	cp.y = rect.topleft.y;	
	conoutc(&cp, TOP_LEFT_CORNER, pBuffer);
	conoutfill(&cp, SINGLE_HOR_LINE, rect.dimension.x - 2, pBuffer);
	sprintf(tbuf, "cp: (x,y): (%i, %i)\n\0", cp.x, cp.y);
	//logger->debug(tbuf);
	conoutc(&cp, TOP_RIGHT_CORNER, pBuffer);

	//-------------------- Sides --------------------
	for (int i = 1; i < rect.dimension.y - 1; i++)
	{
		conoutc(rect.topleft.x, rect.topleft.y + i, VERT_LINE, pBuffer);
		conoutc(rect.topleft.x + rect.dimension.x - 1, rect.topleft.y + i, SINGLE_VERT_LINE, pBuffer);
	}
	//-------------------- Bottom ------------------
	cp.x = rect.topleft.x;
	cp.y = rect.topleft.y + rect.dimension.y-1;

	conoutc(&cp, BOTTOM_LEFT_CORNER, pBuffer);
	conoutfill(&cp, HORIZON_LINE, rect.dimension.x - 2, pBuffer);
	conoutc(&cp, BOTTOM_RIGHT_CORNER, pBuffer);
}

//------------------------------------------------------------------------------------------------------------------------
/// Resets user input.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::resetConsoleInput()
{
	bExtendedKey = false;
	keyPressed = 0;

}


//------------------------------------------------------------------------------------------------------------------------
/// This function retrieves console input, and optionally, returns regardless of input available.
///
/// param:  bWaitForInput (optional: Default: true) If false, will return immeditately, 
///         regardless of input available or not.
/// return: 0 if no input available, else the input ascii key.
///
//------------------------------------------------------------------------------------------------------------------------
int	AlConsoleIO::checkConsoleInput(bool bWaitForInput)
{
	debug("AlConsoleIO::checkConsoleInput:-- START \n");

	bExtendedKey = false;
	keyPressed = 0;

	int result = 0;
	bool process = false;
	if (bWaitForInput || _kbhit())
	{
		result = _getch();
		process = true;
	}
	if (process)
	{
		keyPressed = result;
		if (result == 0 || result == 0xE0)										//Function code or arrow key
		{
			bExtendedKey = true;
			sprintf(tbuf, "\t FUNCTION/ARROW keypress: '%i' \n\0", result);
			debug(tbuf);
			result = _getch();
		}
		if (result > 0)
		{
			byte tmp = LOBYTE(VkKeyScanA(result));
			result = tmp;
		}
	}

	sprintf(tbuf, "\t keypress: '%i', Key: '%c', Extended: '%s'\n\0", result, keyPressed, (bExtendedKey ? "Yes" : "No"));
	debug(tbuf);

	return result;
}


//------------------------------------------------------------------------------------------------------------------------
/// Writes a character array from the current cursor position, to the console back buffer.
///
/// param: x	The x position to write from.
/// param: y	The y position to write from.
/// param: text The characters to write (strlen will be used to determine how many characters to write.)
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::conouts(int x, int y, char* text, HANDLE pBuffer)
{
	if (!pBuffer)
	{
		pBuffer = hBackBuffer;
	}

	COORD p;
	p.X = x;
	p.Y = y;
	DWORD outchars = 0;
	WriteConsoleOutputCharacter(pBuffer, text, strlen(text), p, &outchars);
}


//------------------------------------------------------------------------------------------------------------------------
/// Writes a series of characters to the console back buffer, at a specified position, and updates the iopos (x and y)
/// with the new console cursor position.
///
/// param: iopos	The x/y position to write from. (will be updated after the write, to indicate the new cursor x/y)
/// param: text		The characters to write (strlen will be used to determine how many characters to write.)
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::conouts(Point2D* iopos, char* text, HANDLE pBuffer)
{
	if (!pBuffer)
	{
		pBuffer = hBackBuffer;
	}

	COORD p;
	p.X = iopos->x;
	p.Y = iopos->y;
	DWORD outchars = 0;
	int lent = strlen(text);
	WriteConsoleOutputCharacter(pBuffer, text, lent, p, &outchars);
	
	//Advance the cursor 
	iopos->x += lent;
	while (iopos->x > consoleDimensions.x)
	{
		iopos->x -= consoleDimensions.x;
		iopos->y += 1;
	}
	
}

//------------------------------------------------------------------------------------------------------------------------
/// Writes a single character to the console back buffer, at a specified position.
///
/// param: x		The x position to write from.
/// param: y		The x position to write from.
/// param: text		The character to write.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::conoutc(int x, int y, char c, HANDLE pBuffer)
{
	if (!pBuffer)
	{
		pBuffer = hBackBuffer;
	}

	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	conouts(x, y, buf, pBuffer);
}

//------------------------------------------------------------------------------------------------------------------------
/// Ensures the cursor is in bounds, and not floating in nomans land.
/// param: pos The position to check from, and will adjust the position as required.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::adjustCursor(Point2D *pos)
{
	sprintf(tbuf, "adjustCursor:-- START pos:= (%i, %i) \n\0", pos->x, pos->y);
	debug(tbuf);

	char c[2];
	ZeroMemory(c, sizeof(char) * 2);
	DWORD read = 0;
	COORD coord = {pos->x, pos->y};

	if (!ReadConsoleOutputCharacterA(hBackBuffer, c, 1, coord, &read))
	{
		return;
	}
	sprintf(tbuf, "\t cursor char at: (%i, %i) is '%s' \n\0", pos->x, pos->y, c);
	debug(tbuf);
	while (c[0] == '\0' && pos->x > 0)
	{
		sprintf(tbuf, "\t cursor char at: (%i, %i) is '%s' \n\0", pos->x, pos->y, c);
		debug(tbuf);

		ZeroMemory(c, sizeof(char) * 2);
		read = 0;
		if (!ReadConsoleOutputCharacterA(hBackBuffer, c, 1, coord, &read))
		{
			return;
		}
		coord.X -= 1;
	}

	pos->x = coord.X;
	pos->y = coord.Y;

	sprintf(tbuf, "\t new cursor pos: (%i, %i) \n\0", pos->x, pos->y);
	debug(tbuf);

}

//------------------------------------------------------------------------------------------------------------------------
/// Writes a single character to the console back buffer, at a specified position, and updates the iopos (x and y)
/// with the new console cursor position.
///
/// param: iopos	The x/y position to write from. (will be updated after the write, to indicate the new cursor x/y)
/// param: text		The character to write.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::conoutc(Point2D* iopos, char c, HANDLE pBuffer)
{
	if (!pBuffer)
	{
		pBuffer = hBackBuffer;
	}

	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	conouts(iopos, buf, pBuffer);
}


//------------------------------------------------------------------------------------------------------------------------
/// Clears the back buffer of all current data, and repositions the cursor to the origin (row=0, col=0)
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::clearScreen()
{
	debug("AlConsoleIO::clearScreen:-- START\n");
	clearScreen(hBackBuffer);
}


//------------------------------------------------------------------------------------------------------------------------
/// Sets the console cursor position, on the current active and visible buffer.
/// param: pos The position to set the cursor at.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::setConsoleCursor(Point2D pos)
{
	debug("AlConsoleIO::setConsoleCursor:-- START\n");

	COORD coord = {pos.x, pos.y};
	SetConsoleCursorPosition(hConsole, coord);
}


//------------------------------------------------------------------------------------------------------------------------
/// Clears the specified buffer of all current data, and repositions the cursor to the origin (row=0, col=0)
///
/// param: buffer The buffer to clear, and reposition the cursor for.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::clearScreen(HANDLE buffer)
{
	COORD coord = {0, 0};
	DWORD c = 0;
	DWORD dwConSize = (consoleBufferSize.x > 0 ? consoleBufferSize.x : consoleDimensions.x) *
					  (consoleBufferSize.y > 0 ? consoleBufferSize.y : consoleDimensions.y);

	// Fill the entire screen with blanks.
	FillConsoleOutputCharacterA(buffer, ' ', dwConSize, coord, &c);
	SetConsoleCursorPosition(buffer, coord);
}


//------------------------------------------------------------------------------------------------------------------------
/// Initializes the buffers using current console dimensions, or bufferSize (x/y) is epcified.
///
//------------------------------------------------------------------------------------------------------------------------
void AlConsoleIO::initializeConsole()
{
	debug("AlConsoleIO::initializeConsole:-- START\n");

	//Zero current console dimensions.
	ZeroMemory(&consoleDimensions, sizeof(Point2D));
	ZeroMemory(&consoleCursorPos, sizeof(Point2D));

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	ZeroMemory(&csbi, sizeof(CONSOLE_SCREEN_BUFFER_INFO));
	if (!hConsole)
	{
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	consoleDimensions.x = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	consoleDimensions.y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	consoleCursorPos.x = csbi.dwCursorPosition.X;
	consoleCursorPos.y = csbi.dwCursorPosition.Y;

	if (!hBackBuffer)
	{
		if (hConsole)
		{
			CloseHandle(hConsole);
			hConsole = NULL;
		}

		COORD dim;
		dim.X = (consoleBufferSize.x > 0 ? consoleBufferSize.x : consoleDimensions.x);
		dim.Y = (consoleBufferSize.y > 0 ? consoleBufferSize.y : consoleDimensions.y); 

		sprintf(tbuf, "\t Created primary and back buffer: Dimensions(%i, %i) \n\0", dim.X, dim.Y);
		debug(tbuf);

		hBackBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		hConsole	= CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleScreenBufferSize(hBackBuffer, dim);
		SetConsoleScreenBufferSize(hConsole, dim);
	}

	ZeroMemory(tbuf, sizeof(char) * 2048);
	sprintf(tbuf, "\t console size: (%i, %i), Cursor Pos: (%i, %i)\n",
			consoleDimensions.x, consoleDimensions.y, consoleCursorPos.x, consoleCursorPos.y);
	debug(tbuf);

}

void AlConsoleIO::swapBuffers()
{
	//swop
	HANDLE swop = hConsole;
	hConsole = hBackBuffer;
	hBackBuffer = swop;

	//present
	SetConsoleActiveScreenBuffer(hConsole);
	

	//Clear back buffer
	clearScreen(hBackBuffer);
}

void AlConsoleIO::zeroMem()
{
	logger		= NULL;
	hConsole	= NULL;
	hBackBuffer = NULL;
	bExtendedKey = false;
	keyPressed	 = 0;

	ZeroMemory(&consoleDimensions, sizeof(Point2D));
	ZeroMemory(&consoleCursorPos, sizeof(Point2D));
	ZeroMemory(tbuf, sizeof(char) * 2048);
}

void AlConsoleIO::debug(char* msg)
{
	if (logger)
	{
		logger->debug(msg);
	}
}