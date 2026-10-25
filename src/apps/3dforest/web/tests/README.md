# Web server tests

This directory contains tests of the web server's client-facing protocol. The
server accepts TCP connections and Drogon parses HTTP and WebSocket framing;
clients may be the shipped browser UI or any program that speaks those
protocols. A hand-crafted TCP client can exercise malformed and unusual wire
inputs, but arbitrary TCP bytes are not application messages: they must first
form valid HTTP requests or WebSocket frames to reach the corresponding
handlers.

## Client-facing surface

### Static HTTP files

The document root serves the web client files, including `/` (the configured
`index.html`) and its JavaScript, CSS, and other assets. These requests do not
create a UI session. Drogon handles path resolution and static-file methods.
Clients can vary the request method, path and query, headers, and HTTP framing;
unknown files and unsupported methods exercise the static-file error paths.

### UI WebSocket at `/ui`

The browser connects to `/ui` using `ws:` or `wss:` according to the page
scheme. The upgrade request must carry an `Origin` exactly matching the
server's configured `--allowed-origin`. A connection that does not send the
first protocol message within 10 seconds is closed. The server sends WebSocket
ping messages periodically; clients can also use standard ping, pong, and close
frames.

After upgrade, the client protocol is:

1. Send a UTF-8 text JSON object `{"type":"hello","token":"..."}`. An empty
   token starts a session; a previously issued token reconnects to that
   session. The token may be at most 64 characters. The server responds with
   `welcome` and a token, or `unavailable` / `expired` and closes the socket.
2. Receive a `snapshot` or `patch` with a revision, then acknowledge it with
   `{"type":"ack","revision":N}`. A valid acknowledgement must match the
   revision currently awaiting acknowledgement.
3. Send `{"type":"resync"}` to request a fresh snapshot, or
   `{"type":"event","id":"wN","event":"...",...}` to exercise a
   registered widget, dialog, or action. Event names, fields, types, and value
   bounds are checked by the relevant UI adapter. The shipped client emits
   control events such as `triggered`, dialog events such as `button`, and
   viewer events such as `viewerCamera`, `viewerPick`, and `viewerPull`.

The server accepts text messages up to 1 MiB. Malformed JSON, duplicate JSON
keys, extra JSON data, unknown message types, invalid acknowledgements, and
binary messages on `/ui` are rejected by closing the connection or returning
an `error` where the session can recover. Application messages use JSON text;
viewer point data travels in the opposite direction as WebSocket binary frames.
The browser validates the 64-byte viewer header, version, stream/request IDs,
point count, and 16-byte point records. A client can send crafted `event`
messages (including `viewerPull` fields) over text, but binary viewer packets
are not a client-to-server message type.

### Marteloscope session proxy

`/marteloscope/{token}/...` is scoped to an existing session token.

- Ordinary HTTP requests to this path are proxied to that session's local
  service. The method, suffix path, query, body, and most headers are forwarded;
  hop-by-hop headers and the incoming `Host` and `Origin` are replaced or
  removed. Response cookies are scoped to the token path. A missing/expired
  session returns 404, a mismatching nonempty `Origin` returns 403, and an
  unavailable local service returns 502. Since this route forwards methods and
  bodies, a raw HTTP client can exercise methods and payloads the shipped UI
  does not generate.
- A WebSocket upgrade on the same path is proxied to the session's local
  service. It requires the exact allowed `Origin` and a live token. The
  WebSocket path, query, cookie, and offered subprotocol are forwarded, and
  text/binary messages are bridged between browser and local service.

The browser opens the service URL supplied by the application; it does not
invent a session token or local service port. Direct requests to these proxy
paths can therefore test invalid tokens, invalid suffixes, origin handling,
and proxy behavior without using the UI.

## Current tests

`TestWebServer.cpp` currently has 22 cases using one real Drogon server
instance. The fixture creates a temporary document root, starts the server on
loopback, and stops it after the test suite. The valid `/ui` cases use a
test-only fake plugin with its own controls and viewer. It does not create
navigation entries; the production `WebUiTree` still builds and serializes the
test UI.

The cases cover:

- Serving the index file, returning 404 for a missing file, and preventing
  access outside the document root using plain and encoded traversal paths.
- Marteloscope HTTP origin checks, including missing and mismatched origins,
  unknown sessions, and malformed paths.
- `/ui` and Marteloscope WebSocket origin checks, including missing origins,
  wrong origins, unknown sessions, binary `/ui` messages, malformed JSON,
  duplicate keys, extra data, overlong tokens, and deeply nested JSON.
- A valid session welcome and snapshot, serialization of every supported
  `WebControls` type, and an empty sidebar navigation tree.
- Widget event handling: a checkbox event produces a patch, an out-of-range
  slider event produces an error, invalid state acknowledgements produce an
  error, and `resync` returns a newer snapshot. Reconnecting with a valid token
  reattaches to its session; an unknown token returns `expired`.
- Startup rejection for a missing document root and invalid thread, session,
  origin, and reconnect-timeout options.

The suite does not yet exercise a valid session reconnect, every control's
event variants, viewer binary pulls, or forwarding to a live Marteloscope
service. Its WebSocket tests use Drogon's client, so malformed HTTP/WebSocket
framing that Drogon rejects before invoking the application handlers is not
covered by these tests.

The tests are included in `3DForestTest` when configured with
`BUILD_TESTS=ON`. Keep generated build files under `slop`. For the existing
Clang coverage build, run from the repository root:

```powershell
cmake --build slop/webservertests/build-clang-coverage --target 3DForestTest
```

Run the executable with `slop/webservertests` as its working directory, for
example:

```powershell
Set-Location slop/webservertests
.\build-clang-coverage\src\apps\tests\3DForestTest.exe
```
