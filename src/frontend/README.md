# Fluent Frontend

```bash
source bin/activate
FLASK_APP=main.py FLASK_DEBUG=1 flask run --host 0.0.0.0 --port 8000
```

## History TODO
- [x] Show which node is selected.
- [x] Emphasize that nodes can be clicked.
- [x] Handle bootstrap rules.
- [x] Show which rule is being executed
- [x] Delete tuples sent over channel/stdout.
- [x] Show address of nodes.
- [x] Add collection types.

- [ ] Add column names.
- [ ] Add column types.
- [ ] Better initial time.
- [ ] Better lineage for constant tuples.
- [ ] Don't scroll past max time.

## Lineage TODO
- [x] Get basic lineage working.
- [ ] Get deleted lineage working.
- [ ] Step backwards through lineage stack.
- [ ] Show all derivations.
