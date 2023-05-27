#pragma once
#include <future>
#include <mutex>
#include <string>
#include <unordered_map>


class Commands
{
 public:
    std::future<std::string> query(const std::string& query)
    {
        std::scoped_lock<std::mutex> m_lock{ m_mutex };
        m_queries.emplace(std::make_pair(query, ""));
        m_queryResult = std::promise<std::string>{};
        return m_queryResult.get_future();
    }

    void queryResponse(const std::string& result)
    {
        m_queryResult.set_value(result);
    }

    std::optional<std::string> getNextQuery()
    {
        std::scoped_lock<std::mutex> m_lock{ m_mutex };
        if (m_queries.empty())
        {
            return std::nullopt;
        }
        auto s = m_queries.begin()->first;
        m_queries.erase(m_queries.begin());
        return s;
    }

 private:
    std::unordered_map<std::string, std::string> m_queries{};
    std::mutex m_mutex{};
    std::promise<std::string> m_queryResult{};
};