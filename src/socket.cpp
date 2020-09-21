#include <socket.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <misc.h>

void Socket::setReuseAddr()
{
	int yes = 1;
	std::cout << "Setting reuse address for socket:" << socketDescriptor << "\n";
	if (setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		::close(socketDescriptor);
		throw socket_exception(errorStringCode("Error: set reuse address", errno));
	}
}

Socket Socket::accept() const
{
	struct sockaddr_in client;
	memset(&client, 0, sizeof(client));
	socklen_t clientLength = sizeof(client);
	int clientSD = ::accept(socketDescriptor, (struct sockaddr*)&client, &clientLength);

	if (-1 == clientSD)
	{
		throw socket_exception(errorStringCode("Error: accepting", errno));
	}

	return Socket(clientSD);
}

void Socket::setRecvTimeout(uint32_t sec, uint32_t usec)
{
	struct timeval tm;
	tm.tv_sec = sec;
	tm.tv_usec = usec;

	if (setsockopt(socketDescriptor, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm)) == -1)
	{
		throw socket_exception(errorStringCode("Error: setting timeout", errno));
	}
}

std::string Socket::recv(const std::string &stop) const
{
	const size_t chunk = 128;
	std::string ret;
	ret.reserve(chunk);
	char buf[chunk];

	while(true)
	{
#ifdef __APPLE__
		auto n = ::recv(socketDescriptor, buf, chunk, 0);
#else
		auto n = ::recv(socketDescriptor, buf, chunk, MSG_NOSIGNAL);
#endif
		if (n == -1) throw Socket::socket_exception(errorStringCode("Error: receiving", errno));
		if (n == 0) break;

		ret.append(buf, n);

		if (n < chunk) break;
		if (ret.compare(ret.size() - stop.size(), stop.size(), stop) == 0)
		{
			break;
		}
	}

	return ret;
}

Socket createServerSocket(uint32_t port, uint32_t listenQueueSize) noexcept(false)
{
	std::cout << "Starting serverName...\n";
	auto sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd <= 0)
	{
		throw Socket::socket_exception(errorStringCode("Error: creating socket", errno));
	}
//	std::cout << "Create socket with ds: " << sockfd << "\n";
	Socket s{sockfd};

	s.setReuseAddr();

	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

//	std::cout << "Binding socket " << sockfd << " on port:" << port << '\n';
	if (::bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		::close(sockfd);
		throw Socket::socket_exception(errorStringCode("Error: binding address", errno));
	}

	std::cout << "Start listening by socket " << sockfd << " on port:" << port << '\n';
	if (::listen(sockfd, listenQueueSize) < 0)
	{
		::close(sockfd);
		throw Socket::socket_exception(errorStringCode("Error: socket listen", errno));
	}

	return s;
}

void Socket::send(const std::string &str) const
{
	size_t left = str.size();
	ssize_t sent = 0;
	while(left > 0)
	{
		sent = ::send(socketDescriptor, str.data() + sent, str.size() - sent, 0);
		if (sent < -1) throw socket_exception(errorStringCode("Error: sending data", errno));
		left -= sent;
	}
}
