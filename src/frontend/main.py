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
    collection = {"name": collection_name, "tuples": []}
    cur.execute("""
        SELECT *
        FROM {}_{}
        WHERE (time_inserted = %s AND time_inserted = time_deleted) OR
              (time_inserted <= %s AND (time_deleted IS NULL OR time_deleted > %s))
    """.format(node_name, collection_name), (time, time, time))
    collection["tuples"] += [list(t[3:]) for t in cur.fetchall()]
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
    max_time = max(times)

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
