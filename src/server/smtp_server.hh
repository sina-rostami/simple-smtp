#pragma once

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>

#include "server_socket_wrapper.hh"
#include "smtp_session.hh"

namespace Server
{

static const std::map<std::string, Command> command_by_string = {
    {"HELO ", Command::HELO},        {"MAIL FROM: ", Command::MAIL_FROM},
    {"RCPT TO: ", Command::RCPT_TO}, {"DATA", Command::DATA},
    {"SEND", Command::SEND},         {"QUIT", Command::QUIT}};
static std::map<std::string, SMTPSession> session_state_by_address;
static std::set<Mail> mails;
static std::string clients_path;

struct SMTPServer
{
  TcpSocketServer server;

  SMTPServer(const std::string i_clients_path)
      : server(
            "0.0.0.0", 2525,
            TcpSocketServer::DataReceivedCallbackType(data_received_callback))
  {
    clients_path = i_clients_path;
    update_mail_list(clients_path);
  }

  void start() { server.start(); }

  static std::string convert_reverse_path_to_mail(const std::string path)
  {
    std::string username = path.substr(path.find_last_of('/') + 1);
    auto reversed_domain = path;

    reversed_domain.erase(reversed_domain.begin() +
                              reversed_domain.find_last_of('/'),
                          reversed_domain.end());
    reversed_domain.erase(reversed_domain.begin(),
                          reversed_domain.begin() +
                              reversed_domain.find("src/clients") +
                              std::string("src/clients/").size());

    std::vector<std::string> domain_parts;

    while (reversed_domain.find('/') != std::string::npos)
    {
      domain_parts.emplace_back(reversed_domain.begin(),
                                reversed_domain.begin() +
                                    reversed_domain.find_first_of('/'));
      reversed_domain.erase(reversed_domain.begin(),
                            reversed_domain.begin() +
                                reversed_domain.find_first_of('/') + 1);
    }
    domain_parts.push_back(reversed_domain);

    std::string domain_part;

    std::for_each(
        domain_parts.rbegin(), domain_parts.rend(),
        [&domain_part](const std::string &part) { domain_part += part + "."; });
    domain_part.erase(domain_part.begin() + domain_part.find_last_of("."));

    return "<" + username + "@" + domain_part + ">";
  }

  static void update_mail_list(const std::filesystem::path path)
  {
    if (!std::filesystem::is_directory(path))
      return;

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
      auto sub_path = entry.path();

      if (!std::filesystem::is_directory(sub_path) &&
          sub_path.filename().string() == "client")
      {
        mails.insert(Mail(convert_reverse_path_to_mail(path.string())));

        return;
      }

      update_mail_list(sub_path);
    }
  }

  static std::string convert_email_to_path(const std::string email)
  {
    auto email_copy = email;
    std::vector<std::string> path_parts;

    path_parts.emplace_back(email_copy.begin(),
                            email_copy.begin() + email_copy.find('@'));

    email_copy.erase(email_copy.begin(),
                     email_copy.begin() + email_copy.find('@') + 1);

    while (email_copy.find('.') != std::string::npos)
    {
      path_parts.emplace_back(email_copy.begin(),
                              email_copy.begin() +
                                  email_copy.find_first_of('.'));
      email_copy.erase(email_copy.begin(),
                       email_copy.begin() + email_copy.find_first_of('.') + 1);
    }
    path_parts.push_back(email_copy);

    std::string path;
    std::for_each(path_parts.rbegin(), path_parts.rend(),
                  [&path](const std::string &part) { path += part + "/"; });

    return path;
  }

  static void write_message_to_inbox(const SMTPSession &session_state,
                                     const std::string mail)
  {
    const auto inbox_path =
        convert_email_to_path(session_state.rcpt_to.to_string()) + "inbox/";
    const auto current_time =
        std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto file_name = clients_path + "/" + inbox_path +
                           std::string(std::ctime(&current_time));

    {
      std::ofstream ofstream(file_name);
      ofstream << mail << std::endl;
    }
  }

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

        if (value == Command::QUIT)
        {
          state.current_command = Command::UNKNOWN;
          state.data.clear();

          return "221 Closing Connection\n";
        }

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

          state.mail_from = Mail(data.substr(key.size()));

          if (!mails.contains(state.mail_from))
            return "421 Mail not exists!\n";

          state.current_command = value;

          return "250 " + state.mail_from.to_string() + " ... Sender OK\n";
        case Command::MAIL_FROM:
          if (value != Command::RCPT_TO)
            return "503 Bad sequence of commands\n";

          state.rcpt_to = Mail(data.substr(key.size()));

          if (!mails.contains(state.rcpt_to))
            return "421 Mail not exists!\n";

          state.current_command = value;

          return "250 " + state.rcpt_to.to_string() + " ... Recipient OK\n";
        case Command::RCPT_TO:
          if (value != Command::DATA)
            return "503 Bad sequence of commands\n";

          state.current_command = value;

          return "354 Enter mail, end with \".\" on a line by itself\n";
        case Command::DATA:
          if (data == ".")
          {
            state.current_command = Command::SEND;
            return "250 Message accepted for delivery\n";
          }
          state.data += data + '\n';
          return " ";
        case Command::SEND:
        {
          // send mail
          auto payload = "From: " + state.mail_from.to_string() + "\n" +
                         "To: " + state.rcpt_to.to_string() + "\n" +
                         "Mail :\n'''\n" + state.data + "'''\n";

          write_message_to_inbox(state, payload);

          state.current_command = Command::QUIT;
          return "250 Mail sent\n";
        }
        case Command::QUIT:
          if (value != Command::QUIT)
            return "503 Bad sequence of commands\n";
        default:
          break;
        }

        return key + " command!\n";
      }
    }

    return "504 Command not found\n";
  }
};
} // namespace Server