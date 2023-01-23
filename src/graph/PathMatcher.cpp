#include <graph/PathMatcher.h>
#include <graph/BrowseResultWrapper.h>

PathMatcher::PathMatcher(UA_Server* server, const Path& path, size_t startIndex)
: m_server{ server }
, m_path{ path }
, m_idx{ startIndex }
, m_results{ path.size() + 1 }
{
    auto paths = m_path.split(startIndex);
    m_lhs = paths.first.invert();
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
    auto rResults = checkRightSide(startNode);
    return results;
    /* TODO: left side
    std::vector<path_t> endResult{};
    for (auto& r : rResults)
    {
        if (m_idx > 0)
        {
            // man könnte auch den Path hier invertieren und ein checkrightside machen?
            // die Ergebnisse müsse man den wieder umdrehen und einfügen
            auto lResults = checkLeftSide(r.at(0));
            for (auto& l : lResults)
            {
                r.insert(r.begin(),
                         std::make_move_iterator(l.begin()),
                         std::make_move_iterator(l.end() - 1));
                endResult.push_back(std::move(r));
            }
        }
        else
        {
            endResult.push_back(std::move(r));
        }
    }
    return endResult;
    */
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

// returns paths satisfying the right side
std::vector<path_t> PathMatcher::checkRightSide(const UA_ReferenceDescription& start)
{
    return check<ResultVector<true>>(start,
                                     m_path.cbegin() + static_cast<int>(m_idx),
                                     m_path.cend());
}

std::vector<path_t> PathMatcher::checkLeftSide(const UA_ReferenceDescription& start)
{
    return check<ResultVector<false>>(start,
                                      m_path.crbegin() +
                                      static_cast<int>(m_path.size() - m_idx),
                                      m_path.crend());
}

std::vector<UA_ReferenceDescription>
PathMatcher::check(const UA_ReferenceDescription& start, const Path& path)
{
    std ::vector<Path> paths;
    std::vector<UA_ReferenceDescription> actPath{};
    actPath.addResult(start);


    auto it = path.it();

    while(auto node = it.next())
    {
        auto result = false;
        auto bd = createBrowseDescription(actPath.back().nodeId.nodeId,
                                          it.RelationLHS()->direction,
                                          it.RelationLHS()->referenceType,
                                          node->nodeClass);
        BrowseResultWrapper br{ UA_Server_browse(m_server, 1000, &bd) };
        if (br.raw().statusCode != UA_STATUSCODE_GOOD)
        {
            return Path{};
        }

        if (node.id)
        {
            for (const auto* ref = br.raw().references;
                 ref != br.raw().references + br.raw().referencesSize;
                 ++ref)
            {
                if (UA_NodeId_equal(&node->id, &ref->nodeId.nodeId))
                {
                    actPath.addResult(*ref);
                    result = true;
                    break;
                }
            }
        }
        else
        {
            // we have to instantiate a pathMatcher for each of this
            // subPaths rightToLeft is here not handled correctly
            PathMatcher m{ m_server, path.split().second };
            for (const auto* ref = br.raw().references;
                 ref != br.raw().references + br.raw().referencesSize;
                 ++ref)
            {
                m.match(*ref);
            }
            std::vector<path_t> res;
            for (auto&& pe : m.results().paths())
            {
                path_t newPath{ actPath.data };
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
            return Path{};
        }
    }
    //TODO: make path from UA_ReferenceDescription
    paths.push_back(std::move(actPath.data));
    return paths;
}