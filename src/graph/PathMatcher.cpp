#include <graph/PathMatcher.h>
#include <graph/BrowseResultWrapper.h>
#include <graph/tracing.h>



using namespace graph;
PathMatcher::PathMatcher(UA_Server* server, const Path& path, int startIndex)
: m_server{ server }
, m_path{ path }
, m_idx{ startIndex }
, m_results{ path.size() }
{
    auto paths = m_path.split(startIndex);
    m_lhs = paths.first;
    m_lhs.transformForLeftToRightTraversal();
    m_rhs = paths.second;
}

void PathMatcher::match(const UA_ReferenceDescription& startNode)
{
    if (m_path.empty())
    {
        m_results[0].push_back(std::move(startNode));
        return;
    }

    for (auto&& p : checkPath(startNode))
    {
        m_results.emplace(std::move(p));
    }
}

const PathResult& PathMatcher::results() const
{
    return m_results;
}


std::vector<path_t> PathMatcher::checkPath(const UA_ReferenceDescription& startNode)
{
    //TODO: why always start with rhs path?
    auto rResults = check(startNode, m_rhs);
    std::vector<path_t> endResult{};

    for (const auto& r : rResults)
    {
        assert(!r.empty());
        assert(r.size()==m_rhs.nodeCount());
        
        if (m_idx > 0)
        {
            //check against inverted lhs
            auto lResults = check(r.at(0), m_lhs);

            //invert lResults
            std::reverse(lResults.begin(), lResults.end());
            //merge            
            for (auto& l : lResults)
            {
                assert(l.size()==m_lhs.nodeCount());
                assert(r.size()+l.size()-1==m_path.nodeCount());
                //TODO: +1, because first node is already contained in the rhs result
                auto rcopy = r;
                rcopy.insert(rcopy.begin(),
                         std::make_move_iterator(l.begin()+1),
                         std::make_move_iterator(l.end()));
                assert(rcopy.size()==m_path.nodeCount());
                endResult.push_back(std::move(rcopy));
            }
        }
        else
        {
            assert(r.size() == m_path.nodeCount());
            endResult.push_back(std::move(r));
        }
    }
    return endResult;
}

UA_BrowseDescription PathMatcher::createBrowseDescription(const UA_NodeId& node,
                                                          UA_BrowseDirection direction,
                                                          const UA_NodeId& refType,
                                                          UA_NodeClass nodeClass)
{
    UA_BrowseDescription bd;
    UA_BrowseDescription_init(&bd);
    bd.nodeId = node;
    bd.browseDirection = direction;
    bd.includeSubtypes = true;
    bd.referenceTypeId = refType;
    bd.resultMask = UA_BROWSERESULTMASK_REFERENCETYPEID |
                    UA_BROWSERESULTMASK_ISFORWARD | UA_BROWSERESULTMASK_TYPEDEFINITION |
                    UA_BROWSERESULTMASK_NODECLASS | UA_BROWSERESULTMASK_TARGETINFO;
    bd.nodeClassMask = nodeClass;
    return bd;
}

std::vector<path_t>
PathMatcher::check(const path_element_t& start, const Path& path)
{
    std ::vector<path_t> paths{};
    path_t actPath{};

    if(path.size()==1)
    {
        if (is_matching(start, *path.getNode(0)))
        {
            actPath.push_back(start);
            assert(actPath.size()==path.nodeCount());
            paths.push_back(actPath);
        }
        return paths;
    }
    
    actPath.push_back(start);




    auto it = path.it();
    while(auto node = it.next())
    {
        auto result = false;
        auto bd = createBrowseDescription(actPath.back().nodeId.nodeId,
                                          it.RelationLHS()->direction,
                                          it.RelationLHS()->referenceType,
                                          node->nodeClass);
        browsePathMatcher();
        BrowseResultWrapper br{ UA_Server_browse(m_server, 1000, &bd) };
        if (br.raw().statusCode != UA_STATUSCODE_GOOD)
        {
            return std::vector<path_t>{};
        }

        //this is a special case, because if it is looked for a certain nodeId, there can be only 1 result
        if (node->id.has_value())
        {
            for (const auto* ref = br.raw().references;
                 ref != br.raw().references + br.raw().referencesSize;
                 ++ref)
            {
                if (UA_NodeId_equal(&(node->id.value()), &ref->nodeId.nodeId))
                {
                    actPath.push_back(*ref);
                    result = true;
                    break;
                }
            }
        }
        else
        {
            // we have to instantiate a pathMatcher for each of this
            PathMatcher m{ m_server, path.split(1).second };
            for (const auto* ref = br.raw().references;
                 ref != br.raw().references + br.raw().referencesSize;
                 ++ref)
            {
                m.match(*ref);
            }
            std::vector<path_t> res{};
            for (auto&& pe : m.results().paths())
            {
                path_t newPath{ actPath };
                newPath.insert(newPath.end(),
                               std::make_move_iterator(pe.begin()),
                               std::make_move_iterator(pe.end()));
                assert(newPath.size() == path.nodeCount());
                res.push_back(std::move(newPath));
            }
            return res;
        }
        // no result
        if (!result)
        {
            return std::vector<path_t>{};
        }
    }

    assert(actPath.size()==path.nodeCount());
    paths.push_back(std::move(actPath));
    return paths;
}