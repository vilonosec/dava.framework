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
#ifndef __DAVAENGINE_OGLHELPERS_H__
#define __DAVAENGINE_OGLHELPERS_H__

#ifdef __DAVAENGINE_ANDROID__
#include "Debug/DVAssert.h"
#endif

#define __ENABLE_OGL_DEBUG_BREAK__
#if defined(__ENABLE_OGL_DEBUG_BREAK__)
	#if defined(__DAVAENGINE_WIN32__)
		#define OGLDebugBreak() { __debugbreak(); }
	#elif defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__) // Mac & iPhone & Android
		#include <signal.h>
		#define OGLDebugBreak() { kill( getpid(), SIGINT ) ; }
    #elif defined(__DAVAENGINE_ANDROID__) // Mac & iPhone & Android
        #include <signal.h>
        #include <unistd.h>
        #define OGLDebugBreak() { kill( getpid(), SIGINT ) ; }
	#else //PLATFORMS
		//other platforms
	#endif //PLATFORMS
#else
#define OGLDebugBreak()
#endif



#if defined(__DAVAENGINE_OPENGL__)
namespace DAVA
{

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__) || (defined(__DAVAENGINE_IPHONE__) && defined (__DAVAENGINE_DEBUG__))
#define RENDER_VERIFY(command) \
{ \
	if(!Thread::IsMainThread() && RenderManager::Instance()->GetNonMainLockCount() == 0)\
	{\
		DVASSERT(0 && "Application tried to call GL or DX in separate thread without lock");\
	}\
	command;\
	GLenum err = glGetError();\
	if (err != GL_NO_ERROR)\
    {  \
        Logger::Debug("%s file:%s line:%d gl failed with errorcode: 0x%08x", #command, __FILE__, __LINE__, err);\
        OGLDebugBreak(); \
    }\
}
#elif defined(__DAVAENGINE_ANDROID__)
#define RENDER_VERIFY(command) \
{ \
    command;\
    GLenum err = glGetError();\
    if (err != GL_NO_ERROR)\
    {  \
        Logger::Debug("%s file:%s line:%d gl failed with errorcode: 0x%08x", #command, __FILE__, __LINE__, err);\
        DVASSERT(false);\
		OGLDebugBreak(); \
    }\
}
#else
#define RENDER_VERIFY(command) command;  
#endif //#if defined(__DAVAENGINE_WIN32__)
    

    
// REDEFINED OPENGL FUNCTIONS
    
#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)
    #define glBindBuffer glBindBufferARB
    #define glGenBuffers glGenBuffersARB
    #define glDeleteBuffers glDeleteBuffersARB
    #define glBufferData glBufferDataARB

    #define GL_ARRAY_BUFFER GL_ARRAY_BUFFER_ARB
    #define GL_ARRAY_BUFFER GL_ARRAY_BUFFER_ARB
    #define GL_ELEMENT_ARRAY_BUFFER GL_ELEMENT_ARRAY_BUFFER_ARB
    #define GL_STATIC_DRAW GL_STATIC_DRAW_ARB
    #define GL_DYNAMIC_DRAW GL_DYNAMIC_DRAW_ARB
#endif //#if defined(__DAVAENGINE_OPENGL_ARB_VBO__)


#if defined (__DAVAENGINE_IPHONE__)
    
    #define GL_HALF_FLOAT GL_HALF_FLOAT_OES
    #define glDeleteFramebuffers glDeleteFramebuffersOES
    #define glDeleteRenderbuffers glDeleteRenderbuffersOES
    #define glGenerateMipmap glGenerateMipmapOES
	#define glBindFramebuffer glBindFramebufferOES
    #define DAVA_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT16_OES
    
#elif defined(__DAVAENGINE_ANDROID__)
    
    #define GL_HALF_FLOAT GL_HALF_FLOAT_OES
	#define DAVA_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT16_OES
    
#elif defined(__DAVAENGINE_MACOS__)
	
    #define glDeleteFramebuffers glDeleteFramebuffersEXT
    #define glDeleteRenderbuffers glDeleteRenderbuffersEXT
    #define glGenerateMipmap glGenerateMipmapEXT
	#define glBindFramebuffer glBindFramebufferEXT
	#define glGenRenderbuffers glGenRenderbuffersEXT
	#define glBindRenderbuffer glBindRenderbufferEXT
#if !defined(GL_RENDERBUFFER)
	#define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#endif //#if !defined(GL_RENDERBUFFER)
	#define glRenderbufferStorage glRenderbufferStorageEXT
#if !defined(GL_FRAMEBUFFER)
	#define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
#endif //#if !defined(GL_FRAMEBUFFER)
#if !defined(GL_DEPTH_ATTACHMENT)
	#define GL_DEPTH_ATTACHMENT GL_DEPTH_ATTACHMENT_EXT
#endif //#if !defined(GL_DEPTH_ATTACHMENT)
#if !defined(GL_RENDERBUFFER)
    #define GL_RENDERBUFFER GL_RENDERBUFFER_EXT
#endif //#if !defined(GL_RENDERBUFFER)
    #define glCheckFramebufferStatus glCheckFramebufferStatusEXT
#if !defined(GL_FRAMEBUFFER_COMPLETE)
    #define GL_FRAMEBUFFER_COMPLETE GL_FRAMEBUFFER_COMPLETE_EXT
#endif //#if !defined(GL_FRAMEBUFFER_COMPLETE)

#define DAVA_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT
    
#elif defined(__DAVAENGINE_WIN32__)
	#define DAVA_GL_DEPTH_COMPONENT GL_DEPTH_COMPONENT
#endif //#if defined (__DAVAENGINE_IPHONE__)
    
    
};
#endif // #if defined(__DAVAENGINE_OPENGL__)
#endif // __DAVAENGINE_OGLHELPERS_H__
