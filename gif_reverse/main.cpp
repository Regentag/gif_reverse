#include <iostream>
#include "gif.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "GIF �ִϸ��̼��� �Ųٷ� �������ϴ�." << std::endl;
		std::cerr << "���� : reverse_gif.exe [����.gif] [�����.gif]" << std::endl;
		return -1;
	}

	try
	{
		auto file = gif::gif_file::load(argv[1]);
		std::cout << "GIF ���� ���� : " << file->version_str() << std::endl;
		std::cout << "�̹��� ũ�� (�ȼ�) : "
			<< file->header.width << "x" << file->header.height << std::endl;

		std::cout << "���Ե� ��� : " << file->images.size() << std::endl;
		if (file->graphic_control_ext)
		{
			std::cout << "�ִϸ��̼� ������ : "
				<< file->graphic_control_ext->delay_time
				<< "/100 ��" << std::endl;
		}

		file->reverse_save(argv[2]);
	}
	catch (std::exception &ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}
}
