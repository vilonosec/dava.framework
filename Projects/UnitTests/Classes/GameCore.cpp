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
#include "GameCore.h"

#include "Database/MongodbObject.h"


#include "Config.h"
#include "BaseScreen.h"
#include "SampleTest.h"
#include "EntityTest.h"

using namespace DAVA;

GameCore::GameCore()
{
    logFile = NULL;
    
	dbClient = NULL;

    currentScreen = NULL;
    
    currentScreenIndex = 0;
    currentTestIndex = 0;
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
	RenderManager::Instance()->SetFPS(60);

    CreateDocumentsFolder();

    new SampleTest();
	new EntityTest(); 
    
    errors.reserve(TestCount());

    RunTests();
}

void GameCore::RegisterScreen(BaseScreen *screen)
{
    UIScreenManager::Instance()->RegisterScreen(screen->GetScreenId(), screen);
    screens.push_back(screen);
}


void GameCore::CreateDocumentsFolder()
{
    String documentsPath = String(FileSystem::Instance()->GetUserDocumentsPath()) + "UnitTests/";
    
    FileSystem::Instance()->CreateDirectory(documentsPath, true);
    FileSystem::Instance()->SetCurrentDocumentsDirectory(documentsPath);
}


File * GameCore::CreateDocumentsFile(const String &filePathname)
{
    String workingFilepathname = FileSystem::Instance()->FilepathInDocuments(filePathname);
    
    String folder, filename;
    FileSystem::Instance()->SplitPath(workingFilepathname, folder, filename);
    
    FileSystem::Instance()->CreateDirectory(folder, true);
    
	File *retFile = File::Create(workingFilepathname, File::CREATE | File::WRITE);
    return retFile;
}

void GameCore::OnAppFinished()
{
	int32 errorsSize = errors.size();
    for(int32 i = 0; i < errorsSize; ++i)
    {
        SafeDelete(errors[i]);
    }
    errors.clear();

    
	int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        SafeRelease(screens[i]);
    }
    screens.clear();

    SafeRelease(logFile);
}

void GameCore::OnSuspend()
{
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{	
}

void GameCore::BeginFrame()
{
	ApplicationCore::BeginFrame();
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
}

void GameCore::Update(float32 timeElapsed)
{	
    ProcessTests();
	ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
	ApplicationCore::Draw();
}

void GameCore::RunTests()
{
    currentTestIndex = 0;
	int32 screensSize = screens.size();
    for(int32 iScr = 0; iScr < screensSize; ++iScr)
    {
        int32 count = screens[iScr]->GetTestCount();
        if(0 < count)
        {
            currentScreen = screens[iScr];
            currentScreenIndex = iScr;
            break;
        }
    }
    
    if(currentScreen)
    {
        UIScreenManager::Instance()->SetFirst(currentScreen->GetScreenId());
    }
    else 
    {
        logFile->WriteLine(String("There are no tests."));
        Core::Instance()->Quit();
    }
}


void GameCore::FinishTests()
{
    FlushTestResults();
    Core::Instance()->Quit();
}

void GameCore::LogMessage(const String &message)
{
    if(!logFile)
    {
        time_t logStartTime = time(0);
        logFile = CreateDocumentsFile(Format("Reports/%lld.errorlog", logStartTime));
        DVASSERT(logFile);
    }
    
    if(logFile)
    {
        logFile->WriteLine(message);
    }
}

int32 GameCore::TestCount()
{
    int32 count = 0;
    int32 screensSize = screens.size();
    for(int32 i = 0; i < screensSize; ++i)
    {
        count += screens[i]->GetTestCount();
    }
    
    return count;
}

void GameCore::ProcessTests()
{
    if(currentScreen && currentScreen->ReadyForTests())
    {
        bool ret = currentScreen->RunTest(currentTestIndex);
        if(ret)
        {
            ++currentTestIndex;
            if(currentScreen->GetTestCount() == currentTestIndex)
            {
                ++currentScreenIndex;
                if(currentScreenIndex == screens.size())
                {
                    FinishTests();
                }
                else 
                {
                    currentScreen = screens[currentScreenIndex];
                    currentTestIndex = 0;
                    UIScreenManager::Instance()->SetScreen(currentScreen->GetScreenId());
                }
            }
        }
    }
}


void GameCore::FlushTestResults()
{
    MongodbObject *logObject = NULL;
    if(ConnectToDB())
 	{
        //TODO: test only
//      dbClient->DropCollection();
//      dbClient->DropDatabase();
        //TODO: test only

        logObject = GetObjectForName(PLATFORM_NAME);
 	}
 	else
 	{
        LogMessage(String("Can't connect to DB"));
 	}
    
    
    int32 errorCount = (int32)errors.size();
    
    File *reportFile = CreateDocumentsFile(String("Errors.txt"));
    if(reportFile)
    {
        if(0 < errorCount)
        {
            reportFile->WriteLine(String("Failed tests:"));
            for(int32 i = 0; i < errorCount; ++i)
            {
                ErrorData *error = errors[i];

                String errorString = String(Format("command %s at file %s at line %d", 
                                                   error->command.c_str(), error->filename.c_str(), error->line));
                
                reportFile->WriteLine(String(Format("Error[%d]: ", i+1)) + errorString);
                if(logObject)
                {
                    logObject->AddString(String(Format("Error_%d", i+1)), errorString);
                }
            }
        }
        else 
        {
            String successString = String("All test passed.");
            reportFile->WriteLine(successString);
            if(logObject)
            {
                logObject->AddString(String("TestResult"), successString);
            }
        }
        
        SafeRelease(reportFile);
    }
    
    if(logObject)
    {
        logObject->Finish();
        dbClient->SaveObject(logObject);
        dbClient->DestroyObject(logObject);
    }
    
    if(dbClient)
    {
        dbClient->Disconnect();
        SafeRelease(dbClient);
    }
}


void GameCore::RegisterError(const String &command, const String &fileName, int32 line)
{
    ErrorData *newError = new ErrorData();
        
    if(newError)
    {
        newError->command = command;
        newError->filename = fileName;
        newError->line = line;
        
        errors.push_back(newError);
    }
    else 
    {
        LogMessage(String("Can't allocate ErrorData"));
    }
}

bool GameCore::ConnectToDB()
{
    DVASSERT(NULL == dbClient);
    
    dbClient = MongodbClient::Create(DATABASE_IP, DATAPASE_PORT);
    if(dbClient)
    {
        dbClient->SetDatabaseName(DATABASE_NAME);
        dbClient->SetCollectionName(DATABASE_COLLECTION);
    }
    
    return (NULL != dbClient);
}

MongodbObject * GameCore::GetObjectForName(const String &testName)
{
    MongodbObject *logObject = dbClient->CreateObject();
    if(logObject)
    {
        logObject->SetObjectName(testName);
    }

    return logObject;
}