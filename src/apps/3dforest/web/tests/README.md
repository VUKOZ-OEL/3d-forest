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

The current cases verify static file serving:

- `TestWebServerServesIndexFile` checks that `/` returns the document root's `index.html`.
- `TestWebServerReturnsNotFound` checks that a missing file returns HTTP 404.

Both test cases use the same Drogon server instance. The fixture creates a
temporary document root and starts the server when first used; the test runner
stops it after all test cases complete. The protocol sections above describe
additional cases that can be added, including crafted HTTP requests and
WebSocket frames over a raw TCP client.

The tests are included in the `3DForestTest` executable when configured with `BUILD_TESTS=ON`. To keep build products and test-generated files out of the source tree, configure and build in a directory under `slop`, then run `3DForestTest` with `slop/webservertests` as its working directory.
