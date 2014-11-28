/*
 * tga2mbm.c - converts TGA files to Kerbal Space Program textures
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
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <ctype.h>

#define MAGIC 0x50534B03

// mbm header offsets
#define magic_ofs 0x00
#define width_ofs 0x04
#define height_ofs 0x08
#define type_ofs 0x0C
#define bits_ofs 0x10
#define mbm_ofs 0x14

// tga header offsets
#define IDLength 0x00
#define ColorMapType 0x01
#define ImageType 0x02
#define CMapStart 0x03
#define CMapLength 0x05
#define CMapDepth 0x07
#define XOffset 0x08
#define YOffset 0x0A
#define Width 0x0C
#define Height 0x0E
#define PixelDepth 0x10
#define ImageDescriptor 0x11
#define tga_ofs 0x12

#define bufsz 8192

FILE *fp = NULL;

char *filename = NULL;
char *infile = NULL;
char *outfile = NULL;

unsigned char *inbuf = NULL;
unsigned char *inptr = NULL;
unsigned char *outbuf = NULL;
unsigned char *outptr = NULL;

uint32_t infilesize;
uint32_t outfilesize;
uint32_t bitmapsize;
uint32_t io_size;
uint32_t width;
uint32_t height;
uint32_t type;
uint32_t imgtype;
uint32_t bits;
uint32_t bytes;
uint32_t n;

int cleanup (int rc)
{
	const char *errmsg[] = {
		"",
		"malloc failed",
		"open for read failed",
		"open for write failed",
		"read image failed",
		"write image failed",
		"compressed tga not supported",
	};

	if (fp != NULL) { fclose (fp); fp = NULL; }

	if (filename != NULL) { free (filename); filename = NULL; }

	if (infile != NULL) { free (infile); infile = NULL; }

	if (outfile != NULL) { free (outfile); outfile = NULL; }

	if (inbuf != NULL) { free (inbuf); inbuf = NULL; }

	if (outbuf != NULL) { free (outbuf); outbuf = NULL; }

	if (rc) {
		fprintf (stderr, " ERROR: %s\n", errmsg[rc]);
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

		if (! (infile && outfile && filename)) {
			cleanup (1);
			continue;
		}

		if (argc == 2) {
			sprintf (filename, "%s", argv[1]);

		} else {
			fprintf (stdout, "Filename: ");

			if (! (readline (filename, bufsz, stdin))) {
				fprintf (stdout, "none, exiting");
				fflush (stdout);
				cleanup (0);
				break;

			} else {
				fprintf (stdout, "%s", filename);
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

		infilesize = tga_ofs; // read header only

		inbuf = (unsigned char *) malloc (infilesize * sizeof (unsigned char));

		io_size = fread (inbuf, sizeof (char), infilesize, fp);

		fclose (fp);
		fp = NULL;

		if (io_size != infilesize) {
			cleanup (4);
			continue;
		}

		imgtype = * (uint8_t *) (inbuf + ImageType);
		width = * (uint16_t *) (inbuf + Width);
		height = * (uint16_t *) (inbuf + Height);
		bits = * (uint8_t *) (inbuf + PixelDepth);
		bytes = (bits / 8);

		if (((bits != 24) && (bits != 32)) || (imgtype != 2)) {
			cleanup (6);
			continue;
		}

		free (inbuf);
		inbuf = NULL;

		infilesize = ((width * height * bytes) + tga_ofs);
		outfilesize = ((width * height * bytes) + mbm_ofs);

		inbuf = (unsigned char *) malloc (infilesize * sizeof (unsigned char));
		outbuf = (unsigned char *) malloc (outfilesize * sizeof (unsigned char));

		if (! (inbuf && outbuf)) {
			cleanup (1);
			continue;
		}

		fp = fopen (infile, "rb");

		if (!fp) {
			cleanup (2);
			continue;
		}

		io_size = fread (inbuf, sizeof (char), infilesize, fp);

		fclose (fp);
		fp = NULL;

		if (io_size != infilesize) {
			cleanup (4);
			continue;
		}

		inptr = (inbuf + tga_ofs);
		outptr = (outbuf + mbm_ofs);
		bitmapsize = (infilesize - tga_ofs);

		if (bytes == 4) {
			type = check_type (inptr, bitmapsize);

		} else {
			type = 0;
		}

		* ((uint32_t *) (outbuf + magic_ofs)) = MAGIC;
		* ((uint32_t *) (outbuf + width_ofs)) = width;
		* ((uint32_t *) (outbuf + height_ofs)) = height;
		* ((uint32_t *) (outbuf + type_ofs)) = type;
		* ((uint32_t *) (outbuf + bits_ofs)) = bits;

		for (n = 0; n < bitmapsize; n += bytes) {
			outptr[n + 0] = inptr[n + 2];
			outptr[n + 1] = inptr[n + 1];
			outptr[n + 2] = inptr[n + 0];

			if (bytes == 4) {
				outptr[n + 3] = inptr[n + 3];
			}
		}

		free (inbuf);
		inbuf = NULL;

		fp = fopen (outfile, "wb");

		if (!fp) {
			cleanup (3);
			continue;
		}

		io_size = fwrite (outbuf, sizeof (char), outfilesize, fp);

		fclose (fp);
		fp = NULL;

		free (outbuf);
		outbuf = NULL;

		if (io_size != outfilesize) {
			cleanup (5);
			continue;
		}

		cleanup (0);

		if (argc == 2) {
			break;
		}
	}

	return cleanup (0);
}
