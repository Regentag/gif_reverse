#include <iostream>
#include "gif.h"

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "GIF 애니메이션을 거꾸로 뒤집습니다." << std::endl;
		std::cerr << "사용법 : reverse_gif.exe [원본.gif] [역재생.gif]" << std::endl;
		return -1;
	}

	try
	{
		auto file = gif::gif_file::load(argv[1]);
		std::cout << "GIF 파일 버전 : " << file->version_str() << std::endl;
		std::cout << "이미지 크기 (픽셀) : "
			<< file->header.width << "x" << file->header.height << std::endl;

		std::cout << "포함된 장면 : " << file->images.size() << std::endl;
		if (file->graphic_control_ext)
		{
			std::cout << "애니메이션 딜레이 : "
				<< file->graphic_control_ext->delay_time
				<< "/100 초" << std::endl;
		}

		file->reverse_save(argv[2]);
	}
	catch (std::exception &ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}
}
