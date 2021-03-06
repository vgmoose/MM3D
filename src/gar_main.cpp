// gar_main.cpp --- where the 'gar' program starts

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

#include "lzs.hpp"
#include "gar.hpp"

int main(int argc, char ** argv) {
    if (argc != 3) {
        std::cerr << "Usage:\n";
        std::cerr << "    gar <input file> <output directory>\n";
        return 1;
    }

    std::string infname = argv[1];
    std::string outdir  = argv[2];

    // open archive

    std::ifstream archive(infname, std::ios::binary | std::ios::ate);

    if (archive.fail()) {
        std::cerr << "Could not open \"" << infname << "\".\n";
        return 1;
    }

    const int arcsize = archive.tellg();
    archive.seekg(0);

    // see if it's a gar or lzs file

    char magic[5];
    char arcrawdata[arcsize];
    std::vector<uint8_t> arcdata;

    archive.read(magic, 4);
	magic[4] = '\0';

    archive.seekg(0);
    archive.read(arcrawdata, arcsize);
    arcdata = std::vector<uint8_t>(arcrawdata, arcrawdata + arcsize);

    if (0 == strcmp(magic, std::string("GAR\x02").c_str())) {
        gar_ext(arcdata, outdir);
    } else if (magic == std::string("LzS\x01")) {
        // a bit of trickery; thanks to the use of LZSS, the Gaiden Archive
        // magic word appears in its entirety at 0x11 in the file
        archive.seekg(0x11);
        archive.read(magic, 4);
        if (magic != std::string("GAR\x02")) {
            std::cerr << "Not a Gaiden Archive. Exiting.\n";
            return 1;
        }

        // compressed gaiden archive; let's decompress and extract
        arcdata = lzs_dec(arcdata);
        gar_ext(arcdata, outdir);
    } else {
        std::cerr << "Not a Gaiden Archive. Exiting.\n";
        archive.close();
        return 1;
    }

    archive.close();

    return 0;
}
