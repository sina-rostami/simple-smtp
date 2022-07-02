#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <optional>

#include "smtp_server.hh"

int main()
{
  SMTPServer smtp_server;

  smtp_server.start();

  return 0;
}