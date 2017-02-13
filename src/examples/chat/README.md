# Chat
This directory includes a simple chat application modelled directly after
[Bud's chat application][bud_chat].

```bash
# Start the server in one window.
GLOG_logtostderr=1 ./build/examples/chat/example_chat_chat_server \
    tcp://0.0.0.0:8000

# Start some clients in other windows.
GLOG_logtostderr=1 ./build/examples/chat/example_chat_chat_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8001 foo
GLOG_logtostderr=1 ./build/examples/chat/example_chat_chat_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8002 bar
GLOG_logtostderr=1 ./build/examples/chat/example_chat_chat_client \
    tcp://0.0.0.0:8000 tcp://0.0.0.0:8003 baz

# Enter text in any of the clients and will be broadcast to all the others.
```

[bud_chat]: https://github.com/bloom-lang/bud/tree/master/examples/chat
