import flask
import psycopg2

app = flask.Flask(__name__)
db = psycopg2.connect("dbname=vagrant")

@app.route("/")
def index():
    return flask.send_file("index.html")

@app.route("/node_names")
def node_names():
    cur = db.cursor()
    cur.execute("SELECT name FROM nodes;")
    names = [t[0] for t in cur.fetchall()]
    cur.close()
    return flask.jsonify(node_names=names)

@app.route("/node")
def node():
    node_name = flask.request.args.get('name', "")
    cur = db.cursor()

    # Rules.
    cur.execute("""
        SELECT R.rule
        FROM Nodes N, Rules R
        WHERE N.name = %s AND N.id = R.node_id
        ORDER BY R.rule_number;
    """, (node_name,))
    rules = [t[0] for t in cur.fetchall()]

    # Collection names.
    cur.execute("""
        SELECT C.collection_name
        FROM Nodes N, Collections C
        WHERE N.name = %s AND N.id = C.node_id;
    """, (node_name,))
    collection_names = [t[0] for t in cur.fetchall()]

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
    collections = []
    for collection_name in collection_names:
        collection = {"name": collection_name, "tuples": []}
        cur.execute("""
            SELECT *
            FROM {}_{};
        """.format(node_name, collection_name))
        collection["tuples"] += [list(t[3:]) for t in cur.fetchall()]
        collections.append(collection)

    cur.close()

    n = {
        "name": node_name,
        "rules": rules,
        "time": max_time,
        "collections": collections,
    }
    return flask.jsonify(node=n)
