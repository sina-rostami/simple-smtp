#include <iostream>
#include <memory>
#include <uvw.hpp>

int main() {
  std::cout << "heelo\n";
  auto loop = uvw::Loop::getDefault();
  auto tcp = loop->resource<uvw::TCPHandle>();

  tcp->on<uvw::ErrorEvent>([](const uvw::ErrorEvent &,
                              uvw::TCPHandle &) { std::cout << "something went wrong\n"; });

  tcp->on<uvw::ListenEvent>([](const uvw::ListenEvent &, uvw::TCPHandle &srv) {
    std::shared_ptr<uvw::TCPHandle> client =
        srv.loop().resource<uvw::TCPHandle>();
    client->once<uvw::EndEvent>(
        [](const uvw::EndEvent &, uvw::TCPHandle &client) { client.close(); });
    client->on<uvw::DataEvent>(
        [](const uvw::DataEvent &, uvw::TCPHandle &) { std::cout << "data received\n"; });
    srv.accept(*client);
    client->read();
  });

  tcp->bind("127.0.0.1", 4242);
  tcp->listen();
  loop->run();

  return 0;
}