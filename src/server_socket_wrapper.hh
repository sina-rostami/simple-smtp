#pragma once

#include <functional>
#include <optional>
#include <string>

#include <uvw.hpp>

struct TcpSocketServer
{
  using DataReceivedCallbackType = std::function<std::optional<std::string>(
      const std::string &, const std::string &, const unsigned int)>;

  std::string ip;
  std::uint16_t port;
  DataReceivedCallbackType data_received_callback;

  TcpSocketServer(std::string i_ip, std::uint16_t i_port,
                  DataReceivedCallbackType i_callback)
      : ip(i_ip), port(i_port), data_received_callback(i_callback)
  {
  }

  void start()
  {
    auto loop = uvw::Loop::getDefault();
    auto tcp = loop->resource<uvw::TCPHandle>();

    tcp->on<uvw::ErrorEvent>(
        [](const uvw::ErrorEvent &error, uvw::TCPHandle &) {
          std::cout << "something went wrong in server : " << error.what()
                    << std::endl;
        });

    tcp->on<uvw::ListenEvent>(
        [this](const uvw::ListenEvent &, uvw::TCPHandle &srv) {
          std::shared_ptr<uvw::TCPHandle> client =
              srv.loop().resource<uvw::TCPHandle>();

          client->once<uvw::EndEvent>(
              [](const uvw::EndEvent &, uvw::TCPHandle &client) {
                client.close();
              });

          client->on<uvw::DataEvent>([this](const uvw::DataEvent &data,
                                            uvw::TCPHandle &handle) {
            const std::string received_data(&data.data[0],
                                            &data.data[0] + data.length - 1);

            if (auto response = this->data_received_callback(
                    received_data, handle.peer().ip, handle.peer().port))
              handle.write(response.value().data(), response.value().size());
          });

          srv.accept(*client);

          client->read();
        });

    tcp->bind(ip, port);
    tcp->listen();
    loop->run();
  }
};