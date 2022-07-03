#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "smtp_server.hh"

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cout << "At least 1 arguments must be provided (clinets absolute path)\n";
    return 1;
  }

  Server::SMTPServer smtp_server(std::string(*(argv + 1)));

  smtp_server.start();

  return 0;
}