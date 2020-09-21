#include <thread_pool.h>
#include <misc.h>
#include <socket.h>
#include <http.h>

#include <algorithm>

const std::string rootPath = "/Users/a.postnikov/Documents/Univer/StaticServer/http-test-suite";

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

		Client(Socket s, ThreadPool *p, std::string rp) :
		s(std::move(s)), pool(p), rootPath(std::move(rp)) {}
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

		auto f = std::bind(&Server::Client::receive, c);
		pool.pushJob(f);
		if (clients.size() > 100)
		{
			eraseDone();
		}
	}
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

int main(int argc, char **argv)
{
	std::cout << "Number of available threads: "<< std::thread::hardware_concurrency() << '\n';

	auto threadCount = std::min(std::thread::hardware_concurrency(), unsigned(4));

	auto server = Server(80, 20, threadCount, rootPath);
	server.main();

	return 0;
}
