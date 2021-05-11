// A very simple tool to repackage a DOL into a GCI file, additionally adding an icon
// It adjusts the DOL header for that - this should not break any loader but you never
// know.

#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string>

using namespace std;

// this is only required on DOS-based operating systems which parse the EOF token in files
#ifndef O_BINARY
#define O_BINARY 0
#endif

// useful typedefs
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

// DOL icon (CI8 with palette)
const u8 dol_icon[] = {
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x10, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x0F, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x18, 0x1B, 0x1C, 0x1D, 0x1E, 0x26,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x10, 0x26, 0x1D, 0x1D, 0x1B, 0x18, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x0F, 0x10, 0x10, 0x10, 0x0F, 0x10, 0x0F, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x0F, 0x0F, 0x0F, 0x10, 0x11, 0x18, 0x1D, 0x0F, 0x10, 0x11, 0x18, 0x1E, 0x1F, 0x21, 0x22,
	0x18, 0x19, 0x1E, 0x15, 0x15, 0x15, 0x15, 0x15, 0x14, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
	0x1E, 0x27, 0x28, 0x29, 0x29, 0x2A, 0x2A, 0x2A, 0x20, 0x1F, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
	0x15, 0x15, 0x15, 0x15, 0x15, 0x17, 0x22, 0x22, 0x15, 0x15, 0x15, 0x15, 0x17, 0x22, 0x23, 0x22,
	0x2A, 0x28, 0x29, 0x29, 0x28, 0x1E, 0x1D, 0x1B, 0x22, 0x22, 0x22, 0x29, 0x29, 0x22, 0x21, 0x28,
	0x17, 0x1A, 0x1A, 0x22, 0x20, 0x15, 0x15, 0x1F, 0x1A, 0x15, 0x15, 0x0B, 0x06, 0x03, 0x05, 0x14,
	0x10, 0x10, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x10, 0x27, 0x26, 0x1B, 0x10, 0x0F, 0x10, 0x10, 0x10,
	0x28, 0x28, 0x3B, 0x2E, 0x0F, 0x0F, 0x0F, 0x10, 0x21, 0x28, 0x33, 0x46, 0x1D, 0x0F, 0x0F, 0x10,
	0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
	0x16, 0x16, 0x15, 0x0C, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16, 0x15, 0x15, 0x15, 0x16, 0x17, 0x22,
	0x15, 0x15, 0x16, 0x17, 0x22, 0x24, 0x2A, 0x2A, 0x15, 0x17, 0x22, 0x17, 0x23, 0x2B, 0x34, 0x34,
	0x17, 0x22, 0x23, 0x22, 0x2A, 0x34, 0x3D, 0x3D, 0x2A, 0x2A, 0x2A, 0x23, 0x2A, 0x34, 0x35, 0x3D,
	0x22, 0x22, 0x1F, 0x15, 0x07, 0x04, 0x07, 0x14, 0x3D, 0x49, 0x3C, 0x32, 0x2A, 0x2A, 0x2A, 0x29,
	0x3D, 0x34, 0x22, 0x28, 0x34, 0x3D, 0x47, 0x3D, 0x2C, 0x15, 0x09, 0x0B, 0x15, 0x2A, 0x2A, 0x2A,
	0x1F, 0x28, 0x2A, 0x47, 0x3A, 0x10, 0x10, 0x10, 0x28, 0x28, 0x28, 0x33, 0x46, 0x1B, 0x0F, 0x10,
	0x34, 0x2A, 0x34, 0x3B, 0x46, 0x1B, 0x0F, 0x10, 0x22, 0x16, 0x2A, 0x33, 0x3B, 0x11, 0x0F, 0x10,
	0x16, 0x15, 0x15, 0x15, 0x17, 0x22, 0x24, 0x2C, 0x15, 0x15, 0x16, 0x17, 0x22, 0x24, 0x2B, 0x35,
	0x0C, 0x16, 0x17, 0x24, 0x2A, 0x34, 0x34, 0x34, 0x17, 0x17, 0x24, 0x34, 0x34, 0x3D, 0x34, 0x34,
	0x3D, 0x37, 0x2A, 0x22, 0x2A, 0x34, 0x3D, 0x40, 0x3D, 0x35, 0x23, 0x22, 0x2A, 0x3D, 0x49, 0x47,
	0x34, 0x2B, 0x24, 0x2A, 0x34, 0x47, 0x47, 0x3C, 0x34, 0x2C, 0x2C, 0x3D, 0x50, 0x50, 0x3C, 0x32,
	0x2A, 0x15, 0x09, 0x15, 0x22, 0x22, 0x15, 0x0B, 0x2A, 0x22, 0x28, 0x29, 0x32, 0x2A, 0x28, 0x15,
	0x31, 0x3B, 0x3C, 0x33, 0x39, 0x32, 0x28, 0x1F, 0x32, 0x3C, 0x46, 0x3B, 0x3B, 0x3B, 0x30, 0x28,
	0x15, 0x16, 0x17, 0x17, 0x15, 0x12, 0x10, 0x0F, 0x09, 0x09, 0x0C, 0x0C, 0x0A, 0x17, 0x1E, 0x10,
	0x15, 0x13, 0x09, 0x09, 0x08, 0x0E, 0x2A, 0x10, 0x31, 0x32, 0x33, 0x33, 0x3C, 0x48, 0x31, 0x10,
	0x23, 0x2A, 0x34, 0x34, 0x3D, 0x3D, 0x34, 0x34, 0x35, 0x3D, 0x3D, 0x3D, 0x3D, 0x34, 0x2A, 0x22,
	0x40, 0x40, 0x3D, 0x33, 0x2A, 0x23, 0x22, 0x16, 0x3D, 0x3D, 0x2B, 0x23, 0x17, 0x17, 0x17, 0x17,
	0x2A, 0x17, 0x2B, 0x51, 0x57, 0x46, 0x46, 0x3B, 0x15, 0x0D, 0x49, 0x64, 0x57, 0x50, 0x50, 0x50,
	0x17, 0x3E, 0x65, 0x63, 0x63, 0x60, 0x60, 0x60, 0x49, 0x64, 0x63, 0x63, 0x63, 0x63, 0x62, 0x63,
	0x3C, 0x47, 0x48, 0x45, 0x46, 0x46, 0x3B, 0x38, 0x56, 0x50, 0x56, 0x56, 0x46, 0x38, 0x25, 0x1B,
	0x60, 0x60, 0x63, 0x61, 0x27, 0x10, 0x10, 0x0F, 0x64, 0x65, 0x66, 0x66, 0x33, 0x0F, 0x0F, 0x0F,
	0x38, 0x38, 0x36, 0x2F, 0x2F, 0x25, 0x11, 0x0F, 0x11, 0x10, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x10,
	0x0F, 0x0F, 0x0F, 0x10, 0x10, 0x10, 0x10, 0x10, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x10,
	0x34, 0x2A, 0x23, 0x17, 0x17, 0x17, 0x2C, 0x42, 0x34, 0x2A, 0x22, 0x16, 0x16, 0x2C, 0x4B, 0x58,
	0x2A, 0x22, 0x17, 0x17, 0x34, 0x51, 0x51, 0x63, 0x17, 0x17, 0x24, 0x41, 0x57, 0x60, 0x60, 0x60,
	0x58, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x63, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x62, 0x67, 0x00, 0x67, 0x67, 0x67, 0x00, 0x00, 0x60, 0x67, 0x00, 0x67, 0x00, 0x00, 0x67, 0x00,
	0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x67, 0x67, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00,
	0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x0F,
	0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x2D, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x4A,
	0x24, 0x35, 0x51, 0x61, 0x60, 0x60, 0x60, 0x5E, 0x49, 0x57, 0x60, 0x60, 0x60, 0x5F, 0x5D, 0x53,
	0x60, 0x60, 0x60, 0x60, 0x5D, 0x4F, 0x4A, 0x4D, 0x57, 0x5F, 0x5D, 0x5C, 0x4E, 0x44, 0x43, 0x43,
	0x55, 0x67, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x4E, 0x67, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67,
	0x4D, 0x67, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x4E, 0x67, 0x00, 0x67, 0x00, 0x00, 0x67, 0x00,
	0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00,
	0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x67, 0x00,
	0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x5B, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x5B,
	0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x4C, 0x00, 0x67, 0x00, 0x00, 0x00, 0x00, 0x67, 0x59,
	0x5D, 0x54, 0x52, 0x4D, 0x44, 0x4E, 0x4E, 0x4D, 0x44, 0x43, 0x43, 0x43, 0x43, 0x43, 0x44, 0x44,
	0x3F, 0x3F, 0x43, 0x3F, 0x43, 0x43, 0x43, 0x43, 0x43, 0x43, 0x44, 0x44, 0x4D, 0x4D, 0x43, 0x44,
	0x4E, 0x67, 0x00, 0x67, 0x67, 0x67, 0x00, 0x00, 0x43, 0x67, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x43, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x4E, 0x4D, 0x4D, 0x4E, 0x4E, 0x4E, 0x44, 0x4C,
	0x00, 0x00, 0x00, 0x67, 0x67, 0x67, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x4D, 0x4C, 0x4C, 0x4D, 0x5B, 0x4C, 0x4C, 0x4C,
	0x02, 0x67, 0x67, 0x67, 0x67, 0x00, 0x67, 0x5B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x67, 0x5A,
	0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x67, 0x5B, 0x59, 0x59, 0x5B, 0x5B, 0x5B, 0x5B, 0x5A, 0x5A,
	0xFF, 0xFF, 0xFF, 0xBF, 0xFF, 0x3F, 0xD6, 0xD6, 0xD6, 0xB6, 0xCA, 0x96, 0xC6, 0x94, 0xC6, 0x55,
	0xC6, 0x32, 0xC6, 0x55, 0xC6, 0x31, 0xC2, 0x32, 0xC2, 0x32, 0xC2, 0x11, 0xC1, 0xCE, 0xBA, 0xBC,
	0xBA, 0x9A, 0xBA, 0x5A, 0xBA, 0x36, 0xBA, 0x35, 0xBA, 0x34, 0xBA, 0x32, 0xBA, 0x11, 0xB9, 0xD1,
	0xB6, 0x59, 0xB6, 0x36, 0xB5, 0xD1, 0xB6, 0x39, 0xB6, 0x36, 0xB6, 0x36, 0xB6, 0x35, 0xB6, 0x12,
	0xB6, 0x11, 0xB5, 0xD1, 0xB5, 0xD1, 0xB5, 0xAE, 0xB1, 0xAE, 0xAA, 0x16, 0xAA, 0x15, 0xA9, 0xD1,
	0xA9, 0xD1, 0xA9, 0xB0, 0xA9, 0xAE, 0xA9, 0x8D, 0xA9, 0x4D, 0xA6, 0x39, 0xA5, 0xD5, 0xA1, 0xB4,
	0xA1, 0xB0, 0xA1, 0xB0, 0xA1, 0xAE, 0xA1, 0x4D, 0xA1, 0x4D, 0xA1, 0x2A, 0xA1, 0xB1, 0xA1, 0x0A,
	0x99, 0xB1, 0x99, 0x8D, 0x99, 0x50, 0x99, 0x4E, 0x99, 0x4D, 0x99, 0x0A, 0x98, 0xC8, 0x96, 0x36,
	0x95, 0x08, 0x94, 0xC9, 0x94, 0x86, 0x92, 0x36, 0x92, 0x14, 0x91, 0x2C, 0x91, 0x0D, 0x91, 0x0A,
	0x90, 0xC9, 0x90, 0xA8, 0x91, 0xD4, 0x90, 0x86, 0x8A, 0x16, 0x8A, 0x15, 0x89, 0xD4, 0x89, 0xB1,
	0x88, 0xCA, 0x88, 0x86, 0x85, 0xD2, 0x85, 0xB1, 0x85, 0x50, 0x85, 0x0D, 0x84, 0xA8, 0x84, 0x88,
	0x84, 0x45, 0x82, 0x16, 0x82, 0x15, 0x81, 0xD5, 0x81, 0x50, 0x81, 0x0D, 0x80, 0xA9, 0x80, 0x89,
	0x80, 0x88, 0x80, 0x86, 0x80, 0x48, 0x80, 0x46, 0x80, 0x24, 0x80, 0x04, 0x80, 0x02, 0x80, 0x00
};

// utility functions
int load( const string & name, void ** data )
{
        size_t size = 0;
        *data = NULL;
        int fd = open( name.c_str(), O_RDONLY|O_BINARY, 0 );
        if (fd >= 0) {
                struct stat sb;
                if ( fstat( fd, &sb ) >= 0) {
                        assert( sb.st_size > 0 );
                        void *tmp = malloc( sb.st_size );
                        if ( tmp!=NULL ) {
                                if (read( fd, tmp, sb.st_size ) == sb.st_size) {
                                        *data = tmp;
                                        size = sb.st_size;
                                } else {
                                        free(tmp);
                                }
                        }
                }
                close( fd );
        }
        return size;
}

void save(const string & name, const void *data, int size)
{
        int fd = open( name.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 0666 );
        if (fd >= 0) {
		write( fd, data, size );
                close( fd );
        }
}
 
u32 get_u32be(void const * const buf)
{
	return ((u8*)buf)[3] | (((u8*)buf)[2]<<8) | (((u8*)buf)[1]<<16) | (((u8*)buf)[0]<<24);
}

void set_u16be(void * buf, u16 v)
{
	((u8*)buf)[1] = v & 0xFF;
	((u8*)buf)[0] = (v>>8) & 0xFF;
}

void set_u32be(void * buf, u32 v)
{
	((u8*)buf)[3] = v & 0xFF;
	((u8*)buf)[2] = (v>>8) & 0xFF;
	((u8*)buf)[1] = (v>>16) & 0xFF;
	((u8*)buf)[0] = (v>>24) & 0xFF;
}

int main (int argc, char * const argv[]) 
{
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "dol2gci <dolfile> <gcifile> [<filename>]\n");
		return -1;
	} 

	// strip path from filename, looking for forward and backslash
	string name;
	unsigned int i;
	if (argc == 4) {
		name = argv[3];
	} else {
		name = argv[1];
		i = name.rfind('/');
		if (i!=string::npos) {
			name = name.substr(i+1);
		}
		i = name.rfind('\\');
		if (i!=string::npos) {
			name = name.substr(i+1);
		}
	}
	
	// load DOL file
	u8 *dol_data;
	int dol_size = load(argv[1], (void**)&dol_data);
	if (dol_size < 256) {
		fprintf(stderr, "can't read dol\n");
		return -1;
	} 
	
	// calculate overal size
	int icon_size = ((sizeof(dol_icon) + 31) & ~31);
	int data_size = 64 + icon_size + dol_size;
	int data_blocks = (data_size + 8291) / 8192;
	
	// create GCI file
	int gci_size = 64 + data_blocks * 8192;
	u8 *gci_data = (u8 *)malloc(gci_size);
		
	// set up GCI header
	memcpy(gci_data + 0x00, "DOLX", 4);     // Game Code
	memcpy(gci_data + 0x04, "00", 2);       // maker code
	gci_data[6] = 0xFF;			// unused/reserved
	gci_data[7] = 0x00;			// no banner
	memset(gci_data + 0x08, 0, 32);		// clear name area
	strncpy((char*)(gci_data + 0x08), name.c_str(), 31); // copy name
	set_u32be(gci_data + 0x28, 0);		// last modification
	set_u32be(gci_data + 0x2C, 0x140);      // offset image data
	set_u16be(gci_data + 0x30, 0x0001);     // icon CI8 shared PAL
	set_u16be(gci_data + 0x32, 0x02);       // one frame
	gci_data[0x34] = 0x04;			// public file
	gci_data[0x35] = 0x00;			// copy counter
	set_u16be(gci_data + 0x36, 0x0000);     // first block of file
	set_u16be(gci_data + 0x38, data_blocks); // number of 8k blocks in file
	set_u16be(gci_data + 0x3a, 0xffff);     // unused/reserved
	set_u32be(gci_data + 0x3c, 0x100);      // comment
	
	// copy and fix DOL header
	memcpy(gci_data + 0x40, dol_data, 256);
	for (i=0; i<18; i++) {
		u32 adr = 0x40 + i*4;
		u32 filepos = get_u32be(gci_data + adr);
		if (filepos >= 0x100) {
			set_u32be(gci_data + adr, filepos + 0x40 + icon_size);
		}
	}
		
	// description and icon
	memset(gci_data + 0x140, 0, 64);
	strcpy((char*)(gci_data + 0x140), "Dolphin Application");
	strncpy((char*)(gci_data + 0x160), name.c_str(), 31);
	memcpy(gci_data + 0x180, dol_icon, sizeof(dol_icon));
	
	// copy DOL data
	memcpy(gci_data + 0x180 + icon_size, dol_data + 256, dol_size - 256);
	
	// save GCI file
	save(argv[2], gci_data, gci_size);	
		
	return 0;
}
