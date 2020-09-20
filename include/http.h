#ifndef STATICSERVER_HTTP_H
#define STATICSERVER_HTTP_H

#pragma once

#include <utility>
#include <vector>
#include <string>
#include <unordered_map>

const std::string protocol = "HTTP/1.1";
const std::string server = "A_Postnikov_Technopark_Server 0.1";

const std::string httpStop = "\r\n\r\n";

class http_exception : public std::runtime_error
{
public:
	http_exception(std::string str) : std::runtime_error(str) {};
};

namespace HTTPMethod
{
	enum class MethodCode
	{
		GET = 0,
		HEAD,
		//
		count,
		UNKNOWN,
	};

	const std::vector<std::string> MethodNames = {
		"GET",
		"HEAD",
		"",
		"UNKNOWN",
	};
}

struct HTTPRequest
{
	std::string path = {};
	HTTPMethod::MethodCode method = HTTPMethod::MethodCode::UNKNOWN;
	bool toDir = false;

	HTTPRequest() = default;
	HTTPRequest(HTTPMethod::MethodCode m, std::string p) : path{std::move(p)}, method{m} {}
};

HTTPRequest parseRequest(const std::string& req, const std::string &rootPath);

struct HTTPResponse
{
	enum class ResponseCode
	{
		OK = 200,
		FORBIDDEN = 403,
		NOT_FOUND = 404,
		METHOD_NOT_ALLOWED = 405,
	};

	std::string body = "";
	std::string mimeType = "";
	ResponseCode code = ResponseCode::NOT_FOUND;
	size_t contentLength = 0;
	bool importantHeaders = false;

	std::string toString() const;
};

const std::unordered_map<HTTPResponse::ResponseCode, std::string> ResponseCodeTranscription = {
		{HTTPResponse::ResponseCode::OK, "OK"},
		{HTTPResponse::ResponseCode::FORBIDDEN, "Forbidden"},
		{HTTPResponse::ResponseCode::NOT_FOUND, "Not Found"},
		{HTTPResponse::ResponseCode::METHOD_NOT_ALLOWED, "Method Not Allowed"},
};

std::string fileExtToMimeType(const std::string &ext);

HTTPResponse prepareResponse(const HTTPRequest &req);

#endif //STATICSERVER_HTTP_H
