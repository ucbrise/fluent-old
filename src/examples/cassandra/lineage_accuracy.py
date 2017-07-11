import argparse

import psycopg2

def fetchonly(cur):
    ts = cur.fetchall()
    assert len(ts) == 1, ts
    return ts[0]

def get_by_id(cur, (node, collection, id)):
    query = """SELECT *
               FROM {node}_{collection}
               WHERE id={id}"""
    query = query.format(node=node, collection=collection, id=id)
    cur.execute(query)
    return fetchonly(cur)

def get_by_tid(cur, (node, collection, hash, time)):
    query = """SELECT *
               FROM {node}_{collection}
               WHERE hash={hash} AND time_inserted={time}"""
    query = query.format(node=node, collection=collection, hash=hash, time=time)
    cur.execute(query)
    return fetchonly(cur)

def lineage_accuracy(cur, server, debug_file, data_file):
    query = """SELECT *
               FROM {server}_get_response
               WHERE reply_id != -1;"""
    query = query.format(server=server)
    cur.execute(query)
    get_resps = cur.fetchall()

    for get_resp in get_resps:
        (hash, tadd, tdel, ptadd, ptdel, addr, id, value, reply_id) = get_resp
        if reply_id == -1:
            continue

        get_request = get_by_id(cur, (server, "get_request", id))
        key = get_request[-1]
        debug_file.write("get({}) = {} [{:20d}/{:20d}]\n"
                         .format(key, value, id, reply_id, get_resp))

        query = "SELECT * FROM {server}_get_response_lineage({id});"
        query = query.format(server=server, id=id)
        cur.execute(query)
        lineage = cur.fetchall()
        lineage = [t for t in lineage if t[1] == "set_request"]
        lineage_tuples = [get_by_tid(cur, t) for t in lineage]
        lineage_ids = [t[7] for t in lineage_tuples]

        for lt in lineage_tuples:
            (hash, tadd, tdel, ptadd, ptdel, daddr, saddr, set_id, key, value) = lt
            debug_file.write("  [{}] set({}, {}) [{}]\n"
                             .format(ptadd, key, value, set_id))

        assert reply_id in lineage_ids, (reply_id, lineage_ids)
        data_file.write("{}\n".format(lineage_ids.index(reply_id) + 1))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("user", help="lineage database user")
    parser.add_argument("password", help="lineage database password")
    parser.add_argument("dbname", help="lineage database databse name")
    parser.add_argument("server", help="server name (e.g. cassandra_server_1)")
    parser.add_argument("debug_filename", help="filename to write debug info")
    parser.add_argument("data_filename", help="filename to write data")
    args = parser.parse_args()

    conn_string = ""
    conn_string += "dbname={} ".format(args.dbname)
    conn_string += "user={} ".format(args.user)
    conn_string += "password={} ".format(args.password)
    conn = psycopg2.connect(conn_string)

    with conn.cursor() as cur:
        with open(args.debug_filename, "w") as debug_file:
            with open(args.data_filename, "w") as data_file:
                lineage_accuracy(cur, args.server, debug_file, data_file)

if __name__ == "__main__":
    main()
