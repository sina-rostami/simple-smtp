# SMTP simple implementation

This repository contains a simple implementation of SMTP (Simple Mail Transfer Protocol).

## Server

Server written in `c++` using `uvw` library and `bazel` build system.

### build

To build, you may have installed following requirements.

```bash
bazel
libuv (v1.44)
c++ compiler (with support of standard >= 20)
```

Then you can easily run server. (in the root directory of repo.)

```bash
bazel run server -- $(pwd)/src/clients
```

## Clients

Client written in `python`. you can run multiple cilents at the same time. (Make sure before sending mail, your email exists in clients director like examples)

```bash
python3 client.py
```

## Example

This is a simple output of client.

```bash
╰─$ python3 client.py
>> HELO gmail
<< 250 Hello gmail, pleased to meet you
>> MAIL FROM: <omid@gmail.com>
<< 250 omid@gmail.com ... Sender OK
>> RCPT TO: <keyvan@mail.ir>
<< 250 keyvan@mail.ir ... Recipient OK
>> DATA
<< 354 Enter mail, end with "." on a line by itself
>> Oh Dear Keyvan,
>> How are you?
>> .
<< 250 Message accepted for delivery
>> SEND
<< 250 Mail sent
>> QUIT
<< 221 Closing Connection
```

Now we can see the mail in `keyvan@mail.ir`'s inbox.

```bash
╰─$ tree
.
├── client.py
├── com
│   ├── gmail
│   │   ├── omid
│   │   │   ├── client
│   │   │   └── inbox
│   │   └── reza
│   │       ├── client
│   │       └── inbox
│   └── yahoo
│       └── omid
│           ├── client
│           └── inbox
└── ir
    └── mail
        └── keyvan
            ├── client
            └── inbox
                └── Mon Jul  4 10:36:05 2022\012
```

```bash
╰─$ cat ir/mail/keyvan/inbox/Mon\ Jul\ \ 4\ 10:36:05\ 2022$'\n'
From: omid@gmail.com
To: keyvan@mail.ir
Mail :
'''
Oh Dear Keyvan,
How are you?
'''
```
