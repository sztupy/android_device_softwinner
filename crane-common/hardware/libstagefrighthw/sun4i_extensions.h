#ifndef SUN4I_EXTENSIONS_INCLUDE_H
#define SUN4I_EXTENSIONS_INCLUDE_H

enum
{
    OVERLAY_3D_OUT_MODE_TB = 0x0,//top bottom
    OVERLAY_3D_OUT_MODE_FP = 0x1,//frame packing
    OVERLAY_3D_OUT_MODE_SSF = 0x2,//side by side full
    OVERLAY_3D_OUT_MODE_SSH = 0x3,//side by side half
    OVERLAY_3D_OUT_MODE_LI = 0x4,//line interleaved
    OVERLAY_3D_OUT_MODE_CI_1 = 0x5,//column interlaved 1
    OVERLAY_3D_OUT_MODE_CI_2 = 0x6,//column interlaved 2
    OVERLAY_3D_OUT_MODE_CI_3 = 0x7,//column interlaved 3
    OVERLAY_3D_OUT_MODE_CI_4 = 0x8,//column interlaved 4
    OVERLAY_3D_OUT_MODE_LIRGB = 0x9,//line interleaved rgb
    OVERLAY_3D_OUT_MODE_FA = 0xa,//field alternative
    OVERLAY_3D_OUT_MODE_LA = 0xb,//line alternative
    
    OVERLAY_3D_OUT_MODE_NORMAL = 0xFF,//line alternative
};

/* names for setParameter() */
enum {
	OVERLAY_DISP_MODE_2D 		= 0x0,
	OVERLAY_DISP_MODE_3D 		= 0x1,
	OVERLAY_DISP_MODE_ANAGLAGH 	= 0x2,
	OVERLAY_DISP_MODE_ORIGINAL 	= 0x3,
};

/* names for setParameter() */
enum {
    OVERLAY_SETVIDEOPARA    = 5,
    /* set videoplayer play frame overlay parameter*/
    OVERLAY_SETFRAMEPARA    = 6,
    /* get videoplayer play frame overlay parameter*/
    OVERLAY_GETCURFRAMEPARA = 7,
    /* query video blank interrupt*/
    OVERLAY_QUERYVBI        = 8,
    /* set overlay screen id*/
    OVERLAY_SETSCREEN       = 9,
    
    OVERLAY_SHOW            = 10,
    
    OVERLAY_SET3DMODE       = 11,
    OVERLAY_SETFORMAT       = 12,
};

/*****************************************************************************/

typedef enum tag_RepeatField
{
    REPEAT_FIELD_NONE,          //means no field should be repeated

    REPEAT_FIELD_TOP,           //means the top field should be repeated
    REPEAT_FIELD_BOTTOM,        //means the bottom field should be repeated

    REPEAT_FIELD_
}repeatfield_t;

typedef struct tag_VideoInfo
{
    unsigned short              width;          //the stored picture width for luma because of mapping
    unsigned short              height;         //the stored picture height for luma because of mapping
    unsigned short              frame_rate;     //the source picture frame rate
    unsigned short              eAspectRatio;   //the source picture aspect ratio
    unsigned short              color_format;   //the source picture color format
}videoinfo_t;

typedef struct tag_PanScanInfo
{
    unsigned long               uNumberOfOffset;
    signed short                HorizontalOffsets[3];
} panscaninfo_t;


typedef struct tag_VdrvRect
{
    signed short                uStartX;    // Horizontal start point.
    signed short                uStartY;    // Vertical start point.
    signed short                uWidth;     // Horizontal size.
    signed short                uHeight;    // Vertical size.
} vdrvrect_t;

typedef struct tag_LIBOVERLAYPARA
{
    signed char                 bProgressiveSrc;    // Indicating the source is progressive or not
    signed char                 bTopFieldFirst;     // VPO should check this flag when bProgressiveSrc is FALSE
    repeatfield_t               eRepeatField;       // only check it when frame rate is 24FPS and interlace output
    videoinfo_t                 pVideoInfo;         // a pointer to structure stored video information
    panscaninfo_t               pPanScanInfo;
    vdrvrect_t                  src_rect;           // source valid size
    vdrvrect_t                  dst_rect;           // source display size
    unsigned char               top_index;          // frame buffer index containing the top field
    unsigned long               top_y;              // the address of frame buffer, which contains top field luminance
    unsigned long               top_c;              // the address of frame buffer, which contains top field chrominance

    //the following is just for future
    unsigned char               bottom_index;       // frame buffer index containing the bottom field
    unsigned long               bottom_y;           // the address of frame buffer, which contains bottom field luminance
    unsigned long               bottom_c;           // the address of frame buffer, which contains bottom field chrominance

    //time stamp of the frame
    unsigned long               uPts;               // time stamp of the frame (ms?)
    unsigned char		first_frame_flg;
    unsigned long               number;
}liboverlaypara_t; 

#endif
