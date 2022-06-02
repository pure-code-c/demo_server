#pragma once

#include <string>
#include <map>

namespace demo
{
    namespace http
    {
        using std::string;
        class HttpResponse
        {
        public:
            enum HttpStatusCode
            {
                Unknown,
                OK_200 = 200,
                NotFound_404 = 404,
            };

            explicit HttpResponse(bool close)
                : status_code(Unknown),
                  close_connection(close) {}

            void setStatusCode(HttpStatusCode code) { status_code = code; }

            void setStatusMessage(const string &message) { status_msg = message; }

            bool closeConnection() const { return close_connection; }

            void setContentType(const string &type) { addHeader("Content-Type", type); }

            void setContentLength(uint64_t len) { addHeader("Content-Length", std::to_string(len)); }

            void addHeader(const string &key, const string &value) { headers[key] = value; }

            void addBody(const string &body_) { body.append(body_); }

            string toString();

        private:
            bool close_connection;
            std::map<string, string> headers;
            HttpStatusCode status_code;
            string status_msg;
            string body;
        };

    }

}
