#pragma once

#include <string>

namespace UpgradeValue
{
    class HttpClient
    {
    public:
        HttpClient();
        ~HttpClient();

        HttpClient(const HttpClient&) = delete;
        HttpClient& operator=(const HttpClient&) = delete;

        bool Get(const std::wstring& pathAndQuery,
                 const std::string& bearerToken,
                 std::string& response,
                 std::string& error) const;

    private:
        void* session_ = nullptr;
    };
}
