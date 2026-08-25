"""Throwaway localhost HTTP and WebSocket servers for the Network driver tests (spec 0068).

Both servers bind an OS-assigned port, run on their own thread, and expose enough
bookkeeping for a test to assert what the driver actually sent: request count, method,
path, headers and body for HTTP; received messages for WebSocket.

The HTTP side is stdlib only. The WebSocket side needs the ``websockets`` package
(``pip install -r tests/requirements.txt``); importing this module without it works,
but constructing WebSocketEchoServer raises a clear error.

Typical use::

    with HttpTelemetryServer(payload=b'{"t":1}') as server:
        # point Serial Studio at server.url and connect
        ...
        assert server.request_count >= 1

    with WebSocketEchoServer() as server:
        # point Serial Studio at server.url
        server.broadcast("temp,21.5")
"""

from __future__ import annotations

import asyncio
import json
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any, Optional

try:  # pragma: no cover - exercised only when the optional dep is missing
    import websockets
    from websockets.asyncio.server import serve as websockets_serve

    _WEBSOCKETS_IMPORT_ERROR: Optional[BaseException] = None
except Exception as exc:  # pragma: no cover
    websockets = None  # type: ignore[assignment]
    websockets_serve = None  # type: ignore[assignment]
    _WEBSOCKETS_IMPORT_ERROR = exc


class _RecordingHandler(BaseHTTPRequestHandler):
    """Answers every verb with the server's canned payload and records the request."""

    protocol_version = "HTTP/1.1"

    def log_message(self, fmt: str, *args: Any) -> None:  # noqa: A003
        pass

    def _record_and_reply(self) -> None:
        length = int(self.headers.get("Content-Length") or 0)
        body = self.rfile.read(length) if length else b""

        owner = self.server.owner  # type: ignore[attr-defined]
        with owner.lock:
            owner.requests.append(
                {
                    "method": self.command,
                    "path": self.path,
                    "headers": {k.lower(): v for k, v in self.headers.items()},
                    "body": body,
                }
            )
            status = owner.status
            payload = owner.payload

        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        if payload:
            self.wfile.write(payload)

    do_GET = _record_and_reply
    do_POST = _record_and_reply
    do_PUT = _record_and_reply
    do_PATCH = _record_and_reply
    do_DELETE = _record_and_reply


class HttpTelemetryServer:
    """A localhost HTTP endpoint that returns a fixed body and logs what it was sent."""

    def __init__(self, payload: bytes = b'{"value":1}', status: int = 200) -> None:
        self.payload = payload
        self.status = status
        self.requests: list[dict[str, Any]] = []
        self.lock = threading.Lock()
        self._server: Optional[ThreadingHTTPServer] = None
        self._thread: Optional[threading.Thread] = None

    @property
    def port(self) -> int:
        assert self._server is not None, "server is not running"
        return self._server.server_address[1]

    @property
    def url(self) -> str:
        return f"http://127.0.0.1:{self.port}/telemetry"

    @property
    def request_count(self) -> int:
        with self.lock:
            return len(self.requests)

    def set_payload(self, payload: bytes, status: int = 200) -> None:
        with self.lock:
            self.payload = payload
            self.status = status

    def reset(self) -> None:
        with self.lock:
            self.requests.clear()

    def start(self) -> "HttpTelemetryServer":
        self._server = ThreadingHTTPServer(("127.0.0.1", 0), _RecordingHandler)
        self._server.owner = self  # type: ignore[attr-defined]
        self._thread = threading.Thread(target=self._server.serve_forever, daemon=True)
        self._thread.start()
        return self

    def stop(self) -> None:
        if self._server is not None:
            self._server.shutdown()
            self._server.server_close()
            self._server = None

        if self._thread is not None:
            self._thread.join(timeout=5)
            self._thread = None

    def __enter__(self) -> "HttpTelemetryServer":
        return self.start()

    def __exit__(self, *_exc: Any) -> None:
        self.stop()


class WebSocketEchoServer:
    """A localhost WebSocket endpoint that echoes what it receives and can push frames."""

    def __init__(self, echo: bool = True) -> None:
        if websockets_serve is None:
            raise RuntimeError(
                "the 'websockets' package is required for WebSocketEchoServer; "
                "run: pip install -r tests/requirements.txt"
            ) from _WEBSOCKETS_IMPORT_ERROR

        self.echo = echo
        self.received: list[Any] = []
        self.lock = threading.Lock()
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._thread: Optional[threading.Thread] = None
        self._shutdown: Optional[asyncio.Event] = None
        self._clients: set[Any] = set()
        self._port = 0
        self._ready = threading.Event()

    @property
    def port(self) -> int:
        return self._port

    @property
    def url(self) -> str:
        return f"ws://127.0.0.1:{self._port}/feed"

    @property
    def message_count(self) -> int:
        with self.lock:
            return len(self.received)

    def reset(self) -> None:
        with self.lock:
            self.received.clear()

    async def _handler(self, connection: Any) -> None:
        self._clients.add(connection)
        try:
            async for message in connection:
                with self.lock:
                    self.received.append(message)

                if self.echo:
                    await connection.send(message)
        except Exception:
            pass
        finally:
            self._clients.discard(connection)

    def _run(self) -> None:
        async def main() -> None:
            self._shutdown = asyncio.Event()
            async with websockets_serve(self._handler, "127.0.0.1", 0) as server:
                self._port = next(iter(server.sockets)).getsockname()[1]
                self._ready.set()
                await self._shutdown.wait()

        loop = asyncio.new_event_loop()
        self._loop = loop
        asyncio.set_event_loop(loop)
        try:
            loop.run_until_complete(main())
        finally:
            self._loop = None
            loop.close()

    def broadcast(self, message: Any) -> None:
        """Push one message to every connected client, from any thread."""
        if self._loop is None:
            return

        if isinstance(message, (dict, list)):
            message = json.dumps(message)

        async def send_all() -> None:
            for connection in list(self._clients):
                try:
                    await connection.send(message)
                except Exception:
                    pass

        asyncio.run_coroutine_threadsafe(send_all(), self._loop).result(timeout=5)

    def start(self, timeout: float = 5.0) -> "WebSocketEchoServer":
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()
        if not self._ready.wait(timeout):
            raise RuntimeError("WebSocket test server did not start in time")

        return self

    def stop(self) -> None:
        loop = self._loop
        if loop is not None and self._shutdown is not None:
            loop.call_soon_threadsafe(self._shutdown.set)

        if self._thread is not None:
            self._thread.join(timeout=5)
            self._thread = None

    def __enter__(self) -> "WebSocketEchoServer":
        return self.start()

    def __exit__(self, *_exc: Any) -> None:
        self.stop()
