# Chat
This directory includes a simple chat application modelled directly after
[Bud's chat application][bud_chat]. [`noop_server.cc`](noop_server.cc)
implements a chat server, and [`noop_client.cc`](noop_client.cc) implements a
chat client.

```bash
# Start the server in one window.
GLOG_logtostderr=1 ./build/examples_chat_noop_server \
    tcp://0.0.0.0:8000

# Start some clients in other windows. Enter text in any of the clients and
# it will be broadcasted to all the others.
GLOG_logtostderr=1 ./build/examples_chat_noop_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8001 foo
GLOG_logtostderr=1 ./build/examples_chat_noop_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8002 bar
GLOG_logtostderr=1 ./build/examples_chat_noop_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8003 baz
```

We also implement a pinging client in
[`noop_ping_client.cc`](noop_ping_client.cc).

```bash
GLOG_logtostderr=1 ./build/examples_chat_noop_ping_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8004 pinger yolo
```

This pinging client will send the message "yolo" every second.

We've also implemented versions of the server, client, and ping client that
record history and lineage. Start a postgres instance running on
`localhost:5432` and run
[`reset_database.sql`](../../../scripts/reset_database.sql). Then, run the
following:

```bash
DB_USER="username"
DB_PASS="password"
DB_NAME="the_db"

# Server.
GLOG_logtostderr=1 ./build/examples_chat_noop_server \
    $DB_USER $DB_PASS $DB_NAME tcp://0.0.0.0:8000

# Clients.
GLOG_logtostderr=1 ./build/examples_chat_noop_client \
    $DB_USER $DB_PASS $DB_NAME tcp://0.0.0.0:8000 tcp://0.0.0.0:8001 foo
GLOG_logtostderr=1 ./build/examples_chat_noop_client \
    $DB_USER $DB_PASS $DB_NAME tcp://0.0.0.0:8000 tcp://0.0.0.0:8002 bar
GLOG_logtostderr=1 ./build/examples_chat_noop_client \
    $DB_USER $DB_PASS $DB_NAME tcp://0.0.0.0:8000 tcp://0.0.0.0:8003 baz

# Pinging Client.
GLOG_logtostderr=1 ./build/examples_chat_noop_ping_client \
    $DB_USER $DB_PASS $DB_NAME tcp://0.0.0.0:8000 tcp://0.0.0.0:8004 pinger yolo
```

[bud_chat]: https://github.com/bloom-lang/bud/tree/master/examples/chat
