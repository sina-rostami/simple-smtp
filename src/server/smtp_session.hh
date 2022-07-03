#pragma once
#include <ostream>
#include <string>

namespace Server
{

enum class Command
{
  UNKNOWN,
  HELO,
  MAIL_FROM,
  RCPT_TO,
  DATA,
  SEND,
  QUIT,
};

struct Mail
{
  std::string username;
  std::string domain;

  Mail() {}

  Mail(const std::string mail)
  {
    username =
        std::string(mail.begin() + 1, mail.begin() + mail.find_first_of('@'));
    domain = std::string(mail.begin() + mail.find_first_of('@') + 1,
                         mail.begin() + mail.find_first_of('>'));
  }

  std::string to_string() const { return username + "@" + domain; }

  friend std::ostream &operator<<(std::ostream &os, const Mail &mail)
  {
    os << mail.to_string();
    return os;
  }

  friend bool operator<(const Mail &lhs, const Mail &rhs)
  {
    return (lhs.username + lhs.domain) < (rhs.username + rhs.domain);
  }
};

struct SMTPSession
{
  Command current_command;
  std::string user;
  Mail mail_from;
  Mail rcpt_to;
  std::string data;

  SMTPSession() : current_command(Command::UNKNOWN) {}
};

} // namespace Server