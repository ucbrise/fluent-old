-- psql -f reset_database.sql

-- http://stackoverflow.com/questions/3327312/drop-all-tables-in-postgresql
DROP SCHEMA PUBLIC CASCADE;
CREATE SCHEMA PUBLIC;

CREATE TYPE lineage_type AS ENUM ('regular', 'sql', 'python');

CREATE TABLE Nodes (
    id                    bigint PRIMARY KEY,
    name                  text   NOT NULL,
    address               text   NOT NULL,
    python_lineage_script text
);

CREATE TABLE Collections (
    node_id               bigint       NOT NULL,
    collection_name       text         NOT NULL,
    collection_type       text         NOT NULL,
    column_names          text[]       NOT NULL,
    lineage_type          lineage_type NOT NULL,
    python_lineage_method text,
    PRIMARY KEY (node_id, collection_name)
);

CREATE TABLE Rules (
    node_id      bigint  NOT NULL,
    rule_number  integer NOT NULL,
    is_bootstrap boolean NOT NULL,
    rule         text    NOT NULL,
    PRIMARY KEY (node_id, rule_number, is_bootstrap)
);

-- Vector clock comparators.
-- =
CREATE FUNCTION VectorClockEq(lhs integer[], rhs integer[]) RETURNS boolean AS
$$SELECT lhs = rhs;$$
LANGUAGE SQL;

-- <>
CREATE FUNCTION VectorClockNe(lhs integer[], rhs integer[]) RETURNS boolean AS
$$SELECT lhs <> rhs;$$
LANGUAGE SQL;

-- <=
CREATE FUNCTION VectorClockLe(lhs integer[], rhs integer[]) RETURNS boolean AS
$$SELECT bool_and(x <= y) FROM (SELECT UNNEST(lhs) AS x, UNNEST(rhs) as y) t;$$
LANGUAGE SQL;

-- <
CREATE FUNCTION VectorClockLt(lhs integer[], rhs integer[]) RETURNS boolean AS
$$SELECT VectorClockLe(lhs, rhs) AND VectorClockNe(lhs, rhs);$$
LANGUAGE SQL;

-- >=
CREATE FUNCTION VectorClockGe(lhs integer[], rhs integer[]) RETURNS boolean AS
$$SELECT bool_and(x >= y) FROM (SELECT UNNEST(lhs) AS x, UNNEST(rhs) as y) t;$$
LANGUAGE SQL;

-- >
CREATE FUNCTION VectorClockGt(lhs integer[], rhs integer[]) RETURNS boolean AS
$$SELECT VectorClockGe(lhs, rhs) AND VectorClockNe(lhs, rhs);$$
LANGUAGE SQL;

-- Concurrent
CREATE FUNCTION VectorClockConcurrent(lhs integer[], rhs integer[])
    RETURNS boolean AS
$$SELECT NOT VectorClockLe(lhs, rhs) AND NOT VectorClockGe(lhs, rhs);$$
LANGUAGE SQL;
