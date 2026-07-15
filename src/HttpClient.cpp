#include "HttpClient.h"

#include <Windows.h>
#include <winhttp.h>

#include <string>

#pragma comment(lib, "winhttp.lib")

namespace UpgradeValue
{
    namespace
    {
        std::wstring ToWide(const std::string& value)
        {
            if (value.empty()) return {};
            const int size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);
            if (size <= 0) return {};
            std::wstring result(static_cast<size_t>(size), L'\0');
            MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, result.data(), size);
            if (!result.empty() && result.back() == L'\0') result.pop_back();
            return result;
        }

        std::string ReadBody(HINTERNET request)
        {
            std::string body;
            body.reserve(64 * 1024);

            DWORD available = 0;
            while (WinHttpQueryDataAvailable(request, &available) && available > 0)
            {
                const size_t offset = body.size();
                body.resize(offset + available);
                DWORD downloaded = 0;
                if (!WinHttpReadData(request, body.data() + offset, available, &downloaded))
                {
                    body.resize(offset);
                    break;
                }
                body.resize(offset + downloaded);
            }
            return body;
        }
    }

    HttpClient::HttpClient()
    {
        session_ = WinHttpOpen(
            L"NexusUpgradeValue/1.0",
            WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY,
            WINHTTP_NO_PROXY_NAME,
            WINHTTP_NO_PROXY_BYPASS,
            0);

        if (session_)
        {
            WinHttpSetTimeouts(static_cast<HINTERNET>(session_), 5000, 5000, 10000, 15000);
            DWORD decompression = WINHTTP_DECOMPRESSION_FLAG_GZIP | WINHTTP_DECOMPRESSION_FLAG_DEFLATE;
            WinHttpSetOption(static_cast<HINTERNET>(session_), WINHTTP_OPTION_DECOMPRESSION,
                             &decompression, sizeof(decompression));
        }
    }

    HttpClient::~HttpClient()
    {
        if (session_) WinHttpCloseHandle(static_cast<HINTERNET>(session_));
    }

    bool HttpClient::Get(const std::wstring& pathAndQuery,
                         const std::string& bearerToken,
                         std::string& response,
                         std::string& error) const
    {
        response.clear();
        error.clear();

        if (!session_)
        {
            error = "WinHTTP session unavailable";
            return false;
        }

        HINTERNET connection = WinHttpConnect(
            static_cast<HINTERNET>(session_), L"api.guildwars2.com",
            INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (!connection)
        {
            error = "WinHttpConnect failed";
            return false;
        }

        HINTERNET request = WinHttpOpenRequest(
            connection, L"GET", pathAndQuery.c_str(), nullptr,
            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
        if (!request)
        {
            WinHttpCloseHandle(connection);
            error = "WinHttpOpenRequest failed";
            return false;
        }

        std::wstring headers = L"Accept: application/json\r\n";
        if (!bearerToken.empty())
        {
            headers += L"Authorization: Bearer ";
            headers += ToWide(bearerToken);
            headers += L"\r\n";
        }

        const BOOL sent = WinHttpSendRequest(
            request, headers.c_str(), static_cast<DWORD>(-1L),
            WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

        if (!sent || !WinHttpReceiveResponse(request, nullptr))
        {
            WinHttpCloseHandle(request);
            WinHttpCloseHandle(connection);
            error = "HTTP request failed";
            return false;
        }

        DWORD statusCode = 0;
        DWORD statusSize = sizeof(statusCode);
        WinHttpQueryHeaders(request,
            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusSize,
            WINHTTP_NO_HEADER_INDEX);

        response = ReadBody(request);
        WinHttpCloseHandle(request);
        WinHttpCloseHandle(connection);

        if (statusCode < 200 || statusCode >= 300)
        {
            error = "HTTP " + std::to_string(statusCode);
            return false;
        }
        return true;
    }
}
