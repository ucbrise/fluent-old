# Cassandra
This directory wraps a three-node Cassandra cluster in Fluent.

## Background
If you go the [main Cassandra website](http://cassandra.apache.org/doc/latest/)
for documentation on Cassandra, you're going to be sorely disappointed. Most of
the documentation is empty. On the other hand, [Datastax's
documentation](http://docs.datastax.com/en/cassandra/3.0/) is really good.

## Getting Started
To build Cassandra from source,

```git
cd $HOME
git clone git@github.com:apache/cassandra.git
cd cassandra
ant
```

Datastax has [a nice tutorial on setting up a multi-node
cluster](http://docs.datastax.com/en/cassandra/3.0/cassandra/initialize/initSingleDS.html),
but getting a Cassandra cluster started on a single machine is actually really
hard. Luckily, there's a really sweet utility called
[ccm](https://github.com/pcmanus/ccm) which helps you do exactly this:

```
pip install ccm
ccm create test_cluster --install-dir=$HOME/cassandra -n 3 -s
```
