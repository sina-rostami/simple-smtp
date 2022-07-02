#pragma once
#include <string>

enum class Command
{
  UNKNOWN,
  HELO,
  MAIL_FROM,
  RCPT_TO,
  DATA,
  QUIT,
};

struct Mail
{
  std::string username;
  std::string domain;
};

struct SMTPSession
{
  Command current_command;
  std::string user;
  std::string mail_from;
  std::string rcpt_to;
  std::string data;

  SMTPSession() : current_command(Command::UNKNOWN) {}
};
