#include <thread>

#include <misc.h>
#include <socket.h>
#include <http.h>

const std::string rootPath = "/Users/a.postnikov/Documents/Univer/StaticServer/http-test-suite";

void clientWork(Socket s)
{
	s.setRecvTimeout(5, 0);

	auto msg = s.recv(httpStop);
	auto req = parseRequest(msg, rootPath);

	std::cout << "==========\n";
	std::cout << msg << "\n";
	std::cout << "Requesting: " << req.path << '\n';
	std::cout << "==========\n";

	auto resp = prepareResponse(req);
	std::cout << resp.toString() << "==========\n";

	s.send(resp.toString());
	if (req.method != HTTPMethod::MethodCode::HEAD) s.send(resp.body);
}

int main(int argc, char **argv)
{
	std::ios::sync_with_stdio(false);

	std::cout << "Number of available threads: "<< std::thread::hardware_concurrency() << '\n';

	Socket ss;

	try
	{
		ss = createServerSocket(80, 5);
	}
	catch (const std::runtime_error &err)
	{
		std::cerr << err.what() << "\n";
		return 1;
	}

	while(true)
	{
		Socket cs;
		try
		{
			cs = ss.accept();
			clientWork(std::move(cs));
		}
		catch(const std::runtime_error &err)
		{
			std::cerr << err.what() << "\n";
		}
	}

	return 0;
}
