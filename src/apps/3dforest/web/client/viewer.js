import { ViewerCamera, sub, add, scale, dot, length, cross, normalize } from "./viewer-camera.js";
const MAGIC = 0x33564657,
  HEADER = 64,
  STRIDE = 16,
  MAX_POINTS = 16384,
  MAX_BYTES = 128 * 1024 * 1024;
const nonce = () => crypto.getRandomValues(new Uint32Array(1))[0] || 1;
const vertex = `#version 300 es
precision highp float;
layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
uniform mat3 rotation;
uniform mat4 projection;
uniform vec3 offset;
uniform float distance;
uniform float pointSize;
out vec4 rgb;
out float depth;
out vec3 viewPosition;
void main(){vec3 v=rotation*(position+offset);v.z-=distance;gl_Position=projection*vec4(v,1.0);gl_PointSize=pointSize;rgb=color;depth=-v.z;viewPosition=v;}`;
const fragment = `#version 300 es
precision highp float;
in vec4 rgb;in float depth;in vec3 viewPosition;
uniform bool lighting;
uniform bool fog;
uniform vec2 fogRange;
out vec4 outputColor;
void main(){float fade=fog?clamp((fogRange.y-depth)/max(0.0001,fogRange.y-fogRange.x),0.0,1.0):1.0;float light=lighting?0.2+0.8*abs(dot(normalize(cross(dFdx(viewPosition),dFdy(viewPosition))),normalize(vec3(1,1,1)))):1.0;outputColor=vec4(rgb.rgb*fade*light,rgb.a);}`;
export function createViewer(root, id, emit) {
  const canvas = document.createElement("canvas"),
    overlay = document.createElement("canvas"),
    status = document.createElement("div");
  canvas.className = "viewer-canvas";
  overlay.className = "viewer-overlay";
  status.className = "viewer-status";
  canvas.tabIndex = 0;
  canvas.setAttribute(
    "aria-label",
    "3D point cloud: left drag rotate, right drag pan, wheel zoom",
  );
  root.append(canvas, overlay, status);
  const ctx = overlay.getContext("2d"),
    camera = new ViewerCamera();
  let gl,
    program,
    attributeBuffer,
    uniforms = {},
    chunks = [],
    bytes = 0,
    points = 0,
    props = {},
    scene = {},
    disposed = false,
    lost = false,
    error = false,
    drawFrame = 0,
    pullTimer = 0,
    timeout = 0,
    pending = false,
    stream = nonce(),
    request = 0,
    epoch = -1,
    client = nonce(),
    serial = 0,
    inFlight = 0,
    cameraDirty = false,
    cameraTimer = 0,
    lastCameraSend = 0,
    revision = -1,
    lastRender = -1,
    limited = false;
  function text() {
    status.textContent = error
      ? String(error)
      : `${points.toLocaleString()} points${limited ? " · display limit reached" : ""}${scene.overlayLimited ? " · overlay limit reached" : ""}`;
  }
  function fail(e) {
    error = e.message || String(e);
    text();
    clearTimeout(pullTimer);
    clearTimeout(timeout);
    pending = false;
  }
  function shader(type, source) {
    const s = gl.createShader(type);
    gl.shaderSource(s, source);
    gl.compileShader(s);
    if (!gl.getShaderParameter(s, gl.COMPILE_STATUS)) {
      const message = gl.getShaderInfoLog(s);
      gl.deleteShader(s);
      throw Error(message);
    }
    return s;
  }
  function initialize() {
    gl = canvas.getContext("webgl2", {
      alpha: false,
      antialias: false,
      preserveDrawingBuffer: false,
    });
    if (!gl) throw Error("WebGL 2 is required for the 3D viewer.");
    const vs = shader(gl.VERTEX_SHADER, vertex),
      fs = shader(gl.FRAGMENT_SHADER, fragment);
    program = gl.createProgram();
    gl.attachShader(program, vs);
    gl.attachShader(program, fs);
    gl.linkProgram(program);
    gl.deleteShader(vs);
    gl.deleteShader(fs);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS))
      throw Error(gl.getProgramInfoLog(program));
    attributeBuffer = gl.createBuffer();
    for (const name of [
      "rotation",
      "projection",
      "offset",
      "distance",
      "pointSize",
      "fog",
      "fogRange",
      "lighting",
    ])
      uniforms[name] = gl.getUniformLocation(program, name);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LESS);
    gl.disable(gl.BLEND);
  }
  function clear() {
    if (gl && !lost) for (const c of chunks) gl.deleteBuffer(c.buffer);
    chunks = [];
    bytes = points = 0;
    limited = false;
    root.dataset.loadedPoints = "0";
    root.dataset.gpuBytes = "0";
  }
  function visible() {
    return (
      !disposed &&
      !lost &&
      !error &&
      props.enabled !== false &&
      props.visible !== false &&
      root.isConnected &&
      root.clientWidth > 0 &&
      root.clientHeight > 0 &&
      !root.closest("[inert]") &&
      !document.hidden
    );
  }
  function schedule(ms = 0) {
    clearTimeout(pullTimer);
    if (!disposed) pullTimer = setTimeout(pull, ms);
  }
  function pull() {
    if (pending) return;
    if (!visible()) {
      schedule(250);
      return;
    }
    const next = request + 1;
    const sent = emit(id, "viewerPull", {
      stream,
      request: next,
      ack: request,
      restart: request === 0,
    });
    if (!sent) {
      schedule(150);
      return;
    }
    request = next;
    pending = true;
    timeout = setTimeout(() => {
      // Keep the credit outstanding on a slow link. Repeated restarts here
      // would queue more data before the previous packet had arrived.
      status.textContent = "Waiting for point data…";
    }, 15000);
  }
  function restart() {
    clearTimeout(timeout);
    pending = false;
    stream = nonce();
    request = 0;
    epoch = -1;
    schedule(0);
  }
  function reconnect() {
    inFlight = 0;
    client = nonce();
    serial = 0;
    revision = -1;
    cameraDirty = false;
    restart();
  }
  function sendCamera() {
    if (!cameraDirty || inFlight || !visible()) return;
    const wait = 100 - (performance.now() - lastCameraSend);
    if (wait > 0) {
      clearTimeout(cameraTimer);
      cameraTimer = setTimeout(sendCamera, wait);
      return;
    }
    const next = serial + 1;
    if (
      emit(id, "viewerCamera", {
        camera: camera.state,
        client,
        serial: next,
        revision,
      })
    ) {
      serial = next;
      inFlight = next;
      cameraDirty = false;
      lastCameraSend = performance.now();
      schedule(0);
    } else {
      clearTimeout(cameraTimer);
      cameraTimer = setTimeout(sendCamera, 150);
    }
  }
  function changed() {
    cameraDirty = true;
    sendCamera();
    redraw();
  }
  function update(p) {
    props = p;
    scene = p.scene || scene;
    const c = p.camera;
    if (c) {
      if (c.revision !== revision) {
        camera.set(c);
        revision = c.revision;
        cameraDirty = false;
        inFlight = 0;
        redraw();
      }
      if (c.client === client && c.ack >= inFlight) {
        inFlight = 0;
        sendCamera();
      }
    }
    if (p.renderRevision !== lastRender) {
      lastRender = p.renderRevision;
      if (!pending) schedule(0);
    }
    redraw();
  }
  function receiveBinary(data) {
    if (disposed || lost || gl?.isContextLost()) return;
    try {
      if (!(data instanceof ArrayBuffer) || data.byteLength < HEADER)
        throw Error("Truncated viewer packet");
      const v = new DataView(data);
      if (v.getUint32(0, true) !== MAGIC || v.getUint32(4, true) !== 1)
        throw Error("Unsupported viewer protocol");
      if (v.getUint32(12, true) !== stream) return;
      if (!pending || v.getUint32(16, true) !== request)
        throw Error("Unexpected viewer packet");
      const primitive = v.getUint32(56, true),
        options = v.getUint32(60, true);
      const count = v.getUint32(28, true),
        flags = v.getUint32(24, true),
        nextEpoch = v.getUint32(20, true),
        origin = [0, 1, 2].map((i) => v.getFloat64(32 + i * 8, true));
      if (
        primitive > 2 ||
        options > 1 ||
        count > MAX_POINTS ||
        data.byteLength !== HEADER + count * STRIDE ||
        !origin.every(Number.isFinite)
      )
        throw Error("Invalid viewer packet length");
      if (nextEpoch !== epoch) {
        if (!(flags & 1)) throw Error("Missing viewer frame boundary");
        clear();
        epoch = nextEpoch;
      }
      if (count) {
        const size = count * STRIDE;
        if (bytes + size > Math.min(MAX_BYTES, props.maxGpuBytes || MAX_BYTES))
          throw Error("Viewer memory budget exceeded");
        const buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(
          gl.ARRAY_BUFFER,
          new Uint8Array(data, HEADER),
          gl.STATIC_DRAW,
        );
        if (gl.getError() !== gl.NO_ERROR) {
          gl.deleteBuffer(buffer);
          throw Error(
            "GPU upload failed; reduce the rendering cache or reload the viewer.",
          );
        }
        chunks.push({ buffer, count, origin, primitive, options });
        bytes += size;
        if (primitive === 0) points += count;
      }
      limited = !!(flags & 4);
      clearTimeout(timeout);
      pending = false;
      text();
      redraw();
      // Yield to rendering before acknowledging with the next pull. No CPU point cache.
      schedule(flags & 2 ? 1000 : count ? 16 : 60);
    } catch (e) {
      fail(e);
    }
  }
  function resize() {
    const r = root.getBoundingClientRect(),
      dpr = Math.min(
        2,
        window.devicePixelRatio || 1,
        8192 / Math.max(1, r.width),
        8192 / Math.max(1, r.height),
        Math.sqrt((16 * 1024 * 1024) / Math.max(1, r.width * r.height)),
      ),
      w = Math.max(1, Math.round(r.width * dpr)),
      h = Math.max(1, Math.round(r.height * dpr));
    if (canvas.width !== w || canvas.height !== h) {
      canvas.width = overlay.width = w;
      canvas.height = overlay.height = h;
      redraw();
      if (!pending) schedule(0);
    }
  }
  function redraw() {
    if (!drawFrame && !disposed && !lost && !error)
      drawFrame = requestAnimationFrame(draw);
  }
  function corners(b) {
    return Array.from({ length: 8 }, (_, i) => [
      b[i & 1 ? 3 : 0],
      b[i & 2 ? 4 : 1],
      b[i & 4 ? 5 : 2],
    ]);
  }
  function draw() {
    drawFrame = 0;
    if (!visible() || !gl || gl.isContextLost()) return;
    resize();
    const w = canvas.width,
      h = canvas.height,
      dpr = w / Math.max(1, root.clientWidth),
      bg = scene.background || [0.2, 0.2, 0.2];
    gl.viewport(0, 0, w, h);
    gl.clearColor(...bg, 1);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(program);
    gl.uniformMatrix3fv(uniforms.rotation, false, camera.rotation);
    gl.uniformMatrix4fv(
      uniforms.projection,
      false,
      camera.projection(w, h, scene.bounds),
    );
    gl.uniform1f(uniforms.distance, camera.distance);
    const range = gl.getParameter(gl.ALIASED_POINT_SIZE_RANGE) || [1, 64];
    gl.uniform1f(
      uniforms.pointSize,
      Math.min(range[1], Math.max(range[0], (scene.pointSize || 1) * dpr)),
    );
    gl.uniform1i(uniforms.fog, scene.fog ? 1 : 0);
    let near = 0,
      far = camera.distance * 2;
    if (scene.bounds?.length === 6) {
      const depths = corners(scene.bounds).map(
        (p) => camera.distance - dot(camera.rows[2], sub(p, camera.center)),
      );
      near = Math.min(...depths);
      far = Math.max(...depths);
      far += (far - near) * 0.1;
    }
    gl.uniform2f(uniforms.fogRange, near, far);
    gl.enableVertexAttribArray(0);
    gl.enableVertexAttribArray(1);
    for (const chunk of chunks) {
      if (camera.lock2d && chunk.primitive === 0) continue;
      gl.bindBuffer(gl.ARRAY_BUFFER, chunk.buffer);
      gl.vertexAttribPointer(0, 3, gl.FLOAT, false, STRIDE, 0);
      gl.vertexAttribPointer(1, 4, gl.UNSIGNED_BYTE, true, STRIDE, 12);
      gl.uniform3fv(
        uniforms.offset,
        new Float32Array(sub(chunk.origin, camera.center)),
      );
      gl.uniform1i(
        uniforms.lighting,
        chunk.primitive === 2 && !chunk.options ? 1 : 0,
      );
      if (chunk.options) {
        gl.enable(gl.BLEND);
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
        gl.disable(gl.DEPTH_TEST);
      } else {
        gl.disable(gl.BLEND);
        gl.enable(gl.DEPTH_TEST);
      }
      gl.drawArrays(
        chunk.primitive === 2
          ? gl.TRIANGLES
          : chunk.primitive === 1
            ? gl.LINES
            : gl.POINTS,
        0,
        chunk.count,
      );
    }
    drawDbhCylinders();
    gl.disable(gl.BLEND);
    gl.enable(gl.DEPTH_TEST);
    ctx.clearRect(0, 0, w, h);
    ctx.lineWidth = dpr;
    ctx.font = `${12 * dpr}px 'Segoe UI', sans-serif`;
    drawOverlays(w, h, dpr);
    root.dataset.loadedPoints = String(points);
    root.dataset.gpuBytes = String(bytes);
    root.dataset.streamEpoch = String(epoch);
    root.dataset.cameraRevision = String(revision);
  }
  function drawOverlays(w, h, dpr) {
    const project = (p) => camera.project(p, w, h),
      line = (a, b) => {
        const p = project(a),
          q = project(b);
        if (!p || !q) return;
        ctx.beginPath();
        ctx.moveTo(p[0], p[1]);
        ctx.lineTo(q[0], q[1]);
        ctx.stroke();
      },
      box = (b, color, dash = []) => {
        if (b?.length !== 6) return;
        ctx.strokeStyle = color;
        ctx.setLineDash(dash);
        const c = corners(b);
        for (let i = 0; i < 8; ++i)
          for (let bit = 1; bit <= 4; bit *= 2)
            if (!(i & bit)) line(c[i], c[i | bit]);
        ctx.setLineDash([]);
      };
    if (scene.box) box(scene.bounds, "#404040");
    box(scene.clip, "#aaaaaa", [4 * dpr, 4 * dpr]);
    for (const s of scene.segments || []) {
      const b = s.bounds;
      if (b?.length !== 6) continue;
      if (!camera.lock2d && s.selected) box(b, "#ffff00", [7 * dpr, 7 * dpr]);
      if (camera.lock2d) {
        const p = project(s.dbhPosition),
          q = project(
            add(s.dbhPosition, scale(camera.rows[0], s.diameter * 0.5)),
          );
        if (p && q) {
          const r = Math.max(0.5, Math.hypot(p[0] - q[0], p[1] - q[1]));
          ctx.fillStyle =
            ctx.strokeStyle = `rgb(${(s.color || [1, 1, 1]).map((c) => Math.round(c * 255)).join(",")})`;
          ctx.beginPath();
          if (s.dbhValid) {
            ctx.arc(p[0], p[1], r, 0, 2 * Math.PI);
            ctx.fill();
          } else {
            ctx.moveTo(p[0] - r, p[1]);
            ctx.lineTo(p[0] + r, p[1]);
            ctx.moveTo(p[0], p[1] - r);
            ctx.lineTo(p[0], p[1] + r);
            ctx.stroke();
          }
          if (s.selected) {
            ctx.strokeStyle = "#00ff00";
            ctx.beginPath();
            ctx.arc(p[0], p[1], r * 1.2, 0, 2 * Math.PI);
            ctx.stroke();
          }
        }
      }
      if (s.attributes) {
        ctx.strokeStyle = "#ffff00";
        if (!camera.lock2d && s.position?.length === 3) {
          const p = s.position,
            tip = add(p, [0, 0, s.height || 0]);
          if (s.height > 0) line(p, tip);
          const at = s.positionTop ? tip : p;
          line(
            add(at, [-(b[3] - b[0]) / 2, 0, 0]),
            add(at, [(b[3] - b[0]) / 2, 0, 0]),
          );
          line(
            add(at, [0, -(b[4] - b[1]) / 2, 0]),
            add(at, [0, (b[4] - b[1]) / 2, 0]),
          );
          if (s.crownStart > 0) {
            const crown = add(s.position, [0, 0, s.crownStart]);
            line(add(crown, [-(b[3] - b[0]) / 4, 0, 0]), add(crown, [(b[3] - b[0]) / 4, 0, 0]));
            line(add(crown, [0, -(b[4] - b[1]) / 4, 0]), add(crown, [0, (b[4] - b[1]) / 4, 0]));
          }
        }
        const p = project([(b[0] + b[3]) / 2, (b[1] + b[4]) / 2, b[5]]);
        if (p && p[0] > -200 && p[0] < w + 200 && p[1] > -50 && p[1] < h + 50) {
          ctx.fillStyle = "#ffff00";
          ctx.textAlign = "center";
          ctx.fillText(s.label, p[0], p[1] - 4 * dpr);
        }
      }
    }
    const x = 32 * dpr,
      y = h - 32 * dpr;
    ctx.lineWidth = 2 * dpr;
    ["#ff0000", "#00ff00", "#0080ff"].forEach((color, i) => {
      ctx.strokeStyle = color;
      ctx.beginPath();
      ctx.moveTo(x, y);
      ctx.lineTo(
        x + camera.rows[0][i] * 30 * dpr,
        y - camera.rows[1][i] * 30 * dpr,
      );
      ctx.stroke();
    });
    ctx.lineWidth = dpr;
  }
  function drawDbhCylinders() {
    if (camera.lock2d) return;
    const vertices = [],
      color = [1, 0, 0, 1],
      slices = 10;
    for (const s of scene.segments || []) {
      if (!s.attributes || !s.dbhValid || s.dbhNormal?.length !== 3) continue;
      const a = s.dbhPosition,
        radius = s.dbhDiameter * 0.5,
        direction = normalize(s.dbhNormal),
        axis = scale(direction, radius * 0.75),
        first = cross(direction, [1, 0, 0]),
        basis = normalize(length(first) > 1e-8 ? first : cross(direction, [0, 1, 0])),
        second = normalize(cross(direction, basis)),
        ring = [];
      for (let i = 0; i < slices; ++i) {
        const angle = (2 * Math.PI * i) / slices,
          offset = add(scale(basis, Math.cos(angle) * radius), scale(second, Math.sin(angle) * radius));
        ring.push([add(a, offset), add(add(a, axis), offset)]);
      }
      for (let i = 0; i < slices; ++i) {
        const next = ring[(i + 1) % slices];
        for (const p of [ring[i][0], next[0], next[1], ring[i][0], next[1], ring[i][1]])
          vertices.push(...sub(p, camera.center), ...color);
      }
    }
    if (!vertices.length) return;
    gl.bindBuffer(gl.ARRAY_BUFFER, attributeBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 28, 0);
    gl.vertexAttribPointer(1, 4, gl.FLOAT, false, 28, 12);
    gl.uniform3f(uniforms.offset, 0, 0, 0);
    gl.uniform1i(uniforms.lighting, 1);
    gl.disable(gl.BLEND);
    gl.enable(gl.DEPTH_TEST);
    gl.drawArrays(gl.TRIANGLES, 0, vertices.length / 7);
  }
  let pointer = null;
  canvas.addEventListener("contextmenu", (e) => e.preventDefault());
  canvas.addEventListener("pointerdown", (e) => {
    if (!visible()) return;
    canvas.focus();
    canvas.setPointerCapture(e.pointerId);
    pointer = { id: e.pointerId, x: e.clientX, y: e.clientY, button: e.button };
    e.preventDefault();
  });
  canvas.addEventListener("pointermove", (e) => {
    if (!pointer || pointer.id !== e.pointerId) return;
    const dx = e.clientX - pointer.x,
      dy = e.clientY - pointer.y,
      w = root.clientWidth,
      h = root.clientHeight;
    pointer.x = e.clientX;
    pointer.y = e.clientY;
    if (pointer.button === 0 && !e.shiftKey) camera.rotate(dx, dy, w, h);
    else if (pointer.button === 2 || e.shiftKey) camera.pan(dx, dy, w, h);
    else if (pointer.button === 1) camera.zoom(dy, h);
    changed();
  });
  for (const name of ["pointerup", "pointercancel", "lostpointercapture"])
    canvas.addEventListener(name, () => {
      pointer = null;
      sendCamera();
    });
  canvas.addEventListener(
    "wheel",
    (e) => {
      if (!visible()) return;
      e.preventDefault();
      const delta =
        e.deltaY *
        (e.deltaMode === 1 ? 15 : e.deltaMode === 2 ? root.clientHeight : 0.15);
      camera.zoom(delta, root.clientHeight);
      changed();
    },
    { passive: false },
  );
  canvas.addEventListener("dblclick", (e) => {
    if (e.button !== 0 || !visible()) return;
    const r = canvas.getBoundingClientRect();
    emit(id, "viewerPick", {
      ...camera.ray(
        e.clientX - r.left,
        e.clientY - r.top,
        r.width,
        r.height,
        scene.bounds,
      ),
      ctrl: e.ctrlKey || e.metaKey,
    });
  });
  const onVisibility = () => {
    if (!document.hidden) {
      redraw();
      schedule(0);
    }
  };
  document.addEventListener("visibilitychange", onVisibility);
  const resizeObserver = new ResizeObserver(resize);
  resizeObserver.observe(root);
  canvas.addEventListener("webglcontextlost", (e) => {
    e.preventDefault();
    lost = true;
    clearTimeout(pullTimer);
    clearTimeout(timeout);
    pending = false;
    status.textContent = "Graphics context lost; waiting for recovery…";
  });
  canvas.addEventListener("webglcontextrestored", () => {
    try {
      chunks = [];
      bytes = points = 0;
      lost = false;
      error = false;
      initialize();
      restart();
      redraw();
    } catch (e) {
      fail(e);
    }
  });
  try {
    initialize();
    text();
  } catch (e) {
    fail(e);
  }
  return {
    update,
    receiveBinary,
    reconnect,
    dispose() {
      disposed = true;
      clearTimeout(pullTimer);
      clearTimeout(timeout);
      clearTimeout(cameraTimer);
      cancelAnimationFrame(drawFrame);
      resizeObserver.disconnect();
      document.removeEventListener("visibilitychange", onVisibility);
      clear();
      if (gl && !lost && program) gl.deleteProgram(program);
    },
  };
}
