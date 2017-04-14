import flask
import psycopg2

app = flask.Flask(__name__)
db = psycopg2.connect("dbname=vagrant")

def get_collection_names(cur, node_name):
    cur.execute("""
        SELECT C.collection_name
        FROM Nodes N, Collections C
        WHERE N.name = %s AND N.id = C.node_id;
    """, (node_name,))
    return [t[0] for t in cur.fetchall()]

def get_collection(cur, node_name, collection_name, time):
    collection = {"name": collection_name, "type": "", "tuples": []}

    # Type, Column Names
    cur.execute("""
        SELECT collection_type, column_names
        FROM Collections
        WHERE collection_name = %s;
    """, (collection_name, ))
    t = cur.fetchone()
    collection["type"] = t[0]
    collection["column_names"] = t[1]

    # Tuples
    cur.execute("""
        SELECT *
        FROM {}_{}
        WHERE (time_inserted = %s AND time_inserted = time_deleted) OR
              (time_inserted <= %s AND (time_deleted IS NULL OR time_deleted > %s))
    """.format(node_name, collection_name), (time, time, time))
    collection["tuples"] += [[str(t[0])] + list(t[3:]) for t in cur.fetchall()]

    return collection

@app.route("/")
def index():
    return flask.send_file("index.html")

@app.route("/nodes")
def nodes():
    cur = db.cursor()
    cur.execute("SELECT name, address FROM nodes;")
    nodes = cur.fetchall()
    cur.close()
    return flask.jsonify(nodes=nodes)

@app.route("/node")
def node():
    node_name = flask.request.args.get('name', "")
    cur = db.cursor()

    # Address.
    cur.execute("""
        SELECT N.address
        FROM Nodes N
        WHERE N.name = %s;
    """, (node_name,))
    address = cur.fetchone()[0]

    # Bootstrap rules.
    cur.execute("""
        SELECT R.rule
        FROM Nodes N, Rules R
        WHERE N.name = %s AND N.id = R.node_id AND R.is_bootstrap
        ORDER BY R.rule_number;
    """, (node_name,))
    bootstrap_rules = [t[0] for t in cur.fetchall()]

    # Rules.
    cur.execute("""
        SELECT R.rule
        FROM Nodes N, Rules R
        WHERE N.name = %s AND N.id = R.node_id AND (NOT R.is_bootstrap)
        ORDER BY R.rule_number;
    """, (node_name,))
    rules = [t[0] for t in cur.fetchall()]

    # Collection names.
    collection_names = get_collection_names(cur, node_name)

    # Time.
    times = []
    for collection_name in collection_names:
        cur.execute("""
            SELECT MAX(time_inserted)
            FROM {}_{};
        """.format(node_name, collection_name))
        times.append(cur.fetchone()[0])
    # If no rules have been executed at all, then max(times) is None and we
    # default to a time of 0.
    max_time = max(times) or 0

    # Collections.
    collections = [get_collection(cur, node_name, collection_name, max_time)
                   for collection_name in collection_names]

    cur.close()

    n = {
        "name": node_name,
        "address": address,
        "bootstrap_rules": bootstrap_rules,
        "rules": rules,
        "time": max_time,
        "collections": collections,
        "clicked_tuple": None,
        "lineage_tuples": None,
    }
    return flask.jsonify(node=n)

@app.route("/collections")
def collections():
    node_name = flask.request.args.get("name", "")
    time = flask.request.args.get("time", 0, type=int)
    cur = db.cursor()
    collection_names = get_collection_names(cur, node_name)
    collections = [get_collection(cur, node_name, collection_name, time)
                   for collection_name in collection_names]
    cur.close()
    return flask.jsonify(collections=collections)

@app.route("/lineage")
def lineage():
    node_name = flask.request.args.get("node_name", "")
    collection_name = flask.request.args.get("collection_name", "")
    tuple_hash = flask.request.args.get("hash", 0, type=int)
    time = flask.request.args.get("time", 0, type=int)

    cur = db.cursor()

    # The time of the most recent insertion of the tuple.
    cur.execute("""
        SELECT MAX(time_inserted)
        FROM {}_{}
        WHERE hash = %s AND (
                (time_inserted = %s AND time_inserted = time_deleted) OR
                (time_inserted <= %s AND (time_deleted IS NULL OR time_deleted > %s))
              );
    """.format(node_name, collection_name), (tuple_hash, time, time, time))
    latest_insert_time = cur.fetchone()[0]

    # The lineage.
    cur.execute("""
        SELECT
            N.name,
            L.dep_collection_name,
            L.dep_tuple_hash,
            L.dep_time
        FROM Nodes N, {}_lineage L
        WHERE N.id = L.dep_node_id AND
              L.collection_name = %s AND
              L.tuple_hash = %s AND
              time = %s;
    """.format(node_name), (collection_name, tuple_hash, latest_insert_time))
    lineage_tuples = [
        {
            "node_name": t[0],
            "collection_name": t[1],
            "tuple_hash": str(t[2]),
            "time": t[3] if t[3] is not None else latest_insert_time,
        } for t in cur.fetchall()
    ]

    cur.close()
    return flask.jsonify(lineage=lineage_tuples)
