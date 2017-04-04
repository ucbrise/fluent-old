fluent = {};

////////////////////////////////////////////////////////////////////////////////
// HTML Helpers
////////////////////////////////////////////////////////////////////////////////
// https://plainjs.com/javascript/ajax/send-ajax-get-and-post-requests-47/
fluent.ajax_get = function(url, on_success) {
  var xhr = window.XMLHttpRequest ? new XMLHttpRequest()
                                  : new ActiveXObject('Microsoft.XMLHTTP');
  xhr.open('GET', url);
  xhr.onreadystatechange = function() {
    if (xhr.readyState > 3 && xhr.status == 200) {
        on_success(JSON.parse(xhr.responseText));
    }
  };
  xhr.setRequestHeader('X-Requested-With', 'XMLHttpRequest');
  xhr.send();
  return xhr;
}

// `create_html_table(rows: string list list)` returns an html table created
// from `rows`. For example, if rows is [["a", "b", "c"], ["d", "e", "f"]], the
// returned table will look like this:
//
//   <table>
//     <tr> <td>a</td> <td>b</td> <td>c</td> </tr>
//     <tr> <td>e</td> <td>e</td> <td>f</td> </tr>
//   </table>
fluent.create_html_table = function(rows) {
    var table = document.createElement("table");
    for (var i = 0; i < rows.length; ++i) {
        var row = document.createElement("tr");
        for (var j = 0; j < rows[i].length; ++j) {
            var element = document.createElement("td");
            element.innerHTML = rows[i][j];
            row.appendChild(element);
        }
        table.appendChild(row);
    }
    return table;
}

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////
// type Collection = {
//   name: string,
//   tuples: string list list,
// }
fluent.Collection = function(name, num_columns, tuples) {
    this.name = name;
    this.num_columns = num_columns;
    this.tuples = tuples;
}

// type Node = {
//   name: string,
//   rules: string list,
//   time: int,
//   collections = Collection list,
// }
fluent.Node = function(name, rules, time, collections) {
    this.name = name;
    this.rules = rules;
    this.time = time;
    this.collections = collections;
}

// type State = {
//   node_names: string list,
//   node: Node option,
// }
fluent.State = function(node_names, node) {
    this.node_names = node_names;
    this.node = node;
}

////////////////////////////////////////////////////////////////////////////////
// AJAX Endpoints
////////////////////////////////////////////////////////////////////////////////
fluent.ajax = {};

// node_names: unit -> string list
fluent.ajax.node_names = function(callback) {
    fluent.ajax_get("/node_names", function(result){
        callback(result["node_names"]);
    });
}

// node_names: string -> Node
fluent.ajax.node = function(name, callback) {
    fluent.ajax_get("/node?name=" + name, function(result){
        callback(result["node"]);
    });
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fluent.render_node_names = function(state) {
    var nodes_list = document.getElementById("nodes_list");
    for (var i = 0; i < state.node_names.length; ++i) {
        node_name = state.node_names[i];
        var button = document.createElement("button");
        button.id = "node_" + node_name;
        button.className = "node";
        button.innerHTML = node_name;
        button.onclick = function(node_name) {
            return function() {
                fluent.ajax.node(node_name, function(node) {
                    state.node = node;
                    fluent.render_state(state);
                });
            };
        }(node_name);

        var li = document.createElement("li");
        li.appendChild(button);
        nodes_list.appendChild(li);
    }
}

fluent.render_rules = function(state) {
    if (state.node === null) {
        return;
    }

    var rules_list = document.getElementById("rules_list");
    rules_list.innerHTML = "";
    for (var i = 0; i < state.node.rules.length; ++i) {
        var rule = state.node.rules[i];
        var text = document.createElement("span");
        text.id = "rule_" + i;
        text.className = "rule";
        text.innerHTML = rule;

        var li = document.createElement("li");
        li.appendChild(text);
        rules_list.appendChild(li);
    }
}

fluent.render_collection = function(collection) {
    var collection_name = document.createElement("span");
    collection_name.className = "collection_name";
    collection_name.innerHTML = collection.name;

    var collection_div = document.createElement("div");
    collection_div.id = "collection_" + collection.name;
    collection_div.className = "collection";
    collection_div.appendChild(collection_name);
    collection_div.appendChild(fluent.create_html_table(collection.tuples));

    return collection_div;
}

fluent.render_collections = function(state) {
    if (state.node === null) {
        return;
    }

    var collections_container = document.getElementById("collections_container");
    collections_container.innerHTML = "";
    for (var i = 0; i < state.node.collections.length; ++i) {
        var collection = fluent.render_collection(state.node.collections[i]);
        collections_container.appendChild(collection);
    }
}

fluent.render_state = function(state) {
    console.log(state);
    fluent.render_rules(state);
    fluent.render_collections(state);
}

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////
function main() {
    var state = new fluent.State([], null);
    fluent.ajax.node_names(function(node_names) {
        state.node_names = node_names;
        fluent.render_node_names(state);
    });
}

window.onload = main
