#include "LUT_utils.h"
#include "GC/emp-sh2pc.h"
#include <cstdint>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

int lut_type = LUTType::Filled, input_bits = 16, output_bits = 16, test=false;

inline std::map<uint64_t, uint64_t> get_lut_test() {
    std::vector<uint64_t> lut_content{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, 
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, 
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, 
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, 
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, 
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, 
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, 
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, 
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, 
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, 
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, 
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, 
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, 
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, 
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, 
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 
    }; 
    std::map<uint64_t, uint64_t> lut;
    for (int i=0; i<lut_content.size(); i++) {
        lut[i] = lut_content[i];
    }
	return lut;

}

inline string gen_flute_header(int input_bits, int output_bits) {
    string res = fmt::format("#STATISTICS\n#ORIGINAL LUTS: {}\n\n#GROUPED LUTS: 1\n#DEPTH: 1\n#XOR: 0\n#NOT: 0\n#XNOR: 0\n#ASSIGN: 0\n\n", input_bits);

    res += fmt::format("#INPUTS {}\n", input_bits);
    for (int i = 0; i < input_bits; i++) {
        res += fmt::format("addr[{}] ", i);
    }
    res += "\n";

    res += fmt::format("#OUTPUTS {}\n", output_bits);
    for (int i = 0; i < output_bits; i++) {
        res += fmt::format("dout[{}] ", i);
    }
    res += "\n";
    res += "#constant zero\n0\n#constant one\n1\n#LUTs\n";
    return res;
}

inline string dump_flute_lut(std::map<uint64_t, uint64_t> lut, int input_bits, int output_bits) {
    assert (lut.size() == (1 << input_bits));
	uint64_t lut_size = (1ULL << input_bits);
	std::vector<string> out_string(lut_size);
	for (int i = 0; i < lut_size; i++) {
		out_string[i] = utils::block(lut[i]).to_string().substr(utils::blocksize - output_bits);
	}
    string body = "";
	for (int out_idx = 0; out_idx < output_bits; out_idx++) {
		std::vector<uint8_t> out_mask(lut_size / 8);
		for (int chunk_idx = 0; chunk_idx < lut_size / 8; chunk_idx++) {
			string temp="";
			for (int i = 8*chunk_idx; i < 8*(chunk_idx+1); i++) {
				temp += out_string[lut_size-i-1][output_bits-out_idx-1];
			}
			out_mask[chunk_idx] = utils::block(temp).to_ulong();
		}

		auto all_one_block = utils::block();
		all_one_block.set();

		body += fmt::format(" {} {} 0x", input_bits, all_one_block.to_string().substr(utils::blocksize - input_bits));

		for (int i = 0; i < lut_size / 8; i++) {
			body += fmt::format("{:02x}", out_mask[i]);
		}

		body += fmt::format(" dout[{}]", out_idx);
	}
    
    body += "  \n";
    string header = gen_flute_header(input_bits, output_bits);
    header += fmt::format("LUT {} {}", input_bits, output_bits);
    for (int i = 0; i < input_bits; i++) {
        header += fmt::format(" addr[{}]", i);
    }
    return header + body;
}

int main(int argc, char** argv) {

    ArgMapping amap;
    amap.arg("test", test, "test");
    amap.arg("t", lut_type, "LUTType: Random = 0; Gamma = 1, Filled = 2 (Default: 2)");
    amap.arg("i", input_bits, "Number of input bits (Default: 16)");
    amap.arg("o", output_bits, "Number of output bits (Default: 16)");
    amap.parse(argc, argv);

    utils::check(lut_type < NumLUTTypes && lut_type >= 0, "Invalid LUT type");
    utils::check(input_bits > 0, "Invalid input bits");
    utils::check(output_bits > 0, "Invalid output bits");

    if (test) {
        string filename = fmt::format("flute_luts/LUT_test.lut", input_bits, output_bits, lut_type_to_string((LUTType)lut_type));
        std::ofstream out(filename);
        out << dump_flute_lut(get_lut_test(), 8, 8);
    } else {
        auto lut = get_lut_map((LUTType)lut_type, input_bits, output_bits);

        string filename = fmt::format("flute_luts/LUT_{}_{}_{}.lut", input_bits, output_bits, lut_type_to_string((LUTType)lut_type));
        std::ofstream out(filename);

        out << dump_flute_lut(lut, input_bits, output_bits);
    }

    return 0;
}
    