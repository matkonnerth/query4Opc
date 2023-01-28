#include <graph/PathMatcher.h>
#include <graph/BrowseResultWrapper.h>

using namespace graph;
PathMatcher::PathMatcher(UA_Server* server, const Path& path, size_t startIndex)
: m_server{ server }
, m_path{ path }
, m_idx{ startIndex }
, m_results{ path.size() }
{
    auto paths = m_path.split(startIndex);
    m_lhs = paths.first;
    //traversing the path is always from left to right
    m_lhs.invert();
    m_lhs.insertDummyNode();
    m_rhs = paths.second;
}

PathMatcher::PathMatcher(PathMatcher&& other)
: m_server{ other.m_server }
, m_path{ other.m_path }
, m_idx{ other.m_idx }
, m_results{ other.m_results }
{}

PathMatcher& PathMatcher::operator=(PathMatcher&& other)
{
    m_server = other.m_server;
    m_path = std::move(other.m_path);
    m_idx = other.m_idx;
    m_results = std::move(other.m_results);
    return *this;
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
    auto rResults = check(startNode, m_rhs);
    std::vector<path_t> endResult{};
    for (auto& r : rResults)
    {
        if (m_idx > 0)
        {
            //check against inverted lhs
            auto lResults = check(r.at(0), m_lhs);

            //invert lResults
            std::reverse(lResults.begin(), lResults.end());
            //merge            
            for (auto& l : lResults)
            {
                //TODO: why +1??
                r.insert(r.begin(),
                         std::make_move_iterator(l.begin()+1),
                         std::make_move_iterator(l.end()));
                endResult.push_back(std::move(r));
            }
        }
        else
        {
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
PathMatcher::check(const UA_ReferenceDescription& start, const Path& path)
{
    std ::vector<path_t> paths;
    path_t actPath{};
    actPath.push_back(start);


    auto it = path.it();

    const Node* node{nullptr};

    while(node = it.next())
    {
        auto result = false;
        auto bd = createBrowseDescription(actPath.back().nodeId.nodeId,
                                          it.RelationLHS()->direction,
                                          it.RelationLHS()->referenceType,
                                          node->nodeClass);
        BrowseResultWrapper br{ UA_Server_browse(m_server, 1000, &bd) };
        if (br.raw().statusCode != UA_STATUSCODE_GOOD)
        {
            return std::vector<path_t>{};
        }

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
            std::vector<path_t> res;
            for (auto&& pe : m.results().paths())
            {
                path_t newPath{ actPath };
                newPath.insert(newPath.end(),
                               std::make_move_iterator(pe.begin()),
                               std::make_move_iterator(pe.end()));
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

    paths.push_back(std::move(actPath));
    return paths;
}