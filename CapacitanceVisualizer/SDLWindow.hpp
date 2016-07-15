/*
 * SDLWindow.hpp
 *
 *  Created on: Jun 19, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_gfxPrimitives_font.h>

#include <iostream>

#include <assert.h>

class SDLWindow {
public:
	SDLWindow() :
		_should_call_sdl_quit(true),
		_isFullscreen(false),
		_smallSizeWidth(800),
		_smallSizeHeight(500 + _top_text_height),
		_w(_smallSizeWidth),
		_h(_smallSizeHeight),
		_screen(0)
	{
		if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
		{
			printf("Unable to init SDL: %s\n", SDL_GetError());
			exit(1);
		}

		const SDL_VideoInfo * nativeMode = SDL_GetVideoInfo();

		// TODO: Use this later on...
//		std::cout
//		<< "SDL Native resolution and mode suggestion: w*h="
//		<< nativeMode->current_w << "x" << nativeMode->current_h
//		<< ", bpp=" << int(nativeMode->vfmt->BitsPerPixel) << "\n";

		_fullscreenWidth = nativeMode->current_w;
		_fullscreenHeight = nativeMode->current_h;

		if (_smallSizeWidth > _fullscreenWidth) {
			_w = _smallSizeWidth = _fullscreenWidth;
		}
		if (_smallSizeHeight > _fullscreenHeight) {
			_h = _smallSizeHeight = _fullscreenHeight - 12; // 12 is just a guess of the height of the window managers title bar
		}

		_screen = SDL_SetVideoMode(_w, _h, 32, SDL_HWSURFACE/*|SDL_DOUBLEBUF*/);
		if ( _screen == NULL )
		{
			printf("Unable to set %dx%d video: %s\n", _w, _h, SDL_GetError());
			exit(1);
		}
		SDL_WM_SetCaption("CapacitanceVisualizer", "CapVisualizer");
		clear();
	}

	~SDLWindow()
	{
		if (_should_call_sdl_quit)
			SDL_Quit();
	}

	int getWidth() { return _w; }

	int getHeight() { return _h; }

	bool isFullscreen() { return _isFullscreen; }

	void setFullscreenMode(bool shouldGoFullscreen = true)
	{
		Uint32 flags = 0;
		if (shouldGoFullscreen)
		{
			_w = _fullscreenWidth;
			_h = _fullscreenHeight;
			flags = SDL_HWSURFACE | SDL_FULLSCREEN /*|SDL_DOUBLEBUF*/;
		}
		else
		{
			_w = _smallSizeWidth;
			_h = _smallSizeHeight;
			flags = SDL_HWSURFACE /*|SDL_DOUBLEBUF*/;
		}

//		printf("Requested: %dx%d\n", _w, _h);
		_screen = SDL_SetVideoMode(_w, _h, 32, flags);
		if ( _screen == NULL )
		{
			printf("Unable to set %dx%d video: %s\n", _w, _h, SDL_GetError());
			exit(1);
		}

		_isFullscreen = shouldGoFullscreen;
	}

	void blank()
	{
		clearInternalFramebuffer(_screen);
		SDL_Flip(_screen);
		clearInternalFramebuffer(_screen);
	}

	void clear()
	{
		clearInternalFramebuffer(_screen);
	}

	void drawTopText()
	{
		stringRGBA(
				_screen,
				0,
				0,
				"ESC = quit, F1 = sample background, +/- = change zoom, F11 = toggle fullscreen",
				255, 255, 255, 255);
	}

	void drawString(int x, int y, const char* s)
	{
		stringRGBA(_screen, x, y, s, 255, 255, 255, 255);
	}

	void drawBitmap(int w, int h, int scaling, uint8_t* data)
	{
		lock(_screen);

		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int val = data[y * w + x];

				for (int dy = 0; dy < scaling; dy++)
				{
					for (int dx = 0; dx < scaling; dx++)
					{
						safeDrawPixel(_screen,  x*scaling + dx, y*scaling + dy + 20, val, val, val);
					}
				}
			}
		}

		unlock(_screen);
	}

	void drawLine(Sint16 x1, Sint16 y1,
			Sint16 x2, Sint16 y2,
			Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		lineRGBA(_screen, x1, y1, x2, y2, r, g, b, a);
	}

	void flip()
	{
		SDL_Flip(_screen);
	}

private:
	bool _should_call_sdl_quit;
	bool _isFullscreen;
	int _smallSizeWidth;
	int _smallSizeHeight;
	int _w;
	int _h;
	int _fullscreenWidth;
	int _fullscreenHeight;
	enum { _top_text_height = 10 };
	SDL_Surface *_screen;

	void lock(SDL_Surface *screen)
	{
		if ( SDL_MUSTLOCK(screen) )
		{
			if ( SDL_LockSurface(screen) < 0 )
			{
				return;
			}
		}
	}

	void unlock(SDL_Surface *screen)
	{
		if ( SDL_MUSTLOCK(screen) )
		{
			SDL_UnlockSurface(screen);
		}
	}

	void safeDrawPixel(SDL_Surface *screen, int x, int y,
			Uint8 R, Uint8 G, Uint8 B)
	{
		if( x >= 0 && x < _w && y >= 0 && y < _h)
		{
			drawPixel(screen, x, y, R, G, B);
		}
	}

	void drawPixel(SDL_Surface *screen, int x, int y,
			Uint8 R, Uint8 G, Uint8 B)
	{
		Uint32 color = SDL_MapRGB(screen->format, R, G, B);
		switch (screen->format->BytesPerPixel)
		{
		case 1: // Assuming 8-bpp
		{
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
			*bufp = color;
		}
		break;
		case 2: // Probably 15-bpp or 16-bpp
		{
			Uint16 *bufp;
			bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
			*bufp = color;
		}
		break;
		case 3: // Slow 24-bpp mode, usually not used
		{
			Uint8 *bufp;
			bufp = (Uint8 *)screen->pixels + y*screen->pitch + x * 3;
			if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
			{
				bufp[0] = color;
				bufp[1] = color >> 8;
				bufp[2] = color >> 16;
			} else {
				bufp[2] = color;
				bufp[1] = color >> 8;
				bufp[0] = color >> 16;
			}
		}
		break;
		case 4: // Probably 32-bpp
		{
			Uint32 *bufp;
			bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
			*bufp = color;
		}
		break;
		}
	}


	/**
	 * Clears the internal frame buffer, but does NOT flip buffers
	 */
	void clearInternalFramebuffer(SDL_Surface *screen)
	{
		//lock(screen);
		memset(screen->pixels, 0, screen->pitch*_h);
		//unlock(screen);
	}
};
