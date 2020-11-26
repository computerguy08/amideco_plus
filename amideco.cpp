#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>

using namespace std;
/* AMIDECO rewritten in C++ by computeguy08 */

/* == CONSTANTS ===*/
const char date[] = "1998.03.31..2003.11.24";
const char erw[] = ".dec";
const int amib_longint = 67 + 77 << 8 + 73 << 16 + 66 << 24;
const char known_block_type[64][20] =
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
 "TDSS" };                        /* 3f */ /* SSDT */

/* == VARIABLES ===*/

ifstream fin;
bool new_baseaddr_required, present, mix_used = false;
char src_rom[255], unpacked[255], src_directory[255], src_name[255], src_extension[255],
rom[1048575], mixf0000[65535], char8[8];
string zk, target_dir, filename;
unsigned short int counter, target_seg, target_ofs;
short int block_count;
int min_loadaddress = 0x100000, filecount = 0;
long int file_length, logic_start, position_amibiosc, position_l, intel_tabel_ofs, loadaddress, l, o, oj, p;

/* == STRUCTURES ===*/

struct start {
    long int packed_length;
    long int unpacked_length;
    char date;
}start_const1;

struct pointer_rec {
    short int ofs, seg;
}position, * pointer_rec_z;

struct {
    struct pointer_rec next_block;         /* 00 */
    short int packed_length;        /* 04 */
    char b6;                        /* 06 */
    char b7;                        /* 07 */
    struct pointer_rec dest_address;       /* 08 */
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
    short int w0;                 /* $FFFF */
    short int typ;
    long int source;
    long int packed_length;
    long int unpacked_length;
    long int target;
    long int l14;
} head_ami_intel;

/* == FUNCTIONS ===*/

char* int2hex(long int number, char n)
{
    char result[32];
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
long int issue_address(const long int issue) {
    string solution;
    long int result, control;
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
        else
            val(solution, result, control);

    } while (control != 0);
    return result;
}
void save(char* a, long int number) {
    ofstream fout(target_dir + filename);
    Rewrite(d2, 1);
    BlockWrite(d2, a, number);
    Close(d2);

}

void delete_srcrom() {
    memset(src_rom, 0, $20000);
}

void unzip(void* source) {

    struct start with = start_const1;
    memset(unpacked, 0xcc, with.unpacked_length);

    if (unzip_lzh5(with.date, unpacked, with.unpacked_length, with.packed_length)) {
        save(unpacked, with.unpacked_length);
        cout << int2hex(with.unpacked_length, 8) << "  " << filename;
    }
    else
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


void blockread1l(void* puffer, long int l, int length)
{
    move(rom[l], puffer, length);
}

void blockread1(void* puffer, int seg_, int ofs_, int laenge)
{
    blockread1l(puffer, (long int)(seg_) << 4 + ofs_, laenge);
}

string block_type(char t)
{
    string tmp = "";
    if (set::of(range(low(known_block_type), high(known_block_type)), eos).has(t))
        tmp = known_block_type[t];
    if (tmp == "")
        tmp = int2hex(head.b6, 2) + '?';
    while (tmp.length() < 20)  tmp += ' ';
    return tmp;
}

string find_extension(char t) {
    string tmp;
    switch (t) {
    case 0xc:tmp = ".ver"; break;
    case 0x20:tmp = ".pci"; break;
    case 0x31:tmp = ".oem"; break;
    case 0x36:tmp = ".vir"; break;
    default:tmp = erw;
    }
    return tmp;
}

void open_file()
{
    cout << filename << " ";
    fin.open(filename);
    filecount += 1;
}

struct ibm_head ibm_header_const2 = rom[0xe0000];
struct ibm_head ibm_header_const3 = { rom[0xe0000] };
struct ibm_head ibm_header_const4 = { rom[0xe0000] };
struct ibm_head ibm_header_const5 = { rom[0xe0000] };
struct ibm_head ibm_header_const6 = { rom[0xe0000] };
struct ibm_head ibm_header_const7 = { rom[0xe0000] };
struct ibm_head ibm_header_const8 = { rom[oj] };
struct ibm_head ibm_header_const9 = { rom[oj] };
struct ibm_head ibm_header_const10 = { rom[oj & 0xff0000] };

void try_unpack(long int o)
{
    blockread1l(&head_1994, o, sizeof(head_1994));
    if (!test_archive_or_amibios(head_1994))
        return;
    cout << ":" << int2hex(o, 8) << " " << int2hex(head_1994.packed_length, 4) << " ????:????  T=??" << format("", 16) << "->";
    filename = int2hex(o, 8);
    filename[1] = 'r';
    filename = filename + erw;
    delete_srcrom();
    blockread1l(&src_rom, o, head_1994.packed_length);
    unzip(&src_rom);
    cout << endl;
}

int main(int argc, const char* argv[])
{
    cout << "AMIDECO * V.K * %s" << date;
    if (argc < 1)
    {
        cout << "parameter error!" << endl;
        exit(1);
    }
    cout << endl;
    getmem(src_rom, 1024 * 1024);
    getmem(unpacked, 1024 * 1024);
    memset(mixf0000, 0xcc, sizeof(mixf0000));
    filename = argv[1];

#ifdef DEBUG*
    filename = "C:\PCI546N3.ROM";
#endif

    fsplit(filename, src_directory, src_name, src_extension);
    open_file();
    file_length = filesize(d1);

    if (file_length > 1 * 1024 * 1024) {
        close(d1);
        cout << "File length is too big: " << file_length;
        exit(1);
    }

    memset(rom, 0, sizeof(rom));
    logic_start = 0x100000;
    new_baseaddr_required = true;

    do {
        seek(d1, 0);
        blockread(d1, ami_flash_head, sizeof(ami_flash_head));
        if (strcmp(ami_flash_head.signature, "FLASH") == 0)
        {
            if (new_baseaddr_required)
            {
                logic_start -= ami_flash_head.logical_area_size;
                if (filecount > 1)
                    logic_start = logic_start & 0xffff0000;
                new_baseaddr_required = false;
            }

            seek(d1, filesize(d1) - ami_flash_head.size_of_image_chunk);
            loadaddress = issue_address(logic_start + ami_flash_head.offset_in_image);
            if (loadaddress < min_loadaddress)
                min_loadaddress = loadaddress;
            blockread(d1, rom[loadaddress], ami_flash_head.size_of_image_chunk);
            close(d1);
            /*WriteLn;*/

            if ((strcmp(ami_flash_head.logical_area_name, "Boot Block") == 0)
                && (ami_flash_head.last_file_in_chain == 0xff)) {
                ami_flash_head.last_file_in_chain = 0;
                strcpy(ami_flash_head.filename_of_next_file, strcat(src_name, ".BIO"));
                new_baseaddr_required = true;
            }

            if (ami_flash_head.last_file_in_chain == 0xff) {
                file_length = 0;
                logic_start = min_loadaddress;
                flush();
            }
            filename = strcat(src_directory, ami_flash_head.filename_of_next_file);
            open_file();

        }
        else /* normal single file */
        {
            logic_start -= file_length;
            seek(d1, 0);
            blockread(d1, rom[logic_start], file_length);
            close(d1);
            cout << endl;
            flush();
        }

    } while (!false);


    target_dir = fexpand(paramstr(2));
    if (!(set::of('\\', '/', eos).has(target_dir[target_dir.length()])))
        target_dir += syspathsep;

    mkdir_verschachtelt(target_dir);

#ifdef DEBUG*
    filename = "BIOS.ROM";
    save(&rom, sizeof(rom));
#endif


    /*** IBM without AMIBIOSC *******************************************/
    if ((strncmp(@rom[$fe008], "COPR. IBM 1981", strlen("COPR. IBM 1981")) == 0)
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
                if ((ibm_header.blocklength == 0) && (with.ami_ofs == 0) && (with.ami_seg == 0))
                    cout << "." << endl;
                else
                    if ((set::of('1', '2', eos).has(chr(rom[oj + long int(4)])))
                        && (set::of('0', '9', eos).has(chr(rom[oj + long int(7)]))))
                        cout << copy(char8::make(rom[oj + long int(0)]), 1, 8) << endl;
                    else
                        cout << "?" << endl;
                // skip to next 64KB block
                oj = (o + (long int)(0xffff)) & 0xffff0000;
                o = oj + (long int)(2 * sizeof(ibm_header));
                continue;
            }
            struct ibm_header with = ibm_header_const9;
            / p = (oj and $ffff0000) + (ami_seg - $e000) shl 4 + ami_ofs;
            filename = int2hex(p, 8) + erw;
            cout << "--> " << int2hex(with.ami_seg, 4) << ':' << int2hex(with.ami_ofs, 4) << ',' << int2hex(blocklength, 8);
            if (with.ami_ofs > 0) {
                cout << "  lzh5: " << int2hex(head_1994_type(rom[p]).packed_length, 8) << "->";
                unzip(&rom[p]);
                cout << endl;
                o = max(o, p + blocklength);
            }
            else {
                cout << "  none: " << int2hex(blocklength, 8);
                save(&rom[p], blocklength);
                cout << "=>" << int2hex(blocklength, 8) << "  " << filename << endl;
                o = max(o, p + blocklength);
            }
            oj += sizeof(ibm_header);
            /* do not interpret packed data as ibm blocks */
            ibm_header& with = ibm_header_const10;
            if ((with.ami_seg == 0xe000)
                && (with.ami_ofs > 0) /* contains unpacked data IBM-Block ..*/
                && ((oj & 0xffff) >= with.ami_ofs)) {
                // skip to next 64KB block
                oj = (o + (long int)(0xffff)) & 0xffff0000;
                o = oj + (long int)(2 * sizeof(ibm_header));
            }
        } while (!false);
    }

    /*** Intel without AMIBIOSC *****************************************/
    intel_tabel_ofs = meml[ofs(rom[0xfe000])];

    if ((file_length == 0) && (intel_tabel_ofs >= 0xfffe0000) && (intel_tabel_ofs < 0xffffe000)) {
        cout << " header error 3!";
        do {
            blockread1l(&head_ami_intel, intel_tabel_ofs - 0xfff00000, sizeof(head_ami_intel));
            {
                if ((packed_length == 0) || (packed_length == 0xffffffff))
                    exit(0);

                if (source == 0xffffffff) {
                    intel_tabel_ofs += sizeof(head_ami_intel);
                    continue;
                }

                if (unpacked_length == 0) {   /* language module at FFFe8800 */
                    packed_length = meml[ofs(rom) + (source & 0xfffff) + 0];
                    unpacked_length = meml[ofs(rom) + (source & 0xfffff) + 4];
                    source += 4 + 4;
                }

                cout << ':' << int2hex(source, 8) << "  " << int2hex(packed_length, 4) << "  :" <<
                    int2hex(ziel, 8) << "  T=" << int2hex(typ, 4) << format("", 17);
                filename = int2hex(source, 8) + erw;

                if (packed_length == unpacked_length) {
                    save(&rom[source - 0xfff00000], unpacked_length);
                    cout << int2hex(head_1994.unpacked_length, 8) << "  " << filename << endl;
                }
                else {
                    delete_srcrom();
                    move(packed_length, src_rom[0], 4);
                    move(unpacked_length, src_rom[4], 4);
                    move(rom[source - 0xfff00000], src_rom[8], packed_length);
                    unzip(&src_rom);
                    cout << endl;
                }
            }
            intel_tabel_ofs += sizeof(head_ami_intel);
        } while (!false);
    }    /* intel */


  /*** 1995+ with AMIBIOSC ******************************************/
    position_amibiosc = 0x100000;
    while (position_amibiosc > 0) {
        if (strncmp(@rom[position_amibiosc - (8 + 4 + 6 + 4)], "AMIBIOSC0", strlen("AMIBIOSC0")) == 0)
            flush();
        else
            position_amibiosc -= 0x800;
    }

    if (position_amibiosc > 0) {
        zk[0] = chr(strlen("AMIBIOSC0627"));
        blockread1l(&zk[1], position_amibiosc - long int((8 + 4 + 6 + 4)), atoi(zk[0]));
        cout << '"' << zk << '"' << endl;
        blockread1l(&position, position_amibiosc - long int(4), sizeof(position));
    }

    if (position_amibiosc == 0) {
        /* AMIBOOT ROM at FFFFFF40/44/50/54 */
        /* 7VRX.F1: AMIBOOT ROM at FFFFe000/04, $SIP at FFFFffd4 */
        if ((strncmp(@rom[$Fff40], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || (strncmp(@rom[$Fff44], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || (strncmp(@rom[$Fff50], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || (strncmp(@rom[$Fff54], "AMIBOOT ROM", strlen("AMIBOOT ROM")) == 0)
            || ((strncmp(@rom[$Fffd4], "$SIP", strlen("$SIP")) == 0)
                && ((pLongint(@rom[$Fffd0]) ^ and$fff80000) == 0xfff80000))) {

            position_amibiosc = sizeof(rom) - $10000 + (pLongint(@rom[$Fffa0]) ^ and$ffff);
            if (strncmp(@rom[position_amibiosc], "AMIBIOSC", strlen("AMIBIOSC")) == 0) {
                zk[0] = chr(strlen("AMIBIOSC0627"));
                blockread1l(&zk[1], position_amibiosc, ord(zk[0]));
                cout << '"' << zk << '"' << endl;
                blockread1l(&position, position_amibiosc + $12, sizeof(position));
            }
            else
                position_amibiosc = 0;
        }
    }

    if (position_amibiosc > 0)
    {
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
                cout << int2hex(packed_length, 4) <<
                    "  " << int2hex(ziel_adresse.seg, 4) <<
                    ':' << int2hex(ziel_adresse.ofs, 4) <<
                    "  " << block_typ(b6);

                delete_srcrom();
                blockread1(&src_rom, position.seg, position.ofs, long int(sizeof(head) + packed_length));

                if ((ziel_adresse.seg == 0) && (ziel_adresse.ofs == 0))
                {
                    filename = int2hex((long int)(position.seg) << 4 + position.ofs, 8);
                    filename[1] = 'r';
                }
                else
                    filename = int2hex((long int)(ziel_adresse), 8);
                filename += find_extension(b6);

                if ((b7 & 0x80) == 0x80)
                {     /* not compressed */
                    cout << "=> ";
                    save(&src_rom[4 + 4 + 4], packed_length);
                    cout << int2hex(packed_length, 8) << "  " << filename;
                }
                else
                {
                    cout << "-> ";
                    unzip(&src_rom[4 + 4 + 4]);

                    /* runtime/post */
                    if ((ziel_adresse.seg >= 0xf000) && (set::of(0, 2, eos).has(b6)))
                    {

                        Write(' +mix');
                        Move(unpacked^,
                            mixf0000[(ziel_adresse.seg - $f000) shr 4 + ziel_adresse.ofs],
                            unpacked_length);
                        mix_used = true;
                    }
                }
                cout << endl;
                position = head.next_block;
            }    /* with head */

        } while (!(position.seg == 0xffff));

        if (mix_used) {
            filename = string("MIX") + erw;
            save(&mixf0000, sizeof(mixf0000));
        }

        if (position_amibiosc <= 0xf8000)
            entpack_versuch(0xf8000);
        if (position_amibiosc <= 0xf0000)
            entpack_versuch(0xf0000);

        exit(0);
    }

    /*** AMIBIOSC at E000:0000 **************************************/

    if (file_length >= 128 * 1024)
    {
        zk[0] = chr(8 + 8);
        blockread1(&zk[1], 0xe000, 0, ord(zk[0]));
        blockread1(&block_count, 0xe000, 0x10, sizeof(block_count));
    }
    else
        zk = "";


    if ((copy(zk, 1, 8) == "AMIBIOSC") && (block_count < 30)) { /* 1994; no Z, but already data? */
        cout << '"' << zk << '"' << endl;
        present = false;  /* F000:1000 not found yet */
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

            zielofs = reference_1994.zielofshi << 8;
            Write(int2hex(position.seg, 4), ':', int2hex(position.ofs, 4), '  ',
                int2hex(head_1994.packed_length, 4), '  ????:',
                int2hex(zielofs, 4), '  T=', int2hex(reference_1994.b0, 2), '':16);

            zk[0] = chr(8);
            move(head_1994, zk[1], ord(zk[0]));

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

                if (set::of(0, 2, eos).has(reference_1994.b0))  /* post/runtime */
                    move(unpacked, mixf0000[zielofs], head_1994.unpacked_length);
            }

        }    /* FOR */


        if (~present)
        {
            position.ofs = 0x1000;
            position.seg = 0xf000;
            blockread1(&head_1994, position.seg, position.ofs, sizeof(head_1994));
            if (test_archive_or_amibios(head_1994)) {
                target_ofs = 0;
                cout << int2hex(position.seg, 4) << ':' << int2hex(position.ofs, 4) << "  " <<
                    int2hex(head_1994.packed_length, 4) << "  ????:????  T=??" <<
                    format("", 16);
                zk[0] = chr(8);
                move(head_1994, zk[1], ord(zk[0]));

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
            }    /* gltig */
        }    /* dabei */


        filename = string("MIX") + erw;
        save(&mixf0000, sizeof(mixf0000));
        exit(0);
    }    /* amibiosc 1994 */

  /*** Einzelblock *************************************************/
    position_l = logic_start;
    while (strncmp(@rom[position_l], #$55#$aa, 2) == 0) {
        l = rom[position_l + 2] * 512;
        filename = string(int2hex(position_l, 8)) + erw;
        Write(':', Int2Hex(position_l, 8), '  ',
            Int2Hex(l, 4), '  ????:????  T=??',
            '':16);
        save(rom[position_l], l);
        WriteLn('=> ', Int2Hex(l, 8), '  ', filename);
        position_l += l;
    }

    while ((position_l < 0xfe000) && ((position_l & 0xffff) == 0)
        && (set::of(0, 0xff, eos).has(rom[position_l]))) {
        for (counter = 1; counter <= 4096 - 1; counter++)
            if (rom[position_l + long int(counter)] != rom[position_l])  flush();
        position_l += 4096;
    }

    while (((position_l & 0xffff) != 0) && (set::of(0, 0xff, eos).has(rom[position_l])))
        position_l += 1;
    blockread1l(&head_1994, position_l + long int(0x10), sizeof(head_1994));

    if (test_archive_or_amibios(head_1994))
        position_l += 0x10;
    blockread1l(&head_1994, position_l, sizeof(head_1994));

    if (!test_archive_or_amibios(head_1994))
    {
        cout << "File is not an AMIBIOS!" << endl;
        exit(1);
    }

    cout << "header error 6!" << endl;
    do {
        blockread1l(&head_1994, position_l, sizeof(head_1994));

        if (head_1994.packed_length == amib_longint)   /* 'ae5d.rom' */ {
            position_l += 0x10;
            continue;
        }

        if (!test_archive_or_amibios(head_1994)) {
            position_l = (position_l & 0xfffffff0) + 0x10;
            blockread1l(&head_1994, position_l, sizeof(head_1994));
            do {
                /* file end */
                if (position_l >= high(rom) - 0x10)
                    exit(0);

                /* check */
                blockread1l(&head_1994, position_l, sizeof(head_1994));
                if (head_1994.packed_length == amib_longint) {
                    position_l += 0x10;
                    continue;
                }

                if (test_archive_or_amibios(head_1994))
                    flush();

                /* Schrott */
                if ((head_1994.packed_length != 0xffffffff)
                    && (head_1994.packed_length != 0)) {
                    position_l = (position_l & 0xffff0000) + 0x10000;
                    continue;
                }
                position_l += 0x10;
            } while (!false);
        }
        cout << ':' << int2hex(position_l, 8) << "  " <<
            int2hex(head_1994.packed_length, 4) << "  ????:????  T=??" <<
            format("", 16) << "-> ";

        strcpy(filename, int2hex(position_l, 8));
        filename[1] = 'r';
        strcpy(filename, strcat(filename, erw));
        delete_srcrom();
        blockread1l(&src_rom, position_l, head_1994.packed_length);
        unzip(&src_rom);
        cout << endl;
        position_l += head_1994.packed_length; /* 4+4+ */

        if (position_l >= high(rom) - 0x10)  flush();

        blockread1l(&head_1994, position_l, sizeof(head_1994));
        if (((head_1994.packed_length & 0xff000000) != 0)
            && ((head_1994.unpacked_length & 0xff000000) != 0))
            position_l += 4 + 4;

    } while (!(position_l >= high(rom) - 0x10));
    exit(0);
    return 0;
}