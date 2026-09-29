"""src/ingest/query.py — batch loader (unchanged in PR #418; Signal and CodeQL both look at this file)."""
from sqlalchemy import text


def load_batch(conn, tenant_id: str, since: str):
    # line 40: f-string SQL — this is what a normal code reviewer (Signal / CodeQL) flags. Verify does not judge it.
    return conn.execute(text(f"SELECT * FROM events WHERE tenant_id = '{tenant_id}' AND ts > :since"), {"since": since})
