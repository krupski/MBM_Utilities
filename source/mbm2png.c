/*
 * mbm2png.c - converts Kerbal Space Program textures to png images
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

#define LODEPNG_NO_COMPILE_DECODER
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

#define magic_ofs 0x00
#define width_ofs 0x04
#define height_ofs 0x08
#define type_ofs 0x0C
#define bits_ofs 0x10
#define image_ofs 0x14

#define bufsz 8192

FILE *fp = NULL;

char *filename = NULL;
char *infile = NULL;
char *outfile = NULL;
char *buf = NULL;

unsigned char *buffer = NULL;
unsigned char *image = NULL;

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
		sprintf (outfile, "%s.png", bname (filename));
		fp = fopen (infile, "rb");

		if (!fp) {
			cleanup (2);
			continue;
		}

		fseek (fp, 0, SEEK_END);
		imgsize = (ftell (fp) + 1);
		fseek (fp, 0, SEEK_SET);
		imgsize--;
		buffer = (unsigned char *) malloc (imgsize * sizeof (unsigned char));
		image = (unsigned char *) malloc (imgsize * sizeof (unsigned char));

		if (! (buffer && image)) {
			cleanup (1);
			continue;
		}

		io_size = fread (buffer, sizeof (char), imgsize, fp);
		fclose (fp);
		fp = NULL;

		if (io_size != imgsize) {
			cleanup (4);
			continue;
		}

		magic = * ((uint32_t *) (buffer + magic_ofs));
		width = * ((uint32_t *) (buffer + width_ofs));
		height = * ((uint32_t *) (buffer + height_ofs));
		type = * ((uint32_t *) (buffer + type_ofs));
		bits = * ((uint32_t *) (buffer + bits_ofs));

		if (magic != MAGIC) {
			cleanup (6);
			continue;
		}

		if ((bits != 24) && (bits != 32)) {
			cleanup (7);
			continue;
		}

		bytes = (bits / 8);
		bpl = width * bytes;

		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				src = (bytes * x) + (bpl * (height-y-1)) + image_ofs;
				dst = (bytes * width * y) + (bytes * x);

				for (n = 0; n < bytes; n++) {
					image[dst + n] = buffer[src + n];
				}
			}
		}

		free (buffer);
		buffer = NULL;
		lodepng_state_init (&state);
		state.encoder.auto_convert = 0;

		if (bytes == 3) {
			state.info_raw.colortype = LCT_RGB;
			state.info_png.color.colortype = LCT_RGB;

		} else {
			state.info_raw.colortype = LCT_RGBA;
			state.info_png.color.colortype = LCT_RGBA;
		}

		erc = lodepng_encode (&buffer, &pngsize, image, width, height, &state);
		lodepng_state_cleanup (&state);

		free (image);
		image = NULL;

		if (erc) {
			cleanup (8);
			continue;
		}

		fp = fopen (outfile, "wb");

		if (!fp) {
			cleanup (3);
			continue;
		}

		io_size = fwrite (buffer, sizeof (char), pngsize, fp);
		free (buffer);
		buffer = NULL;

		if (io_size != pngsize) {
			cleanup (4);
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
