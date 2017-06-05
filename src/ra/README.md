# Relational Algebra
This directory implements the relational algebra with
[lineage][provenance_book].

## Getting Started
As an example, consider a relation `R` with a single integer column `x`. The
following SQL query returns the doubled value of all odd integers:

```
SELECT 2 * x
FROM   R
WHERE  x % 2 = 1
```

We can express this same exact query using our relation algebra implementation
like this:

```c++
vector<tuple<int>> R = {{1}, {2}, {3}};
auto relalg = ra::make_iterable("R", &R)
| ra::filter([](const tuple<int>& t) { return get<0>(t) % 2 == 1; })
| ra::map([](const tuple<int>& t) { return make_tuple(2 * get<0>(t)); });
```

`relalg` is **logical relational algebra expression** that doesn't perform any
computation itself. Logical relational algebra expression like `relalg` don't
have any `GetNext` or `Reset` methods or anything like that, and executing the
code above won't actually iterate over `R` at all.

To actually execute the query, we have to convert the **logical relational
algebra expression** into a physical relational algebra expression using the
`ToPhysical` method like this:

```c++
auto relalg_physical = relalg.ToPhysical();
```

Physical relational algebra expressions like `relalg_physical` have a single
`ToRange` method which returns a [range-v3][] range. The range produces
`LineagedTuple`s which include (1) a tuple produced by the relational algebra
expression and (2) the lineage of the tuple. For example, this code:

```c++
ranges::for_each(relalg_physical.ToRange(), [](const LineagedTuple<int>& lt) {
    std::cout << "The tuple " << lt.tuple
              << " has lineage " << lt.lineage
              << std::endl;
});
```

will print

```
The tuple (2) has lineage {("R", hash({1}))}
The tuple (6) has lineage {("R", hash({3}))}
```

Here, the lineage of the tuple `(2)` is a singleton set of the tuple `(1)` from
`R`, and the lineage of the tuple `(6)` is a singleton set of the tuple `(3)`
from `R`. Every tuple in a lineage set is identified by the name of its
collection and its hash.

If you don't care about the lineage, you can do something like this to throw it
away:

```c++
auto rng = relalg_physical.ToRange()
           | transform([](const LineagedTuple<int>& lt) { return lt.tuple; });
ranges::for_each(rng, [](const tuple<int>& t) {
    std::cout << t << std::endl;
});
```

## Relational Operators
We implement the following relational operators.

- [`const`](const.h)
- [`count`](count.h)
- [`cross`](cross.h)
- [`filter`](filter.h)
- [`group_by`](group_by.h)
- [`hash_join`](hash_join.h)
- [`iterable`](iterable.h)
- [`map`](map.h)
- [`project`](project.h)

## Implementation
In this section, we describe how to implement your own relational algebra
operator. Relational algebra operators are not subclasses of some "relational
algebra superclass". Every operator is its own distinct type. Despite this
heterogeneity, every operator supports the same interface.

Let's walk through how we would implement an `id` operator that simply forwards
along its inputs unchanged. The first thing to do is implement a logical `Id`
class and a function to wrap its constructor.

```c++
template <typename LogicalChild>
class Id {
 public:
  using column_types = typename LogicalChild::column_types;

  explicit Id(LogicalChild child) : child_(std::move(child)) {}

  auto ToPhysical() const { return make_physical_id(child_.ToPhysical()); }

  std::string ToDebugString() const {
    return fmt::format("Id({})", child_.ToDebugString());
  }

 private:
  const LogicalChild child_;
};

template <typename LogicalChild>
Id<typename std::decay<LogicalChild>::type> make_id(LogicalChild&& child) {
  using decayed = typename std::decay<LogicalChild>::type;
  return Id<decayed>(std::forward<LogicalChild>(child));
}
```

Note the following which is true about `Id` and all logical relational
operators:

- `Id` is parameterized on the type `LogicalChild` of its logical child.
- `Id` defines a `column_types` type which is the type of its output tuples.
- An `Id` object takes ownership of its child.
- `Id` has a `ToPhysical` and `ToDebugString` method.

Next, we implement the physical id operator and a function to wrap its
constructor.

```c++
template <typename PhysicalChild>
class PhysicalId {
 public:
  explicit PhysicalId(PhysicalChild child) : child_(child) {}

  auto ToRange() { return child_.ToRange(); }

 private:
  PhysicalChild child_;
};

template <typename PhysicalChild>
PhysicalId<typename std::decay<PhysicalChild>::type> make_physical_id(
    PhysicalChild&& child) {
  using decayed = typename std::decay<PhysicalChild>::type;
  return PhysicalId<decayed>(std::forward<PhysicalChild>(child));
}
```

Again, note the following which is true about `PhysicalId` and all physical
relational operators:

- `PhysicalId` is parameterized on the type `PhysicalChild` of its physical child.
- A `PhysicalId` object takes ownership of its child.
- `PhysicalId` has a `ToRange` method.

Finally, we implement a bit of code that allows us to pipe things into the
operator like this `some_child() | ra::id()`.

```c++
struct IdPipe {};

IdPipe id() { return {}; }

template <typename LogicalChild>
Id<typename std::decay<LogicalChild>::type> operator|(LogicalChild&& child,
                                                      IdPipe) {
  return make_id(std::forward<LogicalChild>(child));
}
```

`ra::id()` creates an `IdPipe` object, and we overload the `|` operator to take
in an arbitrary logical operator on the left and a special `IdPipe` object on
the right. The `|` operator then returns an `Id` object.

[range-v3]: https://github.com/ericniebler/range-v3
[provenance_book]: https://scholar.google.com/scholar?cluster=14688264622623487965
