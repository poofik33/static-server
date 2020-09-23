#include "getopt.h"

#include <algorithm>
#include <filesystem>

#include <thread_pool.h>
#include <misc.h>
#include <socket.h>
#include <http.h>

class Server
{
public:
	struct Client
	{
		HTTPRequest request = {};
		HTTPResponse response = {};
		Socket s;
		ThreadPool *pool;
		bool done = false;
		std::string rootPath = {};

		void receive();
		void read();
		void send();

		void clientWork();

		Client(Socket s, ThreadPool *p, std::string rp) :
		s(std::move(s)), pool(p), rootPath(std::move(rp)), done(false) {}
	};

	Server(uint32_t port, uint32_t listenQueueSize, int countThreads, std::string p) :
	s(createServerSocket(port, listenQueueSize)), rootPath(std::move(p)), pool(countThreads)
	{
		clients.reserve(listenQueueSize);
	}

	void main();
	void eraseDone();

	~Server() = default;
private:
	ThreadPool pool;
	std::vector<std::shared_ptr<Client>> clients;
	std::string rootPath;

	Socket s;
};

void Server::Client::receive()
{
	s.setRecvTimeout(5, 0);

	auto msg = s.recv(httpStop);

	request = parseRequest(msg, rootPath);

	auto f = std::bind(&Server::Client::read, this);
	pool->pushTopJob(std::move(f));
}

void Server::Client::read()
{
	response = prepareResponse(request);

	auto f = std::bind(&Server::Client::send, this);
	pool->pushTopJob(std::move(f));
}

void Server::Client::send()
{
	s.send(response.toString());

	if (request.method != HTTPMethod::MethodCode::HEAD) s.send(response.body);

	s.close();
	done = true;
}

void Server::main()
{
	while(true)
	{
		Socket newSock;
		try
		{
			newSock = s.accept();
		}
		catch(const std::runtime_error &err)
		{
			std::cout << "[" << std::this_thread::get_id() << "] " << err.what();
		}
		auto c = std::make_shared<Server::Client>(std::move(newSock), &pool, rootPath);
		clients.push_back(c);

		auto f = std::bind(&Server::Client::clientWork, c);
		pool.pushJob(f);
		if (clients.size() > 128)
		{
			eraseDone();
		}
	}
}

void Server::Client::clientWork()
{
	s.setRecvTimeout(1, 0);

	auto msg = s.recv(httpStop);

	request = parseRequest(msg, rootPath);
	response = prepareResponse(request);
	s.send(response.toString());

	if (request.method != HTTPMethod::MethodCode::HEAD &&
	response.code == HTTPResponse::ResponseCode::OK)
		s.sendFile(request.path, response.contentLength);

	s.close();
	done = true;
}

void Server::eraseDone()
{
	decltype(clients) tmp;
	for(size_t i = 0; i < clients.size(); i++)
	{
		if (!clients[i]->done)
		{
			tmp.push_back(clients[i]);
		}
	}
	clients = std::move(tmp);
}

//n - number of threads;
//q - length of listen queue;
//p - port;
//d - home directory;

int main(int argc, char **argv)
{
	int opt = 0, arg_num = 0;

	unsigned int numThreads = 0, listenQueueSize = 0, port = 0;
	std::string homePath = "";

	while((opt = getopt(argc, argv, "n:q:p:d:")) != -1)
	{
		switch(opt) {
			case 'n':
				numThreads = std::atoi(optarg);
				break;
			case 'q':
				listenQueueSize = std::atoi(optarg);
				break;
			case 'p':
				port = std::atoi(optarg);
				break;
			case 'd':
				homePath = optarg;
				break;
		}
		arg_num++;
	}

	if (numThreads == 0) numThreads = 1;

	if (arg_num < 4 || port == 0 || listenQueueSize == 0)
	{
		std::cerr << "Wrong params number\n";
		return EXIT_FAILURE;
	}

	homePath = std::string(std::filesystem::current_path().string() + "/" + homePath);
	std::cout << "Home path is " << homePath << "\n";

	auto threadCount = std::min(std::thread::hardware_concurrency(), numThreads);

	auto server = Server(port, listenQueueSize, threadCount, homePath);
	server.main();

	return 0;
}
