import flask
app = flask.Flask(__name__)

@app.route("/")
def index():
    return flask.send_file("index.html")

@app.route("/node_names")
def node_names():
    # TODO(mwhittaker): Implement
    return flask.jsonify(node_names=["a", "b"])

@app.route("/node")
def node():
    # TODO(mwhittaker): Implement
    node_name = flask.request.args.get('name', "")
    collection = {
        "name": "a_table",
        "tuples": [
            ["1", "2", "3"],
            ["5", "6", "7"],
            ["9", "10", "11"],
        ],
    }
    n = {
        "name": node_name,
        "rules": ["t <= Count(c)", "s += Map(t)"],
        "time": 42,
        "collections": [collection, collection, collection],
    }
    return flask.jsonify(node=n)
