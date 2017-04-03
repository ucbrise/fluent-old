# Lineage DB Client
Each node in a fluent program stores (a) the history of its state and (b) the
lineage of every tuple it derives in some database. This directory implements
three clients to that database:

1. The first is a [postgres client](pqxx_client.h) that uses
   [libpqxx](libpqxx_site) to store things in postgres.
2. The second is a [noop client](noop_client.h) which actually doesn't store
   anything at all.
3. The third is a [mock client](mock_client.h) which stores everything locally
   for testing.

[libpqxx_site]: http://pqxx.org/development/libpqxx/
