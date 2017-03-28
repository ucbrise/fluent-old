# Relational Algebra
This directory implements the relational algebra. For example, given a relation
`xs` with a single integer column `x`, the following SQL query returns the
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
it has a single method `ToPhysical` method which returns a **physical**
relational algebra expression.

```c++
auto physical = relalg.ToPhysical();
```

This physical relational algebra expression has a single `ToRange` method which
returns a [range-v3][] range. The returned range produces the result of the
relational algebra expression. For example, the following code would print out
`2` and then `6`:

```c++
ranges::for_each(physical.ToRange(), [](int x) {
    std::cout << x << std::endl;
});
```

## Operators
- [`count`](count.h)
- [`cross`](cross.h)
- [`filter`](filter.h)
- [`group_by`](group_by.h)
- [`hash_join`](hash_join.h)
- [`iterable`](iterable.h)
- [`map`](map.h)
- [`project`](project.h)

[range-v3]: https://github.com/ericniebler/range-v3
