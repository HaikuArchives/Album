/**
\file exif.h
\author Matjaž Kovač
\brief EXIF Tag Extraction

Copyright (c) 2005-2006 by Matjaz Kovac

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#ifndef EXIF_H_
#define EXIF_H_

#ifdef __HAIKU__
#define __BEOS__
#endif

/* Integer type casting */
#ifndef __BEOS__

#define uint16 unsigned short
#define int16 short
#define uint32 unsigned int
#define int32 int
#define uint8 unsigned char

#if BYTE_ORDER == LITTLE_ENDIAN
#define ntohs(A) ((((uint16)(A) & 0xff00) >> 8) | (((uint16)(A) & 0x00ff) << 8))
#define ntohl(A) ((((uint32)(A) & 0xff000000) >> 24) | (((uint32)(A) & 0x00ff0000) >> 8)  | (((uint32)(A) & 0x0000ff00) << 8)  | (((uint32)(A) & 0x000000ff) << 24))
#else
#define ntohs(A) (A)
#define ntohl(A) (A)
#endif

#else
#include <SupportDefs.h>
#include <ByteOrder.h>
#endif





/* EXIF data formats */
enum EXIF_FORMAT {
    FORMAT_BYTE = 1,
    FORMAT_STRING = 2,
    FORMAT_USHORT = 3,
    FORMAT_ULONG = 4,
    FORMAT_URATIONAL = 5,
    FORMAT_SBYTE = 6,
    FORMAT_UNDEFINED = 7,
    FORMAT_SSHORT = 8,
    FORMAT_SLONG = 9,
    FORMAT_SRATIONAL = 10,
    FORMAT_SINGLE = 11,
    FORMAT_DOUBLE = 12
};

enum EXIF_TAGS {
    EXIF_INTEROPERABILITY_INDEX = 0x0001,
    EXIF_INTEROPERABILITY_VERSION = 0x0002,
    EXIF_NEW_SUBFILE_TYPE = 0x00fe,
    EXIF_IMAGE_WIDTH = 0x0100,
    EXIF_IMAGE_LENGTH = 0x0101,
    EXIF_BITS_PER_SAMPLE = 0x0102,
    EXIF_COMPRESSION = 0x0103,
    EXIF_PHOTOMETRIC_INTERPRETATION = 0x0106,
    EXIF_FILL_ORDER = 0x010a,
    EXIF_DOCUMENT_NAME = 0x010d,
    EXIF_IMAGE_DESCRIPTION = 0x010e,
    EXIF_MAKE = 0x010f,
    EXIF_MODEL = 0x0110,
    EXIF_STRIP_OFFSETS = 0x0111,
    EXIF_ORIENTATION = 0x0112,
    EXIF_SAMPLES_PER_PIXEL = 0x0115,
    EXIF_ROWS_PER_STRIP = 0x0116,
    EXIF_STRIP_BYTE_COUNTS = 0x0117,
    EXIF_X_RESOLUTION = 0x011a,
    EXIF_Y_RESOLUTION = 0x011b,
    EXIF_PLANAR_CONFIGURATION = 0x011c,
    EXIF_RESOLUTION_UNIT = 0x0128,
    EXIF_TRANSFER_FUNCTION = 0x012d,
    EXIF_SOFTWARE = 0x0131,
    EXIF_DATE_TIME = 0x0132,
    EXIF_ARTIST = 0x013b,
    EXIF_WHITE_POINT = 0x013e,
    EXIF_PRIMARY_CHROMATICITIES = 0x013f,
    EXIF_TRANSFER_RANGE = 0x0156,
    EXIF_SUB_IFDS = 0x014a,
    EXIF_JPEG_PROC = 0x0200,
    EXIF_JPEG_INTERCHANGE_FORMAT = 0x0201,
    EXIF_JPEG_INTERCHANGE_FORMAT_LENGTH = 0x0202,
    EXIF_YCBCR_COEFFICIENTS = 0x0211,
    EXIF_YCBCR_SUB_SAMPLING = 0x0212,
    EXIF_YCBCR_POSITIONING = 0x0213,
    EXIF_REFERENCE_BLACK_WHITE = 0x0214,
    EXIF_XML_PACKET = 0x02bc,
    EXIF_RELATED_IMAGE_FILE_FORMAT = 0x1000,
    EXIF_RELATED_IMAGE_WIDTH = 0x1001,
    EXIF_RELATED_IMAGE_LENGTH = 0x1002,
    EXIF_CFA_REPEAT_PATTERN_DIM = 0x828d,
    EXIF_CFA_PATTERN = 0x828e,
    EXIF_BATTERY_LEVEL = 0x828f,
    EXIF_COPYRIGHT = 0x8298,
    EXIF_EXPOSURE_TIME = 0x829a,
    EXIF_FNUMBER = 0x829d,
    EXIF_IPTC_NAA = 0x83bb,
    EXIF_IMAGE_RESOURCES = 0x8649,
    EXIF_IFD_POINTER = 0x8769,
    EXIF_INTER_COLOR_PROFILE = 0x8773,
    EXIF_EXPOSURE_PROGRAM = 0x8822,
    EXIF_SPECTRAL_SENSITIVITY = 0x8824,
    EXIF_GPS_INFO_IFD_POINTER = 0x8825,
    EXIF_ISO_SPEED_RATINGS = 0x8827,
    EXIF_OECF = 0x8828,
    EXIF_EXIF_VERSION = 0x9000,
    EXIF_DATE_TIME_ORIGINAL = 0x9003,
    EXIF_DATE_TIME_DIGITIZED = 0x9004,
    EXIF_COMPONENTS_CONFIGURATION = 0x9101,
    EXIF_COMPRESSED_BITS_PER_PIXEL = 0x9102,
    EXIF_SHUTTER_SPEED_VALUE = 0x9201,
    EXIF_APERTURE_VALUE = 0x9202,
    EXIF_BRIGHTNESS_VALUE = 0x9203,
    EXIF_EXPOSURE_BIAS_VALUE = 0x9204,
    EXIF_MAX_APERTURE_VALUE = 0x9205,
    EXIF_SUBJECT_DISTANCE = 0x9206,
    EXIF_METERING_MODE = 0x9207,
    EXIF_LIGHT_SOURCE = 0x9208,
    EXIF_FLASH = 0x9209,
    EXIF_FOCAL_LENGTH = 0x920a,
    EXIF_SUBJECT_AREA = 0x9214,
    EXIF_TIFF_EP_STANDARD_ID = 0x9216,
    EXIF_MAKER_NOTE = 0x927c,
    EXIF_USER_COMMENT = 0x9286,
    EXIF_SUB_SEC_TIME = 0x9290,
    EXIF_SUB_SEC_TIME_ORIGINAL = 0x9291,
    EXIF_SUB_SEC_TIME_DIGITIZED = 0x9292,
    EXIF_FLASH_PIX_VERSION = 0xa000,
    EXIF_COLOR_SPACE = 0xa001,
    EXIF_PIXEL_X_DIMENSION = 0xa002,
    EXIF_PIXEL_Y_DIMENSION = 0xa003,
    EXIF_RELATED_SOUND_FILE = 0xa004,
    EXIF_INTEROP_IFD_POINTER = 0xa005,
    EXIF_FLASH_ENERGY = 0xa20b,
    EXIF_SPATIAL_FREQ_RESPONSE = 0xa20c,
    EXIF_FOCAL_PLANE_X_RESOLUTION = 0xa20e,
    EXIF_FOCAL_PLANE_Y_RESOLUTION = 0xa20f,
    EXIF_FOCAL_PLANE_RESOLUTION_UNIT = 0xa210,
    EXIF_SUBJECT_LOCATION = 0xa214,
    EXIF_EXPOSURE_INDEX = 0xa215,
    EXIF_SENSING_METHOD = 0xa217,
    EXIF_FILE_SOURCE = 0xa300,
    EXIF_SCENE_TYPE = 0xa301,
    EXIF_NEW_CFA_PATTERN = 0xa302,
    EXIF_CUSTOM_RENDERED = 0xa401,
    EXIF_EXPOSURE_MODE = 0xa402,
    EXIF_WHITE_BALANCE = 0xa403,
    EXIF_DIGITAL_ZOOM_RATIO = 0xa404,
    EXIF_FOCAL_LENGTH_IN_35MM_FILM = 0xa405,
    EXIF_SCENE_CAPTURE_TYPE = 0xa406,
    EXIF_GAIN_CONTROL = 0xa407,
    EXIF_CONTRAST = 0xa408,
    EXIF_SATURATION = 0xa409,
    EXIF_SHARPNESS = 0xa40a,
    EXIF_DEVICE_SETTING_DESCRIPTION = 0xa40b,
    EXIF_SUBJECT_DISTANCE_RANGE = 0xa40c,
    EXIF_IMAGE_UNIQUE_ID = 0xa420,
    EXIF_GAMMA = 0xa500,
};


#ifdef __cplusplus
extern "C" {
#endif



/* Tag data structures */
typedef struct {
        int id;
        char *name;
} exif_description_t;


typedef struct {
        uint16 number;
        uint16 format;
        uint32 components;
        uint32 value;
        uint16 size;
        char *data;
} exif_tag_t;


typedef struct {
        void *data;
        uint32 ofs;
        uint32 next;
        uint32 sub;
        int16 size;
        char align;
} exif_ifd_t;


/* Look-up tables */
extern exif_description_t exif_tag_name[];


/* Function prototypes */
char* exif_descr(exif_description_t *descr, int key);
char* exif_str(char *s, exif_tag_t *tag);
int read_ifd(exif_ifd_t* ifd, void *data, int ofs);
int read_ifd_tag(exif_ifd_t* ifd, exif_tag_t *tag, void *data, int ofs);


#ifdef __cplusplus
}
#endif


#endif
