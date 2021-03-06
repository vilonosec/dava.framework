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
#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"
#include "Test.h"
#include "ResultScreen.h"
#include "Database/MongodbClient.h"

class TestScreen;

class GameCore : public DAVA::ApplicationCore
{
public:	
	GameCore();
	virtual ~GameCore();
    
    static GameCore * Instance()
	{
		return (GameCore*) DAVA::Core::GetApplicationCore();
	};
    
	virtual void OnAppStarted();
	virtual void OnAppFinished();
	
	virtual void OnSuspend();
	virtual void OnResume();
	virtual void OnBackground();
	
	virtual void BeginFrame();
	virtual void Update(DAVA::float32 update);
	virtual void Draw();
	
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
	virtual void OnDeviceLocked() {}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    
    bool FlushToDB(const String & levelName, const Map<String, String> &results, const String &imagePath);
    
private:
    bool ConnectToDB();

    MongodbClient * dbClient;
	int32 currentRunId;

	DAVA::Cursor * cursor;
    
	int32 testCount;
	List<Test*> tests;
	ResultScreen *resultScreen;
	
	bool appFinished;
};



#endif // __GAMECORE_H__