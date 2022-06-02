#pragma once

#include <string>
#include <map>
#include <assert.h>
#include <algorithm>

namespace demo
{
    namespace http
    {
        using std::string;
        class HttpRequest
        {
        public:
            enum Method
            {
                INVALID,
                GET,
                POST,
                HEAD,
                PUT,
                DELETE
            };
            enum Version
            {
                UNKNOWN,
                HTTP10,
                HTTP11
            };

            HttpRequest() = default;

            void setVersion(Version v) { version = v; }

            Version getVersion() const { return version; }

            void setMethod(Method method_) { method = method_; }

            bool setMethod(const char *start, const char *end);

            Method getMethod() const { return method; }

            const char *methodString() const;

            void setPath(const char *path_) { path = path_; }

            void setPath(const char *start, const char *end) { path.assign(start, end); }

            const string &getPath() const { return path; }

            void setQuery(const char *query_) { query = query_; }

            void setQuery(const char *start, const char *end) { query.assign(start, end); }

            const string &getQuery() const { return query; }

            void addHeader(const char *start, const char *colon, const char *end);

            string getHeader(const string &field) const;

            const std::map<string, string> &getHeaders() const { return headers; }

        private:
            Method method = INVALID;
            Version version = UNKNOWN;
            string path;
            string query;
            std::map<string, string> headers;
        };
    }
}