import psycopg2

################################################################################
# Helpers
################################################################################
def fetchonly(cur):
    rows = cur.fetchall()
    assert len(rows) == 1, rows
    return rows[0]

def file_creation_time(cur, bucket, key, time_inserted_upper_bound):
    # Find the corresponding echo, if one exists.
    query = """SELECT time_inserted
               FROM s3_server_echo_request
               WHERE bucket = %s AND key = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, key, time_inserted_upper_bound))
    echo_rows = cur.fetchall()

    # Find the corresponding cp, if one exists.
    query = """SELECT time_inserted
               FROM s3_server_cp_request
               WHERE dst_bucket = %s AND dst_key = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, key, time_inserted_upper_bound))
    cp_rows = cur.fetchall()

    num_rows = (len(echo_rows), len(cp_rows))
    assert num_rows in [(0, 1), (1, 0), (1, 1)], (echo_rows, cp_rows)
    if num_rows == (1, 0):
        return echo_rows[0][0]
    elif num_rows == (0, 1):
        return cp_rows[0][0]
    else:
        assert num_rows == (1, 1), (echo_rows, cp_rows)
        return max(echo_rows[0][0], cp_rows[0][0])

def file_lineage(cur, bucket, key, time_inserted):
    # Find the corresponding mb.
    query = """SELECT hash, time_inserted
               FROM s3_server_mb_request
               WHERE bucket = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, time_inserted))
    mb_hash, mb_time_inserted = fetchonly(cur)
    lineage = {("s3_server", "mb_request", mb_hash, mb_time_inserted)}

    # Find the corresponding echo, if one exists.
    query = """SELECT hash, time_inserted
               FROM s3_server_echo_request
               WHERE bucket = %s AND key = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, key, time_inserted))
    echo_rows = cur.fetchall()

    # Find the corresponding cp, if one exists.
    query = """SELECT hash, time_inserted, src_bucket, src_key
               FROM s3_server_cp_request
               WHERE dst_bucket = %s AND dst_key = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, key, time_inserted))
    cp_rows = cur.fetchall()

    # Remove the older of the two lineages.
    num_rows = (len(echo_rows), len(cp_rows))
    assert num_rows in [(0, 1), (1, 0), (1, 1)], (echo_rows, cp_rows)
    if num_rows == (1, 1):
        echo_time = echo_rows[0][1]
        cp_time = cp_rows[0][1]

        assert echo_time != cp_time, (echo_time, cp_time)
        if echo_time > cp_rows:
            cp_rows = []
        else:
            echo_rows = []

    if num_rows == (1, 0):
        echo_hash, echo_time_inserted = echo_rows[0]
        t = ("s3_server", "echo_request", echo_hash, echo_time_inserted)
        return lineage | {t}
    else:
        assert num_rows == (0, 1)
        cp_hash, cp_time_inserted, src_bucket, src_key = cp_rows[0]
        t = ("s3_server", "cp_request", cp_hash, cp_time_inserted)
        src_time = file_creation_time(cur, src_bucket, src_key, cp_time_inserted)
        return lineage | {t} | file_lineage(cur, src_bucket, src_key, src_time)

################################################################################
# Lineage
################################################################################
def rb_lineage(cur, id_):
    query = """SELECT hash, time_inserted, bucket
               FROM s3_server_rb_request
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    hash, time_inserted, bucket = fetchonly(cur)

    query = """SELECT hash, time_inserted
               FROM s3_server_mb_request
               WHERE bucket = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, time_inserted))
    mb_hash, mb_time_inserted = fetchonly(cur)

    return [("s3_server", "rb_request", hash, time_inserted),
            ("s3_server", "mb_request", mb_hash, mb_time_inserted)]

def echo_lineage(cur, id_):
    query = """SELECT hash, time_inserted, bucket
               FROM s3_server_echo_request
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    hash, time_inserted, bucket = fetchonly(cur)

    query = """SELECT hash, time_inserted
               FROM s3_server_mb_request
               WHERE bucket = %s AND time_inserted <= %s
               ORDER BY time_inserted DESC
               LIMIT 1;"""
    cur.execute(query, (bucket, time_inserted))
    mb_hash, mb_time_inserted = fetchonly(cur)

    return [("s3_server", "echo_request", hash, time_inserted),
            ("s3_server", "mb_request", mb_hash, mb_time_inserted)]

def rm_lineage(cur, id_):
    query = """SELECT bucket, key, hash, time_inserted
               FROM s3_server_rm_request
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    bucket, key, hash, time_inserted = fetchonly(cur)
    creation_time = file_creation_time(cur, bucket, key, time_inserted)
    return ([("s3_server", "rm_request", hash, time_inserted)] +
            list(file_lineage(cur, bucket, key, creation_time)))

def ls_lineage(cur, id_):
    query = """SELECT bucket, hash, time_inserted
               FROM s3_server_ls_request
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    bucket, hash, time_inserted = fetchonly(cur)

    query = """SELECT keys
               FROM s3_server_ls_response
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    keys, = fetchonly(cur)

    lineage = [("s3_server", "ls_request", hash, time_inserted)]
    for key in keys:
        creation_time = file_creation_time(cur, bucket, key, time_inserted)
        lineage += file_lineage(cur, bucket, key, creation_time)
    return lineage

def cat_lineage(cur, id_):
    query = """SELECT bucket, key, hash, time_inserted
               FROM s3_server_cat_request
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    bucket, key, hash, time_inserted = fetchonly(cur)
    creation_time = file_creation_time(cur, bucket, key, time_inserted)
    return ([("s3_server", "cat_request", hash, time_inserted)] +
            list(file_lineage(cur, bucket, key, creation_time)))

def cp_lineage(cur, id_):
    query = """SELECT src_bucket, src_key, hash, time_inserted
               FROM s3_server_cp_request
               WHERE id = %s;"""
    cur.execute(query, (id_,))
    bucket, key, hash, time_inserted = fetchonly(cur)
    creation_time = file_creation_time(cur, bucket, key, time_inserted)
    return ([("s3_server", "cp_request", hash, time_inserted)] +
            list(file_lineage(cur, bucket, key, creation_time)))

################################################################################
# Main
################################################################################
def main():
    db = psycopg2.connect("dbname=vagrant")
    with db.cursor() as cur:
        print rb_lineage(cur, 6902269523070619635)
        print echo_lineage(cur, -7133981648610981754)
        print rm_lineage(cur, 3878151457363549620)
        print rm_lineage(cur, 2163301650378100963)
        print rm_lineage(cur, -1994180347381543506)
        print cp_lineage(cur, -2728919268645347850)
        print cp_lineage(cur, -4713440728425675276)
        print cat_lineage(cur, 303888871040166030)
        print ls_lineage(cur, 525461355874278247)

if __name__ == "__main__":
    main()
