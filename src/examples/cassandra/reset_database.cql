DROP KEYSPACE test;

CREATE KEYSPACE test
WITH REPLICATION = { 'class': 'SimpleStrategy', 'replication_factor': 3 } ;

CREATE TABLE test.kvs (
    key   text PRIMARY KEY,
    value int
);
