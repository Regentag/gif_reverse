#include "gif.h"
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace gif;

data_sub_block::data_sub_block(const std::uint8_t size)
	: size(size)
{
	if (this->size)
	{
		this->data = new std::uint8_t[size];
	}
	else
	{
		this->data = nullptr;
	}
}

data_sub_block::~data_sub_block()
{
	if (this->size)
	{
		delete[] this->data;
	}
}

bool image::has_local_colour_table()
{
	return (descriptor.packed>>7) & 0x01;
}

gif_file::gif_file()
{
}


gif_file::~gif_file()
{
}

std::string gif_file::version_str()
{
	std::string v((char*)header.version, 3);
	return v;
}

bool gif_file::has_global_colour_table()
{
	return (header.packed>>7) & 0x01;
}

static void throw_e(std::ifstream& f, const char* msg)
{
	f.close();
	throw std::exception(msg);
}

std::shared_ptr<gif_file> gif_file::load(const std::string path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (file.bad())
	{
		throw std::exception("Can't open file.");
	}

	auto g = std::make_shared<gif_file>();

	// Read header
	file.read((char*)(&g->header), sizeof(gif::header));
	if (!file)
	{
		throw_e(file, "Can't read GIF header.");
	}

	// Check signature
	if (g->header.signature[0] != 'G' ||
		g->header.signature[1] != 'I' ||
		g->header.signature[2] != 'F')
	{
		throw_e(file, "Is not a GIF image.");
	}

	// Global Colour Table
	if (g->has_global_colour_table())
	{
		g->g_colour_table = std::make_shared< colour_table >();
		file.read((char*)(*g->g_colour_table), sizeof(colour_table));
		if (!file)
		{
			throw_e(file, "Can't read Global Colour Table.");
		}
	}

	std::uint8_t check = 0;
	std::uint8_t sub_block_sz = 0;

	while (true)
	{
		file.read((char*)&check, 1);
		if (!file)
		{
			throw_e(file, "Read error.");
		}

		if (check == gif::IMAGE_SEPARATOR)
		{
			// Read an image.
			auto img = std::make_shared<image>();

			file.read((char*)(&img->descriptor), sizeof(gif::image_descriptor));
			if (!file)
			{
				throw_e(file, "Can't read Image Descriptor.");
			}

			if (img->has_local_colour_table())
			{
				img->colour_table = std::make_shared< colour_table >();
				file.read((char*)(*img->colour_table), sizeof(colour_table));
				if (!file)
				{
					throw_e(file, "Can't read Local Colour Table.");
				}
			}

			file.read((char*)(&img->lzw_minimum), 1);
			if (!file)
			{
				throw_e(file, "Can't read LZW Minimum Code Size.");
			}

			while (true)
			{
				file.read((char*)(&sub_block_sz), 1);
				if (!file)
				{
					throw_e(file, "Can't read Data Sub-block size.");
				}

				if (sub_block_sz == 0)
				{
					break;
				}

				auto block = std::make_shared<data_sub_block>(sub_block_sz);
				file.read((char*)block->data, block->size);
				if (!file)
				{
					throw_e(file, "Can't read Data Sub-block.");
				}

				img->image_data.emplace_back(block);
			}

			g->images.emplace_back(img);
		}
		else if (check == gif::EXT_INTRODUCER)
		{
			// Read an Extension
			std::uint8_t label;
			file.read((char*)(&label), 1);
			if (!file)
			{
				throw_e(file, "Can't read Extension Label.");
			}

			if (label == gif::EXT_GRAPHIC_CONTROL)
			{
				g->graphic_control_ext
					= std::make_shared<gif::graphic_control_ext>();
				file.read((char*)(g->graphic_control_ext.get()), sizeof(gif::graphic_control_ext));
				if (!file)
				{
					throw_e(file, "Can't read Graphic Control Extension.");
				}
			}
			else
			{
				// Ignore extension block.
				while (true)
				{
					file.read((char*)(&sub_block_sz), 1);
					if (!file)
					{
						throw_e(file, "Can't read Data Sub-block size.");
					}

					if (sub_block_sz == 0)
					{
						break;
					}

					auto block = std::make_shared<data_sub_block>(sub_block_sz);
					file.read((char*)block->data, block->size);
					if (!file)
					{
						throw_e(file, "Can't read Data Sub-block.");
					}
				}
			}
		}
		else if (check == gif::TRAILER)
		{
			break;
		}
		else
		{
			std::stringstream s;
			s << "Unknown block type: 0x"
				<< std::hex << (int)check;

			throw_e(file, s.str().c_str());
		}
	}


	file.close();
	return g;
}

void gif_file::save(const std::string path)
{
	std::ofstream file(path, std::ios::out | std::ios::binary);
	if (file.bad())
	{
		throw std::exception("Can't open file for write.");
	}

	file.write((char*)&header, sizeof(gif::header));
	if (g_colour_table)
	{
		file.write((char*)g_colour_table.get(), sizeof(colour_table));
	}

	if (graphic_control_ext)
	{
		static const std::uint8_t GCE[] = { EXT_INTRODUCER, EXT_GRAPHIC_CONTROL };
		file.write((char*)GCE, 2);
		file.write((char*)graphic_control_ext.get(), sizeof(gif::graphic_control_ext));
	}

	std::for_each(images.begin(), images.end(), [&file](std::shared_ptr<image> img_p)
	{
		static const std::uint8_t IMG[] = { IMAGE_SEPARATOR };
		static const char ZERO[] = { 0x00 };
		file.write((char*)IMG, 1);
		file.write((char*)&img_p->descriptor, sizeof(gif::image_descriptor));
		if (img_p->colour_table)
		{
			file.write((char*)img_p->colour_table.get(), sizeof(colour_table));
		}

		file.write((char*)&img_p->lzw_minimum, 1);

		std::for_each(img_p->image_data.begin(), img_p->image_data.end(),
			[&file](std::shared_ptr<data_sub_block> dsb)
		{
			file.write((char*)&dsb->size, 1);
			file.write((char*)dsb->data, dsb->size);
		});

		file.write(ZERO, 1);
	});

	static const std::uint8_t END[] = { TRAILER };
	file.write((char*)END, 1);

	file.close();
}


void gif_file::reverse_save(const std::string path)
{
	std::ofstream file(path, std::ios::out | std::ios::binary);
	if (file.bad())
	{
		throw std::exception("Can't open file for write.");
	}

	file.write((char*)&header, sizeof(gif::header));
	if (g_colour_table)
	{
		file.write((char*)g_colour_table.get(), sizeof(colour_table));
	}

	if (graphic_control_ext)
	{
		static const std::uint8_t GCE[] = { EXT_INTRODUCER, EXT_GRAPHIC_CONTROL };
		file.write((char*)GCE, 2);
		file.write((char*)graphic_control_ext.get(), sizeof(gif::graphic_control_ext));
	}

	std::for_each(images.rbegin(), images.rend(), [&file](std::shared_ptr<image> img_p)
	{
		static const std::uint8_t IMG[] = { IMAGE_SEPARATOR };
		static const char ZERO[] = { 0x00 };
		file.write((char*)IMG, 1);
		file.write((char*)&img_p->descriptor, sizeof(gif::image_descriptor));
		if (img_p->colour_table)
		{
			file.write((char*)img_p->colour_table.get(), sizeof(colour_table));
		}

		file.write((char*)&img_p->lzw_minimum, 1);

		std::for_each(img_p->image_data.begin(), img_p->image_data.end(),
			[&file](std::shared_ptr<data_sub_block> dsb)
		{
			file.write((char*)&dsb->size, 1);
			file.write((char*)dsb->data, dsb->size);
		});

		file.write(ZERO, 1);
	});

	static const std::uint8_t END[] = { TRAILER };
	file.write((char*)END, 1);

	file.close();
}
