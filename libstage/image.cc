#include "image.hh"
#include <png.h>
#include <cassert>
#include <stdlib.h>

namespace Stg{

void Image::Initialize()
{

}

Image::Image()
{
	width = 0;
	height = 0;
	bpp = 0;
}

int Image::getWidth() const
{
	return width;
}

int Image::getHeight() const
{
	return height;
}

int Image::getDepth() const
{
	return bpp;
}

uint8_t * Image::getData()
{
	if(data.empty())
		return NULL;
	return &data[0];
}

bool Image::load(const char * file_name)
{
	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");

	bool done = false;
	if (!fp)
	{
		printf("[read_png_file] File %s could not be opened for reading\n", file_name);
		return false;
	}

	do
	{
		unsigned char header[8];    // 8 is the maximum size that can be checked
		fread(header, 1, 8, fp);
		if (png_sig_cmp(header, 0, 8))
		{
			printf("[read_png_file] File %s is not recognized as a PNG file\n", file_name);
			break;
		}

		/* initialize stuff */
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

		if (!png_ptr)
		{
			printf("[read_png_file] png_create_read_struct failed\n");
			break;
		}

		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr)
		{
			printf("[read_png_file] png_create_info_struct failed\n");
			break;
		}

		//if (setjmp(png_jmpbuf(png_ptr)))
		//		abort_("[read_png_file] Error during init_io");

		png_init_io(png_ptr, fp);
		png_set_sig_bytes(png_ptr, 8);

		png_read_info(png_ptr, info_ptr);

		;

		width = png_get_image_width(png_ptr, info_ptr);
		height = png_get_image_height(png_ptr, info_ptr);
		png_byte color_type = png_get_color_type(png_ptr, info_ptr);
		png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		//number_of_passes = png_set_interlace_handling(png_ptr);
		png_read_update_info(png_ptr, info_ptr);

		if(bit_depth != 8)
		{
			printf("We support channels of 8 bit\n");
			break;
		}

		switch(color_type)
		{
		case PNG_COLOR_TYPE_GRAY:
			bpp = bit_depth / 8;
			break;
		case PNG_COLOR_TYPE_RGB:
			bpp = 3 * bit_depth / 8;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			bpp = 4 * bit_depth / 8;
			break;
		default:
			bpp = 0;
			printf("Unsupported color type: %d\n", color_type);
			break;
		}

		//bpp = bit_depth / 8;
		/* read file */
		if (setjmp(png_jmpbuf(png_ptr)))
		{
			printf("[read_png_file] Error during read_image");
			break;
		}

		int row_length = png_get_rowbytes(png_ptr, info_ptr);
		std::vector<uint8_t> row_data(row_length);

		this->data.resize(width*height*bpp);
		for (int y=0; y<height; y++)
		{
			//row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
			png_bytep row_pointer = &row_data[0];
			png_read_row(png_ptr, row_pointer, NULL);
			uint8_t * dst = &this->data[y*width*bpp];
			memcpy(dst, row_pointer, width*bpp);
		}

		//png_read_image(png_ptr, row_pointers);
		done = true;
	}while(false);

	fclose(fp);

	return done;
}

}
