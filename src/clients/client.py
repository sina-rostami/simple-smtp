import socket

HOST = "127.0.0.1"
PORT = 2525


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


class ClientSocket():

    def __init__(self) -> None:
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect(self):
        self.socket.connect((HOST, PORT))

    def close(self):
        self.socket.close()

    def send_message(self, message: str):
        self.socket.sendall(message.encode())
        data = self.socket.recv(2048)
        return data.decode()


class Client():
    def __init__(self) -> None:
        self.socket = ClientSocket()

    def run(self):
        self.socket.connect()
        while True:
            command = input(bcolors.OKBLUE + f'>> ' + bcolors.ENDC)
            response = self.socket.send_message(command)

            if response == " ":
                continue

            response_code, message = response.split(' ', 1)

            if response[0] in ['4', '5']:
                print(bcolors.FAIL + f"<< {response}" + bcolors.ENDC, end='')
            else:
                print(bcolors.OKGREEN +
                      f"<< {response}" + bcolors.ENDC, end='')
            if command == 'QUIT':
                return


if __name__ == "__main__":
    client = Client()
    client.run()
