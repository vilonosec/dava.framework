/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#ifndef __DAVAENGINE_IMAGE_H__
#define __DAVAENGINE_IMAGE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"

namespace DAVA 
{

class File;
#ifdef __DAVAENGINE_IPHONE__

class SaveToSystemPhotoCallbackReceiver
{
public:
    virtual void SaveToSystemPhotosFinished() = 0;
};
    
#endif

class Image : public BaseObject
{
public:
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static const int MAX_WIDTH = 1024;
	static const int MIN_WIDTH = 8;
	static const int MAX_HEIGHT = 1024;
	static const int MIN_HEIGHT = 8;
#else //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	static const int MAX_WIDTH = 4096;
	static const int MIN_WIDTH = 8;
	static const int MAX_HEIGHT = 4096;
	static const int MIN_HEIGHT = 8;
#endif //#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
	
	Image();
	virtual ~Image();
	
	static Image * Create(uint32 width, uint32 height, PixelFormat format);
    // \todo Change function name to Image::Create for consistency
	static Vector2 GetImageSize(const String & pathName);
	
	inline uint32 GetWidth() const;
	inline uint32 GetHeight() const;
	inline uint8 * GetData() const;
	inline PixelFormat GetPixelFormat() const;

    
#ifdef __DAVAENGINE_IPHONE__
    void SaveToSystemPhotos(SaveToSystemPhotoCallbackReceiver* callback = 0);
#endif
    
    // changes size of image canvas to required size, if new size is bigger, sets 0 to all new pixels
    void ResizeCanvas(uint32 newWidth, uint32 newHeight);
    
	// changes size of image to required size (without any filtration)
	void ResizeImage(uint32 newWidth, uint32 newHeight);

	/*
     //	void ConvertToFormat(PixelFormat format);
        \todo extract all image format conversion functions to separate functions to allow to use them in different places, like textures.
        enum eAlphaAction
        {  
            ALPHA_ACTION_NONE,
            ALPHA_ACTION_REMOVE_PREMULTIPLICATION,
            ALPHA_ACTION_ADD_PREMULTIPLICATION, 
        };
        static void ConvertFromRGBA8888toRGB565(const uint8 * sourceData, int32 width, int32 height, uint8 * destData, eAlphaAction action = ALPHA_ACTION_NONE);
        static void ConvertFromRGBA8888toRGBA4444(const uint8 * sourceData, int32 width, int32 height, uint8 * destData, eAlphaAction action = ALPHA_ACTION_NONE);
        static void ConvertFromRGBA8888toA8(const uint8 * sourceData, int32 width, int32 height, uint8 * destData, eAlphaAction action = ALPHA_ACTION_NONE);
        
     */

	uint8 * data;
    uint32 dataSize;
	uint32	width;
	uint32	height;
	PixelFormat format;
};
	
// Implementation of inline functions
uint32 Image::GetWidth() const
{
	return width;
}
uint32 Image::GetHeight() const
{
	return height;
}
uint8 * Image::GetData() const
{
	return data;
}
PixelFormat Image::GetPixelFormat() const
{
	return format;
}

	
};

#endif // __DAVAENGINE_IMAGE_H__
