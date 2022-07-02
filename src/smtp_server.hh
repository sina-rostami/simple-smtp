#pragma once

#include <map>

#include "server_socket_wrapper.hh"
#include "smtp_session.hh"

static const std::map<std::string, Command> command_by_string = {
    {"HELO ", Command::HELO},
    {"MAIL FROM: ", Command::MAIL_FROM},
    {"RCPT TO: ", Command::RCPT_TO},
    {"DATA", Command::DATA},
    {"QUIT", Command::QUIT}};
static std::map<std::string, SMTPSession> session_state_by_address;

struct SMTPServer
{
  TcpSocketServer server;

  SMTPServer()
      : server(
            "127.0.0.1", 2525,
            TcpSocketServer::DataReceivedCallbackType(data_received_callback))
  {
  }

  void start() { server.start(); }

  static void add_to_states(const std::string &ip, const std::uint16_t port)
  {
    const auto key = ip + std::to_string(port);

    if (!session_state_by_address.contains(key))
      session_state_by_address[key] = SMTPSession();
  }

  static SMTPSession &get_curret_state(const std::string &ip,
                                       const std::uint16_t port)
  {
    const auto key = ip + std::to_string(port);
    return session_state_by_address[key];
  }

  static std::optional<std::string>
  data_received_callback(const std::string &data, const std::string &ip,
                         const std::uint16_t port)
  {
    add_to_states(ip, port);
    auto &state = get_curret_state(ip, port);

    for (const auto &[key, value] : command_by_string)
    {
      if (data.starts_with(key) || state.current_command == Command::DATA)
      {
        if ((key == "DATA" || key == "QUIT") && data.size() != key.size() &&
            state.current_command != Command::DATA)
          return "500 command not found\n";

        switch (state.current_command)
        {
        case Command::UNKNOWN:
          if (value != Command::HELO)
            return "503 Bad sequence of commands\n";

          state.user = data.substr(key.size());
          state.current_command = value;

          return "250 Hello " + state.user + ", pleased to meet you\n";
        case Command::HELO:
          if (value != Command::MAIL_FROM)
            return "503 Bad sequence of commands\n";

          state.mail_from = data.substr(key.size());
          state.current_command = value;

          // todo check existence
          return "250 " + state.mail_from + " ... Sender OK\n";
        case Command::MAIL_FROM:
          if (value != Command::RCPT_TO)
            return "503 Bad sequence of commands\n";

          state.rcpt_to = data.substr(key.size());
          state.current_command = value;

          // todo check existence
          return "250 " + state.mail_from + " ... Recipient OK\n";
        case Command::RCPT_TO:
          if (value != Command::DATA)
            return "503 Bad sequence of commands\n";

          state.current_command = value;

          return "354 Enter mail, end with \".\" on a line by itself\n";
        case Command::DATA:
          if (data == ".")
          {
            // go for send
            state.current_command = Command::QUIT;
            return "250 Message accepted for delivery\n";
          }
          state.data += data + '\n';
          return "";
        case Command::QUIT:
          if (value != Command::QUIT)
            return "503 Bad sequence of commands\n";

          return "221 Closing Connection\n";
        default:
          break;
        }

        return key + " command!\n";
      }
    }

    return data;
  }
};