-- psql -f reset_database.sql

DROP TABLE IF EXISTS Nodes;
DROP TABLE IF EXISTS Collections;
DROP TABLE IF EXISTS Rules;

CREATE TABLE Nodes (
    id      bigint PRIMARY KEY,
    name    text   NOT NULL
);

CREATE TABLE Collections (
    node_id         bigint NOT NULL,
    collection_name text   NOT NULL
);

CREATE TABLE Rules (
    node_id bigint NOT NULL,
    rule    text   NOT NULL
);
