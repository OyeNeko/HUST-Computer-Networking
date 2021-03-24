#pragma once
#include "Config.h"

struct Request
{
    enum Method {
        GET = 1,
        POST = 2,
    } method;
    std::string methodStr;
    std::string version;
    std::string url;
    std::string ext;
    size_t str_size = 0;
    
    Request Dealrqst(std::stringstream& ss);//½âÎöHttpº¯Êý

    std::string ResponseString(Request q,int statusCode) const;
    std::string statusInfo(int statusCode) const;
    std::string TypeInfo(std::string TypeCode) const;
    std::string body;
};
