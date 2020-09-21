#include <http.h>

#include <string_view>

#include <misc.h>
#include <file.h>

HTTPRequest parseRequest(const std::string& req, const std::string &rootPath)
{
	std::string reqLine = req.substr(0, req.find("\r\n"));
	std::vector<std::string> parts;
	parts.reserve(3);

	while(reqLine.size() > 0)
	{
		auto pos = reqLine.find(' ');
		parts.emplace_back(reqLine.substr(0, pos));
		if (pos == std::string::npos || pos + 1 >= reqLine.size()) break;

		reqLine = reqLine.substr(pos + 1);
	}

	if (parts.size() < 3) throw http_exception(std::string("Error: wrong format of request line"));

	HTTPRequest result;

	for(size_t i = 0; i < static_cast<size_t>(HTTPMethod::MethodCode::count); i++)
	{
		if (parts[0] == HTTPMethod::MethodNames[i])
		{
			result.method = static_cast<HTTPMethod::MethodCode>(i);
			break;
		}
	}

	if (result.method == HTTPMethod::MethodCode::UNKNOWN) return result;

	auto filePath = parts[1].substr(0, parts[1].find('?'));
	result.path = rootPath + urlUnescape(filePath);

	if (result.path.at(result.path.size() - 1) == '/')
	{
		result.path += "index.html";
		result.toDir = true;
	}

	return result;
}

std::string HTTPResponse::toString() const
{
	const size_t responseSize = 512;

	std::string result;
	result.reserve(responseSize);

	result += protocol + " ";
	result += std::to_string(static_cast<int>(code)) + " " + ResponseCodeTranscription.at(code);
	result += "\r\n";

	result += "Server: " + serverName + "\r\n";

	auto t = std::time(nullptr);
	std::string date;
	date.resize(40);

	auto dateSize = std::strftime(date.data(), date.size(), "%a, %d %b %Y %T GMT", std::gmtime(&t));
	result += "Date: " + date.substr(0, dateSize) + "\r\n";

	result += "Connection: close\r\n";

	if (importantHeaders)
		return result + "\r\n";

	result += "Content-Type: " + mimeType + "\r\n";
	result += "Content-Length: " + std::to_string(contentLength) + "\r\n";

	result += "\r\n";

	return result;
}

std::string fileExtToMimeType(const std::string &ext)
{
	const std::vector<std::string> mimeTypes = {
			"text/html",
			"text/css",
			"application/javascript",
			"image/jpeg",
			"image/png",
			"image/gif",
			"application/x-shockwave-flash"
	};

	const std::unordered_map<std::string, int> extensions = {
			{".html", 0},
			{".css", 1},
			{".js", 2},
			{".jpg", 3},
			{".jpeg", 3},
			{".png", 4},
			{".gif", 5},
			{".swf", 6},
	};

	auto iter = extensions.find(ext);

	return iter != extensions.cend() ? mimeTypes[iter->second] : "";
}

HTTPResponse prepareResponse(const HTTPRequest &req)
{
	HTTPResponse resp;

	if (req.method == HTTPMethod::MethodCode::UNKNOWN)
	{
		resp.code = HTTPResponse::ResponseCode::METHOD_NOT_ALLOWED;
		resp.importantHeaders = true;

		return resp;
	}

	if (req.path.find("../") != std::string::npos)
	{
		resp.code = HTTPResponse::ResponseCode::FORBIDDEN;
		resp.importantHeaders = true;

		return resp;
	}

	if (!exists(req.path))
	{
		if (req.toDir) resp.code = HTTPResponse::ResponseCode::FORBIDDEN;
		else resp.code = HTTPResponse::ResponseCode::NOT_FOUND;
		resp.importantHeaders = true;

		return resp;
	}

	resp.code = HTTPResponse::ResponseCode::OK;

	auto ext = req.path.substr(req.path.find_last_of('.'));
	resp.mimeType = fileExtToMimeType(ext);

	if (req.method == HTTPMethod::MethodCode::HEAD)
	{
		resp.contentLength = readSize(req.path);
		return resp;
	}

	resp.body = readAll(req.path);
	resp.contentLength = resp.body.size();

	return resp;
}
