/*
 * FT5406.hpp
 *
 *  Created on: Jul 15, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */


#pragma once

#include "Image.hpp"

#include <stdexcept>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>


class FT5406 {
public:
	enum { NUM_ROWS = 22 };
	enum { NUM_COLUMNS = 12 };

	FT5406(int adapter_nr, int addr = 0x38, bool verbose = false) : _fd(0), _testMode(false), _verbose(verbose)
    {
		char fname[20];
		snprintf(fname, sizeof(fname)-1, "/dev/i2c-%d", adapter_nr);

		_fd = open(fname, O_RDWR);

		if (_fd < 0)
		{
			throw std::runtime_error("Unable to open requested I2C device");
		}

		if (ioctl(_fd, I2C_SLAVE, addr) < 0)
		{
			throw std::runtime_error("Unable to select i2c slave address");
		}
    }


	void setTestMode()
	{
		int status = i2c_smbus_write_byte_data(_fd, 0x00, 0x40);

		if (status < 0)
		{
			throw std::runtime_error("i2c write failed");
		}
		_testMode = true;
	}


	void refreshSensor()
	{
		// We require testMode to be set
		ensureTestMode();

		// Start new scan of touch screen
		startNewSensorScan();

		// Wait for scan to complete
		waitForScanCompletion();
	}

	Image readImage()
	{
		assert(_testMode);

		int rows = NUM_ROWS;
		int cols = NUM_COLUMNS;

		Image retval(cols, rows);
		retval.data.clear();

		for (int rowAddr = 0; rowAddr < rows; rowAddr++)
		{
			int status = i2c_smbus_write_byte_data(_fd, 0x01, rowAddr);
			if (status < 0)
			{
				throw std::runtime_error("i2c write failed");
			}

			usleep(100); // Wait at least 100us

#if 0
			// sample this row
			for (int col = 0; col < cols; col++)
			{
				int val = i2c_smbus_read_word_data(_fd, 16 + (2 * col));
				if (val == -1)
				{
					throw std::runtime_error("i2c read error");
				}
				val = ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
				retval.data.push_back(val);
			}
#else
			// TODO: This won't work for kernel < 2.6.23. Should I care?
			uint8_t values[2*cols];
			status = i2c_smbus_read_i2c_block_data(_fd, 16, 2*cols, values);
			if (status != 2*cols)
			{
				throw std::runtime_error("i2c read error");
			}

			for (int col = 0; col < cols; col++)
			{
				int val = (values[2 * col] << 8) | (values[2 * col + 1]);
				retval.data.push_back(val);

				if (_verbose)
				{
					printf("0x%04x ", val);
				}
			}
#endif
			if (_verbose)
			{
				printf("\n");
			}
		}
		if (_verbose)
		{
			printf("\n");
		}
		return retval;
	}

private:
	void ensureTestMode()
	{
		if (!_testMode)
		{
			setTestMode();
		}
	}

	/** Assumes we already are in test mode */
	void startNewSensorScan()
	{
		int ret = i2c_smbus_write_byte_data(_fd, 0x00, 0xc0);
		if (ret < 0)
		{
			throw std::runtime_error("i2c write failed");
		}
	}

	/** Blocks until sensor scan is completed */
	void waitForScanCompletion()
	{
		while (true)
		{
			int ret = i2c_smbus_read_byte_data(_fd, 0x00);

			if (ret < 0)
			{
				throw std::runtime_error("i2c read failed");
			}
			else if (ret == 0x40)
			{
				return;
			}
			else
			{
				if (_verbose)
				{
					printf("Got 0x%02x - waiting longer\n", ret);
				}
				usleep(3000); // without this, we usually wait something like 17 full i2c transactions.
			}
		}
	}

	int _fd;
	bool _testMode;
	const bool _verbose;
};
