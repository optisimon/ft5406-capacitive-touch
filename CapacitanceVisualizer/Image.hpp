/*
 * Image.hpp
 *
 *  Created on: Jul 15, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#pragma once

#include <vector>

struct Image {
	int w;
	int h;
	std::vector<int> data;

	Image(int w, int h) : w(w), h(h)
	{
		data.resize(w*h, 0);
	}

	/** Flip image along its diagonal*/
	Image getTransposed() const
	{
		Image i(h, w);
		i.data.clear();

		for (int x = 0; x < w; x++)
		{
			for (int y = 0; y < h; y++)
			{
				i.data.push_back(data[y * w + x]);
			}
		}
		return i;
	}
};
