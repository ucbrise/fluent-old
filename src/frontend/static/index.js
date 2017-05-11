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
fluent.State = function(node_name_addresses, node) {
  this.node_name_addresses = node_name_addresses;
  this.node = node;
}

// name: string
// address: string
fluent.NodeNameAddress = function(name, address) {
  assert(typeof(name) === "string");
  assert(typeof(address) === "string");
  this.name = name;
  this.address = address;
}

// name: string,
// address: string,
// bootstrap_rules: string list,
// rules: string list,
// time: number,
// collections: Collection list,
fluent.Node = function(name, address, bootstrap_rules, rules, time,
                       collections) {
  assert(typeof(name) === "string");
  assert(typeof(address) === "string");
  assert(typeof(time) === "number");
  this.name = name;
  this.address = address;
  this.bootstrap_rules = bootstrap_rules;
  this.rules = rules;
  this.time = time;
  this.collections = collections;
}

// name: string,
// type: string,
// column_names: string list,
// tuples: string list list,
fluent.Collection = function(name, type, column_names, tuples) {
  assert(typeof(name) === "string");
  assert(typeof(type) === "string");
  this.name = name;
  this.type = type;
  this.column_names = column_names;
  this.tuples = tuples;
}

// node_name: string
// collection_name: string
// hash: string
// time: number
fluent.TupleId = function(node_name, collection_name, hash, time) {
  assert(typeof(node_name) === "string");
  assert(typeof(collection_name) === "string");
  assert(typeof(hash) === "string");
  assert(typeof(time) === "number");
  this.node_name = node_name;
  this.collection_name = collection_name;
  this.hash = hash;
  this.time = time;
}

// AJAX Endpoints //////////////////////////////////////////////////////////////
fluent.ajax = {};

// nodes: unit -> [string, string] list
fluent.ajax.nodes = function(callback) {
  fluent.ajax_get("/nodes", function(result) {
    callback(result);
  });
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

// backwards_lineage: string -> string -> int -> int -> TupleId list
fluent.ajax.backwards_lineage = function(node_name, collection_name, hash,
                                         time, callback) {
  var url = "/backwards_lineage" +
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
            collection.tuples));
        get_collections_impl();
      });
    }
  }
  get_collections_impl();
}

fluent.select_node = function(name, address) {
  var that = this;
  fluent.ajax.node_bootstrap_rules(name, function(bootstrap_rules) {
  fluent.ajax.node_rules(name, function(rules) {
  fluent.ajax.node_collection_names(name, function(collection_names) {
    var node = new fluent.Node(name, address, bootstrap_rules, rules, 0, []);
    fluent.get_collections(name, collection_names, 0, function(collections) {
      node.collections = collections;
      that.node = node;
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

// Main ////////////////////////////////////////////////////////////////////////
function main() {
  // Create the Vue!
  var vm = new Vue({
    el: "#container",
    data: new fluent.State([], null),
    methods: {
      select_node: fluent.select_node,
      decrement_time: fluent.decrement_time,
      increment_time: fluent.increment_time,
      get_lineage: function(node_name, collection_name, hash, time) {
        // TODO(mwhittaker): Move this into a proper function and use it to
        // populate a lineage graph.
        console.log("node_name: " + node_name);
        console.log("collection_name: " + collection_name);
        console.log("hash: " + hash);
        console.log("time: " + time);
        fluent.ajax.backwards_lineage(node_name, collection_name, hash, time,
                                      function(ts) {
          for (var i = 0; i < ts.length; ++i) {
            console.log(ts[i]);
          }
        });
      }
    },
    updated: function() {
      if (this.node !== null) {
        fluent.render_current_rule(this.node);
      }
    }
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