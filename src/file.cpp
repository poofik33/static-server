#include <file.h>

#include <unistd.h>
#include <sys/stat.h>

#include <misc.h>

std::string readAll(const std::string &path)
{
	const size_t chunk = 1024;

	int fd = ::open(path.c_str(), O_NONBLOCK|O_RDONLY);
	if (fd < 0) throw file_exception(errorStringCode("Error: opening file " + path, errno));

	struct stat statBuf;
	if (stat(path.c_str(), &statBuf) < 0) throw file_exception(errorStringCode("Error: reading stat of file " + path, errno));

	std::string result;
	result.reserve(chunk * statBuf.st_size);

	char buf[chunk];
	while(true)
	{
		auto i = ::read(fd, buf, chunk);
		if (i == -1) throw file_exception(errorStringCode("Error: reading file " + path, errno));
		if (i == 0) break;
		result.append(buf, i);
	}

	::close(fd);
	return result;
}
