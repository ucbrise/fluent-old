# Relational Algebra
This directory implements the relational algebra. For example, given a
single-column relation `xs` of integers, the following SQL query returns the
doubled value of all odd integers:

```
SELECT xs.x + xs.x
FROM   xs
WHERE  xs.x + xs.x % 2 == 1
```

We can construct a corresponding relational algebra expression:

```c++
vector<tuple<int>> xs = {{1}, {2}, {3}};
auto relalg = ra::make_iterable(&xs)
| ra::map([](const auto& t) { return tuple<int, int>(get<0>(t), get<0>(t)); })
| ra::filter([](const auto& t) { return get<0>(t) % 2 == 1; })
| ra::map([](const auto& t) { return get<0>(t) + get<1>(t); });
```

This relational algebra expression `relalg` is **logical** in the sense that it
doesn't have any `GetNext` or `Reset` methods or anything like that. Instead,
it has a single method `ToRange` which returns a [range-v3][] range which
produces the result of the relational algebra expression. For example, the
following code would print out `2` and then `6`:

```c++
ranges::for_each(relalg.ToRange(), [](int x) {
    std::cout << x << std::endl;
});
```

[range-v3]: https://github.com/ericniebler/range-v3
