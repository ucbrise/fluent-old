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

## Logical Time
Each fluent node maintains a [logical time][lamport_clocks] that monotonically
increases as the program executes. When we debug a fluent program and jump
backwards and forwards through the previous states of a fluent node, we are
jumping forwards and backwards through logical time. Here, we describe exactly
how and when the logical times are incremented.

Consider a fluent program with three rules (and no bootstrap rules). When we
run the fluent program, we more or less run the following pseudocode:

```
while True:
    receive tuples from other fluent nodes;
    execute rule 1;
    execute rule 2;
    execute rule 3;
    tick collections;
```

Internally, the fluent node will begin with a time of 0 and increment its time
between every operation. That is, the node runs something like the following
pseudocode:

```
time = 0
while True:
    time++; receive tuples from other fluent nodes;
    time++; execute rule 1;
    time++; execute rule 2;
    time++; execute rule 3;
    time++; tick collections;
```

Whenever a tuple is derived or deleted, it is associated with the current
logical time. For example, on the first iteration of the loop above, any tuples
received from other fluent nodes are associated with time 1. Any tuples derived
or deleted by rule 1, 2, and 3 are associated with time 2, 3, and 4
respectively. Any tuples deleted during a tick are associated with timestamp 5.

If we have bootstrap rules, we update time similarly. For example, image we add
two bootstrap rules to our fluent program. Now, our pseudocode looks like this:

```
time = 0
time++; execute boostrap rule 1;
time++; execute boostrap rule 2;
time++; tick collections;
while True:
    time++; receive tuples from other fluent nodes;
    time++; execute rule 1;
    time++; execute rule 2;
    time++; execute rule 3;
    time++; tick collections;
```

## Implementation
Here's a brief overview of how distributed lineage is implemented in Fluent.
We store all lineage information in PostgreSQL (for now). To explain what
exactly is stored, it's best to look at an example:

```c++
zmq::context_t context(1);
postgres::ConnectionConfig config = GetConnectionConfig();
auto f = fluent<PqxxClient>("my_program", "inproc://addr", &context, config)
  .table<int, char>("my_table", {{"x", "y"}})
  .scratch<int>("my_scratch", {{"x"}})
  .channel<std::string, float>("my_channel", {{"addr", "x"})
  .RegisterRules([](auto& t, auto& s, auto& c) {
    using namespace fluent::infix;
    return std::make_tuple(s <= c.Iterable(), t <= s.Iterable());
  };
```

This fluent program has three collections---

- a table `my_table(x:int, y:char)`,
- a scratch `my_scratch(x:int)`, and
- a channel `my_channel(addr:std::string, x:float)`.

---and two rules:

- `s <= c.Iterable` and
- `t <= s.Iterable()`

Information about this program, its collections, and its rules are found in the
`Nodes`, `Collections`, and `Rules` tables within the postgres database.

- The `Nodes(id, name, address)` relation holds the name, id (the hash of the
  name), and address for every fluent program.
- The `Collections(node_id, collection_name, collection_type, column_names)`
  relation holds the name and type of every collection of every fluent program.
- The `Rules(node_id, rule_number, is_bootstrap, rule)` relation holds every
  rule of every fluent program.

```
> SELECT * FROM Nodes;
+----+------------+-----------+
| id | name       | address   |
|----+------------+-----------+
| 42 | my_program | 127.0.0.1 |
+----+------------+-----------+

> SELECT * FROM Collections;
+---------+-----------------+-----------------+---------------+
| node_id | collection_name | collection_type | column_names  |
|---------+-----------------+-----------------+---------------+
| 42      | my_table        | Table           | ['x', 'y']    |
| 42      | my_sratch       | Scratch         | ['x']         |
| 42      | my_channel      | Channel         | ['addr', 'x'] |
+---------+-----------------+-----------------+---------------+

> SELECT * FROM Rules;
+---------+-------------+--------------+--------+
| node_id | rule_number | is_bootstrap | rule   |
|---------+-------------+--------------+--------+
| 42      | 0           | false        | s <= c |
| 42      | 1           | false        | t <= s |
+---------+-------------+--------------+--------+
```

The postgres database also has a relation for every collection of every
program. For a program `p` and collection `c`, the relation is named `p_c`. The
schema of the relation is the same as the schema of `c` with five additional
columns prepended:

1. the hash of the tuple,
2. the logical time at which the tuple was inserted,
3. the logical time at which the tuple was deleted.
4. the physical time at which the tuple was inserted, and
5. the physical time at which the tuple was deleted.

```
> \d my_program_my_table
+------------------------+--------------------------+-----------+
| Column                 | Type                     | Modifiers |
+------------------------+--------------------------+-----------+
| hash                   | bigint                   | not null  |
| time_inserted          | integer                  | not null  |
| time_deleted           | integer                  |           |
| physical_time_inserted | timestamp with time zone | not null  |
| physical_time_deleted  | timestamp with time zone |           |
| x                      | integer                  | not null  |
| y                      | char(1)                  | not null  |
+------------------------+--------------------------+-----------+

> \d my_program_my_scratch
+------------------------+--------------------------+-----------+
| Column                 | Type                     | Modifiers |
+------------------------+--------------------------+-----------+
| hash                   | bigint                   | not null  |
| time_inserted          | integer                  | not null  |
| time_deleted           | integer                  |           |
| physical_time_inserted | timestamp with time zone | not null  |
| physical_time_deleted  | timestamp with time zone |           |
| x                      | integer                  | not null  |
+------------------------+--------------------------+-----------+
```

Additionally, for each program `p`, we generate a `p_lineage` table:

```
> \d my_program_lineage
+---------------------+--------------------------+-----------+
| Column              | Type                     | Modifiers |
+---------------------+--------------------------+-----------+
| dep_node_id         | bigint                   | not null  |
| dep_collection_name | text                     | not null  |
| dep_tuple_hash      | bigint                   | not null  |
| dep_time            | integer                  | not null  |
| rule number         | integer                  |           |
| inserted            | boolean                  | not null  |
| physical_time       | timestamp with time zone | not null  |
| collection_name     | text                     | not null  |
| tuple_hash          | bigint                   | not null  |
| time                | integer                  | not null  |
+---------------------+--------------------------+-----------+
```

where a tuple `(dn, dc, dt, r, i, pt, c, t, time)` represents that the tuple
`dt` in collection `dc` on node `dn` was used by rule `r` to insert (if `i` is
true) or delete (if `i` is false) the tuple `t` into collection `c` at time
`time` (physical time `pt`).  `dep_time` is left null for local derivations,
and rule number is left null for networked derivations.

[lamport_clocks]: https://scholar.google.com/scholar?cluster=4892527405117123487
[libpqxx_site]: http://pqxx.org/development/libpqxx/
