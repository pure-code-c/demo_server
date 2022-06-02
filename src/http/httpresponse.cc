#include "../../include/http/httpresponse.h"

namespace demo
{
    namespace http
    {
        string HttpResponse::toString()
        {
            string str;
            char buf[32];
            snprintf(buf, sizeof buf, "HTTP/1.1 %d ", status_code);
            str.append(buf);
            str.append(status_msg);
            str.append("\r\n");

            if (close_connection)
            {
                str.append("Connection: close\r\n");
            }
            else
            {
                str.append("Connection: Keep-Alive\r\n");
            }

            for (const auto &header : headers)
            {
                str.append(header.first);
                str.append(": ");
                str.append(header.second);
                str.append("\r\n");
            }

            str.append("\r\n");
            str.append(body);
            return str;
        }
    }
}