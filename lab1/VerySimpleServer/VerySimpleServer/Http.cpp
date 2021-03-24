#include "Config.h"

static const std::map<std::string, Request::Method> MethodMap = { {"GET", Request::GET},
                                                                 {"POST", Request::POST} };
Request Request::Dealrqst(std::stringstream& ss) {
    Request request;
    ss >> request.methodStr >> request.url >> request.version;
    if (request.url.length() == 1&& strcmp(request.url.c_str(),"/")==0)
    {
        request.url = "index.html";
        request.ext = "html";
    }
    else {
        int location = request.url.find_last_of('/');
        request.url = request.url.substr(location + 1);//获得返回文件名

        location = request.url.find_last_of('.');
        request.ext = request.url.substr(location + 1);//得到扩展名
    }

    auto methodIt = MethodMap.find(request.methodStr);
    if (methodIt != MethodMap.end()) {
        request.method = methodIt->second;
    }

    return request;
};

static std::map<int, std::string> StatusMap = {
    {200, "OK"},

    {404, "Not Found"},
   
};

const std::unordered_map<std::string, std::string> MimeType = { {"html", "text/html"},

                                                               {"txt", "text/plain"},
                                                               {"gif", "image/gif"},
                                                               {"jpeg", "image/jpeg"},
                                                               {"jpg", "image/jpeg"},
                                                               {"png", "image/png"},
                                                               {"ico", "image/x-icon"},
 };

std::string Request::ResponseString(Request q,int statusCode) const
{
    std::stringstream ss;

    // 生成状态行
    ss << q.version << ' ' << statusCode << ' ' << statusInfo(statusCode) << "\r\n";
    ss << "Content-Type : " << TypeInfo(q.ext)<<";charset=iso-8859-1\r\n";
    ss << "Connection: keep-alive\r\n";

    // 生成body
    ss << "Content-Length:" << q.body.size() << "\r\n\r\n";
    
    return ss.str();
}

std::string Request::statusInfo(int statusCode) const
{
    if (StatusMap.find(statusCode) != StatusMap.end())
        return StatusMap[statusCode];
    else
        return "";
}
std::string Request::TypeInfo(std::string TypeCode) const
{
    auto iter = MimeType.find(TypeCode);
    if(iter!=MimeType.end())
        return iter->second;
    else  return "text/html";
}

