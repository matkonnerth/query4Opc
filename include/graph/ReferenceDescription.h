#pragma once
#include <open62541/types.h>

namespace graph
{
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
        if (this == &other)
        {
            return *this;
        }
        UA_ReferenceDescription_clear(&m_ref);
        UA_ReferenceDescription_copy(&other.m_ref, &m_ref);
        return *this;
    }

    ReferenceDescription(ReferenceDescription&& other)
    {
        // flat copy, do not copy strings
        m_ref.browseName = other.m_ref.browseName;
        m_ref.displayName = other.m_ref.displayName;
        m_ref.isForward = other.m_ref.isForward;
        m_ref.nodeClass = other.m_ref.nodeClass;
        m_ref.nodeId = other.m_ref.nodeId;
        m_ref.referenceTypeId = other.m_ref.referenceTypeId;
        m_ref.typeDefinition = other.m_ref.typeDefinition;
        //reset other ref description to sane defaults
        UA_ReferenceDescription_init(&other.m_ref);
    }

    ReferenceDescription& operator=(ReferenceDescription&& other)
    {
        if (this == &other)
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
        // reset other ref description to sane defaults
        UA_ReferenceDescription_init(&other.m_ref);
        return *this;
    }

    const UA_ReferenceDescription& impl() const
    {
        return m_ref;
    }

 private:
    UA_ReferenceDescription m_ref{};
};

}