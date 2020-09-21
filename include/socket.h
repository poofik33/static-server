#ifndef STATICSERVER_SOCKET_H
#define STATICSERVER_SOCKET_H

#include <iostream>
#include <unistd.h>

class Socket
{
public:
	class socket_exception : public std::runtime_error
	{
	public:
		socket_exception(std::string str) : std::runtime_error(str) {};
	};

	Socket() = default;
	Socket(int sd) : socketDescriptor(sd) {}

	Socket(const Socket &) = delete;
	Socket& operator= (const Socket &) = delete;

	Socket(Socket &&s) noexcept : socketDescriptor(s.socketDescriptor) { s.socketDescriptor = -1; }
	Socket& operator= (Socket &&s) noexcept
	{
		socketDescriptor = s.socketDescriptor;
		s.socketDescriptor = -1;
		return *this;
	}

	~Socket() {  close(); }

	Socket accept() const;
	void setRecvTimeout(uint32_t sec, uint32_t usec);
	std::string recv(const std::string &stop) const;
	void send(const std::string &str) const;
	void setReuseAddr();

	void close()
	{
		if (socketDescriptor > 0)
		{
			::fsync(socketDescriptor);
			::close(socketDescriptor);
		}

		socketDescriptor = 0;
	}

private:
	int socketDescriptor = -1;
};

Socket createServerSocket(uint32_t port, uint32_t listenQueueSize) noexcept(false);

#endif //STATICSERVER_SOCKET_H
