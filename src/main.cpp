#include "core.h"

#include <yy981/string.h>


int main(int argc, char* argv[]) {
	std::vector<std::string> carg = st::charV(argc,argv);
	if (carg.size() < 4) {
		std::cout << "Usage: \n"
				  << "DeltaMask v1 (c) 2026 yy981\n"
				  << "  DM enc  1 input_A.png example.dat\n"
				  << "  DM dec  1 input_A.png input_B.png\n"
				  << "  DM info 1 input_A.png\n"
				  << "          ^ -- bit mode {1,2} \n";
		return 0;
	}
	Mode mode = static_cast<Mode>(std::stoi(carg[2]));
	if (is_or(carg[1], "e", "enc")) {
		// enc mode
		std::string output = fs::path(carg[3]).stem().string() + ".dm.png";
		encode(mode, carg[3], carg[4], output);
	} else if (is_or(carg[1], "d", "dec")) {
		// dec mode
		std::string output = fs::path(carg[3]).stem().string();
		decode(mode, carg[3], carg[4], output);
	} else if (is_or(carg[1], "i", "info")) {
		// info mode
		info(mode,carg[3]);
	}
}