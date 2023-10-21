#pragma once
#include <vector>
#include <open62541/types.h>

namespace graph
{
using path_element_t = UA_ReferenceDescription;
using path_t = std::vector<path_element_t>;
using column_t = std::vector<path_element_t>;

class ReferenceDescription final
{
public:
    explicit ReferenceDescription(const UA_ReferenceDescription& ref)
    {
        UA_ReferenceDescription_copy(&ref, &m_ref);
    }

    ~ReferenceDescription()
    {
        UA_ReferenceDescription_clear(&m_ref);
    }

    ReferenceDescription(const ReferenceDescription& other)
    {
        UA_ReferenceDescription_copy(&other.m_ref, &m_ref);
    }

    ReferenceDescription& operator=(const ReferenceDescription& other)
    {
        if(this==&other)
        {
            return *this;
        }
        UA_ReferenceDescription_clear(&m_ref);
        UA_ReferenceDescription_copy(&other.m_ref, &m_ref);
    }

    ReferenceDescription(ReferenceDescription&& other)
    {
        //flat copy, do not copy strings
        m_ref.browseName=other.m_ref.browseName;
        m_ref.displayName=other.m_ref.displayName;
        m_ref.isForward=other.m_ref.isForward;
        m_ref.nodeClass=other.m_ref.nodeClass;
        m_ref.nodeId=other.m_ref.nodeId;
        m_ref.referenceTypeId=other.m_ref.referenceTypeId;
        m_ref.typeDefinition=other.m_ref.typeDefinition;
    }

    ReferenceDescription& operator=(ReferenceDescription&& other)
    {
        if(this==&other)
        {
            return *this;
        }
        UA_ReferenceDescription_clear(&m_ref);
        // flat copy, do not copy strings
        m_ref.browseName = other.m_ref.browseName;
        m_ref.displayName = other.m_ref.displayName;
        m_ref.isForward = other.m_ref.isForward;
        m_ref.nodeClass = other.m_ref.nodeClass;
        m_ref.nodeId = other.m_ref.nodeId;
        m_ref.referenceTypeId = other.m_ref.referenceTypeId;
        m_ref.typeDefinition = other.m_ref.typeDefinition;
    }

private:
    UA_ReferenceDescription m_ref{};
};

}