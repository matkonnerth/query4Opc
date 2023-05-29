#include <graph/HierachicalVisitor.h>
#include <gtest/gtest.h>

using graph::HierachicalVisitor;

TEST(HierachicalVisitorTest, nodeClassBrowse)
{
    ASSERT_EQ(UA_NODECLASS_OBJECT | UA_NODECLASS_OBJECTTYPE,
                HierachicalVisitor::calculateNodeClassMaskForBrowse(UA_NODECLASS_OBJECT));
}