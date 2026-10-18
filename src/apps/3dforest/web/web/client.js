import { createWidget, element, richText } from "./widgets.js";
const controls = document.querySelector("#controls"),
  viewer = document.querySelector("#viewer"),
  bottom = document.querySelector("#bottom"),
  main = document.querySelector("#main"),
  status = document.querySelector("#status"),
  restart = document.querySelector("#new-session"),
  serviceLink = document.querySelector("#service-link");
const widgets = new Map(),
  dialogs = new Map(),
  expanded = new Map();
const storageKey = "3dforest.web.session.v1";
let token = sessionStorage.getItem(storageKey) || "",
  socket,
  ready = false,
  stopped = false,
  timer,
  delay = 500,
  lastRevision = 0;
let nextSequence = 0;
let structure = "",
  lastUi = null;
function send(message) {
  if (socket?.readyState === WebSocket.OPEN)
    socket.send(JSON.stringify(message));
}
function event(id, name, props = {}) {
  if (!ready) return 0;
  const sequence = ++nextSequence;
  send({ type: "event", id, event: name, sequence, ...props });
  return sequence;
}
function preserveFocus(fn) {
  const active = document.activeElement;
  const start = active?.selectionStart,
    end = active?.selectionEnd;
  fn();
  if (
    active?.isConnected &&
    active !== document.body &&
    document.activeElement !== active
  ) {
    active.focus({ preventScroll: true });
    if (typeof start === "number")
      try {
        active.setSelectionRange(start, end);
      } catch {}
  }
}
function splitHandle(handle, before, after, vertical, onFinish, min = 0) {
  handle.tabIndex = 0;
  handle.setAttribute("role", "separator");
  handle.setAttribute(
    "aria-orientation",
    (typeof vertical === "function" ? vertical() : vertical)
      ? "horizontal"
      : "vertical",
  );
  handle.addEventListener("pointerdown", (e) => {
    const isVertical = typeof vertical === "function" ? vertical() : vertical;
    e.preventDefault();
    handle.setPointerCapture(e.pointerId);
    const a = before.getBoundingClientRect(),
      b = after.getBoundingClientRect(),
      total = isVertical ? a.height + b.height : a.width + b.width,
      origin = isVertical ? e.clientY : e.clientX,
      start = isVertical ? a.height : a.width;
    const move = (e) => {
      const n = Math.max(
        min,
        Math.min(
          total - min,
          start + (isVertical ? e.clientY : e.clientX) - origin,
        ),
      );
      before.style.flex = `0 0 ${n}px`;
      after.style.flex = "1 1 0";
    };
    const up = () => {
      handle.removeEventListener("pointermove", move);
      handle.removeEventListener("pointerup", up);
      handle.removeEventListener("pointercancel", up);
      onFinish?.();
    };
    handle.addEventListener("pointermove", move);
    handle.addEventListener("pointerup", up);
    handle.addEventListener("pointercancel", up);
  });
}
function renderNode(node) {
  if (node.kind === "widget") {
    const w = widgets.get(node.id);
    if (!w) throw Error(`Unknown widget ${node.id}`);
    if (node.layout) w.content.replaceChildren(renderNode(node.layout));
    else if (node.children) {
      const children = node.children.map((n) =>
        n ? renderNode(n) : element("span", "toolbar-separator"),
      );
      if (w.type === "Splitter") {
        const elements = [];
        children.forEach((child, i) => {
          child.style.flex = `${w.properties.sizes?.[i] || 1} 1 0`;
          elements.push(child);
          if (i < children.length - 1) {
            const handle = element("div", "splitter-handle");
            elements.push(handle);
            splitHandle(
              handle,
              child,
              children[i + 1],
              () => w.properties.orientation === 2,
              () => {
                const sizes = children.map((c) =>
                  Math.round(
                    w.properties.orientation === 2
                      ? c.getBoundingClientRect().height
                      : c.getBoundingClientRect().width,
                  ),
                );
                event(w.id, "splitterMoved", {
                  sizes,
                  index: i + 1,
                  position: sizes.slice(0, i + 1).reduce((a, b) => a + b, 0),
                });
              },
              w.properties.collapsible ? 0 : 24,
            );
          }
        });
        w.content.replaceChildren(...elements);
      } else w.content.replaceChildren(...children);
    }
    return w.root;
  }
  const root = element("div", `layout ${node.kind}`);
  if (node.kind === "stretch") {
    root.style.flexGrow = node.stretch || 1;
    return root;
  }
  if (node.kind === "spacing") {
    root.style.flexBasis = `${node.spacing}px`;
    return root;
  }
  root.style.gap = `${node.spacing >= 0 ? node.spacing : 6}px`;
  const [l, t, r, b] = node.margins || [0, 0, 0, 0];
  root.style.padding = `${Math.max(0, t)}px ${Math.max(0, r)}px ${Math.max(0, b)}px ${Math.max(0, l)}px`;
  for (const item of node.items) {
    const e = renderNode(item);
    if (node.kind === "grid") {
      e.style.gridRow = `${item.row + 1} / span ${Math.max(1, item.rowSpan)}`;
      e.style.gridColumn = `${item.column + 1} / span ${Math.max(1, item.columnSpan)}`;
    } else if (item.stretch) e.style.flexGrow = item.stretch;
    const a = item.alignment || 0;
    if (a & 1) e.style.justifySelf = "start";
    if (a & 2) e.style.justifySelf = "end";
    if (a & 4) e.style.justifySelf = "center";
    if (a & 32) e.style.alignSelf = "start";
    if (a & 64) e.style.alignSelf = "end";
    if (a & 128) e.style.alignSelf = "center";
    root.append(e);
  }
  return root;
}
function renderNavigation(items, parent, depth = 0) {
  for (const item of items) {
    const section = element("section", "nav-section");
    const title = element(
      "button",
      `nav-title ${item.kind === "group" ? "nav-group" : "nav-action"}`,
      item.title,
    );
    title.type = "button";
    title.title = item.toolTip || "";
    title.style.paddingLeft = `${10 + depth * 8}px`;
    section.append(title);
    if (item.kind === "group" || item.panel) {
      const content = element("div", item.panel ? "nav-panel" : "nav-children");
      if (item.panel) content.append(renderNode(item.panel));
      renderNavigation(item.children, content, depth + 1);
      section.append(content);
      const open = expanded.get(item.id) ?? false;
      content.hidden = !open;
      title.setAttribute("aria-expanded", String(open));
      title.addEventListener("click", () => {
        content.hidden = !content.hidden;
        expanded.set(item.id, !content.hidden);
        title.setAttribute("aria-expanded", String(!content.hidden));
      });
    } else title.addEventListener("click", () => event(item.id, "triggered"));
    parent.append(section);
  }
}
const buttons = [
  [1024, "OK"],
  [2048, "Save"],
  [4096, "Save all"],
  [8192, "Open"],
  [16384, "Yes"],
  [32768, "Yes to all"],
  [65536, "No"],
  [131072, "No to all"],
  [262144, "Abort"],
  [524288, "Retry"],
  [1048576, "Ignore"],
  [2097152, "Close"],
  [4194304, "Cancel"],
  [8388608, "Discard"],
  [16777216, "Help"],
  [33554432, "Apply"],
  [67108864, "Reset"],
  [134217728, "Restore defaults"],
];
function renderDialogs(descriptions) {
  const ids = new Set(descriptions.map((d) => d.id));
  for (const [id, d] of dialogs)
    if (!ids.has(id)) {
      d.observer.disconnect();
      d.root.remove();
      if (d.returnFocus?.isConnected)
        d.returnFocus.focus({ preventScroll: true });
      dialogs.delete(id);
    }
  const modalIndex = descriptions.reduce((m, d, i) => (d.modal ? i : m), -1);
  document.querySelector("#application").inert = modalIndex >= 0;
  const shade = document.querySelector("#modal-shade");
  shade.hidden = modalIndex < 0;
  shade.style.zIndex = 100 + modalIndex * 2;
  descriptions.forEach((p, i) => {
    let d = dialogs.get(p.id);
    if (!d) {
      const root = element("section", "dialog-window");
      root.setAttribute("role", "dialog");
      root.tabIndex = -1;
      const bar = element("header", "dialog-title"),
        title = element("span"),
        close = element("button", "dialog-close", "×");
      close.title = "Close";
      bar.append(title, close);
      const body = element("div", "dialog-body"),
        footer = element("footer", "dialog-buttons");
      root.append(bar, body, footer);
      document.querySelector("#dialogs").append(root);
      d = {
        root,
        bar,
        title,
        body,
        footer,
        layoutKey: "",
        last: null,
        returnFocus: document.activeElement,
      };
      dialogs.set(p.id, d);
      close.addEventListener("click", () =>
        event(p.id, d.last.progress ? "cancel" : "close"),
      );
      root.addEventListener("keydown", (e) => {
        if (e.key === "Tab" && d.last?.modal) {
          const focusables = [
            ...root.querySelectorAll("button,input,select,textarea,[tabindex]"),
          ].filter(
            (n) => !n.disabled && n.tabIndex >= 0 && n.getClientRects().length,
          );
          const first = focusables[0],
            last = focusables.at(-1);
          if (!first) {
            e.preventDefault();
            root.focus();
          } else if (
            e.shiftKey &&
            (document.activeElement === first ||
              document.activeElement === root)
          ) {
            e.preventDefault();
            last.focus();
          } else if (!e.shiftKey && document.activeElement === last) {
            e.preventDefault();
            first.focus();
          }
        }
        if (e.key === "Escape") {
          e.preventDefault();
          event(p.id, d.last.progress ? "cancel" : "close");
        }
        if (
          e.key === "Enter" &&
          d.last.defaultButton &&
          e.target.tagName !== "TEXTAREA" &&
          e.target.contentEditable !== "true"
        ) {
          e.preventDefault();
          event(p.id, "button", { button: d.last.defaultButton });
        }
      });
      bar.addEventListener("pointerdown", (e) => {
        if (e.target === close) return;
        bar.setPointerCapture(e.pointerId);
        const box = root.getBoundingClientRect(),
          x = e.clientX,
          y = e.clientY;
        root.style.transform = "none";
        root.style.left = `${box.left}px`;
        root.style.top = `${box.top}px`;
        const move = (e) => {
          root.style.left = `${Math.max(0, Math.min(innerWidth - 60, box.left + e.clientX - x))}px`;
          root.style.top = `${Math.max(0, Math.min(innerHeight - 30, box.top + e.clientY - y))}px`;
        };
        const up = () => {
          bar.removeEventListener("pointermove", move);
          bar.removeEventListener("pointerup", up);
        };
        bar.addEventListener("pointermove", move);
        bar.addEventListener("pointerup", up);
      });
      d.observer = new ResizeObserver(() => {
        clearTimeout(d.resizeTimer);
        d.resizeTimer = setTimeout(() => {
          if (root.isConnected)
            event(p.id, "resize", {
              width: Math.round(root.offsetWidth),
              height: Math.round(root.offsetHeight),
            });
        }, 150);
      });
      d.observer.observe(root);
      root.focus();
    }
    d.root.style.zIndex = 101 + i * 2;
    d.root.inert = i < modalIndex;
    d.root.setAttribute("aria-modal", String(!!p.modal));
    d.title.textContent = p.title;
    if (!d.last) {
      if (p.width > 0) d.root.style.width = `${p.width}px`;
      if (p.height > 0) d.root.style.height = `${p.height}px`;
    }
    d.root.style.maxWidth =
      p.maxWidth > 0 ? `${p.maxWidth}px` : "calc(100vw - 24px)";
    d.root.style.maxHeight =
      p.maxHeight > 0 ? `${p.maxHeight}px` : "calc(100vh - 24px)";
    if (p.fixedHeight >= 0) d.root.style.height = `${p.fixedHeight}px`;
    const lk = JSON.stringify(p.layout || null);
    if (lk !== d.layoutKey) {
      d.body.replaceChildren(...(p.layout ? [renderNode(p.layout)] : []));
      d.layoutKey = lk;
    }
    if (p.message !== undefined) {
      if (!d.message) {
        d.message = element("div", "message-text");
        d.information = element("div", "message-information");
        d.body.append(d.message, d.information);
      }
      richText(d.message, p.message);
      richText(d.information, p.information || "");
    }
    if (p.progress) {
      if (!d.progress) {
        d.label = element("div");
        d.progress = element("progress");
        d.body.append(d.label, d.progress);
      }
      d.label.textContent = p.label;
      d.progress.max = Math.max(1, p.maximum - p.minimum);
      if (p.maximum === p.minimum) d.progress.removeAttribute("value");
      else d.progress.value = Math.max(0, p.value - p.minimum);
    }
    const bk = JSON.stringify([
      p.buttons,
      p.defaultButton,
      p.progress,
      p.cancelText,
    ]);
    if (bk !== d.buttonsKey) {
      d.footer.replaceChildren();
      for (const [code, title] of buttons)
        if (p.buttons & code) {
          const b = element(
            "button",
            code === p.defaultButton ? "default-button" : "",
            title,
          );
          b.addEventListener("click", () =>
            event(p.id, "button", { button: code }),
          );
          d.footer.append(b);
        }
      if (p.progress && p.cancelText) {
        const b = element("button", "", p.cancelText);
        b.addEventListener("click", () => event(p.id, "cancel"));
        d.footer.append(b);
      }
      d.buttonsKey = bk;
    }
    if (d.last && p.raise !== d.last.raise) d.root.focus();
    d.last = p;
  });
}
function snapshot(m) {
  const activeIds = new Set();
  for (const spec of m.widgets) {
    activeIds.add(spec.id);
    let w = widgets.get(spec.id);
    if (!w || w.type !== spec.type) {
      w?.dispose();
      w = createWidget(spec, event);
      widgets.set(spec.id, w);
    } else w.update(spec.properties, m.eventSequence || 0);
  }
  for (const [id, w] of widgets)
    if (!activeIds.has(id)) {
      w.dispose();
      w.root.remove();
      widgets.delete(id);
    }
  document.title = m.application.name || "3D Forest";
  renderServiceLink(m.externalLink);
  applyUi(m.ui);
  ready = true;
}
function renderServiceLink(link) {
  if (!link?.url || !link?.text) {
    serviceLink.hidden = true;
    serviceLink.removeAttribute("href");
    return;
  }
  const url = new URL(link.url, location.origin);
  if (url.origin !== location.origin) {
    serviceLink.hidden = true;
    serviceLink.removeAttribute("href");
    return;
  }
  serviceLink.href = url.href;
  serviceLink.textContent = link.text;
  serviceLink.hidden = false;
}
function applyUi(ui) {
  const shape = JSON.stringify([ui.navigation, ui.viewer, ui.bottom]);
  preserveFocus(() => {
    if (shape !== structure) {
      const scroll = controls.scrollTop;
      controls.replaceChildren();
      renderNavigation(ui.navigation, controls);
      viewer.replaceChildren(...(ui.viewer ? [renderNode(ui.viewer)] : []));
      bottom.replaceChildren(...(ui.bottom ? [renderNode(ui.bottom)] : []));
      controls.scrollTop = scroll;
      structure = shape;
    }
    renderDialogs(ui.dialogs || []);
  });
  bottom.hidden = !ui.bottomVisible;
  document.querySelector("#bottom-handle").hidden = !ui.bottomVisible;
  main.classList.toggle("no-bottom", !ui.bottomVisible);
  lastUi = ui;
}
function connect() {
  ready = false;
  document.body.classList.add("disconnected");
  const url = new URL("/ui", location.href);
  url.protocol = location.protocol === "https:" ? "wss:" : "ws:";
  const current = new WebSocket(url);
  socket = current;
  current.binaryType = "arraybuffer";
  current.addEventListener("open", () => send({ type: "hello", token }));
  current.addEventListener("message", ({ data }) => {
    if (current !== socket) return;
    try {
      if (data instanceof ArrayBuffer) {
        if (data.byteLength < 64) throw Error("Truncated viewer packet");
        const widgetId = new DataView(data).getUint32(8, true);
        widgets.get(`w${widgetId}`)?.receiveBinary(data);
        return;
      }
      const m = JSON.parse(data);
      if (m.type === "welcome") {
        for (const w of widgets.values()) w.resetPending();
        token = m.token;
        sessionStorage.setItem(storageKey, token);
        lastRevision = 0;
        delay = 500;
      } else if (m.type === "snapshot" || m.type === "patch") {
        if (!Number.isSafeInteger(m.revision) || m.revision <= lastRevision)
          throw Error("Invalid state revision");
        if (m.type === "snapshot") snapshot(m);
        else
          for (const [id, props] of Object.entries(m.widgets)) {
            const w = widgets.get(id);
            if (!w) throw Error("Unknown widget");
            w.update(props, m.eventSequence || 0);
          }
        if (m.type === "patch" && m.externalLink)
          renderServiceLink(m.externalLink);
        if (m.type === "patch" && m.ui) preserveFocus(() => applyUi(m.ui));
        for (const w of widgets.values()) w.update({}, m.eventSequence || 0);
        nextSequence = Math.max(nextSequence, m.eventSequence || 0);
        lastRevision = m.revision;
        send({ type: "ack", revision: m.revision });
        ready = true;
        document.body.classList.remove("disconnected");
        status.textContent = "Connected";
        status.classList.remove("error");
      } else if (["expired", "unavailable", "replaced"].includes(m.type)) {
        stopped = true;
        ready = false;
        status.textContent = m.message;
        restart.hidden = false;
      } else if (m.type === "error") {
        status.textContent = m.message;
        status.classList.add("error");
      }
    } catch (e) {
      status.textContent = `UI error: ${e.message}`;
      status.classList.add("error");
      current.close();
    }
  });
  current.addEventListener("close", () => {
    if (current !== socket) return;
    ready = false;
    document.body.classList.add("disconnected");
    if (stopped) return;
    status.textContent = "Reconnecting…";
    timer = setTimeout(connect, delay);
    delay = Math.min(delay * 2, 5000);
  });
}
restart.addEventListener("click", () => {
  clearTimeout(timer);
  socket?.close();
  socket = undefined;
  sessionStorage.removeItem(storageKey);
  token = "";
  stopped = false;
  restart.hidden = true;
  connect();
});
window.addEventListener("pagehide", () => {
  stopped = true;
  clearTimeout(timer);
  socket?.close();
});
window.addEventListener("pageshow", (e) => {
  if (e.persisted) {
    stopped = false;
    connect();
  }
});
// The top-level splitters are client preferences, independent of common widget ownership.
splitHandle(
  document.querySelector("#sidebar-handle"),
  document.querySelector("#sidebar"),
  main,
  false,
  null,
  180,
);
splitHandle(
  document.querySelector("#bottom-handle"),
  viewer,
  bottom,
  true,
  null,
  80,
);
connect();
