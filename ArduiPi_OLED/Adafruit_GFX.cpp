/******************************************************************
 This is the core graphics library for all our displays, providing
 basic graphics primitives (points, lines, circles, etc.). It needs
 to be paired with a hardware-specific library for each display
 device we carry (handling the lower-level functions).
 
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!
 
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.
 
02/18/2013 	Charles-Henri Hallard (http://hallard.me)
						Modified for compiling and use on Raspberry ArduiPi Board
						LCD size and connection are now passed as arguments on 
						the command line (no more #define on compilation needed)
						ArduiPi project documentation http://hallard.me/arduipi
07/01/2013 	Charles-Henri Hallard (http://hallard.me)
						Created Draw Bargraph feature
						Added printf feature

 ******************************************************************/

#include "./ArduiPi_OLED_lib.h"
#include "./Adafruit_GFX.h"
#include "./glcdfont.c"

void Adafruit_GFX::constructor(int16_t w, int16_t h) 
{
  _width = WIDTH = w;
  _height = HEIGHT = h;

  rotation = 0;    
  cursor_y = cursor_x = 0;
  textsize = 1;
  textcolor = textbgcolor = 0xFFFF;
  wrap = true;
}

// the printf function
void Adafruit_GFX::printf( const char * format, ...) 
{

  char buffer[64];
	char * p = buffer;
	int n;
  va_list args;
  va_start (args, format);
  vsnprintf (buffer, sizeof(buffer)-1, format, args);
	n = strlen(buffer);
		
	while (*p != 0 && n-->0)
	{
		write ( (uint8_t) *p++);
	}

  va_end (args);
}

// the print function
void Adafruit_GFX::print( const char * string) 
{

	const char * p = string;
	int n = strlen(string);
	
	while (*p != 0 && n-->0)
	{
		write ( (uint8_t) *p++);
	}

}

// bresenham's algorithm - thx wikpedia
void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) 
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  
	if (steep) 
	{
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) 
	{
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) 
	{
    ystep = 1;
  }
	else 
	{
    ystep = -1;
  }

  for (; x0<=x1; x0++) 
	{
    if (steep) 
		{
      drawPixel(y0, x0, color);
    }
		else 
		{
      drawPixel(x0, y0, color);
		}
    err -= dy;
    
		if (err < 0) 
		{
      y0 += ystep;
      err += dx;
    }
  }
}


void Adafruit_GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
  // stupidest version - update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}


void Adafruit_GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
  // stupidest version - update in subclasses if desired!
  for (int16_t i=x; i<x+w; i++) 
	{
    drawFastVLine(i, y, h, color); 
  }
}

void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) 
{

  int16_t i, j, byteWidth = (w + 7) / 8;

  for(j=0; j<h; j++) 
	{
    for(i=0; i<w; i++ ) 
		{
      if( *(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) 
			{
				drawPixel(x+i, y+j, color);
      }
    }
  }
}


size_t Adafruit_GFX::write(uint8_t c) 
{
  if (c == '\n') 
	{
    cursor_y += textsize*8;
    cursor_x = 0;
  } 
	else if (c == '\r') 
	{
    // skip em
  } 
	else 
	{
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*6;
    
		if (wrap && (cursor_x > (_width - textsize*6))) 
		{
      cursor_y += textsize*8;
      cursor_x = 0;
    }
  }
  return 1;
}

// draw a character
void Adafruit_GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) 
{

  if((x >= _width)            || // Clip right
     (y >= _height)           || // Clip bottom
     ((x + 5 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

  for (int8_t i=0; i<6; i++ ) 
	{
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      //line = pgm_read_byte(font+(c*5)+i);
      line = font[(c*5)+i];
    for (int8_t j = 0; j<8; j++) 
		{
      if (line & 0x1) 
			{
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else 
				{  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
				} 
      } 
			else if (bg != color) 
			{
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else 
				{  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        } 	
      }
			
      line >>= 1;
    }
  }
}

void Adafruit_GFX::setCursor(int16_t x, int16_t y) 
{
  cursor_x = x;
  cursor_y = y;
}


void Adafruit_GFX::setTextSize(uint8_t s) 
{
  textsize = (s > 0) ? s : 1;
}


void Adafruit_GFX::setTextColor(uint16_t c) 
{
  textcolor = c;
  textbgcolor = c; 
  // for 'transparent' background, we'll set the bg 
  // to the same as fg instead of using a flag
}

void Adafruit_GFX::setTextColor(uint16_t c, uint16_t b) 
{
   textcolor = c;
   textbgcolor = b; 
 }

void Adafruit_GFX::setTextWrap(boolean w) 
{
  wrap = w;
}

void Adafruit_GFX::invertDisplay(boolean i) 
{
  // do nothing, can be subclassed
}


// return the size of the display which depends on the rotation!
int16_t Adafruit_GFX::width(void) 
{ 
  return _width; 
}
 
int16_t Adafruit_GFX::height(void) 
{ 
  return _height; 
}
