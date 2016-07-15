/*
 * main.cpp
 *
 *  Created on: Jul 14, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */



#include <assert.h>

#include "FT5406.hpp"
#include "Image.hpp"
#include "SDLWindow.hpp"
#include "SDLEventHandler.hpp"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>

#include <fstream>
#include <limits>
#include <string.h>
#include <vector>


int verbose_flag = 0;
int i2c_adapter_nr = 1;


static bool quit(false);


void sdlDisplayLoop()
{
	FT5406 capTouch(1, 0x38, verbose_flag);
	capTouch.setTestMode();

	SDLWindow win;
	SDLEventHandler eventHandler;
	eventHandler.setZoom(16);

	Image background(FT5406::NUM_COLUMNS, FT5406::NUM_ROWS);
	background.data.resize(FT5406::NUM_COLUMNS * FT5406::NUM_ROWS, 0);

	while (!quit)
	{
		win.drawTopText();

		capTouch.refreshSensor();

		Image img = capTouch.readImage();

		if (eventHandler.shouldSampleBackground())
		{
			background = img;
			eventHandler.clearShouldSampleBackground();
		}

		for (int i = 0; i < FT5406::NUM_COLUMNS * FT5406::NUM_ROWS; i++)
		{
			img.data[i] -= background.data[i];
		}

		// Find min and max capacitance values (to scale intensity values later on)
		int signalMin = std::numeric_limits<int>::max();
		int signalMax = std::numeric_limits<int>::min();
		for (auto& val : img.data)
		{
			if (val > signalMax) { signalMax = val; }
			if (val < signalMin) { signalMin = val; }
		}

		// Do the actual scaling and drawing of the sensed capacitance values
		std::vector<uint8_t> scaled;
		Image corrected = img.getTransposed();
		for (auto& val : corrected.data)
		{
			scaled.push_back( static_cast<int>(255 * (val - signalMin) / static_cast<double>(signalMax - signalMin)) );
		}

		win.drawBitmap(corrected.w, corrected.h, eventHandler.getZoom(), &scaled[0]);

		win.flip();
		win.clear();

		eventHandler.refresh();
		if (eventHandler.shouldQuit())
		{
			quit = true;
		}

		// Should we go fullscreen?
		bool wantFullscreen = eventHandler.shouldGoFullscreen();
		bool isFullscreen = win.isFullscreen();
		if (wantFullscreen != isFullscreen)
		{
			win.setFullscreenMode(wantFullscreen);
		}
	}
}


int main (int argc, char *argv[])
{
  int showHelp_flag = 0;

  while (true)
  {
	  int c;
	  static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose", no_argument,   &verbose_flag, 1},
          {"brief",   no_argument,   &verbose_flag, 0},
          {"help",    no_argument,   &showHelp_flag, 1},
          /* These options don’t set a flag.
             We distinguish them by their indices. */
          //{"file",    required_argument, 0, 'f'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;
      c = getopt_long (argc, argv, "a:vbh",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("*option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'a':
        	i2c_adapter_nr = std::stoi(optarg);
        	break;

        case 'v':
          verbose_flag = 1;
          puts ("option -v\n");
          break;

        case 'b':
          verbose_flag = 0;
          puts ("option -b\n");
          break;

        case 'h':
		  showHelp_flag = 1;
		  printf ("option -h\n");
		  break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }
    
    if (showHelp_flag)
    {
		printf(
		"Tool visualizing raw capacitance values as measured by the Raspberry Pi 7 inch capacitive touch screen \n\n"
		"%s [options]\n"
		"-a i2c_adapter_number\n"
		"-v, --verbose \n"
		"-b, --brief\n"
		"-h, --help\n"
		"\n", argv[0]
		);
		return 1;
	}

	sdlDisplayLoop();
	return 0;
}
