const controls = document.querySelector('#controls');
const status = document.querySelector('#status');
const newSession = document.querySelector('#new-session');
const viewer = document.querySelector('#viewer');
const bottom = document.querySelector('#bottom');
const main = document.querySelector('#main');
const widgets = new Map();
const storageKey = '3dforest.web.session.v1';
let token = sessionStorage.getItem(storageKey) || '';
let socket;
let reconnectTimer;
let reconnectDelay = 500;
let stopped = false;
let ready = false;
let lastRevision = 0;

function send(message) {
  if (socket?.readyState === WebSocket.OPEN)
    socket.send(JSON.stringify(message));
}
function event(id, name, properties = {}) {
  if (ready) send({ type: 'event', id, event: name, ...properties });
}

function createWidget(spec) {
  const p = { ...spec.properties };
  const root = document.createElement(spec.type === 'GroupBox' ? 'fieldset' : 'div');
  root.className = 'control';
  const content = document.createElement('div');
  let apply = () => {};
  if (spec.type === 'Slider') {
    const title = document.createElement('label');
    const row = document.createElement('div');
    row.className = 'slider-row';
    const input = document.createElement('input');
    input.type = 'range';
    input.id = spec.id;
    title.htmlFor = spec.id;
    const output = document.createElement('output');
    output.htmlFor = spec.id;
    const ticks = document.createElement('datalist');
    ticks.id = `${spec.id}-ticks`;
    let dragging = false;
    let frame = 0;
    function changed() {
      event(spec.id, 'valueChanged', { value: Number(input.value) });
    }
    input.addEventListener('pointerdown', e => {
      dragging = true;
      input.setPointerCapture(e.pointerId);
    });
    input.addEventListener('input', () => {
      output.value = input.value;
      cancelAnimationFrame(frame);
      frame = requestAnimationFrame(changed);
    });
    input.addEventListener('change', () => {
      cancelAnimationFrame(frame);
      changed();
    });
    function released() {
      if (!dragging) return;
      dragging = false;
      cancelAnimationFrame(frame);
      event(spec.id, 'sliderReleased', { value: Number(input.value) });
    }
    input.addEventListener('pointerup', released);
    input.addEventListener('pointercancel', released);
    input.addEventListener('lostpointercapture', released);
    // HTML range step restricts legal values; Qt singleStep only controls keys.
    input.step = '1';
    input.addEventListener('keydown', e => {
      const keys = ['ArrowLeft', 'ArrowDown', 'ArrowRight', 'ArrowUp'];
      if (!keys.includes(e.key)) return;
      e.preventDefault();
      const direction = keys.indexOf(e.key) < 2 ? -1 : 1;
      input.value = Math.max(p.minimum, Math.min(p.maximum,
        Number(input.value) + direction * p.singleStep));
      output.value = input.value;
      changed();
    });
    row.append(input, output);
    root.append(title, row, ticks);
    apply = () => {
      title.textContent = p.name || 'Slider';
      input.min = p.minimum;
      input.max = p.maximum;
      if (!dragging) input.value = p.value;
      input.disabled = p.enabled === false;
      input.setAttribute('aria-label', p.name || p.toolTip || 'Slider');
      input.classList.toggle('vertical', p.orientation === 2);
      output.value = input.value;
      ticks.replaceChildren();
      const interval = p.tickInterval || p.singleStep || 1;
      if (p.tickPosition && (p.maximum - p.minimum) / interval <= 1000) {
        for (let v = p.minimum; v <= p.maximum; v += interval) {
          const tick = document.createElement('option');
          tick.value = v;
          ticks.append(tick);
        }
        input.setAttribute('list', ticks.id);
      } else input.removeAttribute('list');
    };
  } else if (spec.type === 'Label') {
    apply = () => { content.textContent = p.text || ''; };
  } else if (spec.type === 'GroupBox') {
    const legend = document.createElement('legend');
    root.append(legend);
    apply = () => { legend.textContent = p.title || ''; };
  } else if (spec.type === 'Unsupported') {
    apply = () => { content.textContent = `Unsupported control${p.name ? ': ' + p.name : ''}`; };
  }
  root.append(content);
  function update(patch) {
    Object.assign(p, patch);
    apply();
    root.hidden = p.visible === false;
    root.title = p.toolTip || '';
    root.setAttribute('aria-disabled', String(p.enabled === false));
  }
  update({});
  return { root, content, update };
}

function renderNode(node) {
  if (node.kind === 'widget') {
    const w = widgets.get(node.id);
    if (!w) throw new Error('Missing widget ' + node.id);
    if (node.layout) w.content.append(renderNode(node.layout));
    return w.root;
  }
  const root = document.createElement('div');
  root.className = `layout ${node.kind}`;
  if (node.kind === 'stretch') { root.style.flexGrow = node.stretch || 1; return root; }
  if (node.kind === 'spacing') { root.style.flexBasis = `${node.spacing}px`; return root; }
  root.style.gap = `${Math.max(0, node.spacing)}px`;
  const [l, t, r, b] = node.margins;
  root.style.padding = `${Math.max(0,t)}px ${Math.max(0,r)}px ${Math.max(0,b)}px ${Math.max(0,l)}px`;
  for (const item of node.items) {
    const element = renderNode(item);
    if (node.kind === 'grid') {
      element.style.gridRow = `${item.row + 1} / span ${Math.max(1,item.rowSpan)}`;
      element.style.gridColumn = `${item.column + 1} / span ${Math.max(1,item.columnSpan)}`;
    } else if (item.stretch) element.style.flexGrow = item.stretch;
    root.append(element);
  }
  return root;
}
function renderNavigation(items, parent) {
  for (const item of items) {
    if (item.kind === 'group' || item.panel) {
      const section = document.createElement('details');
      section.open = true;
      const title = document.createElement('summary');
      title.textContent = item.title;
      section.append(title);
      if (item.panel) section.append(renderNode(item.panel));
      renderNavigation(item.children, section);
      parent.append(section);
    } else {
      const button = document.createElement('button');
      button.type = 'button';
      button.textContent = item.title;
      button.title = item.toolTip || '';
      button.addEventListener('click', () => event(item.id, 'triggered'));
      parent.append(button);
    }
  }
}
function snapshot(message) {
  document.title = message.application.name || '3D Forest';
  document.querySelector('#title').textContent = document.title;
  controls.replaceChildren();
  viewer.replaceChildren();
  bottom.replaceChildren();
  widgets.clear();
  for (const spec of message.widgets) widgets.set(spec.id, createWidget(spec));
  renderNavigation(message.ui.navigation, controls);
  if (message.ui.viewer) viewer.append(renderNode(message.ui.viewer));
  if (message.ui.bottom) bottom.append(renderNode(message.ui.bottom));
  bottom.hidden = !message.ui.bottomVisible;
  main.classList.toggle('no-bottom', !message.ui.bottomVisible);
  controls.disabled = false;
  ready = true;
}

function connect() {
  ready = false;
  controls.disabled = true;
  const url = new URL('/ui', location.href);
  url.protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
  const current = new WebSocket(url);
  socket = current;
  current.addEventListener('open', () => send({ type: 'hello', token }));
  current.addEventListener('message', ({ data }) => {
    if (current !== socket) return;
    try {
      const message = JSON.parse(data);
      if (message.type === 'welcome') {
        token = message.token;
        sessionStorage.setItem(storageKey, token);
        lastRevision = 0;
        reconnectDelay = 500;
      } else if (message.type === 'snapshot' || message.type === 'patch') {
        if (!Number.isSafeInteger(message.revision) || message.revision <= lastRevision)
          throw new Error('Invalid state revision');
        if (message.type === 'snapshot') snapshot(message);
        else {
          for (const [id, properties] of Object.entries(message.widgets)) {
            const widget = widgets.get(id);
            if (!widget) throw new Error('Unknown widget in update');
            widget.update(properties);
          }
        }
        lastRevision = message.revision;
        send({ type: 'ack', revision: message.revision });
        status.textContent = 'Connected';
      } else if (message.type === 'expired' || message.type === 'unavailable' || message.type === 'replaced') {
        stopped = true;
        ready = false;
        controls.disabled = true;
        status.textContent = message.message;
        newSession.hidden = false;
      } else if (message.type === 'error') status.textContent = message.message;
    } catch (error) {
      status.textContent = `Protocol error: ${error.message}`;
      current.close(); // Reconnect obtains a full authoritative snapshot.
    }
  });
  current.addEventListener('close', () => {
    if (current !== socket) return;
    ready = false;
    controls.disabled = true;
    if (stopped) return;
    status.textContent = 'Disconnected — reconnecting…';
    reconnectTimer = setTimeout(connect, reconnectDelay);
    reconnectDelay = Math.min(reconnectDelay * 2, 5000);
  });
}
newSession.addEventListener('click', () => {
  clearTimeout(reconnectTimer);
  socket?.close();
  socket = undefined;
  sessionStorage.removeItem(storageKey);
  token = '';
  stopped = false;
  newSession.hidden = true;
  connect();
});
window.addEventListener('pagehide', () => {
  stopped = true;
  clearTimeout(reconnectTimer);
  socket?.close();
});
window.addEventListener('pageshow', (event) => {
  if (event.persisted) { stopped = false; connect(); }
});
connect();
