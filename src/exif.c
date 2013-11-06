/**
\file exif.c
\author Matjaž Kovač
\brief EXIF Tag Extraction

Copyright (c) 2009 by Matjaz Kovac

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


#include <stdio.h>
#include <string.h>
#include <exif.h>



/* tag name look-up table */
exif_description_t exif_tag_name[] = {
  {   0x100,   "ImageWidth"},
  {   0x101,   "ImageHeight"},
  {   0x102,   "BitsPerSample"},
  {   0x103,   "Compression"},
  {   0x106,   "PhotometricInterpretation"},
  {   0x10A,   "FillOrder"},
  {   0x10D,   "DocumentName"},
  {   0x10E,   "ImageDescription"},
  {   0x10F,   "Make"},
  {   0x110,   "Model"},
  {   0x111,   "StripOffsets"},
  {   0x112,   "Orientation"},
  {   0x115,   "SamplesPerPixel"},
  {   0x116,   "RowsPerStrip"},
  {   0x117,   "StripByteCounts"},
  {   0x11A,   "XResolution"},
  {   0x11B,   "YResolution"},
  {   0x11C,   "PlanarConfiguration"},
  {   0x128,   "ResolutionUnit"},
  {   0x12D,   "TransferFunction"},
  {   0x131,   "Software"},
  {   0x132,   "DateTime"},
  {   0x13B,   "Artist"},
  {   0x13E,   "WhitePoint"},
  {   0x13F,   "PrimaryChromaticities"},
  {   0x156,   "TransferRange"},
  {   0x200,   "JPEGProc"},
  {   0x201,   "ThumbnailOffset"},
  {   0x202,   "ThumbnailLength"},
  {   0x211,   "YCbCrCoefficients"},
  {   0x212,   "YCbCrSubsampling"},
  {   0x213,   "YCbCrPositioning"},
  {   0x214,   "ReferenceBlackWhite"},
  {   0x828D,  "CFARepeatPatternDim"},
  {   0x828E,  "CFAPattern"},
  {   0x828F,  "BatteryLevel"},
  {   0x8298,  "Copyright"},
  {   0x829A,  "ExposureTime"},
  {   0x829D,  "FNumber"},
  {   0x83BB,  "IPTC/NAA"},
  //{   0x8769,  "EXIF Offset"},
  {   0x8773,  "InterColorProfile"},
  {   0x8822,  "ExposureProgram"},
  {   0x8824,  "SpectralSensitivity"},
  {   0x8825,  "GPSInfo"},
  {   0x8827,  "ISOSpeedRatings"},
  {   0x8828,  "OECF"},
  {   0x9000,  "ExifVersion"},
  {   0x9003,  "DateTimeOriginal"},
  {   0x9004,  "DateTimeDigitized"},
  {   0x9101,  "ComponentsConfiguration"},
  {   0x9102,  "CompressedBitsPerPixel"},
  {   0x9201,  "ShutterSpeed"},
  {   0x9202,  "Aperture"},
  {   0x9203,  "Brightness"},
  {   0x9204,  "ExposureBias"},
  {   0x9205,  "MaxAperture"},
  {   0x9206,  "SubjectDistance"},
  {   0x9207,  "MeteringMode"},
  {   0x9208,  "LightSource"},
  {   0x9209,  "Flash"},
  {   0x920A,  "FocalLength"},
  {   0x927C,  "MakerNote"},
  {   0x9286,  "UserComment"},
  {   0x9290,  "SubSecTime"},
  {   0x9291,  "SubSecTimeOriginal"},
  {   0x9292,  "SubSecTimeDigitized"},
  {   0xA000,  "FlashPixVersion"},
  {   0xA001,  "ColorSpace"},
  {   0xA002,  "ExifImageWidth"},
  {   0xA003,  "ExifImageHeight"},
  //{   0xA005,  "InteroperabilityOffset"},
  {   0xA20B,  "FlashEnergy"},
  {   0xA20C,  "SpatialFrequencyResponse"},
  {   0xA20E,  "FocalPlaneXResolution"},
  {   0xA20F,  "FocalPlaneYResolution"},
  {   0xA210,  "FocalPlaneResolutionUnit"},
  {   0xA214,  "SubjectLocation"},
  {   0xA215,  "ExposureIndex"},
  {   0xA217,  "SensingMethod"},
  {   0xA300,  "FileSource"},
  {   0xA301,  "SceneType"},
  {   0xA401,  "CustomRendered"},
  {   0xA402,  "ExposureMode"},
  {   0xA403,  "WhiteBalance"},
  {   0xA404,  "DigitalZoomRatio"},
  {   0xA405,  "FocalLengthIn35mm"},
  {   0xA406,  "SceneCaptureType"},
  {   0xA407,  "GainControl"},
  {   0xA408,  "Contrast"},
  {   0xA409,  "Saturation"},
  {   0xA40A,  "Sharpness"},
  {   0xA40B,  "DeviceSettingDescription"},
  {   0xA40C,  "SubjectDistanceRange"},
  {   0xA500,  "Gamma"},
  {   -1, NULL}
};

/* Tag value lookup tables */
static exif_description_t exif_orientation[] = {
	{ 1,	"Top Left" },
	{ 2,	"Top Right" },
	{ 3,	"Bottom Right" },
	{ 4,	"Bottom Left" },
	{ 5,	"Left Top" },
	{ 6,	"RightTop" },
	{ 7,	"Right Bottom" },
	{ 8,	"Left Bottom" },
	{ -1, NULL}
};


static exif_description_t exif_planar_conf[] = {
	{ 1,	"Chunky Format" },
	{ 2, 	"Planar Format" },
	{-1, NULL}
};


static exif_description_t exif_res_unit[] = {
	{ 2,	"inch" },
	{ 3,	"cm" },
	{-1, NULL},
};


static exif_description_t exif_ycbcr_positionig[] = {
	{ 1,	"Centered" },
	{ 2,	"Co-Sited" },
	{-1, NULL}
};


static exif_description_t exif_exp_prog[] = {
	{ 0,	"Not Defined" },
	{ 1,	"Manual" },
	{ 2,	"Normal" },
	{ 3,	"Aperture Priority" },
	{ 4,	"Shutter Priority" },
	{ 5,	"Creative" },
	{ 6,	"Action" },
	{ 7,	"Portrait Mode" },
	{ 8,	"Landscape Mode" },
	{-1, NULL}
};


static exif_description_t exif_component_conf[] = {
	{ 0,	"None" },
	{ 1,	"Y" },
	{ 2,	"Cb" },
	{ 3,	"Cr" },
	{ 4,	"R" },
	{ 5,	"G" },
	{ 6,	"B" },
	{ 0x030201, "YCbCr" },
	{-1, NULL}
};


static exif_description_t exif_meter_mode[] = {
	{ 0,	"Unknown" },
	{ 1,	"Average" },
	{ 2,	"Center Weighted Average" },
	{ 3,	"Spot" },
	{ 4,	"Multi Spot" },
	{ 5,	"Pattern" },
	{ 6,	"Partial" },
	{ 255,	"Other" },
	{-1, NULL}
};


static exif_description_t exif_light_source[] = {
	{ 0,	"Unknown" },
	{ 1,	"Daylight" },
	{ 2,	"Fluorescent" },
	{ 3,	"Tungsten" },
	{ 4,	"Flash" },
	{ 9,	"Fine Weather" },
	{ 10,	"Cloudy Weather" },
	{ 11,	"Shade" },
	{ 12,	"Daylight Fluorescent" },
	{ 13,	"Day White Fluorescent" },
	{ 14,	"Cool White Fluorescent" },
	{ 15,	"White Fluorescent" },
	{ 17,	"Standard Light A" },
	{ 18,	"Standard Light B" },
	{ 19,	"Standard Light C" },
	{ 20,	"D55" },
	{ 21,	"D65" },
	{ 22,	"D75" },
	{ 23,	"D50" },
	{ 24,	"ISO Studio Tungsten" },
	{ 255,	"Other" },
	{-1, NULL}
};


/* Flash mode bit masks (*not* values) */
static exif_description_t exif_flash[] = {
	{ 0x00,	"No" },
	{ 0x01,	"Yes" },
	{ 0x04, "Return Not Detected" },
	{ 0x06,	"Return Detected" },
	{ 0x08, "Compulsory" },
	{ 0x10,	"Compulsory" },
	{ 0x18,	"Auto" },
	{ 0x20,	"No Flash" },
	{ 0x40,	"Red Eye Reduce" },
	{-1, NULL}
};


/* Color spaces */
static exif_description_t exif_color_space[] = {
	{ 1,	"sRGB" },
	{0xffff, "Uncalibrated" },
	{-1, NULL}
};


/* Image sensor types */
static exif_description_t exif_sensing_method[] = {
	{ 1,	"Not Defined" },
	{ 2,	"One-Chip Color Area" },
	{ 3,	"Two-Chip Color Area" },
	{ 4,	"Three-Chip Color Area" },
	{ 5,	"Color Sequential Area" },
	{ 7,	"Trilinear" },
	{ 8,	"Color Sequential Linear" },
	{-1, NULL}
};


static exif_description_t exif_file_source[] = {
	{ 0,	"Other" },
	{ 1,	"Scanner (Transparent)" },
	{ 2,	"Scanner (Reflex)" },
	{ 3,	"Digital Still Camera" },
	{-1, NULL}
};


static exif_description_t exif_scene_type[] = {
	{ 1,	"Directly Photographed" },
	{-1, NULL}
};


static exif_description_t exif_custom_rendered[] = {
	{ 0,	"Normal" },
	{ 1,	"Custom" },
	{-1, NULL}
};


static exif_description_t exif_exp_mode[] = {
	{ 0,	"Auto" },
	{ 1,	"Manual" },
	{ 2,	"Auto Bracket" },
	{-1, NULL}
};


static exif_description_t exif_white_balance[] = {
	{ 0,	"Auto" },
	{ 1,	"Manual" },
	{-1, NULL}
};


static exif_description_t exif_scene_capture[] = {
	{ 0,	"Standard" },
	{ 1,	"Landscape" },
	{ 2,	"Portrait" },
	{ 3,	"Night Scene" },
	{-1, NULL}
};


/* Gain control levels. */
static exif_description_t exif_gain_ctrl[] = {
	{ 0,	"None" },
	{ 1,	"Low Gain Up" },
	{ 2,	"High Gain Up" },
	{ 3,	"Low Gain Down" },
	{ 4,	"High Gain Down" },
	{-1, NULL}
};


static exif_description_t exif_process[] = {
	{ 0,	"Normal" },
	{ 1,	"Soft" },
	{ 2,	"Hard" },
	{-1, NULL}
};


static exif_description_t exif_saturation[] = {
	{ 0,	"Normal" },
	{ 1,	"Low" },
	{ 2,	"High" },
	{-1, NULL}
};


static exif_description_t exif_subject_dist[] = {
	{ 0,	"Unknown" },
	{ 1,	"Macro" },
	{ 2,	"Close View" },
	{ 3,	"Distant View" },
	{-1, NULL}
};

static exif_description_t exif_compression[] = {
	{ 1,	"None" },
	{ 3,	"CCITT Group 3" },
	{ 4,	"CCITT Group 4" },
	{ 5,	"LZW" },
	{ 6,	"JPEG" },
	{-1, NULL}
};

/* Bytes per component type */
uint8 format_bytes[12] = { 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8 };


/*
  Gets a description string from a look-up table.
  Tables must be NULL-terminated.
*/
char* exif_descr(exif_description_t *descr, int key)
{
    for ( ; descr && descr->id >= 0; descr++)
        if (descr->id == (uint16)key)
            return descr->name;
    return NULL;
}


float rat2float(void *data)
{
    float f = *(int32*)data;
    if (f == 0)
       return 0;
    return f / *(int32*)(data+4);
}


/*
  Get tag entry value as a string.
*/
char* exif_str(char *s, exif_tag_t *tag)
{
    char *val = NULL;
    switch (tag->number) {
        case EXIF_ORIENTATION:
            val = exif_descr(exif_orientation, tag->value);
            break;
        case EXIF_FLASH:
            val = exif_descr(exif_flash, tag->value & 1);
            break;
        case EXIF_RESOLUTION_UNIT:
        case EXIF_FOCAL_PLANE_RESOLUTION_UNIT:
            val = exif_descr(exif_res_unit, tag->value);
            break;
        case EXIF_METERING_MODE:
            val = exif_descr(exif_meter_mode, tag->value);
            break;
        case EXIF_YCBCR_POSITIONING:
            val = exif_descr(exif_ycbcr_positionig, tag->value);
            break;
        case EXIF_PLANAR_CONFIGURATION:
            val = exif_descr(exif_planar_conf, tag->value);
            break;
        case EXIF_COMPONENTS_CONFIGURATION:
            val = exif_descr(exif_component_conf, tag->value);
            break;
        case EXIF_LIGHT_SOURCE:
            val = exif_descr(exif_light_source, tag->value);
            break;
        case EXIF_EXPOSURE_PROGRAM:
            val = exif_descr(exif_exp_prog, tag->value);
            break;
        case EXIF_COMPRESSION:
            val = exif_descr(exif_compression, tag->value);
            break;
        case EXIF_COLOR_SPACE:
            val = exif_descr(exif_color_space, tag->value);
            break;
        case EXIF_SCENE_TYPE:
            val = exif_descr(exif_scene_type, tag->value);
            break;
        case EXIF_EXIF_VERSION:
        case EXIF_FLASH_PIX_VERSION:
            sprintf(s, "%c%c.%c%c", (char)(tag->value>>24), (char)(0xff&tag->value>>16), (char)(0xff&tag->value>>8), (char)(0xff&tag->value));
            return s;
        case EXIF_FILE_SOURCE:
            val = exif_descr(exif_file_source, tag->value);
            break;
        case EXIF_SENSING_METHOD:
            val = exif_descr(exif_sensing_method, tag->value);
            break;
        case EXIF_WHITE_BALANCE:
            val = exif_descr(exif_white_balance, tag->value);
            break;
        case EXIF_CUSTOM_RENDERED:
            val = exif_descr(exif_custom_rendered, tag->value);
            break;
        case EXIF_EXPOSURE_MODE:
            val = exif_descr(exif_exp_mode, tag->value);
            break;
        case EXIF_SCENE_CAPTURE_TYPE:
            val = exif_descr(exif_scene_capture, tag->value);
            break;
        case EXIF_GAIN_CONTROL:
            val = exif_descr(exif_gain_ctrl, tag->value);
            break;
        case EXIF_SATURATION:
            val = exif_descr(exif_saturation, tag->value);
            break;
        case EXIF_SHARPNESS:
        case EXIF_CONTRAST:
            val = exif_descr(exif_process, tag->value);
            break;
        case EXIF_SUBJECT_DISTANCE_RANGE:
            val = exif_descr(exif_subject_dist, tag->value);
            break;
        default:
            if (tag->format == FORMAT_STRING && tag->data)
               strncpy(s, tag->data, tag->size);
            else if (tag->format == FORMAT_URATIONAL || tag->format == FORMAT_SRATIONAL)
                sprintf(s, "%g", rat2float(tag->data));
            else if (tag->format == FORMAT_ULONG || tag->format == FORMAT_SLONG)
                sprintf(s, "%d", tag->value);
            else if (tag->format == FORMAT_USHORT || tag->format == FORMAT_SSHORT)
                sprintf(s, "%hd", (uint16)tag->value);
            else
                return NULL;
            // got something
            return s;
    }
    if (val)
        strcpy(s, val);
    return val;
}


/**
    Scans EXIF IFD bytes.
    If successful, returns an offset to the first tag.
    'ofs' must point to a IFD start.
    'data' must point to TIFF header start
*/
int read_ifd(exif_ifd_t* ifd, void *data, int ofs)
{
    // Sub-IFD offset will be found later with tags.
    ifd->sub = 0;
	ifd->next = 0;
    if (ofs < 8 || ofs > 0xffff )
        return 0;
    ifd->ofs = ofs;
    // TIFF header starts with byte ordering type.
    ifd->align = *(char*)data;
    if (ifd->align != 'M' && ifd->align != 'I')
        return 0;
    // IFD start - number of entries
    ifd->size = *(int16*)(data + ofs);
    if (ifd->align == 'M')
       ifd->size = ntohs(ifd->size);
    if (ifd->size < 1)
        return 0;
    // Next IFD offset is right after tag records.
    ifd->next = *(uint32*)(data + ofs + 2 + ifd->size * 12);
    if (ifd->align == 'M')
       ifd->next = ntohl(ifd->next);
    if (ifd->next > 0xffff || ifd->next < ofs)
    	ifd->next = 0;
    ofs += 2;
    // should point to the first tag
    return ofs;
}


/**
    Scans EXIF tag bytes.
    'ofs' must point to a tag start.
*/
int read_ifd_tag(exif_ifd_t* ifd, exif_tag_t *tag, void *data, int ofs)
{
    tag->number = *(uint16*)(data + ofs);
    tag->format = *(uint16*)(data + ofs + 2);
    tag->components = *(uint32*)(data + ofs + 4);
    tag->value = *(uint32*)(data + ofs + 8);
    tag->data = NULL;
    if (ifd->align == 'M') {
        // Big-endian ordering.
        tag->number = ntohs(tag->number);
        tag->format = ntohs(tag->format);
        tag->components = ntohl(tag->components);
        if (tag->format == FORMAT_USHORT || tag->format == FORMAT_SSHORT)
            tag->value = ntohs(tag->value);
        else if (tag->format != FORMAT_BYTE )
            tag->value = ntohl(tag->value);
    }
    tag->size = tag->components * format_bytes[tag->format - 1];
    //printf("* TAG %X: %d, %d\n", tag->number, tag->format, tag->size);
    if (tag->number == EXIF_IFD_POINTER)
        ifd->sub = tag->value;
    else if (tag->size > 4) 
    {
        // out-of-place values
        tag->data = data + tag->value;
        if (ifd->align == 'M' && (tag->format == FORMAT_SRATIONAL || tag->format == FORMAT_URATIONAL)) {
            *(int32*)(tag->data) = ntohl(*(int32*)(tag->data));
            *(int32*)(tag->data + 4) = ntohl(*(int32*)(tag->data + 4));
        }
    }
    else if (tag->format == FORMAT_STRING) {
    	// a short string
    	tag->data = data + ofs + 8;
    }
    ofs += 12;
    // should point to the next tag
    return ofs;
}
