#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using std::ifstream;

using namespace std;

string tool_path;
string build_path;
string toolchain_prefix;

int image2_pos=0;

int copy_target_file(void)
{
	int ret;

	string file1 = build_path + ".\\target.axf";
	string file2 = build_path + ".\\target.axf.bak";
	string file3 = build_path + ".\\target_pure.axf";

	ret = CopyFile(file1.c_str(), file2.c_str(), false);
	if ( ret == 0 ) {
  	   cout << " CopyFile : " << file1 << " - " << file2 << ", ret=" << ret << endl;
	   return -1;
	}
	ret = CopyFile(file1.c_str(), file3.c_str(), false);
	if ( ret == 0 ) {
  	   cout << " CopyFile : " << file1 << " - " << file3 << ", ret=" << ret << endl;
	   return -1;
	}
	return 0;
}

void generate_binfiles(void)
{
	string build_path1 = "\"" + build_path;
	
	string strip_command = toolchain_prefix + "-strip.exe\" " + build_path1 + ".\\target_pure.axf\" ";
    string make_bin1 = toolchain_prefix + "-objcopy.exe\" -j .ram.start.table -j .ram_image1.text -Obinary " + build_path1 + ".\\target_pure.axf\" " + build_path1 + ".\\ram_1.bin\" ";
    string make_bin2 = toolchain_prefix + "-objcopy.exe\" -j .image2.start.table -j .ram_image2.text  -Obinary " + build_path1 + ".\\target_pure.axf\" " + build_path1 + ".\\ram_2.bin\" ";
    string make_bin3 = toolchain_prefix + "-objcopy.exe\" -j .image3 -j .ARM.exidx -j .sdr_data -Obinary " + build_path1 + ".\\target_pure.axf\" " + build_path1 + ".\\ram_3.bin\" ";

    strip_command = "\"" + strip_command + "\"";
	//printf(strip_command.c_str());
    system(strip_command.c_str());

	make_bin1 = "\"" + make_bin1 + "\"";
	//printf(make_bin1.c_str());
	system(make_bin1.c_str());

	make_bin2 = "\"" + make_bin2 + "\"";
	system(make_bin2.c_str());


	make_bin3 = "\"" + make_bin3 + "\"";
	system(make_bin3.c_str());
    //string make_asm = toolchain_prefix + "-objdump -d " + build_path +".\\target.axf > " + build_path + ".\\target.asm";

}


void sort_mapfile()
{
	vector<string> lines;

	ifstream infile;

	string infilestr = build_path + ".\\target.nm";
	infile.open( infilestr.c_str() );

	string line;
	while ( getline(infile, line) )
	{
		lines.push_back(line);
	}
	infile.close();

	sort(lines.begin(), lines.end());

	string outfilestr = build_path + ".\\target.map";
	ofstream outfile( outfilestr.c_str(), ios::out);
	if ( !outfile) 
	{
		cerr << "File cound not be opened: target.map" << endl;
		return;
	}

	for (size_t i=0; i < lines.size(); i++) {
		if ( lines[i].find("__ram_image2_text_start__") != std::string::npos ) {
			//cout << lines[i] << endl;
			char pos_str[9];
			memcpy(pos_str, lines[i].c_str(), 8);
			pos_str[8] = 0;
			image2_pos = strtoul(pos_str, NULL, 16);
			//cout << "image2_pos=" << image2_pos << endl;
		}
		outfile << lines[i] << endl;
	}
}




void generate_mapfile(void)
{
	string build_path1 = "\"" + build_path;
	string map_gen = toolchain_prefix + "-nm.exe\" " + build_path1 + ".\\target.axf\" > " + build_path1 + ".\\target.nm\" ";

	map_gen = "\"" + map_gen + "\"";
	system(map_gen.c_str());
	sort_mapfile();
}

int get_binfile_size(const char *filename)
{
	ifstream infile(filename, ios::binary);

	infile.seekg(0, ios::end);
	int size = infile.tellg();
	infile.seekg(0, ios::beg);
	infile.close();

	return size;
}


void generate_ram1_bin(void)
{

	string infilestr = build_path + ".\\ram_1.bin";
	string outfilestr = build_path + ".\\ram_1_prepend.bin.pad";

	int filesize = get_binfile_size(infilestr.c_str());
	//cout << "file size=" << filesize << endl;

	ifstream infile(infilestr.c_str(), ios::binary);
	ofstream outfile(outfilestr.c_str(), ios::binary);

	const unsigned char pattern[] = { 0x99, 0x99, 0x96, 0x96, 
		                     0x3F, 0xCC, 0x66, 0xFC, 
							 0xC0, 0x33, 0xCC, 0x03, 
							 0xE5, 0xDC, 0x31, 0x62 };

	outfile.write((const char*)pattern, sizeof(pattern));

	// write length
	char lenHexStr[4];
	memcpy(lenHexStr, (char*)&filesize, 4);
	outfile.write(lenHexStr, sizeof(lenHexStr));

	// write address
	int address = 0x10000bc8;
	memcpy(lenHexStr, (char*)&address, 4);
	outfile.write(lenHexStr, sizeof(lenHexStr));

	// write header_len
	short header_len = 44;
	memcpy(lenHexStr, (char*)&header_len, 2);
	outfile.write(lenHexStr, 2);

	for (int i=0; i<6; i++) {
		outfile.put((char)(0xFF));
	}

	// write body
	int bytes = filesize;
	if (infile.is_open() && outfile.is_open() ) {
		while ( bytes > 0 ) {
			outfile.put(infile.get());
			bytes--;
		}
	}

	for (int i=filesize; i<(45056-32); i++) {
		outfile.put((char)(0xFF));
	}

	outfile.close();
	infile.close();
}

void generate_ram2_bin(void)
{

	string infilestr = build_path + ".\\ram_2.bin";
	string outfilestr = build_path + ".\\ram_2_prepend.bin";

	int filesize = get_binfile_size(infilestr.c_str());
	//cout << "file size=" << filesize << endl;

	ifstream infile(infilestr.c_str(), ios::binary);
	ofstream outfile(outfilestr.c_str(), ios::binary);
//


	// write length
	char lenHexStr[4];
	memcpy(lenHexStr, (char*)&filesize, 4);
	outfile.write(lenHexStr, sizeof(lenHexStr));

	// write address
	memcpy(lenHexStr, (char*)&image2_pos, 4);
	outfile.write(lenHexStr, sizeof(lenHexStr));

	for (int i=0; i<8; i++) {
		outfile.put((char)(0xFF));
	}

	// write body
	int bytes = filesize;
	if (infile.is_open() && outfile.is_open() ) {
		while ( bytes > 0 ) {
			outfile.put(infile.get());
			bytes--;
		}
	}

	outfile.close();
	infile.close();
}

void generate_ram3_bin(void)
{
	string infilestr = build_path + ".\\ram_3.bin";
	string outfilestr = build_path + ".\\ram_3_prepend.bin";

	int filesize = get_binfile_size(infilestr.c_str());
	//cout << "file size=" << filesize << endl;

	ifstream infile(infilestr.c_str(), ios::binary);
	ofstream outfile(outfilestr.c_str(), ios::binary);
//

	// write length
	char lenHexStr[4];
	memcpy(lenHexStr, (char*)&filesize, 4);
	outfile.write(lenHexStr, sizeof(lenHexStr));

	// write address
	int address = 0x30000000;
	memcpy(lenHexStr, (char*)&address, 4);
	outfile.write(lenHexStr, sizeof(lenHexStr));

	for (int i=0; i<8; i++) {
		outfile.put((char)(0xFF));
	}

	// write body
	int bytes = filesize;
	if (infile.is_open() && outfile.is_open() ) {
		while ( bytes > 0 ) {
			outfile.put(infile.get());
			bytes--;
		}
	}

	outfile.close();
	infile.close();
}

void merge_to_ram_all_bin(void)
{
	string outfilestr = build_path + ".\\ram_all.bin";

	ofstream outfile(outfilestr.c_str(), ios::binary);

	// ram_1
	string infilestr = build_path + ".\\ram_1_prepend.bin.pad";
	int filesize = get_binfile_size(infilestr.c_str());
	ifstream infile( infilestr.c_str(), ios::binary);

	// write body
	int bytes = filesize;
	if (infile.is_open() && outfile.is_open() ) {
		while ( bytes > 0 ) {
			outfile.put(infile.get());
			bytes--;
		}
	}
	infile.close();

	// ram_2
	infilestr = build_path + ".\\ram_2_prepend.bin";
	filesize = get_binfile_size(infilestr.c_str());
	ifstream infile2(infilestr.c_str(), ios::binary);

	// write body
	bytes = filesize;
	if (infile2.is_open() && outfile.is_open() ) {
		while ( bytes > 0 ) {
			outfile.put(infile2.get());
			bytes--;
		}
	}
	infile2.close();

	// ram_3
	infilestr = build_path + ".\\ram_3_prepend.bin";
	filesize = get_binfile_size( infilestr.c_str() );
	ifstream infile3( infilestr.c_str(), ios::binary);

	// write body
	bytes = filesize;
	if (infile3.is_open() && outfile.is_open() ) {
		while ( bytes > 0 ) {
			outfile.put(infile3.get());
			bytes--;
		}
	}
	infile3.close();

	outfile.close();
}


void download_bin_file(void)
{
	char disk;
	// find out disk
	for ( disk='D'; disk<='Z'; disk++ ) {
		string filename = "";
		filename.push_back(disk);
		filename.append(":\\mbed.htm");
		ifstream mbed_file(filename.c_str());
		if (mbed_file.good()) break;
	}

	if ( disk == '[' ) // not found
	{
		cout << "No Ameba Arduino board. Please plug in" << endl;
		return;
	}

	string build_path1 = build_path;
	build_path1.erase(std::find(build_path1.begin(), build_path1.end(), '\"'));
	build_path1.erase(std::find(build_path1.begin(), build_path1.end(), '\"'));

	string copy_command="copy "+ build_path1 + ".\\ram_all.bin ";
	copy_command.push_back(disk);
	copy_command.append(":\\");
	cout << copy_command << endl;

	string file1 = build_path + ".\\ram_all.bin";
	string file2 = "";
	file2.push_back(disk);
	file2.append(":\\ram_all.bin");
	CopyFile(file1.c_str(), file2.c_str(), true);

}

int main(int argc, char* argv[])
{
	int ret;

	if (argc != 3) {
		cout << argv[0] << " <this tool path>" << " <build_path> " << endl;
		return -1;
	}

	tool_path = argv[1];
	tool_path.erase(std::find(tool_path.begin(), tool_path.end(), '\"'));
	tool_path.erase(std::find(tool_path.begin(), tool_path.end(), '\"'));
	tool_path.append("\\");

	build_path = argv[2];
	build_path.erase(std::find(build_path.begin(), build_path.end(), '\"'));
	build_path.erase(std::find(build_path.begin(), build_path.end(), '\"'));
	build_path.append("\\");

    toolchain_prefix = "\"" + tool_path + "\\gcc-arm-none-eabi-4.8.3-2014q1\\bin\\arm-none-eabi";


	ret = copy_target_file();
	if ( ret != 0 ) {
		cout << "Copy files error" << endl;
		return ret;
	}

	generate_binfiles();
	generate_mapfile();
	generate_ram1_bin();
	generate_ram2_bin();
	generate_ram3_bin();
	merge_to_ram_all_bin();
	download_bin_file();

	return 0;
}

