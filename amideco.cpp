#include <iostream>  
#include <fstream>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
//#include "lzh5x.cpp"

using namespace std;
/* AMIDECO rewritten in C++ by computeguy08 */

/* == CONSTANTS ===*/
const char date[] = "ver 1.0 Nov.2020";
const char erw[] = ".dec";
const int amib_longint = 67 + 77 << 8 + 73 << 16 + 66 << 24;
const string known_block_type[64] =
{ "POST",                        /* 00 */
 "Setup Server",                /* 01 */
 "Runtime",                     /* 02 */
 "DIM",                         /* 03 */
 "Setup Client",                /* 04 */
 "Remote Server",               /* 05 */
 "DMI Data",                    /* 06 */
 "GreenPC",                     /* 07 */
 "Interface",                   /* 08 */
 "MP",                          /* 09 */
 "Notebook",                    /* 0a */
 "Int-10",                      /* 0b */
 "ROM-ID",                      /* 0c */
 "Int-13",                      /* 0d */
 "OEM Logo",                    /* 0e */
 "ACPI Table",                  /* 0f */
 "ACPI AML",                    /* 10 */
 "P6 MicroCode",                /* 11 */
 "Configuration",               /* 12 */
 "DMI Code",                    /* 13 */
 "System Health",               /* 14 */
 "UserDefined",                 /* 15 */
 "",                            /* 16 */
 "",                            /* 17 */
 "? Menu/VGA code",             /* 18 */ /* P07-0014.BIO */
 "Text mode font",              /* 19 */ /* P07-0014.BIO */
 "Graphics",                    /* 1a */ /* P07-0014.BIO */
 "? Setup code",                /* 1b */ /* P07-0014.BIO */
 "",                            /* 1c */
 "",                            /* 1d */
 "",                            /* 1e */
 "",                            /* 1f */
 "PCI AddOn ROM",               /* 20 */
 "Multilanguage",               /* 21 */
 "UserDefined",                 /* 22 */
 "",                            /* 23 */
 "",                            /* 24 */
 "",                            /* 25 */
 "",                            /* 26 */
 "",                            /* 27 */
 "",                            /* 28 */
 "",                            /* 29 */
 "LANG",                        /* 2a */ /* 'GNAL' 'su' */
 "? PXE ROM",                   /* 2b */ /* P07-0014.BIO */
 "FONT",                        /* 2c */ /* 'TNOF' */
 "",                            /* 2d */
 "? Code+revision",             /* 2e */ /* P07-0014.BIO */
 "",                            /* 2f */
 "Font Database",               /* 30 */
 "OEM Logo Data",               /* 31 */
 "Graphic Logo Code",           /* 32 */
 "Graphic Logo Data",           /* 33 */
 "Action Logo Code",            /* 34 */
 "Action Logo Data",            /* 35 */
 "Virus",                       /* 36 */
 "Online Menu",                 /* 37 */
 "",                            /* 38 */
 "",                            /* 39 */
 "",                            /* 3a */
 "",                            /* 3b */
 "",                            /* 3c */
 "GRFX",                        /* 3d */ /* XFRG */
 "",                            /* 3e */
 "TDSS" };                      /* 3f */ /* SSDT */

/* ============== STRUCTURES ================= */

struct start {
    long int packed_length;
    long int unpacked_length;
    char date;
}start_const1;

struct pointer_rec {
    short int ofs, seg;
}position, * pointer_rec_z;

struct head {
    struct pointer_rec next_block;  /* 00 */
    short int packed_length;        /* 04 */
    char b6;                        /* 06 */
    char b7;                        /* 07 */
    struct pointer_rec dest_address;/* 08 */
    long int lc;                    /* 0c */
    long int unpacked_length;       /* 10 */
} head;                             /* 14 */

struct head_1994_type {
    long int packed_length;
    long int unpacked_length;
}head_1994;

struct ibm_head {
    short int ami_ofs;
    short int ami_seg;
    long int blocklength;
}ibm_header;

struct {
    string name;
    string src_dir;
    string src_name;
    string src_ext;
    int length = 0;
    char count = 0;
}file;

struct {
    char b0;
    char target_ofs_hi;
    short int src_ofs;
} reference_1994;

struct {
    long int logical_area_size, offset_in_image, size_of_image_chunk;
    char comment[31], logical_area_name[23], time_stamp[14];
    char signature[5], filename_of_next_file[15], bios_reserved[15];
    char logical_area_type, load_from_file, reboot_after_update, update_entire_image;
    char checksum_for_this_header, logical_area_type2, last_file_in_chain;
} ami_flash_head;        /* I: T00544 */


struct {
    short int w0;                 /* 0xFFFF */
    short int typ;
    long int source;
    long int packed_length;
    long int unpacked_length;
    long int target;
    long int l14;
} head_ami_intel;

/* ============= VARIABLES ============== */

ifstream fin;
bool no_save = false, split = false;
char bios_type = 0, argt[16], signature[16], zk[5], * rom = NULL;
string target_dir, tmp;
int arg, mem_alc;
short int block_count;
int min_loadaddress = 0x100000, counter, target_seg, target_ofs, max_s, rom_start;
long rom_size, logic_start, position_amibiosc, position_l, intel_tabel_ofs, loadaddress, l, o, oj, p;


bool new_baseaddr_required, present, mix_used = false;
char* src_rom, * unpacked, src_directory[255], src_name[255], src_extension[255], result[32],
mixf0000[65535], char8[8], buffer[255] = "", * rom_tmp;


/*struct ibm_head ibm_header_const2 = { rom[0xe0000] };
struct ibm_head ibm_header_const3 = { rom[0xe0000] };
struct ibm_head ibm_header_const4 = { rom[0xe0000] };
struct ibm_head ibm_header_const5 = { rom[0xe0000] };
struct ibm_head ibm_header_const6 = { rom[0xe0000] };
struct ibm_head ibm_header_const7 = { rom[0xe0000] };
struct ibm_head ibm_header_const8 = { rom[oj] };
struct ibm_head ibm_header_const9 = { rom[oj] };
struct ibm_head ibm_header_const10 = { rom[oj & 0xff0000] };*/

/* ============= FUNCTIONS =============== */

/* == int2hex - converts integer values to hexadecimal arrays == */
char* int2hex(long int number, char n)
{
    int temp, i = 0;
    while (n != 0)
    {
        temp = number % 16;
        if (temp < 10) {
            result[i] = temp + 48;
            i++;
        }
        else {
            result[i] = temp + 55;
            i++;
        }
        n = n / 16;
    }
    return result;
}

/* == filesize - determine the size of a file in KB == */
long filesize(string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

/* == copy - strncpy for strings == */
string copy(string str, int index, int count) {
    if (str == "") return "";
    size_t length = str.copy(buffer, count, index - 1);
    buffer[length] = '\0';
    return string(buffer);
}

long int issue_address(const long int issue) {
    string solution;
    long int result, control = 1;
    do {
        cout << "[$" << int2hex(issue, 5) << "] ?";
        if (fin.eof())
            solution = "";
        else
            cin >> solution;
        if (solution == "") {
            control = 0;
            result = issue;
        }
        /*else
            val(solution, result, control);*/
    } while (control != 0);
    return result;
}
/* == save - write contents of an n-sized array to the disk == */
void save(char* a, long int n) {
    ofstream fout(target_dir + file.name);
    fout.write(a, n);
    fout.close();
}

void delete_srcrom() {
    memset(src_rom, 0, 0x20000);
}

/* == unzip - expand compressed AMIBIOS files == */
void unzip(void* source) {

    struct start with = start_const1;
    memset(unpacked, 0xcc, with.unpacked_length);

    /*if (unzip_lzh5x(with.date, unpacked, with.unpacked_length, with.packed_length)) {
        save(unpacked, with.unpacked_length);
        cout << int2hex(with.unpacked_length, 8) << "  " << filename;
    }
    else*/
    cout << "File unpack error!";
}

bool test_archive_or_amibios(head_1994_type& head)
{
    if ((head.packed_length < 0xe000) && (head.packed_length > 0x100) && (head.unpacked_length < 0x11000) && (head.unpacked_length > 0x100))
        return true;
    if (head.packed_length == amib_longint)
        return true;
    return false;
}

void blockread1(void* puffer, int seg, int ofs, int length)
{
    memmove(puffer, &rom[(long int)(seg) << 4 + ofs], length);
}

string block_type(char t)
{
    tmp = "";
    for (int i = 0; i < 64; i++) {
        if (t == i)
            tmp = known_block_type[t];
    }
    if (tmp == "")
        tmp = int2hex(head.b6, 2) + '?';
    while (tmp.length() < 20)  tmp += ' ';
    return tmp;
}

string find_extension(char t) {
    switch (t) {
    case 0xc:tmp = ".ver"; break;
    case 0x20:tmp = ".pci"; break;
    case 0x31:tmp = ".oem"; break;
    case 0x36:tmp = ".vir"; break;
    default:tmp = erw;
    }
    return tmp;
}

void try_unpack(long int o)
{
    memmove(&head_1994, &rom[o], sizeof(head_1994));
    if (!test_archive_or_amibios(head_1994))
        return;
    cout << ":" << int2hex(o, 8) << " " << int2hex(head_1994.packed_length, 4) << " ????:????  T=??" << string(16, ' ') << "->";
    file.name = int2hex(o, 8);
    file.name[1] = 'r';
    file.name = file.name + erw;
    delete_srcrom();
    memmove(&src_rom, &rom[o], head_1994.packed_length);
    unzip(&src_rom);
    cout << endl;
}
void alloc_mem() {
    char* tmp;
    if (rom != NULL) {
        tmp = (char*)realloc(rom, mem_alc * sizeof(char));
        if (tmp != NULL) {
            rom = tmp;
        }
        else {
            cout << "Memory allocation error!";
            exit(1);
        }
    }
    else {
        cout << "Memory allocation error!";
        exit(1);
    }
}
/* == open_file - attempt to open a file on the disk == */
void open_file()
{
    fin.open(file.name, std::ios::binary);
    if (!fin) {
        cout << "ERROR: cannot open file!" << endl;
        exit(4);
    }
    else {
        if ((file.length = filesize(file.name)) > 0x100000) {
            fin.close();
            cout << "ERROR: file is too big! (over 1 MB)";
            exit(5);
        }
        file.count += 1;
        cout << "File: " << file.name << " (" << file.length / 1024 << " KB)" << endl;

    }

}

string upps(string s1) {
    for (unsigned int i = 0; i < s1.length(); i++)
        zk[i] = toupper(s1[i]);
    return string(zk);
}
/* == check_file - function used to check for valid filenames  == */

void check_file() {
    char tmp = 0;
    bool dot = false;
    for (unsigned int i = 0; i < file.name.length(); i++) {
        if (file.name[i] == 0x5c) // check for slash and dot
            tmp = i;
        if (file.name[i] == 0x2e)
            dot = true;
    }
    if (!dot) { // if dot is missing => there is no file extension
        cout << "ERROR: invalid file extension!" << endl;
        exit(3);
    }
    if (tmp) {
        file.src_dir = copy(file.name, 1, tmp).c_str();
        file.src_name = copy(file.name, tmp + 2, file.name.length() - tmp - 5).c_str();
    }
    else {
        file.src_dir = "";
        file.src_name = copy(file.name, 1, file.name.length() - tmp - 4).c_str();
    }
    file.src_ext = copy(file.name, file.name.length() - 3, 4).c_str();
    //cout << dir << " " << name << " " << ext << endl;
    if ((upps(file.src_ext) != ".BIN") && (upps(file.src_ext) != ".ROM") && (upps(file.src_ext) != ".AMI")) {
        cout << "ERROR: invalid file extension!" << endl;
        exit(3);
    }
}
void read_file() {
    fin.seekg(0);
    fin.read(signature, 16);
    if (split) {
        if ((!string(signature).compare(0, 2, "ª@") || !string(signature).compare(0, 2, "Ué"))
            && (bios_type == 0 || bios_type == 1))
            bios_type = 1; //286 AMI split
        else if ((!string(signature).compare(0, 8, "13AAMMII") || !string(signature).compare(0, 8, "02AAMMII"))
            && (bios_type == 0 || bios_type == 2))
            bios_type = 2; // 386 AMI split
        else
            bios_type = 127; //unknown
    }
    else {
        if (!string(signature).compare(0, 16, "0123AAAAMMMMIIII"))
            bios_type = 3; // 386 AMI 
        if (!string(signature).compare(0, 8, "AMIBIOSC"))
            bios_type = 5; // 1994 AMI
        fin.seekg(0x8000);
        fin.read(signature, 16); // checking if it's 1993 AMI with AMIBIOSC
        if (!string(signature).compare(0, 16, "(AAMMIIBBIIOOSS)"))
            bios_type = 4; // 1993 AMIBIOSC
        if (!bios_type)
            bios_type = 127; //unknown
    }
    fin.seekg(0);
    fin.read(rom + logic_start, file.length);
    logic_start += file.length;
    fin.close();
}

void rom_join() {
    mem_alc += rom_size;
    int j = rom_size;
    alloc_mem();
    for (long i = rom_size; i <= mem_alc; i += 2) {
        rom[i] = rom[j - 2 * file.length];
        rom[i + 1] = rom[j - file.length];
        j++;
    }
}

/* =================================== MAIN FUNCTION =============================== */
/* ==================================== STARTS HERE ================================ */

/* important flags (can be placed anywhere at command line):

    /ns - use this flag to disable file saving to the disk
    /sp - use this flag for split images, filename order: LOW -> HIGH (EVEN -> ODD)
    /lst - given a text file with filenames, AMIDECO will output data to another text file for each BIOS [WIP]
*/

int main(int argc, const char* argv[])
{
    cout << "==============================" << endl;
    cout << "AMIDECO Plus " << date << endl;
    rom = (char*)malloc(10);
    if (rom == NULL)
        exit(1);
    if (argc < 2) {
        cout << "ERROR: argument missing" << endl;
        exit(2);
    }
    for (arg = 1; arg < argc; arg++) {
        if (string(argv[arg]) == "/ns") no_save = true;
        else if (string(argv[arg]) == "/sp") split = true;
        else argt[arg] = 1;
    }
    logic_start = 0;
    for (arg = 1; arg < argc; arg++) {
        if (argt[arg]) {
            file.name = argv[arg];
            check_file();
            open_file();
            mem_alc += file.length;
            alloc_mem();
            read_file();
        }
    }
    rom_size = logic_start;
    logic_start = 0;
    cout << "------------------------------" << endl;
    // start identifying the type of BIOS
    // 1. AMIBIOS prior to 1993
    if (bios_type < 5) {
        cout << "Type: AMIBIOS '90-'93 uncompressed";
        if (split) {
            cout << " and split";
            rom_join();
            logic_start += 2 * file.length;
        }
        if (bios_type == 4)
            logic_start += 0x8000; //skip to middle of the file
        cout << endl;
        cout << "Core version: " << copy(&rom[logic_start + 0x90], 1, 6) << endl;
        cout << "POST string: " << &rom[logic_start + 0x78] << endl;
        if (split && !no_save) {
            file.name = string(src_name) + erw;
            save(&rom[rom_size], 2 * file.length);
            cout << "File saved as: " << file.name << endl;
        }
    }
    if (bios_type == 5)
        cout << copy(&rom[logic_start + 0x90], 1, 6) << endl;
    /// 2. AMIBIOSC 1994 at E000:0000 
    exit(0);/*
    if (file_length >= 131072){
        zk[0] = (char)(0x10);
        blockread1(&rom_tmp, 0xe000, 0, 16);
        cout << '"' << rom[0xe000] << '"' << endl;
        blockread1(&block_count, 0xe000, 0x10, sizeof(block_count));
    }
    /*else
        rom_tmp = "";*//*
    cout << '"' << rom_tmp[0] << '"' << endl;
    if ((copy(rom_tmp, 1, 8) == "AMIBIOSC") && (block_count < 30)) {
        exit(0); /* 1994; no Z, but already data? *//*
        cout << '"' << rom_tmp << '"' << endl;
        present = false;  /* F000:1000 not found yet *//*
        cout << "header error 5!" << endl;

        for (counter = 0; counter <= block_count - 1; counter++)
        {
            blockread1(&reference_1994, 0xe000, 0x12 + counter * 4, sizeof(reference_1994));
            position.ofs = reference_1994.src_ofs;
            position.seg = 0xe000;
            blockread1(&head_1994, position.seg, position.ofs, sizeof(head_1994));
            if (!test_archive_or_amibios(head_1994))
            {
                position.seg = 0xf000;
                blockread1(&head_1994, position.seg, position.ofs, sizeof(head_1994));
                if (!test_archive_or_amibios(head_1994))
                {
                    cout << "Cannot find source segment!" << endl;
                    exit(1);
                }
            }

            if ((position.seg == 0xf000) && (position.ofs == 0x1000))
                present = true;

            target_ofs = reference_1994.target_ofs_hi << 8;
            cout << int2hex(position.seg, 4) << ':' << int2hex(position.ofs, 4) << ' ' <<
                int2hex(head_1994.packed_length, 4) << " ????:" <<
                int2hex(target_ofs, 4) << "  T=" << int2hex(reference_1994.b0, 2) << ' ';

            zk[0] = (char)(8);
            memmove(&zk[1], &head_1994, atoi(&zk[0]));

            if (copy(zk, 1, 8 - 1) == "AMIBIOS")
                cout << "= \"" << zk << '"' << endl;

            else {
                cout << "-> ";
                filename = int2hex((long int)(position.seg) << 4 + position.ofs, 8);
                filename[1] = 'r';
                filename += erw;
                delete_srcrom();
                blockread1(&src_rom, position.seg, position.ofs, head_1994.packed_length);
                unzip(&src_rom);
                cout << endl;
                for (int i = 0; i < 3; i++) /* post/runtime *//*
                    if (i == reference_1994.b0)
                        memmove(&mixf0000[target_ofs], &unpacked, head_1994.unpacked_length);
            }
        }    /* FOR *//*


        if (!present)
        {
            position.ofs = 0x1000;
            position.seg = 0xf000;
            blockread1(&head_1994, position.seg, position.ofs, sizeof(head_1994));
            if (test_archive_or_amibios(head_1994)) {
                target_ofs = 0;
                cout << int2hex(position.seg, 4) << ':' << int2hex(position.ofs, 4) << "  " <<
                    int2hex(head_1994.packed_length, 4) << "  ????:????  T=??" <<
                    string(16, ' ');
                zk[0] = (char)(0x08);
                memmove(&zk[1], &head_1994, atoi(&zk[0]));

                if (copy(zk, 1, 8 - 1) == "AMIBIOS")
                    cout << "= \"" << zk << '"' << endl;
                else {
                    cout << "-> ";

                    filename = int2hex((long int)(position.seg) << 4 + position.ofs, 8);
                    filename[1] = 'r';
                    filename += erw;
                    delete_srcrom();
                    blockread1(&src_rom, position.seg, position.ofs, head_1994.packed_length);
                    unzip(&src_rom);
                    cout << endl;
                }
            }    /* valid *//*
        }
        filename = string("MIX") + erw;
        save(mixf0000, sizeof(mixf0000));
        exit(0);
    }    /* amibiosc 1994 */
    /*** 1995+ with AMIBIOSC ******************************************//*
    position_amibiosc = 0x100000;
    while (position_amibiosc > 0) {
        if (strncmp(&rom[position_amibiosc - 22], "AMIBIOSC0", 9) == 0)
            break;
        else
            position_amibiosc -= 0x800;
    }

    if (position_amibiosc > 0) {
        zk[0] = string("AMIBIOSC0627").length();
        memmove(&zk[1], &rom[position_amibiosc - (long)22], atoi(&zk[0]));
        cout << '"' << zk << '"' << endl;
        memmove(&position, &rom[position_amibiosc - (long)4], sizeof(position));
    }

    if (position_amibiosc == 0) {
        /* AMIBOOT ROM at FFFFFF40/44/50/54 *//*
        if ((strncmp(&rom[0xFff40], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || (strncmp(&rom[0xFff44], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || (strncmp(&rom[0xFff50], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || (strncmp(&rom[0xFff54], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)) {

            position_amibiosc = sizeof(rom) - 0x10000 + (&rom[0xFffa0] && 0xffff);
            if (strncmp(&rom[position_amibiosc], "AMIBIOSC", strlen("AMIBIOSC")) == 0) {
                zk[0] = 12; //string("AMIBIOSC0627").length();
                memmove(&zk[1], &rom[position_amibiosc], atoi(&zk[0]));
                cout << '"' << zk << '"' << endl;
                memmove(&position, &rom[position_amibiosc + 0x12], sizeof(position));
            }
            else
                position_amibiosc = 0;
        }
    }
    if (position_amibiosc == 0) {
        /* 7VRX.F1: AMIBOOT ROM at FFFFe000/04, $SIP at FFFFffd4 *//*
        if ((strncmp(&rom[0xFffd4], "$SIP", strlen("$SIP")) == 0)
            && ((&rom[0xFffd0] && 0xfff80000) == 0xfff80000)) {
            position_amibiosc = (long)((&rom[0xFffd0]) && 0xfffff) - 0x16;
            if (strncmp(&rom[position_amibiosc], "AMIBIOSC", strlen("AMIBIOSC")) == 0) {
                zk[0] = 12; //string("AMIBIOSC0627").length();
                memmove(&zk[1], &rom[position_amibiosc], atoi(&zk[0]));
                cout << '"' << zk << '"' << endl;
                memmove(&position, &rom[position_amibiosc + 0x12], sizeof(position));
            }
            else
                position_amibiosc = 0;
        }
    }

    if (position_amibiosc > 0){
        cout << "header error 4!" << endl;
        do {
            if ((position.seg == 0) || (position.seg == 0xffff))
            {
                cout << "chain failure 1!" << endl;
                exit(1);
            }

            cout << int2hex(position.seg, 4) << ':' << int2hex(position.ofs, 4) << "  ";
            blockread1(&head, position.seg, position.ofs, sizeof(head));
            {
                if (head_1994.packed_length == 0) {
                    cout << "chain failure 2!" << endl;
                    exit(1);
                }
                cout << int2hex(head_1994.packed_length, 4) <<
                    "  " << int2hex(head.dest_address.seg, 4) <<
                    ':' << int2hex(head.dest_address.ofs, 4) <<
                    "  " << block_type(head.b6);

                delete_srcrom();
                blockread1(&src_rom, position.seg, position.ofs, long(sizeof(head) + head.packed_length));

                if ((head.dest_address.seg == 0) && (head.dest_address.ofs == 0))
                {
                    filename = int2hex((long int)(position.seg) << 4 + position.ofs, 8);
                    filename[1] = 'r';
                }
                /*else
                    filename = int2hex(head.dest_address, 8);*//*
                filename += find_extension(head.b6);

                if ((head.b7 & 0x80) == 0x80) {      // not compressed
                    cout << "=> ";
                    save(&src_rom[4 + 4 + 4], head.packed_length);
                    cout << int2hex(head.packed_length, 8) << "  " << filename;
                }
                else {
                    cout << "-> ";
                    unzip(&src_rom[4 + 4 + 4]);
                    if (head.dest_address.seg >= 0xf000) // runtime/post
                        for (int i = 0; i < 3; i++)
                            if (i == head.b6) {
                                cout << " +mix";
                                mix_used = true;
                                memmove(&mixf0000[(head.dest_address.seg - 0xf000) >> 4 + head.dest_address.ofs], unpacked,
                                    head.unpacked_length);
                            }
                }
                cout << endl;
                position = head.next_block;
            }    // with head

        } while (!(position.seg == 0xffff));

        if (mix_used) {
            filename = string("MIX") + erw;
            save(mixf0000, sizeof(mixf0000));
        }

        if (position_amibiosc <= 0xf8000)
            try_unpack(0xf8000);
        if (position_amibiosc <= 0xf0000)
            try_unpack(0xf0000);

        exit(0);
    }
    exit(0);
    // IBM without AMIBIOSC
    if ((strncmp(&rom[0xfe008], "COPR. IBM 1981", strlen("COPR. IBM 1981")) == 0)
        && (logic_start <= 0xe0000)
        && ((logic_start & 0x1ffff) == 0)
        && (ibm_header_const2.ami_ofs >= 0)
        && (ibm_header_const3.ami_ofs <= 0x30)
        && ((ibm_header_const4.ami_ofs & 0xff0f) == 0)
        && (ibm_header_const5.ami_seg == 0xe000)
        && (ibm_header_const6.blocklength < 1024 * 1024)
        && (ibm_header_const7.blocklength > 0)) {
        cout << "IBM ROM (Thinkpad 770).." << endl;
        oj = logic_start;
        o = oj + (long int)(2 * sizeof(ibm_header));
        do {
            if (o >= 0x100000)
                exit(0); // end of file
            cout << int2hex(oj, 8) << ":";

            struct ibm_head with = ibm_header_const8;
            if ((ibm_header.blocklength <= 8)  // invalid value
                || (ibm_header.blocklength > 0x20000)
                || ((with.ami_ofs > 0) && (with.ami_ofs < 2 * sizeof(ibm_header)))
                || ((with.ami_seg & 0xf000) == 0)
                || ((with.ami_seg & 0xfff) != 0)) {
                if ((with.blocklength == 0) && (with.ami_ofs == 0) && (with.ami_seg == 0))
                    cout << "." << endl;
                else
                    if (rom[oj + 4] == 1 || rom[oj + 4] == 2)
                        for (char i = 0; i < 10; i++)
                            if (i == (char)(rom[oj + 7]))
                                cout << copy(&char8[rom[oj]], 1, 8) << endl;
                            else
                                cout << "?" << endl;
                // skip to next 64KB block
                oj = (o + (long int)(0xffff)) & 0xffff0000;
                o = oj + (long int)(2 * sizeof(ibm_header));
                continue;
            }
            struct ibm_head with2 = ibm_header_const9;
            p = (oj & 0xffff0000) + (with2.ami_seg - 0xe000) << 4 + with2.ami_ofs;
            filename = (string)int2hex(p, 8) + erw;
            cout << "--> " << int2hex(with2.ami_seg, 4) << ':' << int2hex(with2.ami_ofs, 4) <<
                ',' << int2hex(with2.blocklength, 8);
            if (with2.ami_ofs > 0) {
                //cout << "  lzh5: " << int2hex(head_1994_type(rom[p]).packed_length, 8) << "->";
                unzip(&rom[p]);
                cout << endl;
                o = max(o, p + with2.blocklength);
            }
            else {
                cout << "  none: " << int2hex(with2.blocklength, 8);
                save(&rom[p], with2.blocklength);
                cout << "=>" << int2hex(with2.blocklength, 8) << "  " << filename << endl;
                o = max(o, p + with2.blocklength);
            }
            oj += sizeof(ibm_header);
            // do not interpret packed data as ibm blocks
            struct ibm_head with3 = ibm_header_const10;
            if ((with3.ami_seg == 0xe000)
                && (with3.ami_ofs > 0) // contains unpacked data IBM-Block
                && ((oj & 0xffff) >= with3.ami_ofs)) {
                // skip to next 64KB block
                oj = (o + (long int)(0xffff)) & 0xffff0000;
                o = oj + (long int)(2 * sizeof(ibm_header));
            }
        } while (!false);
    }

    // Intel without AMIBIOSC
    intel_tabel_ofs = rom[0xfe000];

    if ((file_length == 0) && (intel_tabel_ofs >= 0xfffe0000) && (intel_tabel_ofs < 0xffffe000)) {
        cout << " header error 3!";
        do {
            memmove(&head_ami_intel, &rom[intel_tabel_ofs - 0xfff00000], sizeof(head_ami_intel));
            {
                if ((head_ami_intel.packed_length == 0) || (head_ami_intel.packed_length == 0xffffffff))
                    exit(0);

                if (head_ami_intel.source == 0xffffffff) {
                    intel_tabel_ofs += sizeof(head_ami_intel);
                    continue;
                }

                /*if (head_ami_intel.unpacked_length == 0) {   /* language module at FFFe8800 *//*
                    head_ami_intel.packed_length = (long)(&rom + (head_ami_intel.source & 0xfffff));
                    head_ami_intel.unpacked_length = (long)(&rom + (head_ami_intel.source & 0xfffff) + 4);
                    head_ami_intel.source += 4 + 4;
                }*//*

                cout << ':' << int2hex(head_ami_intel.source, 8) << "  " << int2hex(head_ami_intel.packed_length, 4)
                    << "  :" << int2hex(head_ami_intel.target, 8) << "  T=" << int2hex(head_ami_intel.typ, 4) << string(17, ' ');
                filename = string(int2hex(head_ami_intel.source, 8)) + erw;

                if (head_ami_intel.packed_length == head_ami_intel.unpacked_length) {
                    save(&rom[head_ami_intel.source - 0xfff00000], head_ami_intel.unpacked_length);
                    cout << int2hex(head_1994.unpacked_length, 8) << "  " << filename << endl;
                }
                else {
                    delete_srcrom();
                    memmove(&src_rom[0], &head_ami_intel.packed_length, 4);
                    memmove(&src_rom[4], &head_ami_intel.unpacked_length, 4);
                    memmove(&src_rom[8], &rom[head_ami_intel.source - 0xfff00000], head_ami_intel.packed_length);
                    unzip(&src_rom);
                    cout << endl;
                }
            }
            intel_tabel_ofs += sizeof(head_ami_intel);
        } while (!false);
    }    // intel

  // single block
    position_l = logic_start;
    while (strncmp(&rom[position_l], "Uª", 2) == 0) {
        l = rom[position_l + 2] * 512;
        filename = string(int2hex(position_l, 8)) + erw;
        cout << ':' << int2hex(position_l, 8) << " " << int2hex(l, 4) << " ????:????  T=??" << string(16, ' ');
        save(&rom[position_l], l);
        cout << " => " << int2hex(l, 8) << " " << filename;
        position_l += l;
    }

    while ((position_l < 0xfe000) && ((position_l & 0xffff) == 0)) {
        for (char i = 0; i <= 255; i++)
            if (i == rom[position_l]) {
                for (counter = 1; counter <= 4096 - 1; counter++)
                    if (rom[position_l + (long)counter] != rom[position_l])  break;
                position_l += 4096;
            }
    }

    while ((position_l & 0xffff) != 0) {
        for (int i = 0; i <= 255; i++)
            if (i == rom[position_l])
                position_l += 1;
    }
    memmove(&head_1994, &rom[position_l + 0x10], sizeof(head_1994));

    if (test_archive_or_amibios(head_1994))
        position_l += 0x10;
    memmove(&head_1994, &rom[position_l], sizeof(head_1994));

    if (!test_archive_or_amibios(head_1994))
    {
        cout << "File is not an AMIBIOS!" << endl;
        exit(1);
    }

    cout << "header error 6!" << endl;
    do {
        memmove(&head_1994, &rom[position_l], sizeof(head_1994));

        if (head_1994.packed_length == amib_longint) { // 'ae5d.rom'
            position_l += 0x10;
            continue;
        }

        if (!test_archive_or_amibios(head_1994)) {
            position_l = (position_l & 0xfffffff0) + 0x10;
            memmove(&head_1994, &rom[position_l], sizeof(head_1994));
            do {
                // file end
                if (position_l >= 0XFFFFF - 0x10)
                    exit(0);

                // check
                memmove(&head_1994, &rom[position_l], sizeof(head_1994));
                if (head_1994.packed_length == amib_longint) {
                    position_l += 0x10;
                    continue;
                }

                /*if (test_archive_or_amibios(head_1994))
                    flush();*//*

                    // remainders
                if ((head_1994.packed_length != 0xffffffff)
                    && (head_1994.packed_length != 0)) {
                    position_l = (position_l & 0xffff0000) + 0x10000;
                    continue;
                }
                position_l += 0x10;
            } while (!false);
        }
        cout << ':' << int2hex(position_l, 8) << "  " <<
            int2hex(head_1994.packed_length, 4) << "  ????:????  T=??" << string(16, ' ') << "-> ";

        filename = int2hex(position_l, 8);
        filename[1] = 'r';
        filename += erw;
        delete_srcrom();
        memmove(&src_rom, &rom[position_l], head_1994.packed_length);
        unzip(&src_rom);
        cout << endl;
        position_l += head_1994.packed_length; // 4+4+

        //if (position_l >= 0xFFFFF - 0x10)  flush();

        memmove(&head_1994, &rom[position_l], sizeof(head_1994));
        if (((head_1994.packed_length & 0xff000000) != 0)
            && ((head_1994.unpacked_length & 0xff000000) != 0))
            position_l += 4 + 4;

    } while (!(position_l >= 0XFFFFF - 0x10));
    exit(0);*/
    return 0;
}