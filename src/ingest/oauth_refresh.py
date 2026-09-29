"""OAuth2 token refresh for the ingest client (src/ingest/oauth_refresh.py)."""
import threading
import time
from dataclasses import dataclass, field
from typing import Callable, Optional

from requests_oauthlib import OAuth2Session


@dataclass
class TokenState:
    access_token: str
    refresh_token: Optional[str]
    expires_at: float
    scope: tuple[str, ...] = field(default_factory=tuple)

    def remaining(self) -> float:
        return self.expires_at - time.time()


class RefreshingClient:
    """Wraps OAuth2Session so callers never see an expired bearer token."""

    def __init__(self, client_id: str, token_url: str, initial: TokenState,
                 on_rotate: Optional[Callable[[TokenState], None]] = None, skew: float = 30.0):
        self._client_id = client_id
        self._token_url = token_url
        self._state = initial
        self._on_rotate = on_rotate
        self._skew = skew
        self._lock = threading.Lock()
        self._session = OAuth2Session(client_id, token={"access_token": initial.access_token,
                                                        "refresh_token": initial.refresh_token,
                                                        "expires_at": initial.expires_at})

    def _needs_refresh(self) -> bool:
        return self._state.remaining() <= self._skew

    def _refresh(self) -> None:
        tok = self._session.refresh_token(self._token_url, client_id=self._client_id)
        self._state = TokenState(tok["access_token"], tok.get("refresh_token"), float(tok["expires_at"]))
        if self._on_rotate:
            self._on_rotate(self._state)

    def get(self, url: str, **kw):
        with self._lock:
            if self._needs_refresh():
                self._refresh()
        return self._session.get(url, timeout=kw.pop("timeout", 10), **kw)
