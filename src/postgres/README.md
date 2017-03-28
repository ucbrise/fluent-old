# PostgreSQL Client
Each node in a fluent program stores (a) the history of its state and (b) the
lineage of every tuple it derives in a postgres database. This directory
implements a postgres client using [libpqxx](libpqxx_site) as well as some mock
clients that are useful for unit tests.

[libpqxx_site]: http://pqxx.org/development/libpqxx/
