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
//   bootstrap_rules: string list,
//   rules: string list,
//   time: int,
//   collections = Collection list,
// }
fluent.Node = function(name, bootstrap_rules, rules, time, collections) {
    this.name = name;
    this.bootstrap_rules = bootstrap_rules;
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

// type NodesUi = {
//   refresh_button: button,
//   node_list: ul,
//   node_buttons: button list,
//   clicked_node: button option,
// }
fluent.NodesUi = function(refresh_button, node_list, node_buttons, clicked_node) {
    this.refresh_button = refresh_button;
    this.node_list = node_list;
    this.node_buttons = node_buttons;
    this.clicked_node = clicked_node;
}

// type RulesUi = {
//   bootstrap_rule_list: ol,
//   bootstrap_rule_spans: span list
//   bootstrap_tick_list: ol,
//   rule_list: ol,
//   rule_spans: span list
// }
fluent.RulesUi = function(bootstrap_rule_list, bootstrap_rule_spans,
                          bootstrap_tick_list, rule_list, rule_spans) {
    this.bootstrap_rule_list = bootstrap_rule_list;
    this.bootstrap_rule_spans = bootstrap_rule_spans;
    this.bootstrap_tick_list = bootstrap_tick_list;
    this.rule_list = rule_list;
    this.rule_spans = rule_spans;
}

// type TimeUi = {
//   minus_button: button,
//   time_span: span,
//   plus_button: button,
// }
fluent.TimeUi = function(minus_button, time_span, plus_button) {
    this.minus_button = minus_button;
    this.time_span = time_span;
    this.plus_button = plus_button;
}

// type CollectionUi = {
//   collection_div: div,
//   name_span: span,
//   tuples: table,
// }
fluent.CollectionUi = function(collection_div, name_span, tuples) {
    this.collection_div = collection_div;
    this.name_span = name_span;
    this.tuples = tuples;
}

// type CollectionsUi = {
//   collections_div: div,
//   collections: CollectionUi list
// }
fluent.CollectionsUi = function(collections_div, collections) {
    this.collections_div = collections_div;
    this.collections = collections;
}

// type StateUi = {
//   nodes: NodesUi,
//   rules: RulesUi,
//   time: TimeUi,
//   collections: CollectionUi,
// }
fluent.StateUi = function(nodes, rules, time, collections) {
    this.nodes = nodes;
    this.rules = rules;
    this.time = time;
    this.collections = collections;
}

fluent.CreateStateUi = function() {
    var nodes = new fluent.NodesUi(
        document.getElementById("refresh_nodes_list"),
        document.getElementById("nodes_list"),
        [],
        null);
    var rules = new fluent.RulesUi(
        document.getElementById("bootstrap_rules_list"),
        [],
        document.getElementById("bootstrap_tick_list"),
        document.getElementById("rules_list"),
        []);
    var time = new fluent.TimeUi(
        document.getElementById("time_down"),
        document.getElementById("time"),
        document.getElementById("time_up"));
    var collections = new fluent.CollectionsUi(
        document.getElementById("collections_container"),
        []);
    return new fluent.StateUi(nodes, rules, time, collections);
}

////////////////////////////////////////////////////////////////////////////////
// AJAX Endpoints
////////////////////////////////////////////////////////////////////////////////
fluent.ajax = {};

// node_names: unit -> string list
fluent.ajax.node_names = function(callback) {
    fluent.ajax_get("/node_names", function(result) {
        callback(result["node_names"]);
    });
}

// node_names: string -> Node
fluent.ajax.node = function(name, callback) {
    fluent.ajax_get("/node?name=" + name, function(result) {
        callback(result["node"]);
    });
}

// collections: string -> int -> Collection list
fluent.ajax.collections = function(name, time, callback) {
    var url = "/collections?name=" + name + "&time=" + time;
    fluent.ajax_get(url, function(result) {
        callback(result["collections"]);
    });
}

////////////////////////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////////////////////////
fluent.callbacks = {}

fluent.callbacks.refresh_nodes = function(state, state_ui) {
    return function() {
        fluent.ajax.node_names(function(node_names) {
            state.node_names = node_names;
            fluent.render_node_names(state, state_ui);
        });
    };
}

fluent.callbacks.click_node = function(node_name, state, state_ui) {
    return function() {
        fluent.ajax.node(node_name, function(node) {
            state.node = node;
            fluent.render_state(state, state_ui);
        });
    }
}

fluent.callbacks.time_down = function(state, state_ui) {
    return function() {
        if (state.node !== null) {
            var node = state.node;
            node.time = Math.max(0, node.time - 1);
            fluent.ajax.collections(node.name, node.time, function(collections) {
                node.collections = collections;
                fluent.render_state(state, state_ui);
            });
        }
    }
}

fluent.callbacks.time_up = function(state, state_ui) {
    return function() {
        if (state.node !== null) {
            var node = state.node;
            node.time += 1;
            fluent.ajax.collections(node.name, node.time, function(collections) {
                node.collections = collections;
                fluent.render_state(state, state_ui);
            });
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// UI Initialization
////////////////////////////////////////////////////////////////////////////////
fluent.init_ui = function(state, state_ui) {
    // Refresh nodes button.
    state_ui.nodes.refresh_button.onclick =
        fluent.callbacks.refresh_nodes(state, state_ui);

    // Time up and down.
    state_ui.time.minus_button.onclick =
        fluent.callbacks.time_down(state, state_ui);
    state_ui.time.plus_button.onclick =
        fluent.callbacks.time_up(state, state_ui);
    document.onkeydown = function(e) {
        if (e.key === "ArrowLeft") {
            fluent.callbacks.time_down(state, state_ui)();
        } else if (e.key === "ArrowRight")  {
            fluent.callbacks.time_up(state, state_ui)();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Rendering Functions
////////////////////////////////////////////////////////////////////////////////
fluent.render_node_names = function(state, state_ui) {
    var nodes = state_ui.nodes;
    nodes.node_list.innerHTML = "";
    nodes.node_buttons = [];
    nodes.clicked_node = null;

    for (var i = 0; i < state.node_names.length; ++i) {
        node_name = state.node_names[i];
        var button = document.createElement("button");
        button.id = "node_" + node_name;
        button.className = "node";
        button.innerHTML = node_name;
        if (state.node !== null && state.node.name === node_name) {
            button.classList.add("bolded");
        }
        button.onclick = fluent.callbacks.click_node(node_name, state, state_ui);
        nodes.node_buttons.push(button);

        var li = document.createElement("li");
        li.appendChild(button);
        nodes.node_list.appendChild(li);
    }
}

fluent.render_state = function(state, state_ui) {
    fluent.render_node(state, state_ui);
    fluent.render_rules(state, state_ui);
    fluent.render_time(state, state_ui);
    fluent.render_collections(state, state_ui);
}

fluent.render_node = function(state, state_ui) {
    if (state.node === null) {
        return;
    }

    if (state_ui.nodes.clicked_node !== null) {
        state_ui.nodes.clicked_node.classList.remove("bolded");
    }

    for (var i = 0; i < state_ui.nodes.node_buttons.length; ++i) {
        var button = state_ui.nodes.node_buttons[i];
        if (button.innerHTML === state.node.name) {
            button.classList.add("bolded");
            state_ui.nodes.clicked_node = button;
            return;
        }
    }
}

fluent.render_rules = function(state, state_ui) {
    if (state.node === null) {
        return;
    }

    var render = function(rules, rule_list, rule_spans) {
        console.log(rules);
        rule_list.innerHTML = "";
        rule_spans = [];
        for (var i = 0; i < rules.length; ++i) {
            var rule = rules[i];
            var text = document.createElement("span");
            text.id = "rule_" + i;
            text.className = "rule";
            text.innerHTML = rule;
            rule_spans.push(text);

            var li = document.createElement("li");
            li.appendChild(text);
            rule_list.appendChild(li);
        }
    };

    var rules_ui = state_ui.rules;
    render(state.node.bootstrap_rules, rules_ui.bootstrap_rule_list,
           rules_ui.bootstrap_rule_spans);
    render(state.node.rules, rules_ui.rule_list, rules_ui.rule_spans);

    rules_ui.bootstrap_tick_list.innerHTML = "";
    if (state.node.bootstrap_rules.length > 0) {
        var li = document.createElement("li");
        li.appendChild(document.createTextNode("Tick"));
        rules_ui.bootstrap_tick_list.appendChild(li);
    }
}

fluent.render_time = function(state, state_ui) {
    if (state.node !== null) {
        state_ui.time.time_span.innerHTML = state.node.time;
    }
}

fluent.render_collection = function(collection) {
    var collection_name = document.createElement("span");
    collection_name.className = "collection_name";
    collection_name.innerHTML = collection.name;

    var tuples = fluent.create_html_table(collection.tuples);

    var collection_div = document.createElement("div");
    collection_div.id = "collection_" + collection.name;
    collection_div.className = "collection";
    collection_div.appendChild(collection_name);
    collection_div.appendChild(tuples);

    return new fluent.CollectionUi(collection_div, collection_name, tuples);
}

fluent.render_collections = function(state, state_ui) {
    if (state.node === null) {
        return;
    }

    var collections_div = state_ui.collections.collections_div;
    var collections = state_ui.collections.collections;
    collections_div.innerHTML = "";
    collections = [];
    for (var i = 0; i < state.node.collections.length; ++i) {
        var collection_ui = fluent.render_collection(state.node.collections[i]);
        collections_div.appendChild(collection_ui.collection_div);
        collections.push(collection_ui);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////
function main() {
    var state = new fluent.State([], null);
    var state_ui = fluent.CreateStateUi();
    fluent.init_ui(state, state_ui);
}

window.onload = main
