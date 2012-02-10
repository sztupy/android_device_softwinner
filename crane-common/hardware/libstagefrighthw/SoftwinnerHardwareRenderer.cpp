/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANSoftwinnerES OR CONDISoftwinnerONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//#define LOG_NDEBUG 0

#define LOG_TAG "SoftwinnerHardwareRenderer"
#include <utils/Log.h>

#include "SoftwinnerHardwareRenderer.h"

#include <media/stagefright/MediaDebug.h>
#include <surfaceflinger/ISurface.h>
#include <ui/Overlay.h>

#include "v4l2_utils.h"

#include "overlay.h"

#define CACHEABLE_BUFFERS 0x1

namespace android {

////////////////////////////////////////////////////////////////////////////////

SoftwinnerHardwareRenderer::SoftwinnerHardwareRenderer(
        const sp<ISurface> &surface,
        size_t displayWidth, size_t displayHeight,
        size_t decodedWidth, size_t decodedHeight,
        OMX_COLOR_FORMATTYPE colorFormat/*,int screen_id*/)
    : mISurface(surface),
      mDisplayWidth(displayWidth),
      mDisplayHeight(displayHeight),
      mDecodedWidth(decodedWidth),
      mDecodedHeight(decodedHeight),
      mColorFormat(colorFormat),
      mInitCheck(NO_INIT),
      mFrameSize(mDecodedWidth * mDecodedHeight * 2),
      mIsFirstFrame(true){
//    CHECK(mISurface.get() != NULL);
//    CHECK(mDecodedWidth > 0);
//    CHECK(mDecodedHeight > 0);
//
    if (mColorFormat < OMX_COLOR_FormatVendorStartUnused) {
    	LOGE("stagefright hw not support color format");
        return;
    }

	//LOGD("==== SoftwinnerHardwareRenderer mColorFormat:%d", mColorFormat);

    sp<OverlayRef> ref = mISurface->createOverlay(
            mDecodedWidth, mDecodedHeight, mColorFormat, 0/*, screen_id*/);

    if (ref.get() == NULL) {
        LOGE("Unable to create the overlay!");
        return;
    }

    mOverlay = new Overlay(ref);
    //mOverlay->setParameter(CACHEABLE_BUFFERS, 0);

    mInitCheck = OK;
}

SoftwinnerHardwareRenderer::~SoftwinnerHardwareRenderer() {
    if (mOverlay.get() != NULL) {
        mOverlay->destroy();
        mOverlay.clear();

        // XXX apparently destroying an overlay is an asynchronous process...
        sleep(1);
    }
}

void SoftwinnerHardwareRenderer::render(
        const void *data, size_t size,
		void *platformPrivate) {
	// CHECK_EQ(size, mFrameSize);

	if (mOverlay.get() == NULL) {
		return;
	}

	liboverlaypara_t *overlay_para;

	overlay_para = (liboverlaypara_t *) data;
	overlay_para->first_frame_flg = mIsFirstFrame;
	//LOGV("HAL::render(0x%08x 0x%08x)",overlay_para->top_y,overlay_para->top_c);

	if (!mIsFirstFrame) {
		;
	} else {
		mIsFirstFrame = false;
	}

	mOverlay->setParameter(OVERLAY_SETFRAMEPARA, (int)overlay_para);
}

int SoftwinnerHardwareRenderer::control(int cmd, int para){

	int ret;
	switch(cmd){
	case VIDEORENDER_CMD_GETFRAMEID:
		ret = mOverlay->setParameter(OVERLAY_GETCURFRAMEPARA, 0);
		break;
	case VIDEORENDER_CMD_SETSCREEN:
		ret = mOverlay->setParameter(OVERLAY_SETSCREEN, para);
		break;
	case VIDEORENDER_CMD_SET3DMODE:
		ret = mOverlay->setParameter(OVERLAY_SET3DMODE, para);
		break;
	default:
		//LOGW("unkown command");
		break;
	}

	return ret;
}

}  // namespace android

