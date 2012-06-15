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
#include "Database/MongodbClient.h"
#include "Database/MongodbObject.h"
#include "mongodb/mongo.h"

#include "Utils/StringFormat.h"


namespace DAVA 
{
class MongodbClientInternalData: public BaseObject
{
public:

	MongodbClientInternalData()
	{
		connection = new mongo();
		DVASSERT(connection);

		Memset(connection, 0, sizeof(mongo));
		mongo_init(connection);
	}

	virtual ~MongodbClientInternalData()
	{
		mongo_destroy(connection);
		SafeDelete(connection);
	}

public:
	mongo *connection;
};


MongodbClient * MongodbClient::Create(const String &ip, int32 port)
{
	MongodbClient * client = new MongodbClient();
	if (client)
	{
        bool ret = client->Connect(ip, port);
        if(!ret)
        {
            SafeRelease(client);
            Logger::Error("[MongodbClient] can't connect to database");
        }
	}
	return client;
}
	
MongodbClient::MongodbClient()
{
#if defined (__DAVAENGINE_WIN32__)
    mongo_init_sockets();
#endif //#if defined (__DAVAENGINE_WIN32__)
    
	clientData = new MongodbClientInternalData();
    
    SetDatabaseName(String("Database"));
    SetCollectionName(String("Collection"));
}

MongodbClient::~MongodbClient()
{
    for(int32 i = 0; i < (int32)objects.size(); ++i)
    {
        SafeRelease(objects[i]);
    }
	objects.clear();
	
	Disconnect();

	SafeRelease(clientData);
}

bool MongodbClient::Connect(const String &ip, int32 port)
{
    
    int32 status = mongo_connect(clientData->connection, ip.c_str(), port );
    if(MONGO_OK != status)
    {
        LogError(String("Connect"), clientData->connection->err);
    }
    
    return (MONGO_OK == status);
}
    
void MongodbClient::Disconnect()
{
    if(IsConnected())
    {
        mongo_disconnect(clientData->connection);
    }
}

void MongodbClient::LogError(const String functionName, int32 errorCode)
{
    static String ERROR_MESSAGES[] = 
    {
        String("Connection success!"), //MONGO_CONN_SUCCESS
        String("Could not create a socket."), //MONGO_CONN_NO_SOCKET
        String("An error occured while calling connect()."), //MONGO_CONN_FAIL
        String("An error occured while calling getaddrinfo()."), //MONGO_CONN_ADDR_FAIL
        String("Warning: connected to a non-master node (read-only)."), //MONGO_CONN_NOT_MASTER
        String("Given rs name doesn't match this replica set."), //MONGO_CONN_BAD_SET_NAME
        String("Can't find primary in replica set. Connection closed."), //MONGO_CONN_NO_PRIMARY
        String("An error occurred while reading or writing on the socket."), //MONGO_IO_ERROR
        String("Other socket error."), //MONGO_SOCKET_ERROR
        String("The response is not the expected length."), //MONGO_READ_SIZE_ERROR
        String("The command returned with 'ok' value of 0."), //MONGO_COMMAND_FAILED
        String("Write with given write_concern returned an error."), //MONGO_WRITE_ERROR
        String("The name for the ns (database or collection) is invalid."), //MONGO_NS_INVALID
        String("BSON not valid for the specified op."), //MONGO_BSON_INVALID
        String("BSON object has not been finished."), //MONGO_BSON_NOT_FINISHED
        String("BSON object exceeds max BSON size."), //MONGO_BSON_TOO_LARGE
        String("Supplied write concern object is invalid.") //MONGO_WRITE_CONCERN_INVALID
    };
    
    DVASSERT((MONGO_CONN_SUCCESS <= errorCode) && (errorCode <= MONGO_WRITE_CONCERN_INVALID));
    
    Logger::Error("[MongodbClient] error %d at function %s", errorCode, functionName.c_str());
    Logger::Error("[MongodbClient] %s", ERROR_MESSAGES[errorCode].c_str()); 
}
    
void MongodbClient::SetDatabaseName(const String &newDatabase)
{
    database = newDatabase;
    namespaceName = database + String(".") + collection;
}

void MongodbClient::SetCollectionName(const String &newCollection)
{
    collection = newCollection;
    namespaceName = database + String(".") + collection;
}
    
    
void MongodbClient::DropDatabase()
{
    int32 status = mongo_cmd_drop_db(clientData->connection, database.c_str());
    if(MONGO_OK != status)
    {
        LogError(String("DropDatabase"), clientData->connection->err);
    }
}
    
void MongodbClient::DropCollection()
{
    int32 status = mongo_cmd_drop_collection(clientData->connection, database.c_str(), collection.c_str(), NULL);
    if(MONGO_OK != status)
    {
        LogError(String("DropCollection"), clientData->connection->err);
    }
}

    
bool MongodbClient::IsConnected()
{
    return (0 != mongo_is_connected(clientData->connection));
}

bool MongodbClient::SaveBinary(const String &key, uint8 *data, int32 dataSize)
{
    int32 status = MONGO_ERROR;
    if(IsConnected())
    {
        MongodbObject * binary = CreateObject();
        DVASSERT(binary);
        
        binary->SetObjectName(key);
        binary->AddInt32(String("DataSize").c_str(), dataSize);
        binary->AddData(String("Data").c_str(), data, dataSize);
        binary->Finish();
        
        
        MongodbObject *foundObject = FindObjectByKey(key);
        if(foundObject)
        {
            status = mongo_update(clientData->connection, namespaceName.c_str(), (bson *)foundObject->InternalObject(), (bson *)binary->InternalObject(), 0, NULL);
            if(MONGO_OK != status)
            {
                LogError(String("SaveBinary, update"), clientData->connection->err);
            }
            
            DestroyObject(foundObject);
        }
        else 
        {
            status = mongo_insert(clientData->connection, namespaceName.c_str(), (bson *)binary->InternalObject(), NULL);
            if(MONGO_OK != status)
            {
                LogError(String("SaveBinary, insert"), clientData->connection->err);
            }
        }
        
        DestroyObject(binary);
    }
    
    return (MONGO_OK == status);
}



int32 MongodbClient::GetBinarySize(const String &key)
{
    int32 retSize = 0;
    
    MongodbObject *object = FindObjectByKey(key);
    if(object)
    {
        retSize = object->GetInt32(String("DataSize"));
        DestroyObject(object);
    }
    else 
    {
        Logger::Error("[MongodbClient] Can't find binary to get size.");
    }
    
    return retSize;
}
    
bool MongodbClient::GetBinary(const String &key, uint8 *outData, int32 dataSize)
{
    bool found = false;
    
    MongodbObject *object = FindObjectByKey(key);
    if(object)
    {
        found = object->GetData(String("Data"), outData, dataSize);
        DestroyObject(object);
    }
    else 
    {
        Logger::Error("[MongodbClient] Can't find binary to get data.");
    }
    
    return found;
}

MongodbObject * MongodbClient::FindObjectByKey(const String &key)
{
    MongodbObject *query = CreateObject();
    DVASSERT(query);
    
    query->SetObjectName(key);
    query->Finish();
    
    MongodbObject *foundObject = CreateObject();
    DVASSERT(foundObject);
    
    int32 status = mongo_find_one(clientData->connection, namespaceName.c_str(), (bson *)query->InternalObject(), 0, (bson *)foundObject->InternalObject());
    if(MONGO_OK != status)
    {
        DestroyObject(foundObject);
        foundObject = NULL;
    }
        
    DestroyObject(query);
    return foundObject;
}
    
MongodbObject * MongodbClient::CreateObject()
{
    MongodbObject *object = new MongodbObject();
    objects.push_back(object);
    
    return object;
}

void MongodbClient::DestroyObject(DAVA::MongodbObject *object)
{
    Vector<MongodbObject *>::const_iterator endIt = objects.end();
    for(Vector<MongodbObject *>::iterator it= objects.begin(); it != endIt; ++it)
    {
        if(*it == object)
        {
            SafeRelease(object);
            objects.erase(it);
            break;
        }
    }
}
    
bool MongodbClient::SaveObject(MongodbObject *object)
{
    int32 status = MONGO_ERROR;
    if(IsConnected())
    {
        MongodbObject *foundObject = FindObjectByKey(object->GetObjectName());
        if(foundObject)
        {
            status = mongo_update(clientData->connection, namespaceName.c_str(), (bson *)foundObject->InternalObject(), (bson *)object->InternalObject(), 0, NULL);
            if(MONGO_OK != status)
            {
                LogError(String("SaveObject, update"), clientData->connection->err);
            }
            
            DestroyObject(foundObject);
        }
        else 
        {
            status = mongo_insert(clientData->connection, namespaceName.c_str(), (bson *)object->InternalObject(), NULL);
            if(MONGO_OK != status)
            {
                LogError(String("SaveObject, insert"), clientData->connection->err);
            }
        }
    }
    
    return (MONGO_OK == status);
}

    
void MongodbClient::DumpDB()
{
    Logger::Debug("***** MONGO DUMP *******");

    bson query;
    bson_empty(&query);
    
    mongo_cursor *cursor = mongo_find(clientData->connection, namespaceName.c_str(), &query, NULL, 0, 0, 0);
    int32 count = 0;
    while( mongo_cursor_next( cursor ) == MONGO_OK )
    {
        const bson *currentObject = mongo_cursor_bson(cursor);
        
        Logger::Debug(Format("BSON[%d]:", count));
        bson_print(currentObject);
        
        ++count;
    }

    mongo_cursor_destroy(cursor);
    
    Logger::Debug("Count: %d", count);
    Logger::Debug("************************");
}
    
    
}