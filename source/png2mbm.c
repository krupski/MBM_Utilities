/*
 * png2mbm.c - converts png images to Kerbal Space Program textures
 *
 * (c) 2013, 2014 roger a. krupski <rakrupski@verizon.net>
 * Last update 28 november 2014
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * This program uses the "lodepng" library written by Lode Vandevenne.
 * Please see "lodepng.c" and "lodepng.h" for license and copyright
 * information. The lodepng library URL is: <http://lodev.org/lodepng/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>

#define LODEPNG_NO_COMPILE_ENCODER
#define LODEPNG_NO_COMPILE_DISK
#define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
#define LODEPNG_NO_COMPILE_ERROR_TEXT

/*
 * lodepng image library
 * Copyright (c) 2005-2014 Lode Vandevenne
 * Website: http://lodev.org/lodepng
 */
#include "../lodepng/lodepng.c"

#define MAGIC 0x50534B03
#define IHDR 0x49484452

#define magic_ofs 0x00
#define width_ofs 0x04
#define height_ofs 0x08
#define type_ofs 0x0C
#define bits_ofs 0x10
#define image_ofs 0x14
#define png_ihdr 0x0C
#define png_width 0x10
#define png_height 0x14
#define png_bitdepth 0x18
#define png_compression 0x19

#define bufsz 8192

FILE *fp = NULL;

char *filename = NULL;
char *infile = NULL;
char *outfile = NULL;
char *buf = NULL;

unsigned char *buffer = NULL;
unsigned char *image = NULL;

uint32_t ihdr;
uint32_t imgsize;
uint32_t magic;
uint32_t io_size;
uint32_t width;
uint32_t height;
uint32_t type;
uint32_t bits;
uint32_t bytes;
uint32_t bpl;
uint32_t n;
uint32_t x;
uint32_t y;
uint32_t src;
uint32_t dst;
uint32_t erc;

size_t pngsize;

LodePNGState state;

int cleanup (int rc)
{
	const char *errmsg[] = {
		"",
		"malloc",
		"open for read",
		"open for write",
		"read image",
		"write image",
		"header check",
		"image type",
		"image convert",
	};

	if (fp != NULL) { fclose (fp); fp = NULL; }

	if (filename != NULL) { free (filename); filename = NULL; }

	if (infile != NULL) { free (infile); infile = NULL; }

	if (outfile != NULL) { free (outfile); outfile = NULL; }

	if (buf != NULL) { free (buf); buf = NULL; }

	if (buffer != NULL) { free (buffer); buffer = NULL; }

	if (image != NULL) { free (image); image = NULL; }

	if (rc) {
		fprintf (stderr, "\n%s failed\n", errmsg[rc]);
		fflush (stderr);

	} else {
		fprintf (stdout, "\n");
		fflush (stdout);
	}

	return rc;
}

uint32_t check_type (const unsigned char *buf, uint32_t bytes)
{
	uint32_t r, b, x, count, delta = 0;

	for (x = 0; x < bytes; x += 4) {

		r = * (buf + x + 0);
		b = * (buf + x + 2);

		if (r != b) {
			count++;
			delta += (r < b) ? (b - r) : (r - b);
		}
	}

	if (count) {
		delta /= count;
	}

	return (delta < 8) ? 1 : 0;
}

uint32_t big_endian (const unsigned char *buf, uint32_t cnt)
{
	uint32_t n;
	uint32_t result = 0;

	for (n = 0; n < cnt; n++) {
		result *= 0x0100;
		result += * (buf + n);
	}

	return result;
}

int readline (char *str, int limit, FILE *fp)
{
	int len;
	*str = 0;
	len = 0;

	if (fgets (str, limit, fp)) {
		len = strlen (str);
	}

	while (len--) {
		if (str[len] > 0x20) {
			len++;
			break;

		} else {
			str[len] = 0;
		}
	}

	len++;
	return len;
}

char *bname (char *str)
{
	int len = strlen (str);

	while (len--) {
		if (str[len] == '.') {
			str[len] = 0;
			break;
		}
	}

	return str;
}

int main (int argc, char *argv[])
{
	while (1) {
		infile = (char *) malloc (bufsz * sizeof (char));
		outfile = (char *) malloc (bufsz * sizeof (char));
		filename = (char *) malloc (bufsz * sizeof (char));
		buf = (char *) malloc (bufsz * sizeof (char));

		if (! (infile && outfile && filename && buf)) {
			cleanup (1);
			continue;
		}

		if (argc == 2) {
			sprintf (filename, "%s", argv[1]);

		} else {
			fprintf (stdout, "Filename ");

			if (! (readline (filename, bufsz, stdin))) {
				fprintf (stdout, "-> none, exiting");
				fflush (stdout);
				cleanup (0);
				break;

			} else {
				fprintf (stdout, "-> %s", filename);
				fflush (stdout);
			}
		}

		sprintf (infile, "%s", filename);
		sprintf (outfile, "%s.mbm", bname (filename));
		fp = fopen (infile, "rb");

		if (!fp) {
			cleanup (2);
			continue;
		}

		fseek (fp, 0, SEEK_END);
		pngsize = (ftell (fp) + 1);
		fseek (fp, 0, SEEK_SET);
		pngsize--;
		buffer = (unsigned char *) malloc (pngsize * sizeof (unsigned char));

		if (! buffer) {
			cleanup (1);
			continue;
		}

		io_size = fread (buffer, sizeof (char), pngsize, fp);
		fclose (fp);
		fp = NULL;

		if (io_size != pngsize) {
			cleanup (4);
			continue;
		}

		ihdr = (uint32_t) big_endian ((buffer + png_ihdr), sizeof (uint32_t));

		if (ihdr != IHDR) {
			cleanup (6);
			continue;
		}

		width = (uint32_t) big_endian ((buffer + png_width), sizeof (uint32_t));
		height = (uint32_t) big_endian ((buffer + png_height), sizeof (uint32_t));
		type = (uint8_t) big_endian ((buffer + png_compression), sizeof (uint8_t));

		if (! (type == LCT_RGB || type == LCT_RGBA)) {
			cleanup (8);
			continue;
		}

		if (type == LCT_RGB) {
			bits = 24;

		} else if (type == LCT_RGBA) {
			bits = 32;
		}

		bytes = (bits / 8);
		bpl = width * bytes;
		imgsize = (width * height * bytes);
		lodepng_state_init (&state);
		state.info_raw.colortype = type;
		state.info_png.color.colortype = type;
		erc = lodepng_decode (&image, &width, &height, &state, buffer, pngsize);
		lodepng_state_cleanup (&state);

		if (erc) {
			cleanup (9);
			continue;
		}

		free (buffer);
		buffer = NULL;

		if (bytes == 4) {
			type = check_type (image, imgsize);

		} else {
			type = 0;
		}

		buffer = (unsigned char *) malloc (imgsize * sizeof (unsigned char));

		if (! buffer) {
			fclose (fp);
			fp = NULL;
			cleanup (1);
			continue;
		}

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				src = (bytes * x) + (bpl * (height-y-1));
				dst = (bytes * width * y) + (bytes * x);

				for (n = 0; n < bytes; n++) {
					buffer[dst + n] = image[src + n];
				}
			}
		}

		free (image);
		image = NULL;
		fp = fopen (outfile, "wb");

		if (!fp) {
			cleanup (3);
			continue;
		}

		magic = MAGIC;
		fwrite (&magic, sizeof (uint32_t), 1, fp);
		fwrite (&width, sizeof (uint32_t), 1, fp);
		fwrite (&height, sizeof (uint32_t), 1, fp);
		fwrite (&type, sizeof (uint32_t), 1, fp);
		fwrite (&bits, sizeof (uint32_t), 1, fp);
		io_size = fwrite (buffer, sizeof (char), imgsize, fp);
		free (buffer);
		buffer = NULL;

		if (io_size != imgsize) {
			cleanup (5);
			continue;
		}

		fclose (fp);
		fp = NULL;

		cleanup (0);

		if (argc == 2) {
			break;
		}
	}

	return cleanup (0);
}
