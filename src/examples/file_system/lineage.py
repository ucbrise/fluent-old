def overlap((a, b), (x, y)):
    return not (y < a or x > b)

def union((a, b), (x, y)):
    assert overlap((a, b), (x, y)) or (b + 1 == x) or (y + 1 == a)
    return (min(a, x), max(b, y))

def intersect((a, b), (x, y)):
    assert overlap((a, b), (x, y))
    return (max(a, x), min(b, y))

def subsumes((a, b), (x, y)):
    return a <= x and b >= y

class DisjointRanges(object):
    def __init__(self):
        self.ranges = []

    def condense(self):
        """Combine consecutive ranges."""
        ranges = self.ranges[0:1]
        for (a, b) in self.ranges[1:]:
            (x, y) = ranges[-1]
            if y + 1 == a:
                ranges[-1] = (x, b)
            else:
                ranges.append((a, b))
        self.ranges = ranges

    def union(self, r):
        enumerated = enumerate(self.ranges)
        overlaps = [(i, rng) for (i, rng) in enumerated if overlap(rng, r)]

        subsumed = False
        if len(overlaps) == 0:
            self.ranges.append(r)
            self.ranges.sort(key=lambda (a, b): a)
        elif len(overlaps) == 1 and subsumes(overlaps[0][1], r):
            subsumed = True
        else:
            for (i, rng) in overlaps:
                r = union(rng, r)
            (low, _) = overlaps[0]
            (high, _) = overlaps[-1]
            self.ranges[low:high + 1] = [r]

        self.condense()
        return subsumed

    def __str__(self):
        return str(self.ranges)

    def __repr__(self):
        return str(self.ranges)

def read_lineage(cur, id_):
    cur.execute("""
        SELECT time_inserted, start, stop
        FROM file_system_server_read_request
        WHERE id = %s;
    """, (id_, ))
    rows = cur.fetchall()
    assert len(rows) == 1
    row = rows[0]
    (time, a, b) = row
    b = b - 1

    cur.execute("""
        SELECT hash, time_inserted, start, data
        FROM file_system_server_write_request
        WHERE time_inserted < %s
        ORDER BY time_inserted DESC;
    """, (time,))
    lineage = []
    ranges = DisjointRanges()
    for (hash_, time, start, data) in cur.fetchall():
        (x, y) = (start, start + len(data) - 1)
        if overlap((a, b), (x, y)):
            (x, y) = intersect((x, y), (a, b))
            subsumed = ranges.union((x, y))
            if not subsumed:
                id_ = ("file_system_server", "write_request", hash_, time)
                lineage.append(id_)

        if ranges.ranges == [(a, b)]:
            return lineage

    return lineage
