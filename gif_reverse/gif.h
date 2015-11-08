#ifndef GIFFILE_H
#define GIFFILE_H
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gif
{

	static const std::uint8_t IMAGE_SEPARATOR = 0x2C;
	static const std::uint8_t EXT_INTRODUCER = 0x21;
	static const std::uint8_t TRAILER = 0x3B;

	static const std::uint8_t EXT_GRAPHIC_CONTROL = 0xF9;
	static const std::uint8_t EXT_COMMENT = 0xFE;
	static const std::uint8_t EXT_PLAIN_TEXT = 0x01;
	static const std::uint8_t EXT_APPLICATION = 0xFF;

#pragma pack( push, gif, 1 )
	struct header
	{
		// HEADER
		std::uint8_t signature[3]; // "GIF"
		std::uint8_t version[3];   // "89a" or "87a"

		// LOGICAL SCREEN DESCRIPTOR
		std::uint16_t width;
		std::uint16_t height;
		std::uint8_t packed;
		std::uint8_t bg_colour_index;
		std::uint8_t pixel_aspect_ratio;
	};

	struct colour_t
	{
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
	};

	using  colour_table = colour_t[256];

	struct graphic_control_ext
	{
		std::uint8_t block_size; // Fixed value "4"
		std::uint8_t packed;
		std::uint16_t delay_time;
		std::uint8_t transparent_index;

		std::uint8_t terminator;
	};

	struct image_descriptor
	{
		std::uint16_t left_pos;
		std::uint16_t top_pos;
		std::uint16_t width;
		std::uint16_t height;
		std::uint8_t packed;
	};

#pragma pack(pop, gif)

	class data_sub_block
	{
	public:
		data_sub_block(const std::uint8_t size);
		~data_sub_block();

		std::uint8_t size;
		std::uint8_t *data;
	};

	class image
		// GIF Image Frame
	{
	public:
		bool has_local_colour_table();

		// Image Descriptor
		image_descriptor descriptor;

		// Local colour table (optional)
		std::shared_ptr< colour_table > colour_table;

		// LZW Minimum Code Size
		std::uint8_t lzw_minimum;

		// Image Data
		std::vector<std::shared_ptr<data_sub_block>> image_data;
	};

	class gif_file
		// GIF Image File
	{
	public:
		gif_file();
		~gif_file();

		// Load from...
		static std::shared_ptr<gif_file> load(const std::string path);

		// GIF version string.
		std::string version_str();

		// Global colour table.
		bool has_global_colour_table();

		// Save as...
		void save(const std::string path);
		void reverse_save(const std::string path);

		// GIF Header
		header header;

		// null-able fields
		std::shared_ptr< colour_table > g_colour_table;
		std::shared_ptr< graphic_control_ext > graphic_control_ext;

		// Image list.
		std::vector< std::shared_ptr< image > > images;
	};
}


#endif
