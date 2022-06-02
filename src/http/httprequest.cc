#include "../../include/http/httprequest.h"

namespace demo
{
    namespace http
    {

        bool HttpRequest::setMethod(const char *start, const char *end)
        {
            assert(method == INVALID);
            string m(start, end);
            if (m == "GET")
            {
                method = GET;
            }
            else if (m == "POST")
            {
                method = POST;
            }
            else if (m == "HEAD")
            {
                method = HEAD;
            }
            else if (m == "PUT")
            {
                method = PUT;
            }
            else if (m == "DELETE")
            {
                method = DELETE;
            }
            else
            {
                method = INVALID;
            }
            return method != INVALID;
        }

        const char *HttpRequest::methodString() const
        {
            const char *result = "UNKNOWN";
            switch (method)
            {
            case GET:
                result = "GET";
                break;
            case POST:
                result = "POST";
                break;
            case HEAD:
                result = "HEAD";
                break;
            case PUT:
                result = "PUT";
                break;
            case DELETE:
                result = "DELETE";
                break;
            default:
                break;
            }
            return result;
        }

        void HttpRequest::addHeader(const char *start, const char *colon, const char *end)
        {
            string field(start, colon);
            ++colon;
            while (colon < end && isspace(*colon))
            {
                ++colon;
            }
            string value(colon, end);
            while (!value.empty() && isspace(value[value.size() - 1]))
            {
                value.resize(value.size() - 1);
            }
            headers[field] = value;
        }

        string HttpRequest::getHeader(const string &field) const
        {
            string result;
            std::map<string, string>::const_iterator it = headers.find(field);
            if (it != headers.end())
            {
                result = it->second;
            }
            return result;
        }

    }
}