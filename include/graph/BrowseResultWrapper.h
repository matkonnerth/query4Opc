#pragma once

class BrowseResultWrapper
{
 public:
    BrowseResultWrapper(UA_BrowseResult br)
    : m_br{ std::move(br) } {};
    ~BrowseResultWrapper()
    {
        UA_BrowseResult_clear(&m_br);
    }
    BrowseResultWrapper(const BrowseResultWrapper&) = delete;
    BrowseResultWrapper& operator=(const BrowseResultWrapper&) = delete;

    UA_BrowseResult& raw()
    {
        return m_br;
    };

 private:
    UA_BrowseResult m_br;
};