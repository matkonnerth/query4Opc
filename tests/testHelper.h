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
                free((void*)(uintptr_t)type->typeName);
                UA_UInt32 mSize = type->membersSize;
                if (type->typeKind == UA_DATATYPEKIND_UNION)
                {
                    mSize--;
                }
                for (UA_DataTypeMember* m = type->members; m != type->members + mSize; m++)
                {
                    free((void*)m->memberName);
                    m->memberName = NULL;
                }
                free(type->members);
            }
        }
        free((void*)(uintptr_t)types->types);
        free((void*)types);
        types = next;
    }
}