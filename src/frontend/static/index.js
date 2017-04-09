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
//   type: string,
//   tuples: string list list,
// }
fluent.Collection = function(name, type, tuples) {
    this.name = name;
    this.type = type;
    this.tuples = tuples;
}

// type LineageTuple = {
//   node_name: string,
//   collection_name: string,
//   tuple_hash: int,
//   time: int,
// }
fluent.LineageTuple = function(node_name, collection_name, tuple_hash, time) {
    this.node_name = node_name;
    this.collection_name = collection_name;
    this.tuple_hash = tuple_hash;
    this.time = time;
}

// type Node = {
//   name: string,
//   bootstrap_rules: string list,
//   rules: string list,
//   time: int,
//   collections: Collection list,
//   clicked_tuple: LineageTuple option,
//   lineage_tuples: LineageTuple list option,
// }
fluent.Node = function(name, bootstrap_rules, rules, time, collections,
                       clicked_tuple, lineage_tuples) {
    this.name = name;
    this.bootstrap_rules = bootstrap_rules;
    this.rules = rules;
    this.time = time;
    this.collections = collections;
    this.clicked_tuple = clicked_tuple;
    this.lineage_tuples = lineage_tuples;
}

// type State = {
//   nodes: [string, string] list,
//   node: Node option,
// }
fluent.State = function(nodes, node) {
    this.nodes = nodes;
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
//   receive_list: ol,
//   rule_list: ol,
//   rule_spans: span list
//   tick_list: ol,
// }
fluent.RulesUi = function(bootstrap_rule_list, bootstrap_rule_spans,
                          bootstrap_tick_list, receive_list, rule_list,
                          rule_spans, tick_list) {
    this.bootstrap_rule_list = bootstrap_rule_list;
    this.bootstrap_rule_spans = bootstrap_rule_spans;
    this.bootstrap_tick_list = bootstrap_tick_list;
    this.receive_list = receive_list;
    this.rule_list = rule_list;
    this.rule_spans = rule_spans;
    this.tick_list = tick_list;
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
fluent.CollectionUi = function(collection_div, name_span, type_span, tuples) {
    this.collection_div = collection_div;
    this.name_span = name_span;
    this.type_span = type_span;
    this.tuples = tuples;
}

// type CollectionsUi = {
//   collections_div: div,
//   collections: CollectionUi list
//   index: {hash -> tr}
//   clicked_tuple: tr option
// }
fluent.CollectionsUi = function(collections_div, collections, index,
                                clicked_tuple) {
    this.collections_div = collections_div;
    this.collections = collections;
    this.index = index;
    this.clicked_tuple = clicked_tuple;
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
        document.getElementById("receive_list"),
        document.getElementById("rules_list"),
        [],
        document.getElementById("tick_list"));
    var time = new fluent.TimeUi(
        document.getElementById("time_down"),
        document.getElementById("time"),
        document.getElementById("time_up"));
    var collections = new fluent.CollectionsUi(
        document.getElementById("collections_container"),
        [],
        {},
        null);
    return new fluent.StateUi(nodes, rules, time, collections);
}

////////////////////////////////////////////////////////////////////////////////
// AJAX Endpoints
////////////////////////////////////////////////////////////////////////////////
fluent.ajax = {};

// nodes: unit -> [string, string] list
fluent.ajax.nodes = function(callback) {
    fluent.ajax_get("/nodes", function(result) {
        callback(result["nodes"]);
    });
}

// node: string -> Node
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

// lineage: lineage_tuple -> lineage_tuple list
fluent.ajax.lineage = function(lt, callback) {
    var url = "/lineage" +
        "?node_name=" + lt.node_name +
        "&collection_name=" + lt.collection_name +
        "&hash=" + lt.tuple_hash +
        "&time=" + lt.time;
    fluent.ajax_get(url, function(result) {
        callback(result["lineage"]);
    });
}

////////////////////////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////////////////////////
fluent.callbacks = {}

fluent.callbacks.refresh_nodes = function(state, state_ui) {
    return function() {
        fluent.ajax.nodes(function(nodes) {
            state.nodes = nodes;
            fluent.render_nodes(state, state_ui);
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
            node.clicked_tuple = null;
            node.lineage = null;
            fluent.ajax.collections(node.name, node.time, function(collections) {
                node.collections = collections;
                fluent.render_state(state, state_ui);
            });
        }
    }
}

fluent.callbacks.time_set = function(time, state, state_ui) {
    return function() {
        if (state.node !== null) {
            var node = state.node;
            node.time = time;
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
            node.clicked_tuple = null;
            node.lineage = null;
            fluent.ajax.collections(node.name, node.time, function(collections) {
                node.collections = collections;
                fluent.render_state(state, state_ui);
            });
        }
    }
}

fluent.callbacks.click_tuple = function(clicked_tuple, state, state_ui) {
    return function() {
        fluent.ajax.lineage(clicked_tuple, function(lineage) {
            state.node.clicked_tuple = clicked_tuple;
            state.node.lineage = lineage;

            if (lineage.length == 1 &&
                lineage[0].node_name != clicked_tuple.node_name)  {
                // Network lineage.
                fluent.ajax.node(lineage[0].node_name, function(node) {
                    state.node = node;
                    state.node.clicked_tuple = clicked_tuple;
                    state.node.lineage = lineage;
                    fluent.callbacks.time_set(lineage[0].time, state, state_ui)();
                })
            } else if (lineage.length > 0) {
                // Derived lineage.
                fluent.callbacks.time_set(
                    lineage[0].time, state, state_ui)();
            } else {
                fluent.render_state(state, state_ui);
            }
        });
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
fluent.render_nodes = function(state, state_ui) {
    var nodes = state_ui.nodes;
    nodes.node_list.innerHTML = "";
    nodes.node_buttons = [];
    nodes.clicked_node = null;

    for (var i = 0; i < state.nodes.length; ++i) {
        var node = state.nodes[i];
        var name = node[0];
        var address = node[1];

        var name_span = document.createElement("span");
        name_span.id = "node_" + name + "_name";
        name_span.className = "name";
        name_span.innerHTML = name;

        var address_span = document.createElement("span");
        address_span.id = "node_" + name + "_address";
        address_span.className = "address";
        address_span.innerHTML = address;

        var button = document.createElement("button");
        button.id = "node_" + name;
        button.className = "node";
        button.appendChild(name_span);
        button.appendChild(address_span);
        if (state.node !== null && state.node.name === name) {
            button.classList.add("bolded");
            state_ui.nodes.clicked_node = button;
        }
        button.onclick = fluent.callbacks.click_node(name, state, state_ui);
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
    fluent.render_lineage(state, state_ui);
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
        if (button.children[0].innerHTML === state.node.name) {
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

    var rules_ui = state_ui.rules;

    // Render rules.
    var render = function(rules, rule_list, rule_spans) {
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
    rules_ui.bootstrap_rule_list.innerHTML = "";
    rules_ui.bootstrap_rule_spans = [];
    rules_ui.rule_list.innerHTML = "";
    rules_ui.rule_spans = [];
    render(state.node.bootstrap_rules, rules_ui.bootstrap_rule_list,
           rules_ui.bootstrap_rule_spans);
    render(state.node.rules, rules_ui.rule_list, rules_ui.rule_spans);

    // Toggle bootstrap tick.
    rules_ui.bootstrap_tick_list.innerHTML = "";
    if (state.node.bootstrap_rules.length > 0) {
        var li = document.createElement("li");
        li.appendChild(document.createTextNode("Tick"));
        rules_ui.bootstrap_tick_list.appendChild(li);
    }

    // Show current rule.
    var time = state.node.time;
    if (state.node.bootstrap_rules.length > 0) {
        var num_head = rules_ui.bootstrap_rule_spans.length + 1;
        var lis = [];
        lis = lis.concat(rules_ui.bootstrap_rule_spans);
        lis.push(rules_ui.bootstrap_tick_list.children[0]);
        lis.push(rules_ui.receive_list.children[0]);
        lis = lis.concat(rules_ui.rule_spans);
        lis.push(rules_ui.tick_list.children[0]);
    } else {
        var num_head = 0;
        var lis = [];
        lis.push(rules_ui.receive_list.children[0]);
        lis = lis.concat(rules_ui.rule_spans);
        lis.push(rules_ui.tick_list.children[0]);
    }

    for (var i = 0; i < lis.length; ++i) {
        lis[i].classList.remove("current_rule");
    }

    if (time == 0) {
        // Do nothing.
    } else if (time <= num_head) {
        lis[time - 1].classList.add("current_rule");
    } else {
        var i = (time - num_head - 1) % (lis.length - num_head);
        lis[i + num_head].classList.add("current_rule");
    }
}

fluent.render_time = function(state, state_ui) {
    if (state.node !== null) {
        state_ui.time.time_span.innerHTML = state.node.time;
    }
}

fluent.render_collection = function(state, state_ui, collection, index) {
    var collection_name = document.createElement("span");
    collection_name.className = "collection_name";
    collection_name.innerHTML = collection.name;

    var collection_type = document.createElement("span");
    collection_type.className = "collection_type";
    collection_type.innerHTML = ": " + collection.type;

    var tuples = document.createElement("table");
    for (var i = 0; i < collection.tuples.length; ++i) {
        var tuple = collection.tuples[i];
        var row = document.createElement("tr");
        var hash = tuple[0];
        index[hash] = row;

        // We skip the hash (j = 0).
        for (var j = 1; j < tuple.length; ++j) {
            var element = document.createElement("td");
            element.innerHTML = tuple[j];
            row.appendChild(element);
        }
        tuples.appendChild(row);

        var clicked_tuple = new fluent.LineageTuple(state.node.name,
                                                    collection.name, hash,
                                                    state.node.time);
        row.onclick = fluent.callbacks.click_tuple(clicked_tuple, state,
                                                   state_ui);
    }

    var collection_div = document.createElement("div");
    collection_div.id = "collection_" + collection.name;
    collection_div.className = "collection";
    collection_div.appendChild(collection_name);
    collection_div.appendChild(collection_type);
    collection_div.appendChild(tuples);

    return new fluent.CollectionUi(collection_div, collection_name,
                                   collection_type, tuples);
}

fluent.render_collections = function(state, state_ui) {
    if (state.node === null) {
        return;
    }

    var collections = state_ui.collections;
    collections.collections_div.innerHTML = "";
    collections.collections = [];
    collections.index = {};
    for (var i = 0; i < state.node.collections.length; ++i) {
        var collection_ui = fluent.render_collection(
            state, state_ui, state.node.collections[i], collections.index);
        collections.collections_div.appendChild(collection_ui.collection_div);
        collections.collections.push(collection_ui);
    }
}

fluent.render_lineage = function(state, state_ui) {
    if (state.node === null) {
        return;
    }
    if (state.node.clicked_tuple === null) {
        return;
    }

    var index = state_ui.collections.index;
    var clicked_tuple = index[state.node.clicked_tuple.tuple_hash];
    clicked_tuple.classList.add("clicked_tuple");
    state_ui.collections.clicked_tuple = clicked_tuple;

    for (var i = 0; i < state.node.lineage.length; ++i) {
        var lineage_tuple = state.node.lineage[i];
        index[lineage_tuple.tuple_hash].classList.add("lineage_tuple");
    }
}

////////////////////////////////////////////////////////////////////////////////
// Main
////////////////////////////////////////////////////////////////////////////////
function main() {
    var state = new fluent.State([], null);
    var state_ui = fluent.CreateStateUi();
    fluent.init_ui(state, state_ui);
    fluent.callbacks.refresh_nodes(state, state_ui)();
}

window.onload = main
