#include <graph/ReferenceDescription.h>
#include <gtest/gtest.h>

using graph::ReferenceDescription;

class ReferenceDescriptionTest : public ::testing::Test
{
 protected:
    UA_ReferenceDescription m_ref;

    void SetUp() override
    {
        UA_ReferenceDescription_init(&m_ref);
        m_ref.browseName = UA_QUALIFIEDNAME_ALLOC(2, "myBrowseName");
        m_ref.nodeId = UA_EXPANDEDNODEID_STRING_ALLOC(2, "myId");
    }

    void TearDown() override
    {
        UA_ReferenceDescription_clear(&m_ref);
    }
};

TEST_F(ReferenceDescriptionTest, ctor)
{
    //ctor
    ReferenceDescription ref{m_ref};

    //copy ctor
    ReferenceDescription myRef{ref};

    //copy assignment
    ReferenceDescription ref2{m_ref};
    ref2=myRef;

    ASSERT_TRUE(UA_NodeId_equal(&ref2.impl().nodeId.nodeId, &m_ref.nodeId.nodeId));
    ASSERT_FALSE(ref2.impl().nodeId.nodeId.identifier.string.data==m_ref.nodeId.nodeId.identifier.string.data);
}