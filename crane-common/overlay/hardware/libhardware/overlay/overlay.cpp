/*-------------------------------------------------------------------------
    
-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------
    
-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------
    
-------------------------------------------------------------------------*/
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#define LOG_NDEBUG 0
#define LOG_TAG "Overlay"

#include <hardware/hardware.h>
#include <hardware/overlay.h>
#include <pthread.h>
#include <drv_display_sun4i.h>
#include <OMX_Video.h>

#include <fcntl.h>
#include <errno.h>


#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev.h>

#include <cutils/log.h>
#include <cutils/ashmem.h>
#include <cutils/atomic.h>

#include "linux/fb.h"

#define LOG_FUNCTION_NAME LOGV(" %s %s",  __FILE__, __func__)

#define NUM_OVERLAY_BUFFERS_REQUESTED  (3)
/* OVRLYSHM on phone keypad*/
#define SHARED_DATA_MARKER             (0x68759746)

/* These values should come from Surface Flinger */
unsigned int                    g_lcd_width        = 480;
unsigned int                    g_lcd_height       = 800;
unsigned int                    g_lcd_bpp          = 32;

unsigned long                   args[4];
unsigned long 				    fb_layer_hdl;

#define CACHEABLE_BUFFERS       0x1

/* shared with Camera/Video Playback HAL */
#define ALL_BUFFERS_FLUSHED     -66

uint32_t phyAddr;

typedef struct
{
    uint32_t    posX;
    uint32_t    posY;
    uint32_t    posW;
    uint32_t    posH;
    uint32_t    rotation;
    uint32_t    flip;

    uint32_t    posX_org;
    uint32_t    posY_org;
    uint32_t    posW_org;
    uint32_t    posH_org;
} overlay_ctrl_t;

typedef struct
{
    uint32_t    cropX;
    uint32_t    cropY;
    uint32_t    cropW;
    uint32_t    cropH;
} overlay_data_t;

typedef struct
{
    uint32_t            marker;
    uint32_t            size;

    volatile int32_t    refCnt;

    uint32_t            controlReady; /* Only updated by the control side */
    uint32_t            dataReady;    /* Only updated by the data side */

    pthread_mutex_t     lock;
    pthread_mutexattr_t attr;

    uint32_t            streamEn;
    uint32_t            streamingReset;

    uint32_t            dispW;
    uint32_t            dispH;
    uint32_t			org_dispW;
    uint32_t			org_dispH;
    uint32_t			g_screen;
    uint32_t			g_currenthandle;
    uint32_t			cur_hdmimode;
	uint32_t			cur_3dmode;
	bool				cur_half_enable;
	bool				cur_3denable;
} overlay_shared_t;

/* Only one instance is created per platform */
struct overlay_control_context_t 
{
    struct overlay_control_device_t     device;
    /* our private state goes below here */
    struct overlay_t*                   overlay_video1;
    struct overlay_t*                   overlay_video2;
    struct overlay_t*                   overlay_subtitle;
};

/* A separate instance is created per overlay data side user*/
struct overlay_data_context_t 
{
    struct overlay_data_device_t    device;
    /* our private state goes below here */
    int                             ctl_fd;
    int                             shared_fd;
    int                             shared_size;
    int                             width;
    int                             height;
    int                             format;
    bool							overlay_ready;

    void*							overlayhandle;

    overlay_data_t                  data;
    overlay_shared_t                *shared;
    struct mapping_data             *mapping_data;
    /* Need to count Qd buffers
       to be sure we don't block DQ'ing when exiting */
	overlay_handle_t                overlay_handle;
	uint32_t						cur_frameid;
	
};

static int  create_shared_data          (overlay_shared_t **shared);
static void destroy_shared_data         (int shared_fd, overlay_shared_t *shared,bool closefd);
static int  open_shared_data            (overlay_data_context_t *ctx);
static void close_shared_data           (overlay_data_context_t *ctx);
enum { LOCK_REQUIRED = 1, NO_LOCK_NEEDED = 0 };

static int overlay_device_open          (const struct hw_module_t* module,const char* name, struct hw_device_t** device);

static struct hw_module_methods_t overlay_module_methods = 
{
    open: overlay_device_open
};

struct overlay_module_t HAL_MODULE_INFO_SYM = 
{
	common: 
	{
	tag: HARDWARE_MODULE_TAG,
	     version_major: 1,
	     version_minor: 0,
	     id: OVERLAY_HARDWARE_MODULE_ID,
	     name: "SEC Overlay module",
	     author: "The Android Open Source Project",
	     methods: &overlay_module_methods,
	}
};

/*****************************************************************************/

/*
 * This is the overlay_t object, it is returned to the user and represents
 * an overlay. here we use a subclass, where we can store our own state.
 * This handles will be passed across processes and possibly given to other
 * HAL modules (for instance video decode modules).
 */
struct handle_t : public native_handle 
{
    /* add the data fields we need here, for instance: */
    int     ctl_fd;
    int     shared_fd;
    int     width;
    int     height;
    int     format;
    void*   overlayhandle;
    int     shared_size;
    int		screen_width;
    int     screen_height;
    int     screen_id;
};

static int handle_format(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->format;
}

static int handle_ctl_fd(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->ctl_fd;
}

static int handle_width(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->width;
}

static int handle_shared_fd(const overlay_handle_t overlay) 
{
    return static_cast<const struct handle_t *>(overlay)->shared_fd;
}

static int handle_height(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->height;
}

static void *handle_overlay(const overlay_handle_t overlay)
{
    return ((struct handle_t *)overlay)->overlayhandle;
}

static int handle_screenwidth(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->screen_width;
}

static int handle_screenheight(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->screen_height;
}

static int handle_screenid(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)overlay)->screen_id;
}

static int handle_shared_size(const overlay_handle_t overlay) 
{
    return ((struct handle_t *)(overlay))->shared_size;
}

static void handle_setwidth(overlay_handle_t overlay,int width) 
{
    ((struct handle_t *)overlay)->width = width;
}

static void handle_setheight(overlay_handle_t overlay,int height) 
{
    ((struct handle_t *)overlay)->height = 	height;
}

static void handle_setoverlay(overlay_handle_t overlay,void *overlayhandle)
{
    ((struct handle_t *)overlay)->overlayhandle = overlayhandle;
}

static void handle_setscreenwidth(overlay_handle_t overlay,int screen_width) 
{
    ((struct handle_t *)overlay)->screen_width = screen_width;
}

static void handle_setscreenheight(overlay_handle_t overlay,int screen_height) 
{
    ((struct handle_t *)overlay)->screen_height = screen_height;
}

static void handle_setscreenid(const overlay_handle_t overlay,int screen_id) 
{
    ((struct handle_t *)overlay)->screen_id = screen_id;
}


/* A separate instance of this class is created per overlay */
class overlay_object : public overlay_t
{
    handle_t            mHandle;

    overlay_ctrl_t      mCtl;
    overlay_ctrl_t      mCtlStage;
    overlay_shared_t    *mShared;

    static overlay_handle_t getHandleRef(struct overlay_t* overlay) 
    {
    	overlay_handle_t  ret;

    	ret = &(((overlay_object *)overlay)->mHandle);

    	LOGV("overlay handle = %x\n",(unsigned int)handle_overlay(ret));
    	LOGV("&(static_cast<overlay_object *>(overlay)->mHandle) = %x\n",&(static_cast<overlay_object *>(overlay)->mHandle));
        /* returns a reference to the handle, caller doesn't take ownership */
        return &(((overlay_object *)overlay)->mHandle);
    }

    public:
    overlay_object(int ctl_fd,int shared_fd,int shared_size,int w, int h,int format,void *overlayhdl,int screen_id,int screen_width,int screen_height) 
    {
        this->overlay_t::getHandleRef   = getHandleRef;
        mHandle.version                 = sizeof(native_handle);
        mHandle.numFds                  = 2;
        mHandle.numInts                 = 10; /* extra ints we have in our handle */
        mHandle.ctl_fd                  = ctl_fd;
        mHandle.shared_fd               = shared_fd;
        mHandle.width                   = w;
        mHandle.height                  = h;
        mHandle.format                  = format;
        mHandle.shared_size             = shared_size;
        mHandle.overlayhandle           = overlayhdl;
        mHandle.screen_id				= screen_id;
        mHandle.screen_width			= screen_width;
        mHandle.screen_height			= screen_height;
        this->w                         = w;
        this->h                         = h;
        this->format                    = format;
        this->screenid                  = screen_id;

        memset( &mCtl, 0, sizeof( mCtl ) );
        memset( &mCtlStage, 0, sizeof( mCtlStage ) );
    }

    int               ctl_fd()      		{ return mHandle.ctl_fd; }
    int               shared_fd()   { return mHandle.shared_fd; }
    overlay_ctrl_t*   data()        		{ return &mCtl; }
    overlay_ctrl_t*   staging()        		{ return &mCtlStage; }
    overlay_shared_t* getShared()   { return mShared; }
    void*             getoverlay()  		{ return mHandle.overlayhandle; }
    int				  getscreenwidth()		{ return mHandle.screen_width;	}
    int				  getscreenheight()		{ return mHandle.screen_height;	}
    int				  getscreenid()			{ return mHandle.screen_id;		}
    void              setShared( overlay_shared_t *p ) { mShared = p; }
    
    void  	setscreen(int width,int height,int id)
    {
    	mHandle.screen_width		= width;
    	mHandle.screen_height		= height;
    	mHandle.screen_id			= id;
    }
    
    void   setsize(int width,int height)
    {
    	mHandle.width	= width;
    	mHandle.height	= height;
    }	
    
    void   setoverlay(void *overlayhandle)	
    {
    	mHandle.overlayhandle = overlayhandle;
    }
};

/*****************************************************************************
 *  Local Functions
 *****************************************************************************/

static int create_shared_data(overlay_shared_t **shared)
{
    int fd;
    /* assuming sizeof(overlay_shared_t) < a single page */
    int size = getpagesize();
    overlay_shared_t *p;

    if ((fd = ashmem_create_region("overlay_data", size)) < 0) 
    {
        LOGE("Failed to Create Overlay Shared Data!\n");
        return fd;
    }

    p = (overlay_shared_t*)mmap(NULL, size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) 
    {
        LOGE("Failed to Map Overlay Shared Data!\n");
        close(fd);
        return -1;
    }

    memset(p, 0, size);
    p->marker = SHARED_DATA_MARKER;
    p->size   = size;
    p->refCnt = 1;
    if (pthread_mutexattr_init(&p->attr) != 0) 
    {
        LOGE("Failed to initialize overlay mutex attr");
        goto MutexAttrErr;
    }
    
    if (pthread_mutexattr_setpshared(&p->attr, PTHREAD_PROCESS_SHARED) != 0) 
    {
        LOGE("Failed to set the overlay mutex attr to be shared across-processes");
        goto MutexAttrSetErr;
    }
    
    if (pthread_mutex_init(&p->lock, &p->attr) != 0) 
    {
        LOGE("Failed to initialize overlay mutex\n");
        goto MutexErr;
    }

    *shared = p;
    return fd;

MutexErr:
MutexAttrSetErr:
    pthread_mutexattr_destroy(&p->attr);
MutexAttrErr:
    munmap(p, size);
    close(fd);
    return -1;
}

static void destroy_shared_data(int shared_fd, overlay_shared_t *shared,
                                bool closefd )
{
    if (shared == NULL)
        return;

    /* Last side deallocated releases the mutex, otherwise the remaining */
    /* side will deadlock trying to use an already released mutex */
    if (android_atomic_dec(&shared->refCnt) == 1) 
    {
        if (pthread_mutex_destroy(&shared->lock)) 
        {
            LOGE("Failed to uninitialize overlay mutex!\n");
        }

        if (pthread_mutexattr_destroy(&shared->attr)) 
        {
            LOGE("Failed to uninitialize the overlay mutex attr!\n");
        }
        shared->marker = 0;
    }

    if (munmap(shared, shared->size)) 
    {
        LOGE("Failed to Unmap Overlay Shared Data!\n");
    }

    if (closefd && close(shared_fd)) 
    {
        LOGE("Failed to Close Overlay Shared Data!\n");
    }
}

static int open_shared_data( overlay_data_context_t *ctx )
{
    int rc   = -1;
    int mode = PROT_READ | PROT_WRITE;
    int fd   = ctx->shared_fd;
    int size = ctx->shared_size;

    if (ctx->shared != NULL) 
    {
        /* Already open, return success */
        LOGI("Overlay Shared Data Already Open\n");
        return 0;
    }
    ctx->shared = (overlay_shared_t*)mmap(0, size, mode, MAP_SHARED, fd, 0);

    if (ctx->shared == MAP_FAILED) 
    {
        LOGE("Failed to Map Overlay Shared Data!\n");
    } 
    else if ( ctx->shared->marker != SHARED_DATA_MARKER ) 
    {
        LOGE("Invalid Overlay Shared Marker!\n");
        munmap( ctx->shared, size);
    } 
    else if ( (int)ctx->shared->size != size ) 
    {
        LOGE("Invalid Overlay Shared Size!\n");
        munmap(ctx->shared, size);
    } 
    else 
    {
        android_atomic_inc(&ctx->shared->refCnt);
        rc = 0;
    }

    return rc;
}

static void close_shared_data(overlay_data_context_t *ctx)
{
    destroy_shared_data(ctx->shared_fd, ctx->shared, false);
    ctx->shared = NULL;
}

/****************************************************************************
 *  Control module
 *****************************************************************************/

static int overlay_get(struct overlay_control_device_t *dev, int name)
{
    int result = -1;

    switch (name) 
    {
        /* 0 = no limit */
        case OVERLAY_MINIFICATION_LIMIT:   result = 0;  break;
        /* 0 = no limit */
        case OVERLAY_MAGNIFICATION_LIMIT:  result = 0;  break;
        /* 0 = infinite */
        case OVERLAY_SCALING_FRAC_BITS:    result = 0;  break;
        /* 90 rotation steps (for instance) */
        case OVERLAY_ROTATION_STEP_DEG:    result = 90; break;
        /* 1-pixel alignment */
        case OVERLAY_HORIZONTAL_ALIGNMENT: result = 1;  break;
        /* 1-pixel alignment */
        case OVERLAY_VERTICAL_ALIGNMENT:   result = 1;  break;
        /* 1-pixel alignment */
        case OVERLAY_WIDTH_ALIGNMENT:      result = 1;  break;
        case OVERLAY_HEIGHT_ALIGNMENT:     break;
    }

    return result;
}

static overlay_t* overlay_createOverlay(struct overlay_control_device_t *dev,uint32_t w, uint32_t h, int32_t  format,uint32_t screen_id)
{
    LOGV("overlay_createOverlay:IN w=%d h=%d format=%x,screen_id = %d\n", w, h, format,screen_id);
    LOG_FUNCTION_NAME;

    overlay_object              *overlay = 0;
    overlay_control_context_t   *ctx = (overlay_control_context_t *)dev;
    overlay_shared_t            *shared = 0;
    void                        *overlayhandle = 0;
    __disp_layer_info_t 		tmpLayerAttr;
	int  						fbfh0;
	__disp_colorkey_t 			ck;
    int                         ret;
    int                         fd;
    int                         shared_fd;
	__disp_pixel_fmt_t                         disp_format;
	// star add 
	__disp_pixel_mod_t			fb_mode = DISP_MOD_MB_UV_COMBINED;

	switch(format)
	{
		case OVERLAY_FORMAT_DEFAULT:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_NON_MB_UV_COMBINED;
			break;
		case OMX_COLOR_FormatVendorMBYUV420:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_MB_UV_COMBINED;
			break;
		case OMX_COLOR_FormatVendorMBYUV422:
			disp_format = DISP_FORMAT_YUV422;
			fb_mode = DISP_MOD_MB_UV_COMBINED;
			break;
		case OMX_COLOR_FormatYUV420Planar:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_NON_MB_PLANAR;
			break;
		default:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_NON_MB_PLANAR;
			break;
	}

    if (ctx->overlay_video1) 
    {
        LOGE("Error - overlays already in use\n");
        
        return NULL;
    }

    LOGV("overlay.cpp:w:%d,h:%d  %d:%d",w,h,g_lcd_width,g_lcd_height);

    shared_fd 						= create_shared_data(&shared);
    if (shared_fd < 0) 
    {
        LOGE("Failed to create shared data");
        return NULL;
    }

    fd 								= open("/dev/disp", O_RDWR);
    if (fd < 0)
    {
        LOGE("Failed to open overlay device : %s\n", strerror(errno));
        goto error;
    }

    args[0] 						= screen_id;
	args[1] 						= 0;	
	args[2] 						= 0;
    overlayhandle = (void *)ioctl(fd, DISP_CMD_LAYER_REQUEST,args);
    if(overlayhandle == 0)
	{
		goto error;
	}

    g_lcd_width                     = ioctl(fd, DISP_CMD_SCN_GET_WIDTH,args);
    g_lcd_height                    = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT,args);
    
    LOGV("overlay handle = %x,fd = %d\n",overlayhandle,fd);

    overlay = new overlay_object(fd, shared_fd,shared->size,w, h, format,overlayhandle,screen_id,g_lcd_width,g_lcd_width);
    if (overlay == NULL) 
    {
        LOGE("Failed to create overlay object\n");
        goto error1;
    }

    ctx->overlay_video1             = overlay;

    overlay->setShared(shared);

    shared->controlReady            = 0;
    shared->streamEn                = 0;
    shared->streamingReset          = 0;

    g_lcd_width                     = ioctl(fd, DISP_CMD_SCN_GET_WIDTH,args);
    g_lcd_height                    = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT,args);
    shared->dispW                   = g_lcd_width;
    shared->dispH                   = g_lcd_height;
    args[0] 						= 0;
	args[1] 						= 0;	
	args[2] 						= 0;
    shared->org_dispW               = ioctl(fd, DISP_CMD_SCN_GET_WIDTH,args);
    shared->org_dispH               = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT,args);
	
	tmpLayerAttr.fb.mode 			= fb_mode;	// DISP_MOD_NON_MB_UV_COMBINED;	// DISP_MOD_MB_UV_COMBINED;
	tmpLayerAttr.fb.format 			= disp_format; //DISP_FORMAT_YUV420;//__disp_pixel_fmt_t(format);
	tmpLayerAttr.fb.br_swap 		= 0;
	tmpLayerAttr.fb.seq 			= DISP_SEQ_UVUV;
	
	tmpLayerAttr.fb.addr[0] 		= 0;
	tmpLayerAttr.fb.addr[1] 		= 0;

	tmpLayerAttr.fb.size.width 		= w;
	tmpLayerAttr.fb.size.height 	= h;
	
	//set color space
	if (h < 720) 
    {
		tmpLayerAttr.fb.cs_mode 	= DISP_BT601;
	} 
	else 
	{
		tmpLayerAttr.fb.cs_mode 	= DISP_BT709;
	}
	
	//set video layer attribute
    tmpLayerAttr.mode 				= DISP_LAYER_WORK_MODE_SCALER;
    tmpLayerAttr.alpha_en 			= 1;
	tmpLayerAttr.alpha_val 			= 0xff;
	tmpLayerAttr.pipe 				= 1;
	tmpLayerAttr.scn_win.x 			= 0;
	tmpLayerAttr.scn_win.y 			= 0;
	tmpLayerAttr.scn_win.width 		= g_lcd_width;
	tmpLayerAttr.scn_win.height 	= g_lcd_height;
    tmpLayerAttr.prio               = 0xff;
    //screen window information
    //frame buffer pst and size information
    tmpLayerAttr.src_win.x          = 0;//tmpVFrmInf->dst_rect.uStartX;
    tmpLayerAttr.src_win.y          = 0;//tmpVFrmInf->dst_rect.uStartY;
    tmpLayerAttr.src_win.width      = w;//tmpVFrmInf->dst_rect.uWidth;
    tmpLayerAttr.src_win.height     = h;//tmpVFrmInf->dst_rect.uHeight;
	tmpLayerAttr.fb.b_trd_src	= false;
	tmpLayerAttr.b_trd_out	= false;
	tmpLayerAttr.fb.trd_mode 		=  (__disp_3d_src_mode_t)3;
	tmpLayerAttr.out_trd_mode		= DISP_3D_OUT_MODE_FP;
	tmpLayerAttr.b_from_screen 		= 0;
	LOGV("overlay.cpp:w:%d,h:%d  %d:%d",w,h,g_lcd_width,g_lcd_height);
    LOGV("overlay.cpp:fb_mode:%d,disp_format:%d  %d:%d",fb_mode,disp_format,g_lcd_width,g_lcd_height);

    //set channel
    args[0] 						= screen_id;
	args[1] 						= (unsigned long)overlayhandle;
	args[2] 						= (unsigned long) (&tmpLayerAttr);
	args[3] 						= 0;
	ret = ioctl(fd, DISP_CMD_LAYER_SET_PARA, args);
	LOGV("SET_PARA ret:%d",ret);

	args[0]							= screen_id;
	args[1]                 		= (unsigned long)overlayhandle;
	args[2]                 		= 0;
	args[3]                 		= 0;
	ret                     		= ioctl(fd, DISP_CMD_LAYER_TOP,args);
	if(ret != 0)
	{
		//open display layer failed, need send play end command, and exit
		LOGI("Open video display layer failed!\n");

		return NULL;
	}
	
	args[0] 						= screen_id;
	args[1] 						= (unsigned long)overlayhandle;
	ioctl(fd, DISP_CMD_LAYER_CK_ON,args);
	
	overlay->setscreen(g_lcd_width,g_lcd_height,screen_id);
	overlay->setsize(w,h);
	
    fbfh0 = open("/dev/graphics/fb0",O_RDWR);
	if(fbfh0 < 0)
	{
		LOGE("open fb0 fail \n ");
		
		goto error1;
	}
	
	ioctl(fbfh0,FBIOGET_LAYER_HDL_0,&fb_layer_hdl);
	close(fbfh0);	

	ck.ck_min.alpha 	= 0xff;
	ck.ck_min.red 		= 0x05; //0x01;
	ck.ck_min.green 	= 0x01; //0x03;
	ck.ck_min.blue 		= 0x07; //0x05;
	ck.ck_max.alpha 	= 0xff;
	ck.ck_max.red 		= 0x05; //0x01;
	ck.ck_max.green 	= 0x01; //0x03;
	ck.ck_max.blue 		= 0x07; //0x05;
	ck.red_match_rule 	= 2;
	ck.green_match_rule = 2;
	ck.blue_match_rule 	= 2;
	args[0] 			= 0;
    args[1] 			= (unsigned long)&ck;
    ioctl(fd,DISP_CMD_SET_COLORKEY,(void*)args);//pipe1, different with video layer's pipe

	args[0] 			= screen_id;
    args[1] 			= fb_layer_hdl;
    args[2] 			= 0;
    ioctl(fd,DISP_CMD_LAYER_SET_PIPE,(void*)args);//pipe1, different with video layer's pipe

    args[0] 			= screen_id;
    args[1] 			= fb_layer_hdl;
    ioctl(fd,DISP_CMD_LAYER_TOP,(void*)args);

    args[0] 			= screen_id;
	args[1] 			= fb_layer_hdl;
	args[2]             = 0xFF;
	ioctl(fd,DISP_CMD_LAYER_SET_ALPHA_VALUE,(void*)args);//disable the global alpha, use the pixel's alpha

    args[0] 			= screen_id;
    args[1] 			= fb_layer_hdl;
    ioctl(fd,DISP_CMD_LAYER_ALPHA_OFF,(void*)args);//disable the global alpha, use the pixel's alpha

	args[0]				= screen_id;
    args[1] 			= fb_layer_hdl;
    ioctl(fd,DISP_CMD_LAYER_CK_OFF,(void*)args);//disable the global alpha, use the pixel's alpha

	LOGI("layer open hdl:%d,ret :%d\n",(unsigned long)overlayhandle,ret);
    
    LOGI("Opened video1/fd=%d/obj=%08lx/shm=%d/size=%d", fd,(unsigned long)overlay, shared_fd, shared->size);

	shared->g_screen	= screen_id;
	shared->g_currenthandle= (unsigned long)overlayhandle;
    return overlay;

error1:
    if(overlayhandle)
    {
    	args[0] = screen_id;
        args[1] = (unsigned long)overlayhandle;
	    args[2] = 0;	
	    args[3] = 0;
        ioctl(fd, DISP_CMD_LAYER_RELEASE,args);
    }
    close(fd);
error:
	
    destroy_shared_data(shared_fd, shared, true);
    return NULL;
}

static void overlay_destroyOverlay(struct overlay_control_device_t *dev,overlay_t* overlay)
{
    LOGV("overlay_destroyOverlay:IN dev (%p) and overlay (%p)", dev, overlay);
    LOG_FUNCTION_NAME;

    overlay_control_context_t *	ctx = (overlay_control_context_t *)dev;
    overlay_object *			obj = (overlay_object *)(overlay);
	int 						ret;
    int 						rc;
    int 						fd = obj->ctl_fd();
    uint32_t 					num = 0;
    overlay_shared_t *			shared = obj->getShared();

    if (shared == NULL) 
    {
        LOGE("Overlay was already destroyed - nothing needs to be done\n");
        return;
    }

    LOGI("Destroying overlay/fd=%d/obj=%08lx", fd, (unsigned long)overlay);

    if(shared->g_currenthandle)
    {
        args[0]					= shared->g_screen;
		args[1]					= shared->g_currenthandle;
		args[2]					= 0xff;
		ioctl(fd,DISP_CMD_LAYER_SET_ALPHA_VALUE,(void *)args);

		args[0] 				= shared->g_screen;
		args[1] 				= shared->g_currenthandle;
		ioctl(fd,DISP_CMD_LAYER_ALPHA_ON,(void*)args);//disable the global alpha, use the pixel's alpha

        args[0] 				= shared->g_screen;
        args[1] 				= shared->g_currenthandle;
	    args[2] 				= 0;	
	    args[3] 				= 0;
        ioctl(fd, DISP_CMD_VIDEO_STOP, args);
        ioctl(fd, DISP_CMD_LAYER_RELEASE,args);
        
        args[0] 						= shared->g_screen;
	    ret = ioctl(fd,DISP_CMD_GET_OUTPUT_TYPE,args);
	    if(ret == DISP_OUTPUT_TYPE_HDMI && (shared->cur_3denable == true))
	    {
	    	args[0] 					= shared->g_screen;
	    	args[1] 					= 0;
	    	args[2] 					= 0;
	    	args[3] 					= 0;
	    	ioctl(fd,DISP_CMD_HDMI_OFF,(unsigned long)args);
	    	
	    	LOGV("overlay_destroyOverlay shared->cur_hdmimode = %d\n",shared->cur_hdmimode);
	    	args[0] 					= shared->g_screen;
	    	args[1] 					= shared->cur_hdmimode;
	    	args[2] 					= 0;
	    	args[3] 					= 0;
	    	ioctl(fd, DISP_CMD_HDMI_SET_MODE, args);
	    	
	    	args[0] 					= shared->g_screen;
	    	args[1] 					= 0;
	    	args[2] 					= 0;
	    	args[3] 					= 0;
	    	ioctl(fd,DISP_CMD_HDMI_ON,(unsigned long)args);
	    }
    }

    destroy_shared_data(obj->shared_fd(), shared, true);
    obj->setShared(NULL);

    if (close(fd)) 
    {
        LOGE( "Error closing overly fd/%d\n", errno);
    }

    if (overlay) 
    {
        if (ctx->overlay_video1 == overlay)
        {
            ctx->overlay_video1 = NULL;
        }
        
        delete overlay;
        
        overlay = NULL;
    }
    
    LOGV("overlay_destroyOverlay:OUT");
}

static int overlay_setPosition(struct overlay_control_device_t *dev,
        overlay_t* overlay, int x, int y, uint32_t w,
        uint32_t h)
{
    LOG_FUNCTION_NAME;

    overlay_object              *obj = (overlay_object *)(overlay);
    overlay_ctrl_t              *stage  = obj->staging();
    overlay_shared_t            *shared = obj->getShared();
    int                         fd     = obj->ctl_fd();;
    void                        *overlayhandle;
    __disp_rect_t               rect;
    int                         rc = 0;
    int                         temp_x = x;
    int                         temp_y = y;
    int                         temp_w = w;
    int                         temp_h = h;
    int                         scn_w;
    int                         scn_h;
    int							ret;

    /*
     * This logic here is to return an error if the rectangle is not fully
     * within the display, unless we have not received a valid position yet,
     * in which case we will do our best to adjust the rectangle to be within
     * the display.
     */
	args[0] 						= shared->g_screen;
    ret = ioctl(fd,DISP_CMD_GET_OUTPUT_TYPE,args);
    LOGV("hdmi mode = %d\n",ret);
	LOGV("shared->cur_3denable = %d\n",shared->cur_3denable);
    if(ret == DISP_OUTPUT_TYPE_HDMI && (shared->cur_3denable == true || shared->cur_half_enable == true))
    {
    	return  0;
    }
    
    /* Require a minimum size */
    if (temp_w < 16)
    {
        temp_w = 16;
    }
    if (temp_h < 8)
    {
        temp_h = 8;
    }
    
    if(temp_x < 0)
    {
    	temp_x = 0;
    }
    
    if(temp_y < 0)
    {
    	temp_y = 0;
    }
    
    if(temp_x + temp_w > shared->org_dispW)
    {
    	temp_w = shared->org_dispW - temp_x;
    }
    
    if(temp_y + temp_h > shared->org_dispH)
    {
    	temp_h = shared->org_dispH - temp_y;
    }

	if(shared->dispW != shared->org_dispW || shared->dispH != shared->org_dispH)
	{
	    scn_w  = temp_w * shared->dispW/shared->org_dispW;
	    scn_h  = scn_w * temp_h/temp_w;
	    temp_w = scn_w;
	    temp_h = scn_h;
	    temp_x = temp_x * shared->dispW/shared->org_dispW;
	    temp_y = temp_y * shared->dispH/shared->org_dispH;
	    
	    if (temp_w < 16)
	    {
	        temp_w = 16;
	    }
	    if (temp_h < 8)
	    {
	        temp_h = 8;
	    }
	    LOGV("shared->dispH = %d\n",shared->dispH);
	    LOGV("shared->dispW = %d\n",shared->dispW);
	    LOGV("x = %d\n",x);
	    LOGV("y = %d\n",y);
	    LOGV("w = %d\n",w);
	    LOGV("h = %d\n",h);
	    LOGV("scn_w = %d\n",scn_w);
	    LOGV("scn_h = %d\n",scn_h);

	}
	
	LOGV("shared->dispH = %d\n",shared->dispH);
    LOGV("shared->dispW = %d\n",shared->dispW);
    LOGV("shared->dispH = %d\n",shared->org_dispH);
    LOGV("shared->dispW = %d\n",shared->org_dispW);
    LOGV("x = %d\n",x);
    LOGV("y = %d\n",y);
    LOGV("w = %d\n",w);
    LOGV("h = %d\n",h);
    LOGV("scn_w = %d\n",scn_w);
    LOGV("scn_h = %d\n",scn_h);

    if (!shared->controlReady) 
    {
        if ( temp_x < 0 ) 
        {
            temp_x = 0;
        }
        if ( temp_y < 0 ) 
        {
            temp_y = 0;
        }
        if ( temp_w > shared->dispW ) 
        {
            temp_w = shared->dispW;
        }
        if ( temp_h > shared->dispH )
        {
            temp_h = shared->dispH;
        }
        if ( (temp_x + temp_w) > shared->dispW ) 
        {
            temp_w = shared->dispW - temp_x;
        }
        if ( (temp_y + temp_h) > shared->dispH ) 
        {
            temp_h = shared->dispH - temp_y;
        }
    } 
    else if (temp_x < 0 || temp_y < 0 || (temp_x + temp_w) > shared->dispW ||
            (temp_y + temp_h) > shared->dispH) 
    {
        /* Return an error */
        rc = -1;
    }

    if (rc == 0) 
    {
    	if((stage->posX != temp_x) || (stage->posY != temp_y) 
    	  ||(stage->posW != temp_w) || (stage->posH != temp_h) )
    	{
	        stage->posX         = temp_x;
	        stage->posY         = temp_y;
	        stage->posW         = temp_w;
	        stage->posH         = temp_h;
	
	        stage->posX_org     = x;
	        stage->posY_org     = y;
	        stage->posW_org     = w;
	        stage->posH_org     = h;
	
	        overlayhandle       = obj->getoverlay();
	        rect.x              = temp_x;
	        rect.y              = temp_y;
	        rect.width          = temp_w;
	        rect.height         = temp_h;
	
	        args[0] 			= shared->g_screen;
	        args[1] 			= shared->g_currenthandle;
		    args[2] 			= (unsigned long)&rect;	
		    args[3] 			= sizeof(__disp_rect_t);
		    
		    LOGV("obj->getscreenid() = %d\n",shared->g_screen);
		    LOGV("(unsigned long)overlayhandle = %d\n",shared->g_currenthandle);
		    LOGV("temp_x = %d\n",temp_x);
		    LOGV("temp_y = %d\n",temp_y);
		    LOGV("temp_w = %d\n",temp_w);
		    LOGV("temp_h = %d\n",temp_h);
	        
	        ioctl(fd, DISP_CMD_LAYER_SET_SCN_WINDOW,args);
   	 	}
    }

    return rc;
}

static int overlay_getPosition(struct overlay_control_device_t *dev,
        overlay_t* overlay, int* x, int* y, uint32_t* w,
        uint32_t* h)
{
    LOG_FUNCTION_NAME;

    overlay_object *obj = (overlay_object *)(overlay);
    overlay_ctrl_t *stage  = obj->staging();

    *x = stage->posX_org;
    *y = stage->posY_org;
    *w = stage->posW_org;
    *h = stage->posH_org;

    return 0;
}

static int overlay_setScreenid(struct overlay_data_context_t* ctx,int value)
{
    int                         fd;
    __disp_layer_info_t         layer_info;
    int 						ret;
    int                         output_mode;
	int							old_screen;
	void*						overlayhandle;
	void  						*overlay;
    int    						ctl_fd;
    overlay_handle_t			overlay_handle;

    overlay_handle				= ctx->overlay_handle;
    LOGV("overlay_handle = %d\n",(unsigned long)overlay_handle);
    old_screen   				= handle_screenid(overlay_handle);
    LOGV("old_screen = %d\n",(unsigned long)old_screen);
    overlay  					= ctx->overlayhandle;
    ctl_fd   					= ctx->ctl_fd;
    fd                          = ctl_fd;
    
    if(old_screen  == value)
    {
    	LOGV("nothing to do!");
    	
    	return  0;
    }
    
    if(value != 0)
    {
    	value = 1;
    }

    if (open_shared_data(ctx)) 
    {
        return -1;
    }

    ctx->shared->g_currenthandle  = 0;
    
    LOGV("overlay release first");
    ctx->overlay_ready			= false;
    
	args[0] 					= old_screen;
	args[1] 					= (unsigned long) overlay;
	args[2]		 				= (unsigned long) (&layer_info);
	args[3] 					= 0;
	ioctl(fd, DISP_CMD_LAYER_GET_PARA, args);
    
    args[0] 					= old_screen;
    args[1] 					= (unsigned long) overlay;
    args[2] 					= 0;	
    args[3] 					= 0;
    ioctl(fd, DISP_CMD_VIDEO_STOP, args);
    ioctl(fd, DISP_CMD_LAYER_RELEASE,args);

	LOGV("release overlay = %d,value = %d\n",(unsigned long) overlay,value);
	
	sleep(2);
	
    args[0] 					= value;
	args[1] 					= 0;	
	args[2] 					= 0;
    overlayhandle = (void *)ioctl(fd, DISP_CMD_LAYER_REQUEST,args);
    if(overlayhandle == 0)
	{
		LOGE("request layer failed!\n");
		
		goto error;
	}

	args[0] 						= value;
    ret = ioctl(fd,DISP_CMD_GET_OUTPUT_TYPE,args);
    output_mode = ret;
    if(ret == DISP_OUTPUT_TYPE_HDMI && (ctx->shared->cur_3denable == true))
    {
    	args[0] 					= value;
    	args[1] 					= 0;
    	args[2] 					= 0;
    	args[3] 					= 0;
    	ctx->shared->cur_hdmimode	= ioctl(fd, DISP_CMD_HDMI_GET_MODE, args);
    	LOGV("overlay_setScreenid ctx->shared->cur_hdmimode = %d\n",ctx->shared->cur_hdmimode);
    	args[0] 					= value;
    	args[1] 					= 0;
    	args[2] 					= 0;
    	args[3] 					= 0;
    	ioctl(fd,DISP_CMD_HDMI_OFF,(unsigned long)args);
    	
    	args[0] 					= value;
    	args[1] 					= DISP_TV_MOD_1080P_24HZ_3D_FP;
    	args[2] 					= 0;
    	args[3] 					= 0;
    	ioctl(fd, DISP_CMD_HDMI_SET_MODE, args);
    	
    	args[0] 					= value;
    	args[1] 					= 0;
    	args[2] 					= 0;
    	args[3] 					= 0;
    	ioctl(fd,DISP_CMD_HDMI_ON,(unsigned long)args);
	}

	args[0] 						= value;
	args[1] 						= 0;	
	args[2] 						= 0;
    g_lcd_width                     = ioctl(fd, DISP_CMD_SCN_GET_WIDTH,args);
    g_lcd_height                    = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT,args);
	
	layer_info.scn_win.x 			= 0;
	layer_info.scn_win.y 			= 0;
	layer_info.scn_win.width 		= g_lcd_width;
	layer_info.scn_win.height 		= g_lcd_height;
	//frame buffer pst and size information
    layer_info.alpha_en           	= 0xff;
    layer_info.alpha_val          	= 0xff;  
    layer_info.pipe              	= 1;
    layer_info.prio               	= 0xff;
    layer_info.b_from_screen		= false;
    
    layer_info.fb.b_trd_src 		= ctx->shared->cur_half_enable;
	layer_info.fb.trd_mode 			= (__disp_3d_src_mode_t)ctx->shared->cur_3dmode;
	layer_info.b_trd_out			= ctx->shared->cur_3denable;
	layer_info.out_trd_mode			= DISP_3D_OUT_MODE_FP;
    if(output_mode != DISP_OUTPUT_TYPE_HDMI && (ctx->shared->cur_3denable == true))
    {
        layer_info.fb.b_trd_src         = false;    
	    layer_info.b_trd_out			= false;
    }

	LOGV("request overlay.cpp:w:%d,h:%d  %d:%d",layer_info.fb.size.width,layer_info.fb.size.height,g_lcd_width,g_lcd_height);
    LOGV("request overlay.cpp:fb_mode:%d,disp_format:%d  %d:%d",layer_info.fb.mode,layer_info.fb.format,g_lcd_width,g_lcd_height);
    
    //set channel
    args[0] 						= value;
	args[1] 						= (unsigned long)overlayhandle;
	args[2] 						= (unsigned long) (&layer_info);
	args[3] 						= 0;
	ret = ioctl(fd, DISP_CMD_LAYER_SET_PARA, args);
	LOGV("SET_PARA ret:%d",ret);

	args[0]							= value;
	args[1]                 		= (unsigned long)overlayhandle;
	args[2]                 		= 0;
	args[3]                 		= 0;
	ret                     		= ioctl(fd, DISP_CMD_LAYER_BOTTOM,args);
	if(ret != 0)
	{
		//open display layer failed, need send play end command, and exit
		LOGI("Open video display layer failed!\n");

		return NULL;
	}
	
	args[0] 						= value;
	args[1] 						= (unsigned long)overlayhandle;
	ioctl(fd, DISP_CMD_LAYER_CK_ON,args);
	
	handle_setscreenid(overlay_handle,value);
	handle_setscreenwidth(overlay_handle,g_lcd_width);
	handle_setscreenheight(overlay_handle,g_lcd_height);
	handle_setoverlay(overlay_handle,overlayhandle);

	args[0] 						= value;
	args[1] 						= (unsigned long) overlayhandle;
	args[2] 						= 0;
	args[3] 						= 0;
	ioctl(fd, DISP_CMD_LAYER_OPEN, args);

	args[0] 						= value;
	args[1] 						= (unsigned long) overlayhandle;
	args[2] 						= 0;
	args[3] 						= 0;
	ioctl(fd, DISP_CMD_VIDEO_START, args);
	
	ctx->overlayhandle              = overlayhandle;
	ctx->overlay_ready				= true;
	ctx->shared->g_screen			= value;
	LOGV("g_screen = %d",ctx->shared->g_screen);
	ctx->shared->g_currenthandle  	= (uint32_t)overlayhandle;
	LOGV("g_currenthandle = %d",ctx->shared->g_currenthandle);
    ctx->shared->dispW              = g_lcd_width;
    ctx->shared->dispH              = g_lcd_height;
    LOGV("ctx->shared->dispW = %d,ctx->shared->dispH = %d\n",ctx->shared->dispW,ctx->shared->dispH);
    return 0;

error:
    if(overlayhandle)
    {
    	args[0] = value;
        args[1] = (unsigned long)overlayhandle;
	    args[2] = 0;	
	    args[3] = 0;
        ioctl(fd, DISP_CMD_LAYER_RELEASE,args);
    }
    
    return -1;	
}

// for taking photo to avoid preview wrong
static int overlay_show(struct overlay_data_context_t* ctx,int value)
{
	int                         ctl_fd;
    void                        *overlay;
    int                         fd;
    int							handle;
    int                         ret = 0;
    int                         screen;
    
    LOGD("overlay_show");

    overlay                         = ctx->overlayhandle;
    ctl_fd                          = ctx->ctl_fd;
    screen                          = handle_screenid(ctx->overlay_handle);
   
	handle							= (unsigned long)overlay;
//	LOGV("handle = %x,tmpFrmBufAddr.addr[0] = %x,tmpFrmBufAddr.addr[1] = %x,screen = %d\n",handle,tmpFrmBufAddr.addr[0],tmpFrmBufAddr.addr[1],screen);
    fd                              = ctl_fd;
    
    if(ctx->overlay_ready)
   	{
	    args[0]							= screen;
        args[1]                         = (unsigned long)overlay;
    	args[2]                         = 0;
    	args[3]                         = 0;
		if(value == 1)
		{
			ret = ioctl(fd, DISP_CMD_LAYER_OPEN,args);
		}
		else
		{
			ret = ioctl(fd, DISP_CMD_LAYER_CLOSE,args);
		}
    	
	}

    return ret;
}

// for camera app, such as QQ , it use pixel sequence of DISP_SEQ_VUVU
static int overlay_setFormat(struct overlay_data_context_t* ctx, int value)
{
	int                         ctl_fd;
    void                        *overlay;
    int                         fd;
    int							handle;
    int                         ret = 0;
    int                         screen;
    
    LOGD("overlay_setFormat");

    overlay                         = ctx->overlayhandle;
    ctl_fd                          = ctx->ctl_fd;
    screen                          = handle_screenid(ctx->overlay_handle);
   
	handle							= (unsigned long)overlay;
    fd                              = ctl_fd;
    
    if(ctx->overlay_ready)
   	{
	    unsigned long			tmp_args[4];
		__disp_layer_info_t		tmpLayerAttr;

		tmp_args[0] 			= screen;
		tmp_args[1] 			= (unsigned long)overlay;
		tmp_args[2] 			= (unsigned long) (&tmpLayerAttr);
		tmp_args[3] 			= 0;

		ret = ioctl(fd, DISP_CMD_LAYER_GET_PARA, &tmp_args);
		LOGV("DISP_CMD_LAYER_GET_PARA, ret %d", ret);

		tmpLayerAttr.fb.seq = (__disp_pixel_seq_t)value;

		ret = ioctl(fd, DISP_CMD_LAYER_SET_PARA, &tmp_args);
		LOGV("DISP_CMD_LAYER_GET_PARA, ret %d", ret);
	}

    return ret;
}

static int overlay_set_layer_para(struct overlay_data_context_t* ctx, int width, int height, int format)
{
	void        *overlay;
	int 		ctl_fd;
	int 		screen_id;
	int 		ret;

	__disp_pixel_fmt_t          disp_format;
	__disp_pixel_mod_t			fb_mode;
	__disp_layer_info_t 		tmpLayerAttr;
	__disp_pixel_seq_t			disp_seq;
	__disp_cs_mode_t			disp_cs_mode;

	overlay                         = ctx->overlayhandle;
	ctl_fd                          = ctx->ctl_fd;
	screen_id                       = handle_screenid(ctx->overlay_handle);
	disp_seq = DISP_SEQ_UVUV;

	if (height < 720)
	{
		disp_cs_mode	= DISP_BT601;
	}
	else
	{
		disp_cs_mode	= DISP_BT709;
	}
	switch(format)
	{
		case OVERLAY_FORMAT_DEFAULT:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_NON_MB_UV_COMBINED;
			break;
		case OMX_COLOR_FormatVendorMBYUV420:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_MB_UV_COMBINED;
			break;
		case OMX_COLOR_FormatVendorMBYUV422:
			disp_format = DISP_FORMAT_YUV422;
			fb_mode = DISP_MOD_MB_UV_COMBINED;
			break;  				

		case OMX_COLOR_FormatYUV420Planar:
			disp_format = DISP_FORMAT_ARGB8888;
			fb_mode = DISP_MOD_NON_MB_PLANAR;
			disp_seq = DISP_SEQ_P3210;
			disp_cs_mode	= DISP_YCC;
			break;
		default:
			disp_format = DISP_FORMAT_YUV420;
			fb_mode = DISP_MOD_NON_MB_PLANAR;
			break;
	}	
	g_lcd_width                 = ioctl(ctl_fd, DISP_CMD_SCN_GET_WIDTH,args);
	g_lcd_height                = ioctl(ctl_fd, DISP_CMD_SCN_GET_HEIGHT,args);

	LOGV("overlay.cpp:fb_mode:%d,disp_format:%d  %d:%d, %d",fb_mode,disp_format,g_lcd_width,g_lcd_height, __LINE__);
	args[0] 						= screen_id;
	args[1] 						= (unsigned long)overlay;
	args[2] 						= (unsigned long) (&tmpLayerAttr);
	args[3] 						= 0;
	ret = ioctl(ctl_fd, DISP_CMD_LAYER_GET_PARA, args);

	tmpLayerAttr.fb.mode 			= fb_mode;	// DISP_MOD_NON_MB_UV_COMBINED;	// DISP_MOD_MB_UV_COMBINED;
	tmpLayerAttr.fb.format 			= disp_format; //DISP_FORMAT_YUV420;//__disp_pixel_fmt_t(format);
	tmpLayerAttr.fb.br_swap 		= 0;
	tmpLayerAttr.fb.seq 			= disp_seq;
	tmpLayerAttr.fb.cs_mode 		 = disp_cs_mode;
//
//	tmpLayerAttr.fb.addr[0] 		= 0;
//	tmpLayerAttr.fb.addr[1] 		= 0;
//	tmpLayerAttr.fb.addr[2]			= 0;
//	tmpLayerAttr.fb.trd_right_addr[0] = 0;
//	tmpLayerAttr.fb.trd_right_addr[1] = 0;
//	tmpLayerAttr.fb.trd_right_addr[2] = 0;

	tmpLayerAttr.fb.size.width 		= width;
	tmpLayerAttr.fb.size.height 	= height;
	LOGV("%d, fb.mode %d, fb.seq %d, fb.cs_mode %d, fb.format %d", __LINE__, tmpLayerAttr.fb.mode ,tmpLayerAttr.fb.seq,tmpLayerAttr.fb.cs_mode,tmpLayerAttr.fb.format );



	tmpLayerAttr.src_win.x          = 0;//tmpVFrmInf->dst_rect.uStartX;
	tmpLayerAttr.src_win.y          = 0;//tmpVFrmInf->dst_rect.uStartY;
	tmpLayerAttr.src_win.width      = width;//tmpVFrmInf->dst_rect.uWidth;
	tmpLayerAttr.src_win.height     = height;//tmpVFrmInf->dst_rect.uHeight;
	tmpLayerAttr.b_from_screen 		= 0;

	LOGV("overlay.cpp:w:%d,h:%d  %d:%d",width,height,g_lcd_width,g_lcd_height);
	LOGV("overlay.cpp:fb_mode:%d,disp_format:%d  %d:%d",fb_mode,disp_format,g_lcd_width,g_lcd_height);

	//set channel
	args[0] 						= screen_id;
	args[1] 						= (unsigned long)overlay;
	args[2] 						= (unsigned long) (&tmpLayerAttr);
	args[3] 						= 0;
	ret = ioctl(ctl_fd, DISP_CMD_LAYER_SET_PARA, args);
	LOGV("SET_PARA ret:%d",ret);
	return 0;
}

static int overlay_set3Dmode(struct overlay_data_context_t* ctx,int para)
{
	int                         ctl_fd;
	void                        *overlay;
	int                         fd;
	int							handle;
	int                         ret;
	int                         screen;
	int 						value;
	int 						mode;
	int							is_mode_changed;
	int *						tmp;

	__disp_layer_info_t 		layer_info;

	LOGD("overlay_show");

	memset(&layer_info, 0, sizeof(__disp_layer_info_t));

	overlay                         = ctx->overlayhandle;
	ctl_fd                          = ctx->ctl_fd;
	screen                          = handle_screenid(ctx->overlay_handle);

	handle							= (unsigned long)overlay;

	fd                              = ctl_fd;
	tmp = (int *)para;
	value = tmp[3];
	mode  = tmp[4];
	is_mode_changed = tmp[5];

	if(is_mode_changed){
		args[0] 					= screen;
		args[1] 					= (unsigned long) overlay;
		args[2] 					= 0;
		args[3] 					= 0;
		ioctl(fd, DISP_CMD_VIDEO_STOP, args);
		ioctl(fd, DISP_CMD_LAYER_CLOSE,args);
	}
	LOGV("%d, width %d, height %d, format %x, value %d, mode %d, is mode changed %d", __LINE__, tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5]);



	overlay_set_layer_para(ctx, tmp[0], tmp[1], tmp[2]);

	if(ctx->overlay_ready)
	{
		args[0] 					= screen;
		args[1] 					= (unsigned long) overlay;
		args[2]	 					= (unsigned long) (&layer_info);
		args[3] 					= 0;
		ioctl(fd, DISP_CMD_LAYER_GET_PARA, args);
		args[0] = screen;
		ret = ioctl(fd,DISP_CMD_GET_OUTPUT_TYPE,args);        
		if(ret == DISP_OUTPUT_TYPE_HDMI)
		{
			LOGV("value = %d, f_trd_srd = %d, trd_mode = %d, b_trd_out %d", value, layer_info.fb.b_trd_src, layer_info.fb.trd_mode, layer_info.b_trd_out);

			if(mode == OVERLAY_DISP_MODE_3D && value != OVERLAY_3D_OUT_MODE_NORMAL)
			{
				if(layer_info.b_trd_out == false || value != (int)layer_info.fb.trd_mode )
				{
					if(layer_info.b_trd_out == false){
						args[0] 					= screen;
						args[1] 					= 0;
						args[2] 					= 0;
						args[3] 					= 0;
						ctx->shared->cur_hdmimode	= ioctl(fd, DISP_CMD_HDMI_GET_MODE, args);
					}

					args[0] 					= screen;
					args[1] 					= 0;
					args[2] 					= 0;
					args[3] 					= 0;
					ioctl(fd,DISP_CMD_HDMI_OFF,(unsigned long)args);

					args[0] 					= screen;
					args[1] 					= DISP_TV_MOD_1080P_24HZ_3D_FP;
					args[2] 					= 0;
					args[3] 					= 0;
					ioctl(fd, DISP_CMD_HDMI_SET_MODE, args);

					args[0] 					= screen;
					args[1] 					= 0;
					args[2] 					= 0;
					args[3] 					= 0;
					ioctl(fd,DISP_CMD_HDMI_ON,(unsigned long)args);
					args[0] 					= screen;
					args[1] 					= 0;	
					args[2] 					= 0;
					g_lcd_width                 = ioctl(fd, DISP_CMD_SCN_GET_WIDTH,args);
					g_lcd_height                = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT,args);

					ctx->shared->dispW          = g_lcd_width;
					ctx->shared->dispH          = g_lcd_height;
					layer_info.scn_win.x 		= 0;
					layer_info.scn_win.y 		= 0;
					layer_info.scn_win.width 	= g_lcd_width;
					layer_info.scn_win.height 	= g_lcd_height;
					LOGV("%d, 3d mode %d, value is %d====================================", __LINE__, mode, value);

					layer_info.fb.trd_mode 		=  (__disp_3d_src_mode_t)value;
					layer_info.out_trd_mode		= DISP_3D_OUT_MODE_FP;
					layer_info.fb.b_trd_src	= true;
					layer_info.b_trd_out	= true;

					ctx->shared->cur_3dmode		= value;
					ctx->shared->cur_3denable	= true;
					ctx->shared->cur_half_enable	= true;
					LOGV("line %d, screen width %d, screen height %d", __LINE__, g_lcd_width, g_lcd_height);

					args[0] 					= screen;
					args[1] 					= (unsigned long) overlay;
					args[2] 					= (unsigned long) (&layer_info);
					args[3] 					= 0;
					ioctl(fd, DISP_CMD_LAYER_SET_PARA, args);	
				}
			}
			else
			{
				if(value != OVERLAY_3D_OUT_MODE_NORMAL)
				{
					if(layer_info.b_trd_out == true ){
						args[0] 					= screen;
						args[1] 					= 0;
						args[2] 					= 0;
						args[3] 					= 0;
						ioctl(fd,DISP_CMD_HDMI_OFF,(unsigned long)args);

						args[0] 					= screen;
						args[1] 					= ctx->shared->cur_hdmimode;
						args[2] 					= 0;
						args[3] 					= 0;
						ioctl(fd, DISP_CMD_HDMI_SET_MODE, args);

						args[0] 					= screen;
						args[1] 					= 0;
						args[2] 					= 0;
						args[3] 					= 0;
						ioctl(fd,DISP_CMD_HDMI_ON,(unsigned long)args);

						args[0] 					= screen;
						args[1] 					= 0;
						args[2] 					= 0;
						g_lcd_width                 = ioctl(fd, DISP_CMD_SCN_GET_WIDTH,args);
						g_lcd_height                = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT,args);
						ctx->shared->dispW          = g_lcd_width;
						ctx->shared->dispH          = g_lcd_height;

						layer_info.scn_win.x 		= 0;
						layer_info.scn_win.y 		= 0;
						layer_info.scn_win.width 	= g_lcd_width;
						layer_info.scn_win.height 	= g_lcd_height;
					}
					if(mode == OVERLAY_DISP_MODE_2D)
					{
						LOGV("%d, 3d mode with one picture*****************************************", __LINE__);
						layer_info.fb.b_trd_src 	= true;
						layer_info.b_trd_out		= false;
					}
					else
					{
						LOGV("%d, original mode -----------------------------------------------", __LINE__);
						layer_info.fb.b_trd_src 	= false;
						layer_info.b_trd_out		= false;
					}
					layer_info.fb.trd_mode 		= (__disp_3d_src_mode_t)value;
					layer_info.out_trd_mode		= DISP_3D_OUT_MODE_FP;

					ctx->shared->cur_3denable		= layer_info.b_trd_out;
					ctx->shared->cur_half_enable	= layer_info.fb.b_trd_src;
					ctx->shared->cur_3dmode		= value;

					LOGV("line %d, screen width %d, screen height %d", __LINE__, g_lcd_width, g_lcd_height);
					args[0] 					= screen;
					args[1] 					= (unsigned long) overlay;
					args[2] 					= (unsigned long) (&layer_info);
					args[3] 					= 0;
					ioctl(fd, DISP_CMD_LAYER_SET_PARA, args);
				}
			}
		}
		else
		{			
			args[0] 					= 0;
			args[1] 					= 0;
			args[2] 					= 0;
			args[3] 					= 0;
			ioctl(fd,DISP_CMD_HDMI_OFF, args);

			args[0] 					= 0;
			args[1] 					= 0;
			args[2] 					= 0;
			args[3] 					= 0;
			ioctl(fd,DISP_CMD_LCD_ON, args);
        

			layer_info.fb.b_trd_src 	= (mode == OVERLAY_DISP_MODE_2D)?true:false;
			layer_info.b_trd_out		= false;			
            layer_info.fb.trd_mode 		=  (__disp_3d_src_mode_t)value;            

			ctx->shared->cur_3dmode			= value;
			ctx->shared->cur_3denable		= false;
			ctx->shared->cur_half_enable	= layer_info.fb.b_trd_src;
			
            if(mode == OVERLAY_DISP_MODE_3D)
            {
                ctx->shared->cur_3denable       = true;
                ctx->shared->cur_half_enable	= true;                
            }
            
			args[0] 					= 0;
			args[1] 					= (unsigned long) overlay;
			args[2] 					= (unsigned long) (&layer_info);
			args[3] 					= 0;
			ioctl(fd, DISP_CMD_LAYER_SET_PARA, args);
		}
	}
	if(is_mode_changed){
		   args[0] 					= screen;
		   args[1] 					= (unsigned long) overlay;
		   args[2] 					= 0;
		   args[3] 					= 0;
		   ioctl(fd, DISP_CMD_LAYER_OPEN,args);

		   args[0] 					= screen;
		   args[1] 					= (unsigned long) overlay;
		   args[2] 					= 0;
		   args[3] 					= 0;
		   ioctl(fd, DISP_CMD_VIDEO_START, args);
	}

	return 0;
}

static int overlay_setFrameParameter(struct overlay_data_context_t* ctx,int value)
{
    int                         ctl_fd;
    void                        *overlay;
    __disp_video_fb_t      		tmpFrmBufAddr;
    liboverlaypara_t            *overlaypara;
    int                         fd;
    int							handle;
    __disp_layer_info_t         layer_info;
    int                         ret;
    int                         screen;

    overlay                         = ctx->overlayhandle;
    ctl_fd                          = ctx->ctl_fd;
    screen                          = handle_screenid(ctx->overlay_handle);
    overlaypara                     = (liboverlaypara_t *)value;
    //set framebuffer parameter to display driver
	tmpFrmBufAddr.interlace         = (overlaypara->bProgressiveSrc?0:1);
	tmpFrmBufAddr.top_field_first   = overlaypara->bTopFieldFirst;
	tmpFrmBufAddr.frame_rate        = overlaypara->pVideoInfo.frame_rate;
	tmpFrmBufAddr.addr[0]           = overlaypara->top_y;
	tmpFrmBufAddr.addr[1]           = overlaypara->top_c;
	tmpFrmBufAddr.addr[2]			= 0;


	if(overlaypara->bottom_c){
		tmpFrmBufAddr.addr[2]			= 0;
		tmpFrmBufAddr.addr_right[0]		= overlaypara->bottom_y;
		tmpFrmBufAddr.addr_right[1]		= overlaypara->bottom_c;
		tmpFrmBufAddr.addr_right[2]		= 0;

	}else{
		if(overlaypara->bottom_y){
			tmpFrmBufAddr.addr[2]			= overlaypara->bottom_y;
		}
		else{
			tmpFrmBufAddr.addr[2]			= 0;
		}

		tmpFrmBufAddr.addr_right[0]		= 0;
		tmpFrmBufAddr.addr_right[1]		= 0;
		tmpFrmBufAddr.addr_right[2]		= 0;
	}

	tmpFrmBufAddr.id                = overlaypara->number; 
	ctx->cur_frameid				= tmpFrmBufAddr.id;
	
	handle							= (unsigned long)overlay;
	//LOGV("handle = %x,tmpFrmBufAddr.addr[0] = %x,tmpFrmBufAddr.addr[1] = %x,screen = %d\n",handle,tmpFrmBufAddr.addr[0],tmpFrmBufAddr.addr[1],screen);
    fd                              = ctl_fd;
    
    //LOGV("overlay->ctl_fd() = %x",ctl_fd);
//    LOGV("overlaypara->bProgressiveSrc = %x",overlaypara->bProgressiveSrc);
//    LOGV("overlaypara->bTopFieldFirst = %x",overlaypara->bTopFieldFirst);
//    LOGV("overlaypara->pVideoInfo.frame_rate = %x",overlaypara->pVideoInfo.frame_rate);
//    LOGV("overlaypara->first_frame_flg = %x",overlaypara->first_frame_flg);
    //LOGV("tmpFrmBufAddr.first_frame = %x",tmpFrmBufAddr.first_frame);
    //LOGV("overlay = %x",(unsigned long)overlay);
    if(ctx->overlay_ready)
   	{
	    if(overlaypara->first_frame_flg)
	    {
	    	//LOGV("overlay first");
	    	args[0] = screen;
	    	args[1] = (unsigned long) overlay;
	    	args[2] = (unsigned long) (&layer_info);
	    	args[3] = 0;
	    	ioctl(fd, DISP_CMD_LAYER_GET_PARA, args);
	
	    	layer_info.fb.addr[0] 	= tmpFrmBufAddr.addr[0];
	    	layer_info.fb.addr[1] 	= tmpFrmBufAddr.addr[1];
	    	args[0] 				= screen;
	    	args[1] 				= (unsigned long) overlay;
	    	args[2] 				= (unsigned long) (&layer_info);
	    	args[3] 				= 0;
	    	ret = ioctl(fd, DISP_CMD_LAYER_SET_PARA, args);
	    	//LOGV("------------------------------SET_PARA--0 addr0:%x addr1:%x ret:%d",layer_info.fb.addr[0],layer_info.fb.addr[1],ret);
	
			args[0] = screen;
			args[1] = (unsigned long) overlay;
			args[2] = 0;
			args[3] = 0;
			ioctl(fd, DISP_CMD_LAYER_OPEN, args);
	
			args[0] = screen;
			args[1] = (unsigned long) overlay;
			args[2] = 0;
			args[3] = 0;
			ioctl(fd, DISP_CMD_VIDEO_START, args);
	
		}
	    else
	    {
	    	//LOGV("set FB");
	    	args[0]							= screen;
	        args[1]                         = (unsigned long)overlay;
	    	args[2]                         = (unsigned long)(&tmpFrmBufAddr);
	    	args[3]                         = 0;
	    	ret = ioctl(fd, DISP_CMD_VIDEO_SET_FB,args);
	    }
	}

    return 0;
}

static int overlay_setParameter(struct overlay_control_device_t *dev,
        overlay_t* overlay, int param, int value)
{
    LOG_FUNCTION_NAME;

    overlay_object  *overlayobj = (overlay_object *)(overlay);
    overlay_ctrl_t  *stage      = ((overlay_object *)(overlay))->staging();
    int             rc = 0;

    switch (param) 
    {
        case OVERLAY_DITHER:
            break;

        case OVERLAY_SETVIDEOPARA:
            
            break;

        case OVERLAY_GETCURFRAMEPARA:

            break;

        case OVERLAY_QUERYVBI:
            break;
        
        case OVERLAY_TRANSFORM:
            switch ( value )
            {
                case 0:
                    stage->rotation = 0;
                    stage->flip = 0;
                    break;
                case OVERLAY_TRANSFORM_ROT_90:
                    stage->rotation = 90;
                    stage->flip = 0;
                    break;
                case OVERLAY_TRANSFORM_ROT_180:
                    stage->rotation = 180;
                    stage->flip = 0;
                    break;
                case OVERLAY_TRANSFORM_ROT_270:
                    stage->rotation = 270;
                    stage->flip = 0;
                    break;
                // FIMC VFLIP = android overlay FLIP_H.
                case OVERLAY_TRANSFORM_FLIP_H:
                    stage->rotation = 0;
                    stage->flip = V4L2_CID_VFLIP;
                    break;
                case OVERLAY_TRANSFORM_FLIP_V:
                    stage->rotation = 0;
                    stage->flip = V4L2_CID_HFLIP;
                    break;
                // FIMC rotates first but android flips first.
                case OVERLAY_TRANSFORM_ROT_90+OVERLAY_TRANSFORM_FLIP_H:
                    stage->rotation = 90;
                    stage->flip = V4L2_CID_HFLIP;
                    break;
                case OVERLAY_TRANSFORM_ROT_90+OVERLAY_TRANSFORM_FLIP_V:
                    stage->rotation = 90;
                    stage->flip = V4L2_CID_VFLIP;
                    break;

                default:
                    rc = -EINVAL;
                    break;
            }
            break;
    }

    return rc;
}

static int overlay_stage(struct overlay_control_device_t *dev,
        overlay_t* overlay) 
{
    return 0;
}

static int overlay_commit(struct overlay_control_device_t *dev,
        overlay_t* overlay) 
{
    LOG_FUNCTION_NAME;

    return 0;
}

static int overlay_control_close(struct hw_device_t *dev)
{
    LOG_FUNCTION_NAME;

    struct overlay_control_context_t* ctx =(struct overlay_control_context_t*)dev;
    overlay_object *overlay_v1;

    if (ctx) 
    {
        overlay_v1 = (overlay_object *)(ctx->overlay_video1);

        overlay_destroyOverlay((struct overlay_control_device_t *)ctx,
                overlay_v1);

        free(ctx);
    }
    return 0;
}

/****************************************************************************
 * Data module
 *****************************************************************************/

int overlay_initialize(struct overlay_data_device_t *dev,
        overlay_handle_t handle)
{
    LOG_FUNCTION_NAME;

    struct overlay_data_context_t* ctx = (struct overlay_data_context_t*)dev;
    struct stat stat;

    int i;
    int rc = 0;

    LOGV("init handle = %x\n",(unsigned long)handle);
    LOGV("handle_overlay(handle) = %x\n",handle_overlay(handle));
    LOGV("handle_width(handle) = %x\n",handle_width(handle));
    LOGV("handle_height(handle) = %x\n",handle_height(handle));
    LOGV("handle_format(handle) = %x\n",handle_format(handle));
    LOGV("handle_ctl_fd(handle) = %x\n",handle_ctl_fd(handle));
    ctx->width        		= handle_width(handle);
    ctx->height       		= handle_height(handle);
    ctx->format       		= handle_format(handle);
    ctx->ctl_fd       		= handle_ctl_fd(handle);
    ctx->shared_fd    		= handle_shared_fd(handle);
    ctx->shared_size  		= handle_shared_size(handle);
    ctx->overlayhandle		= handle_overlay(handle);
    ctx->overlay_handle		= handle;
    ctx->shared       		= NULL;
    ctx->overlay_ready		= true;

    if (fstat(ctx->ctl_fd, &stat)) 
    {
        LOGE("Error = %s from %s\n", strerror(errno), "overlay initialize");
        return -1;
    }

    if (open_shared_data(ctx)) 
    {
        return -1;
    }

    ctx->shared->dataReady  = 0;

    return ( rc );
}

static int overlay_resizeInput(struct overlay_data_device_t *dev, uint32_t w,
        uint32_t h)
{
    int rc = -1;

    return rc;
}


static int overlay_data_setParameter(struct overlay_data_device_t *dev,
        int param, int value)
{
    int ret = 0;
    struct overlay_data_context_t* ctx = (struct overlay_data_context_t*)dev;
    void  *overlay;
    int    ctl_fd;

    if (ctx->shared == NULL) 
    {
        LOGI("Shared Data Not Init'd!\n");
        return -1;
    }

    if (ctx->shared->dataReady) 
    {
        LOGI("Too late. Cant set it now!\n");
        return -1;
    }

    overlay  = ctx->overlayhandle;
    ctl_fd   = ctx->ctl_fd;

    if(param == OVERLAY_SETFRAMEPARA)
    {
    	//LOGV("set parameter overlay = %x",(unsigned long)overlay);
    	ret = overlay_setFrameParameter(ctx,value);
    }
    else if(param == OVERLAY_GETCURFRAMEPARA)
    {
    	ret = ioctl(ctl_fd, DISP_CMD_VIDEO_GET_FRAME_ID, args);
    	if(ret == -1)
    	{
    		ret = ctx->cur_frameid;
    		LOGV("OVERLAY_GETCURFRAMEPARA =%d",ret);
    	}
    }
    else if(param == OVERLAY_SETSCREEN)
    {
    	LOGV("param == OVERLAY_SETSCREEN,value = %d\n",value);
    	ret = overlay_setScreenid(ctx,value);
    }
	else if(param == OVERLAY_SHOW)
	{
		LOGV("param == OVERLAY_SHOW,value = %d\n",value);
    	ret = overlay_show(ctx,value);
	}
	else if(param == OVERLAY_SET3DMODE)
	{
		LOGV("param == OVERLAY_SET3DMODE,value = %d\n",value);
    	ret = overlay_set3Dmode(ctx,value);
	}
	else if(param == OVERLAY_SETFORMAT)
	{
		LOGV("param == OVERLAY_SETFORMAT,value = %d\n",value);
    	ret = overlay_setFormat(ctx,value);
	}

    return ( ret );
}


static int overlay_setCrop(struct overlay_data_device_t *dev, uint32_t x,
        uint32_t y, uint32_t w, uint32_t h) 
{
    LOG_FUNCTION_NAME;

	int                         ctl_fd;
    void                        *overlay;
    int                         fd;
    int							handle;
    int                         ret = 0;
    int                         screen;
    
    LOGV("overlay_setCrop, (%d, %d) wxh: %dx%d", x, y, w, h);

	struct overlay_data_context_t* ctx = (struct overlay_data_context_t*)dev;
    overlay                         = ctx->overlayhandle;
    ctl_fd                          = ctx->ctl_fd;
    screen                          = handle_screenid(ctx->overlay_handle);
   
	handle							= (unsigned long)overlay;
    fd                              = ctl_fd;
    
    if(ctx->overlay_ready)
   	{
	    unsigned long			tmp_args[4];
		__disp_layer_info_t		tmpLayerAttr;

		tmp_args[0] 			= screen;
		tmp_args[1] 			= (unsigned long)overlay;
		tmp_args[2] 			= (unsigned long) (&tmpLayerAttr);
		tmp_args[3] 			= 0;

		ret = ioctl(fd, DISP_CMD_LAYER_GET_PARA, &tmp_args);
		LOGV("DISP_CMD_LAYER_GET_PARA, ret %d", ret);

		tmpLayerAttr.fb.size.width 		= w;
		tmpLayerAttr.fb.size.height 	= h;

		tmpLayerAttr.src_win.x			= x;
	    tmpLayerAttr.src_win.y			= y;
	    tmpLayerAttr.src_win.width		= w;
	    tmpLayerAttr.src_win.height		= h;

		ret = ioctl(fd, DISP_CMD_LAYER_SET_PARA, &tmp_args);
		LOGV("DISP_CMD_LAYER_GET_PARA, ret %d", ret);
	}

    return ret;
}

static int overlay_getCrop(struct overlay_data_device_t *dev , uint32_t* x,
        uint32_t* y, uint32_t* w, uint32_t* h) 
{
    return 0;
}

int overlay_dequeueBuffer(struct overlay_data_device_t *dev,
        overlay_buffer_t *buffer) 
{
    /* blocks until a buffer is available and return an opaque structure
     * representing this buffer.
     */

    return 0;
}

int overlay_queueBuffer(struct overlay_data_device_t *dev,
        overlay_buffer_t buffer) 
{
    return 0;
}

void *overlay_getBufferAddress(struct overlay_data_device_t *dev,
        overlay_buffer_t buffer)
{
    LOG_FUNCTION_NAME;

    /* this may fail (NULL) if this feature is not supported. In that case,
     * presumably, there is some other HAL module that can fill the buffer,
     * using a DSP for instance
     */
    int ret;
    struct v4l2_buffer buf;
    struct overlay_data_context_t* ctx = (struct overlay_data_context_t*)dev;

    return NULL;
}

int overlay_getBufferCount(struct overlay_data_device_t *dev)
{
    LOG_FUNCTION_NAME;

    struct overlay_data_context_t* ctx = (struct overlay_data_context_t*)dev;

    return (0);
}

static int overlay_data_close(struct hw_device_t *dev) {

    LOG_FUNCTION_NAME;

    struct overlay_data_context_t* ctx = (struct overlay_data_context_t*)dev;
    int rc;

    if (ctx) 
    {
        overlay_data_device_t *overlay_dev = &ctx->device;

        free(ctx);
    }

    return 0;
}

/*****************************************************************************/

static int overlay_device_open(const struct hw_module_t* module,
        const char* name, struct hw_device_t** device)
{
    LOG_FUNCTION_NAME;
    int status = -EINVAL;

    if (!strcmp(name, OVERLAY_HARDWARE_CONTROL)) 
    {
        struct overlay_control_context_t *dev;
        dev = (overlay_control_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag          = HARDWARE_DEVICE_TAG;
        dev->device.common.version      = 0;
        dev->device.common.module       = const_cast<hw_module_t*>(module);
        dev->device.common.close        = overlay_control_close;

        dev->device.get                 = overlay_get;
        dev->device.createOverlay       = overlay_createOverlay;
        dev->device.destroyOverlay      = overlay_destroyOverlay;
        dev->device.setPosition         = overlay_setPosition;
        dev->device.getPosition         = overlay_getPosition;
        dev->device.setParameter        = overlay_setParameter;
        dev->device.stage               = overlay_stage;
        dev->device.commit              = overlay_commit;

        *device                         = &dev->device.common;
        status                          = 0;
    } 
    else if (!strcmp(name, OVERLAY_HARDWARE_DATA)) 
    {
        struct overlay_data_context_t *dev;
        dev = (overlay_data_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* initialize the procs */
        dev->device.common.tag          = HARDWARE_DEVICE_TAG;
        dev->device.common.version      = 0;
        dev->device.common.module       = const_cast<hw_module_t*>(module);
        dev->device.common.close        = overlay_data_close;

        dev->device.initialize          = overlay_initialize;
        dev->device.resizeInput         = overlay_resizeInput;
        dev->device.setCrop             = overlay_setCrop;
        dev->device.getCrop             = overlay_getCrop;
        dev->device.setParameter        = overlay_data_setParameter;
        dev->device.dequeueBuffer       = overlay_dequeueBuffer;
        dev->device.queueBuffer         = overlay_queueBuffer;
        dev->device.getBufferAddress    = overlay_getBufferAddress;
        dev->device.getBufferCount      = overlay_getBufferCount;

        *device                         = &dev->device.common;
        status                          = 0;
    }
    return status;
}

