# Fluent
Fluent is a C++ reimplementation of [Bloom][bloom_lang].

# Getting Started
In this section, we'll walk through how to write a very simple fluent program.
Here it is in full (we'll walk through it piece by piece momentarily):

```c++
zmq::context_t context(1);
ConnectionConfig conn;
std::set<std::tuple<int, char, float>> ts = {{42, 'a', 9001.}};
auto f = fluent<NoopClient>("name", "tcp://0.0.0.0:8000", &context, conn)
  .table<int, char, float>("t1")
  .table<float, int>("t2")
  .scratch<int, int, float>("s")
  .channel<std::string, float, char>("c")
  .RegisterBootstrapRules([&](auto& t1, auto& t2, auto& s, auto& c) {
    return std::make_tuple(t1 <= ra::make_iterable("ts", &ts));
  })
  .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
    return std::make_tuple(t2 <= t1.iterable | ra::project<2, 0>());
  });
f.Run();
```

First, we call the `fluent` function which starts building a `FluentExecutor`:
the thing that actually executes a fluent program.

```c++
  zmq::context_t context(1);
  ConnectionConfig conn;
  auto f = fluent<NoopClient>("name", "tcp://0.0.0.0:8000", &context, conn)
```

We provide the name of the program (i.e. `name`), the address on which the
program should listen (i.e. `tcp://0.0.0.0:8000`), and some miscellaneous stuff
that you can ignore for now. Next, we declare the collections our program will
use:

```c++
  .table<int, char, float>("t1")
  .table<float, int>("t2")
  .scratch<int, int, float>("s")
  .channel<std::string, float, char>("c")
```

This code declares that our fluent program will use

1. a 3-column table named `t1` with types `int`, `char`, and `float`;
2. a 2-column table named `t2` with types `float` and `int`;
3. a 3-column scratch named `s` with types `int`, `int`, and `float`; and
4. a 3-column channel named `c` with types `string`, `float`, and `char`.

Next, we register a single bootstrap rule.

```c++
  .RegisterBootstrapRules([&](auto& t1, auto& t2, auto& s, auto& c) {
    return std::make_tuple(t1 <= ra::make_iterable("ts", &ts));
  })
```

This rule says to merge the contents of `ts` into the `t1` table. When we run
the fluent program, this rule will be executed once at the beginning of the
program.

Then, we register a regular rule.

```c++
  .RegisterRules([](auto& t1, auto& t2, auto& s, auto& c) {
    return std::make_tuple(t2 <= t1.Iterable() | ra::project<2, 0>());
  });
```

This rule says to project out the second and zeroth columns of `t1` and merge
them into the `t2` relation. This rule will be executed every tick.

Finally, we run the program!

```c++
f.Run();
```

`Run` will block and repeatedly execute our registered rules and receive
messages from other fluent nodes.

## Tour
There are six different collections:

- [`Channel`](channel.h)
- [`Periodic`](periodic.h)
- [`Scratch`](scratch.h)
- [`Stdin`](stdin.h)
- [`Stdout`](stdout.h)
- [`Table`](table.h)

lots of miscellaneous files for networking, serialization, etc.:

- [`infix.h`](infix.h)
- [`network_state.h`](network_state.h)
- [`rule.h`](rule.h)
- [`rule_tags.h`](rule_tags.h)
- [`serialization.h`](serialization.h)
- [`socket_cache.cc`](socket_cache.cc)
- [`socket_cache.h`](socket_cache.h)

and two main classes for building and executing fluent programs:

- [`FluentBuilder`](fluent_builder.h)
- [`FluentExecutor`](fluent_executor.h)

[bloom_lang]: http://bloom-lang.net
