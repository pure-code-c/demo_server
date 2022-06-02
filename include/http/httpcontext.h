#pragma once

#include <algorithm>
#include "httprequest.h"
#include "../buffer.h"

namespace demo
{
    namespace http
    {
        class HttpContext
        {
        public:
            //当前解析状态
            enum HttpRequestParseState
            {
                EXPECT_REQUEST_LINE,
                EXPECT_HEADERS,
                EXPECT_BODY,
                GOT_ALL,
            };

            HttpContext() = default;

            bool parseRequest(Buffer *buf);

            bool gotAll() const { return state == GOT_ALL; }

            const HttpRequest &getRequest() const { return request; }

            HttpRequest &getRequest() { return request; }

        private:
            bool processRequestLine(const char *begin, const char *end);

            HttpRequestParseState state = EXPECT_REQUEST_LINE;
            HttpRequest request;
        };

    }
}