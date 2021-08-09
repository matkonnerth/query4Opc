#pragma once
#include <open62541/server.h>

static void cleanupServer(UA_Server* server)
{
    const UA_DataTypeArray* types = UA_Server_getConfig(server)->customDataTypes;
    UA_Server_delete(server);
    while (types)
    {
        const UA_DataTypeArray* next = types->next;
        if (types->types)
        {
            for (const UA_DataType* type = types->types;
                 type != types->types + types->typesSize;
                 type++)
            {
                free(const_cast<char*>(type->typeName));
                UA_UInt32 mSize = type->membersSize;
                if (type->typeKind == UA_DATATYPEKIND_UNION)
                {
                    mSize--;
                }
                for (UA_DataTypeMember* m = type->members; m != type->members + mSize; m++)
                {
                    free(const_cast<char*>(m->memberName));
                    m->memberName = NULL;
                }
                free(type->members);
            }
        }
        free(const_cast<UA_DataType*>(types->types));
        free(const_cast<UA_DataTypeArray*>(types));
        types = next;
    }
}