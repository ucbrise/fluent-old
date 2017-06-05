# Lineage DB Client

## Overview
Each node in a fluent program stores (a) the history of its state and (b) the
lineage of every tuple it derives in some database. This directory implements
three clients to that database:

1. The first is a [postgres client](pqxx_client.h) that uses
   [libpqxx](libpqxx_site) to store things in postgres.
2. The second is a [noop client](noop_client.h) which actually doesn't store
   anything at all.
3. The third is a [mock client](mock_client.h) which stores everything locally
   for testing.

## Implementation
Here's a brief overview of how distributed lineage is implemented in Fluent.
We store all lineage information in PostgreSQL (for now). To explain what
exactly is stored, it's best to look at an example:

```c++
zmq::context_t context(1);
postgres::ConnectionConfig config = GetConnectionConfig();
auto f = fluent<PqxxClient>("my_program", "inproc://addr", &context, config)
  .table<int, char>("my_table")
  .scratch<int>("my_scratch")
  .channel<std::string, float>("my_channel")
  .RegisterRules([](auto& t, auto& s, auto& c) {
    using namespace fluent::infix;
    return std::make_tuple(s <= c.Iterable(), t <= s.Iterable());
  };
```

This fluent program has three collections---

- a table `my_table(int, char)`,
- a scratch `my_scratch(int)`, and
- a channel `my_channel(std::string, float)`.

---and two rules:

- `s <= c.Iterable` and
- `t <= s.Iterable()`

Information about this program, its collections, and its rules are found in the
`Nodes`, `Collections`, and `Rules` tables within the postgres database.

- The `Nodes(id, name)` relation holds the name and id (the hash of the name)
  for every fluent program.
- The `Collections(node_id, collection_name)` relation holds the name of every
  collection of every fluent program.
- The `Rules(node_id, rule_number, rule)` relation holds every rule of every
  fluent program.

```
> SELECT * FROM Nodes;
+----+------------+
| id | name       |
|----+------------|
| 42 | my_program |
+----+------------+

> SELECT * FROM Collections;
+---------+-----------------+
| node_id | collection_name |
|---------+-----------------|
| 42      | my_table        |
| 42      | my_sratch       |
| 42      | my_channel      |
+---------+-----------------+

> SELECT * FROM Rules;
+---------+-------------+--------+
| node_id | rule_number | rule   |
|---------+-------------+--------|
| 42      | 0           | s <= c |
| 42      | 1           | t <= s |
+---------+-------------+--------+
```

The postgres database also has a relation for every collection of every
program. For a program `p` and collection `c`, the relation is named `p_c`. The
schema of the relation is the same as the schema of `c` with three additional
columns prepended: the hash of the tuple, the logical time at which the tuple
was inserted, and the logical time at which the tuple was deleted.

```
> \d my_program_my_table
+---------------+---------+-----------+
| Column        | Type    | Modifiers |
+---------------+---------+-----------+
| hash          | bigint  | not null  |
| time_inserted | integer | not null  |
| time_deleted  | integer |           |
| col_0         | integer | not null  |
| col_1         | char(1) | not null  |
+---------------+---------+-----------+

> \d my_program_my_scratch
+---------------+---------+-----------+
| Column        | Type    | Modifiers |
+---------------+---------+-----------+
| hash          | bigint  | not null  |
| time_inserted | integer | not null  |
| time_deleted  | integer |           |
| col_0         | integer | not null  |
+---------------+---------+-----------+
```

Additionally, for each program `p`, we generate a `p_lineage` table:

```
> \d my_program_lineage
+---------------------+---------+-----------+
| Column              | Type    | Modifiers |
+---------------------+---------+-----------+
| dep_node_id         | bigint  | not null  |
| dep_collection_name | text    | not null  |
| dep_tuple_hash      | bigint  | not null  |
| dep_time            | integer |           |
| rule number         | integer |           |
| inserted            | boolean | not null  |
| collection_name     | text    | not null  |
| tuple_hash          | bigint  | not null  |
| time                | integer | not null  |
+---------------------+---------+-----------+
```

where a tuple `(dn, dc, dt, r, i, c, t, time)` represents that the tuple `dt`
in collection `dc` on node `dn` was used by rule `r` to insert (if `i` is true)
or delete (if `i` is false) the tuple `t` into collection `c` at time `time`.
`dep_time` is left null for local derivations, and rule number is left null for
networked derivations.

TODO(mwhittaker): Explain how logical time works.

[libpqxx_site]: http://pqxx.org/development/libpqxx/
