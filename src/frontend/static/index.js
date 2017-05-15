fluent = {};

// Helper Functions ////////////////////////////////////////////////////////////
// http://stackoverflow.com/a/15313435/3187068
function assert(condition, message) {
  if (!condition) {
    message = message || "Assertion failed";
    if (typeof Error !== "undefined") {
      throw new Error(message);
    }
    throw message; // Fallback
  }
}

// The first five columns of a tuple stored in the lineage database are:
//   0. hash,
//   1. logical time inserted,
//   2. logical time deleted,
//   3. physical time inserted, and
//   4. physical time deleted.
// This function will take a tuple and strip off these header columns.
fluent.strip_header = function(tuple) {
  return tuple.slice(5);
}

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

// Types ///////////////////////////////////////////////////////////////////////
// node_names_addresses: NodeNameAddress list
// node: Node option
// cy: cytoscape
fluent.State = function(node_name_addresses, node, cy) {
  this.node_name_addresses = node_name_addresses;
  this.node = node;
  this.cy = cy;
}

// name: string
// address: string
fluent.NodeNameAddress = function(name, address) {
  assert(typeof(name) === "string");
  assert(typeof(address) === "string");
  this.name = name;
  this.address = address;
}

// name: string
// address: string
// bootstrap_rules: string list
// rules: string list
// time: number
// collections: Collection list
// clicked_hash: string option
fluent.Node = function(name, address, bootstrap_rules, rules, time,
                       collections, clicked_hash) {
  assert(typeof(name) === "string");
  assert(typeof(address) === "string");
  assert(typeof(time) === "number");
  this.name = name;
  this.address = address;
  this.bootstrap_rules = bootstrap_rules;
  this.rules = rules;
  this.time = time;
  this.collections = collections;
  this.clicked_hash = clicked_hash;
}

// name: string,
// type: string,
// column_names: string list,
// tuples: string list list,
fluent.Collection = function(name, type, column_names, black_box_lineage, tuples) {
  assert(typeof(name) === "string");
  assert(typeof(type) === "string");
  assert(typeof(black_box_lineage) === "boolean");
  this.name = name;
  this.type = type;
  this.column_names = column_names;
  this.black_box_lineage = black_box_lineage;
  this.tuples = tuples;
}

// node_name: string
// collection_name: string
// hash: string
// time: number
fluent.TupleId = function(node_name, collection_name, hash, time) {
  assert(typeof(node_name) === "string", node_name);
  assert(typeof(collection_name) === "string", collection_name);
  assert(typeof(hash) === "string", hash);
  assert(typeof(time) === "number", time);
  this.node_name = node_name;
  this.collection_name = collection_name;
  this.hash = hash;
  this.time = time;
}

fluent.tuple_id_to_string = function(tid) {
  return [tid.node_name, tid.collection_name, tid.hash, tid.time].join("_");
}

// AJAX Endpoints //////////////////////////////////////////////////////////////
fluent.ajax = {};

// nodes: unit -> [string, string] list
fluent.ajax.nodes = function(callback) {
  fluent.ajax_get("/nodes", function(result) {
    callback(result);
  });
}

// node_address: string -> string
fluent.ajax.node_address = function(node_name, callback) {
  var url = "/node_address?node_name=" + node_name;
  fluent.ajax_get(url, callback);
}

// node_bootstrap_rules: string -> string list
fluent.ajax.node_bootstrap_rules = function(node_name, callback) {
  var url = "/node_bootstrap_rules?node_name=" + node_name;
  fluent.ajax_get(url, callback);
}

// node_rules: string -> string list
fluent.ajax.node_rules = function(node_name, callback) {
  var url = "/node_rules?node_name=" + node_name;
  fluent.ajax_get(url, callback);
}

// node_collection_names: string -> string list
fluent.ajax.node_collection_names = function(node_name, callback) {
  var url = "/node_collection_names?node_name=" + node_name;
  fluent.ajax_get(url, callback);
}

// node_collection: string -> string -> int -> {
//   type: string,
//   column_names: string list,
//   tuples: string list list,
// }
fluent.ajax.node_collection = function(node_name, collection_name, time, callback) {
  var url = "/node_collection" +
    "?node_name=" + node_name +
    "&collection_name=" + collection_name +
    "&time=" + time;
  fluent.ajax_get(url, callback);
}

// black_box_backwards_lineage: string -> string -> int -> TupleId list
fluent.ajax.black_box_backwards_lineage = function(node_name, collection_name,
                                                   id, callback) {
  var url = "/black_box_backwards_lineage" +
    "?node_name=" + node_name +
    "&collection_name=" + collection_name +
    "&id=" + id;
  fluent.ajax_get(url, callback);
}

// regular_backwards_lineage: string -> string -> int -> int -> TupleId list
fluent.ajax.regular_backwards_lineage = function(node_name, collection_name,
                                                 hash, time, callback) {
  var url = "/regular_backwards_lineage" +
    "?node_name=" + node_name +
    "&collection_name=" + collection_name +
    "&hash=" + hash +
    "&time=" + time;
  fluent.ajax_get(url, callback);
}

// Callbacks ///////////////////////////////////////////////////////////////////
fluent.get_collections = function(node_name, collection_names, time, callback) {
  var i = 0;
  var collections = [];
  var get_collections_impl = function() {
    if (i == collection_names.length) {
      callback(collections);
    } else {
      var cname = collection_names[i];
      i += 1;
      fluent.ajax.node_collection(node_name, cname, time, function(collection) {
        collections.push(new fluent.Collection(
            cname, collection.type, collection.column_names,
            collection.black_box_lineage, collection.tuples));
        get_collections_impl();
      });
    }
  }
  get_collections_impl();
}

fluent.select_node = function(name, time, callback) {
  var that = this;
  fluent.ajax.node_address(name, function(address) {
  fluent.ajax.node_bootstrap_rules(name, function(bootstrap_rules) {
  fluent.ajax.node_rules(name, function(rules) {
  fluent.ajax.node_collection_names(name, function(collection_names) {
    var node = new fluent.Node(name, address, bootstrap_rules, rules, time, [],
                               null);
    fluent.get_collections(name, collection_names, time, function(collections) {
      node.collections = collections;
      that.node = node;
      if (callback) {
        callback();
      }
    });
  });
  });
  });
  });
}

fluent.refresh_collections = function(node) {
  assert(node !== null);
  var collection_names = [];
  for (var i in node.collections) {
    collection_names.push(node.collections[i].name);
  }
  var callback = function(collections) { node.collections = collections; }
  fluent.get_collections(node.name, collection_names, node.time, callback);
}

fluent.decrement_time = function() {
  if (this.node !== null) {
    this.node.time = Math.max(0, this.node.time - 1);
    fluent.refresh_collections(this.node);
  }
}

fluent.increment_time = function() {
  if (this.node !== null) {
    this.node.time += 1;
    fluent.refresh_collections(this.node);
  }
}

fluent.render_current_rule = function(node) {
  assert(node !== null);

  var time = node.time;
  var rules = document.getElementById("rules");
  var lis = rules.getElementsByTagName("li");
  for (var i = 0; i < lis.length; ++i) {
    lis[i].classList.remove("current_rule");
  }

  if (node.bootstrap_rules.length == 0) {
    var num_head = 0;
  } else {
    var num_head = node.bootstrap_rules.length + 1;
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

fluent.add_node = function(tid, tuple) {
  var id = fluent.tuple_id_to_string(tid);
  if (this.cy.getElementById(id).size() == 0) {
    this.cy.add({
      group: "nodes",
      data: {
        id: fluent.tuple_id_to_string(tid),
        node_name: tid.node_name,
        collection_name: tid.collection_name,
        hash: tid.hash,
        time: tid.time,
        tuple: "(" + fluent.strip_header(tuple).join(",") + ")",
      }
    });
  } else {
    // The node already exists, so we do nothing.
  }
}

fluent.add_edge = function(source_tid, target_tid) {
  var source_id = fluent.tuple_id_to_string(source_tid);
  var target_id = fluent.tuple_id_to_string(target_tid);
  var selector = "edge[source='" + source_id + "'][target='" + target_id + "']";
  if (this.cy.edges(selector).size() == 0) {
    this.cy.add({
      group: "edges",
      data: {
        source: source_id,
        target: target_id,
      },
    });
  } else {
    // The edge already exists, so we do nothing.
  }
}

fluent.backwards_lineage = function(node, collection, tuple) {
  var hash = tuple[0];
  var time = tuple[1];
  var target_tid = new fluent.TupleId(node.name, collection.name, hash, time);
  fluent.add_node.call(this, target_tid, tuple);
  this.node.clicked_hash = hash;

  var that = this;
  var callback = function(lineage_tuples) {
    for (var i = 0; i < lineage_tuples.length; ++i) {
      var t = lineage_tuples[i];
      var source_tid = new fluent.TupleId(
          t.node_name, t.collection_name, t.hash, t.time);
      fluent.add_node.call(that, source_tid, t.tuple);
      fluent.add_edge.call(that, source_tid, target_tid);
    }
    that.cy.layout({
      name: "dagre",
      rankDir: "LR",
      animate: true,
      fit: false,
    }).run();
  }

  if (collection.black_box_lineage) {
    // The tuple looks like this:
    //   0. hash
    //   1. logical time inserted
    //   2. logical time deleted
    //   3. physical time inserted
    //   4. physical time deleted
    //   5. address
    //   6. id
    fluent.ajax.black_box_backwards_lineage(node.name, collection.name,
                                            tuple[6], callback);
  } else {
    fluent.ajax.regular_backwards_lineage(node.name, collection.name, hash,
                                          time, callback);
  }
}

// Main ////////////////////////////////////////////////////////////////////////
function main() {
  // Create the Vue!
  var vm = new Vue({
    el: "#container",
    data: new fluent.State([], null, null),
    methods: {
      select_node: fluent.select_node,
      decrement_time: fluent.decrement_time,
      increment_time: fluent.increment_time,
      backwards_lineage: fluent.backwards_lineage,
    },
    updated: function() {
      if (this.node !== null) {
        fluent.render_current_rule(this.node);
      }
    }
  });

  // Create the cytoscape graph.
  vm.cy = cytoscape({
    container: document.getElementById("cy"),
    style: [
      {
        selector: 'node',
        style: {
          "shape": "rectangle",
          "font-family": "monospace",
          "width": "label",
          "height": "label",
          "text-wrap": "wrap",
          "text-halign": "center",
          "text-valign": "center",
          "border-color": "black",
          "border-width": "1",
          "padding": "5",
          "background-color": "white",
          'label': function(e) {
            return e.data("node_name") + ":" + e.data("collection_name")
                   + "@" + e.data("time") + "\n" + e.data("tuple");
          }
        }
      },
      {
        selector: 'edge',
        style: {
          "width": 3,
          "line-color": "#ccc",
          "target-arrow-color": "#ccc",
          "target-arrow-shape": "triangle",
          "target-arrow-fill": 'filled',
          "curve-style": "bezier",
          "label": "data(rule)",
        }
      }
    ],
  });

  vm.cy.on('tap', 'node', function(evt) {
    var node = evt.target;
    var node_name = node.data("node_name");
    var time = node.data("time");
    var hash = node.data("hash");
    fluent.select_node.call(vm, node_name, time, function() {
      vm.node.clicked_hash = hash;
    });
  });

  // Bind left and right keys.
  document.onkeydown = function(e) {
    if (e.key === "ArrowLeft") {
      fluent.decrement_time.call(vm, vm.node);
    } else if (e.key === "ArrowRight")  {
      fluent.increment_time.call(vm, vm.node);
    }
  }

  // Populate the list of nodes.
  fluent.ajax.nodes(function(name_addresses) {
    for (var i in name_addresses) {
      name = name_addresses[i][0];
      address = name_addresses[i][1];
      vm.node_name_addresses.push(new fluent.NodeNameAddress(name, address));
    }
  });
}

window.onload = main
