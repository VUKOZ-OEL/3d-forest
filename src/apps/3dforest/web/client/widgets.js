import { createViewer } from "./viewer.js";
import { createItemView } from "./itemview.js";
export function element(tag, cls, text) {
  const e = document.createElement(tag);
  if (cls) e.className = cls;
  if (text !== undefined) e.textContent = text;
  return e;
}
export function richText(target, text) {
  const doc = new DOMParser().parseFromString(String(text), "text/html");
  const allowed = new Set([
    "P",
    "BR",
    "B",
    "STRONG",
    "I",
    "EM",
    "U",
    "S",
    "H1",
    "H2",
    "H3",
    "H4",
    "UL",
    "OL",
    "LI",
    "TABLE",
    "TBODY",
    "THEAD",
    "TR",
    "TH",
    "TD",
    "PRE",
    "CODE",
    "SPAN",
    "DIV",
    "A",
    "HR",
    "SUB",
    "SUP",
  ]);
  const clean = (node) => {
    if (node.nodeType === Node.TEXT_NODE)
      return document.createTextNode(node.textContent);
    if (node.nodeType !== Node.ELEMENT_NODE) return document.createTextNode("");
    if (
      ["SCRIPT", "STYLE", "IFRAME", "OBJECT", "SVG", "MATH", "IMG"].includes(
        node.tagName,
      )
    )
      return document.createTextNode("");
    const out = element(
      allowed.has(node.tagName) ? node.tagName.toLowerCase() : "span",
    );
    if (node.tagName === "A") {
      const href = node.getAttribute("href") || "";
      if (/^(https?:|mailto:)/i.test(href)) {
        out.href = href;
        out.target = "_blank";
        out.rel = "noopener noreferrer";
      }
    }
    for (const child of node.childNodes) out.append(clean(child));
    return out;
  };
  if (!/<\/?[a-z][^>]*>/i.test(text)) {
    target.textContent = text;
    return;
  }
  target.replaceChildren(...Array.from(doc.body.childNodes, clean));
}
function icon(button, p, only = false) {
  const image = element("img", "icon");
  image.alt = "";
  image.src = p.icon || "";
  image.hidden = !p.icon;
  button.replaceChildren(image);
  if (!only) button.append(document.createTextNode(p.text || ""));
  button.setAttribute("aria-label", p.text || p.toolTip || "Button");
}
function pointerRange(input, notify, release) {
  let frame = 0,
    drag = false;
  input.addEventListener("pointerdown", (e) => {
    drag = true;
    input.setPointerCapture(e.pointerId);
  });
  input.addEventListener("input", () => {
    cancelAnimationFrame(frame);
    frame = requestAnimationFrame(notify);
  });
  input.addEventListener("change", () => {
    cancelAnimationFrame(frame);
    notify();
  });
  const up = () => {
    if (!drag) return;
    drag = false;
    cancelAnimationFrame(frame);
    release();
  };
  for (const name of ["pointerup", "pointercancel", "lostpointercapture"])
    input.addEventListener(name, up);
  return () => drag;
}
export function createWidget(spec, emit) {
  const id = spec.id,
    type = spec.type,
    p = { ...spec.properties };
  const root = element(
    type === "GroupBox" ? "fieldset" : "div",
    `widget ${type}`,
  );
  root.dataset.widgetId = id;
  const content = element("div", "widget-content");
  let viewer = null;
  let apply = () => {},
    dispose = () => {};
  let pendingSequence = 0,
    canApply = true;
  const send = (name, extra = {}) => {
    pendingSequence = emit(id, name, extra) || pendingSequence;
  };
  if (type === "TableWidget" || type === "TreeWidget") {
    const view = createItemView(type, p, send);
    root.append(view.root);
    apply = (patch) => {
      view.update(patch);
      root.classList.toggle("fit-rows", type === "TreeWidget" && p.fitRowsToContents);
    };
    dispose = view.dispose || dispose;
  } else if (type === "CheckBox" || type === "RadioButton") {
    const label = element("label", "check-label");
    const input = element("input");
    input.type = type === "CheckBox" ? "checkbox" : "radio";
    const text = element("span");
    label.append(input, text);
    root.append(label);
    input.addEventListener("change", () =>
      send("toggled", { checked: input.checked }),
    );
    apply = () => {
      input.checked = p.checked;
      input.disabled = !p.enabled;
      text.textContent = p.text;
    };
  } else if (type === "ComboBox") {
    const input = element("select");
    root.append(input);
    let previous = "";
    input.addEventListener("change", () =>
      send("activated", { value: input.selectedIndex }),
    );
    apply = () => {
      const next = JSON.stringify(p.items);
      if (next !== previous) {
        input.replaceChildren(...p.items.map((t) => element("option", "", t)));
        previous = next;
      }
      input.selectedIndex = p.value;
      input.disabled = !p.enabled;
    };
  } else if (type === "PushButton" || type === "ToolButton") {
    const button = element("button");
    button.type = "button";
    root.append(button);
    let previous = "";
    button.addEventListener("click", () => send("clicked"));
    apply = () => {
      const next = JSON.stringify([p.text, p.icon, p.buttonStyle]);
      if (next !== previous) {
        icon(
          button,
          p,
          type === "ToolButton" && p.buttonStyle === 0 && !!p.icon,
        );
        previous = next;
      }
      button.disabled = !p.enabled;
      button.classList.toggle("auto-raise", !!p.autoRaise);
      button.classList.toggle("primary-action", !!p.primary);
      button.classList.toggle("text-under", p.buttonStyle === 3);
    };
  } else if (type === "LineEdit") {
    const input = element("input");
    input.type = "text";
    root.append(input);
    const breadcrumbs = p.name === "file-location"
      ? element("nav", "file-breadcrumbs") : null;
    let breadcrumbPath;
    if (breadcrumbs) {
      breadcrumbs.setAttribute("aria-label", "Folder path");
      breadcrumbs.addEventListener("keydown", (e) => {
        if (e.key === "Enter") e.stopPropagation();
      });
      root.append(breadcrumbs);
      input.setAttribute("aria-label", "Folder path");
      input.addEventListener("focus", () => root.classList.add("editing-path"));
      input.addEventListener("blur", () => root.classList.remove("editing-path"));
      breadcrumbs.addEventListener("click", (e) => {
        if (e.target === breadcrumbs) input.focus();
      });
    }
    input.addEventListener("input", () =>
      send("textChanged", { text: input.value }),
    );
    input.addEventListener("blur", () => {
      if (!input.readOnly) send("editingFinished", { text: input.value });
    });
    input.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        e.stopPropagation();
        if (!input.readOnly) {
          send("returnPressed", { text: input.value });
          send("editingFinished", { text: input.value });
        }
      }
    });
    apply = () => {
      if (canApply && input.value !== p.text) {
        const start = input.selectionStart,
          end = input.selectionEnd;
        input.value = p.text;
        if (document.activeElement === input && start !== null)
          input.setSelectionRange(
            Math.min(start, input.value.length),
            Math.min(end, input.value.length),
          );
      }
      input.placeholder = p.placeholder || "";
      input.readOnly = p.readOnly;
      input.disabled = !p.enabled;
      if (breadcrumbs && breadcrumbPath !== p.text) {
        breadcrumbPath = p.text;
        breadcrumbs.replaceChildren();
        const parts = (p.text || "").split(/[\\/]/).filter(Boolean);
        parts.forEach((part, index) => {
          const button = element("button", "", part);
          button.type = "button";
          button.disabled = !p.enabled;
          if (index === parts.length - 1)
            button.setAttribute("aria-current", "location");
          button.addEventListener("click", () =>
            send("returnPressed", { text: parts.slice(0, index + 1).join("/") }));
          breadcrumbs.append(button, element("span", "file-path-separator", "›"));
        });
        const edit = element("button", "file-edit-path", "⌄");
        edit.title = "Edit folder path";
        edit.addEventListener("click", () => { input.focus(); input.select(); });
        breadcrumbs.append(edit);
      }
    };
  } else if (type === "TextEdit") {
    const input = element("div", "text-editor");
    input.setAttribute("role", "textbox");
    input.setAttribute("aria-multiline", "true");
    root.append(input);
    let previous;
    input.addEventListener("input", () =>
      send("textChanged", { text: input.innerHTML }),
    );
    apply = () => {
      if (canApply && p.text !== previous && input.innerHTML !== p.text) {
        richText(input, p.text);
        previous = p.text;
      }
      input.contentEditable = String(p.enabled && !p.readOnly);
      input.tabIndex = 0;
    };
  } else if (type === "SpinBox" || type === "DoubleSpinBox") {
    const input = element("input");
    input.type = "number";
    root.append(input);
    const arrows = element("span", "spin-arrows"),
      up = element("button", "", "⌃"),
      down = element("button", "", "⌄");
    up.type = down.type = "button";
    up.tabIndex = down.tabIndex = -1;
    up.setAttribute("aria-label", "Increase");
    down.setAttribute("aria-label", "Decrease");
    arrows.append(up, down);
    root.append(arrows);
    for (const [button, sign] of [
      [up, 1],
      [down, -1],
    ]) {
      button.addEventListener("pointerdown", (e) => e.preventDefault());
      button.addEventListener("click", () => {
        input.focus();
        const next = Math.max(
          p.minimum,
          Math.min(
            p.maximum,
            (Number(input.value) || 0) + sign * (p.singleStep || 0),
          ),
        );
        input.value =
          type === "DoubleSpinBox"
            ? Number(next.toFixed(p.decimals ?? 2))
            : next;
        send("valueChanged", { value: Number(input.value) });
      });
    }
    const value = () => Number(input.value);
    const valid = () =>
      input.value !== "" &&
      Number.isFinite(value()) &&
      value() >= p.minimum &&
      value() <= p.maximum &&
      (type !== "SpinBox" || Number.isInteger(value()));
    input.addEventListener("input", () => {
      if (valid()) send("valueChanged", { value: value() });
    });
    const finish = () => {
      if (valid()) send("editingFinished", { value: value() });
      else input.value = p.value;
    };
    input.addEventListener("blur", finish);
    input.addEventListener("keydown", (e) => {
      if (e.key === "Enter") {
        e.stopPropagation();
        finish();
      }
    });
    apply = () => {
      input.min = p.minimum;
      input.max = p.maximum;
      input.step = p.singleStep || "any";
      if (
        canApply &&
        (document.activeElement !== input || Number(input.value) !== p.value)
      )
        input.value =
          type === "DoubleSpinBox"
            ? Number(p.value).toFixed(p.decimals ?? 2)
            : p.value;
      input.disabled = !p.enabled;
      up.disabled = down.disabled = !p.enabled;
    };
  } else if (type === "Slider") {
    const input = element("input");
    input.type = "range";
    input.step = "1";
    root.append(input);
    const ticks = element("datalist");
    ticks.id = `${id}-ticks`;
    root.append(ticks);
    const dragging = pointerRange(
      input,
      () => send("valueChanged", { value: Number(input.value) }),
      () => send("sliderReleased", { value: Number(input.value) }),
    );
    const paint = () => {
      const span = p.maximum - p.minimum || 1;
      input.style.setProperty(
        "--slider-position",
        `${Math.max(0, Math.min(100, ((Number(input.value) - p.minimum) / span) * 100))}%`,
      );
    };
    input.addEventListener("input", paint);
    input.addEventListener("keydown", (e) => {
      const delta = { ArrowLeft: -1, ArrowDown: -1, ArrowRight: 1, ArrowUp: 1 }[
        e.key
      ];
      if (delta) {
        e.preventDefault();
        input.value = Math.max(
          p.minimum,
          Math.min(p.maximum, Number(input.value) + delta * p.singleStep),
        );
        paint();
        send("valueChanged", { value: Number(input.value) });
      }
    });
    let previous = "";
    apply = () => {
      input.min = p.minimum;
      input.max = p.maximum;
      if (!dragging()) input.value = p.value;
      paint();
      input.disabled = !p.enabled;
      input.classList.toggle("vertical", p.orientation === 2);
      input.setAttribute("aria-label", p.name || p.toolTip || "Slider");
      const key = JSON.stringify([
        p.minimum,
        p.maximum,
        p.tickInterval,
        p.tickPosition,
      ]);
      if (key !== previous) {
        ticks.replaceChildren();
        const step = p.tickInterval || p.singleStep || 1;
        if (p.tickPosition && (p.maximum - p.minimum) / step <= 1000) {
          for (let v = p.minimum; v <= p.maximum; v += step) {
            const o = element("option");
            o.value = v;
            ticks.append(o);
          }
          input.setAttribute("list", ticks.id);
        } else input.removeAttribute("list");
        previous = key;
      }
    };
  } else if (type === "DoubleRangeSlider") {
    const track = element("div", "range-track"),
      fill = element("div", "range-fill");
    track.append(fill);
    const low = element("input", "range-low"),
      high = element("input", "range-high");
    low.type = high.type = "range";
    low.step = high.step = "any";
    root.append(track, low, high);
    low.setAttribute("aria-label", "Minimum");
    high.setAttribute("aria-label", "Maximum");
    const values = () => ({ low: Number(low.value), high: Number(high.value) });
    const paint = () => {
      const span = p.maximum - p.minimum || 1;
      const a = ((Number(low.value) - p.minimum) / span) * 100,
        b = ((Number(high.value) - p.minimum) / span) * 100;
      fill.style.left = `${a}%`;
      fill.style.width = `${b - a}%`;
    };
    low.addEventListener("input", () => {
      if (Number(low.value) > Number(high.value)) low.value = high.value;
      paint();
    });
    high.addEventListener("input", () => {
      if (Number(high.value) < Number(low.value)) high.value = low.value;
      paint();
    });
    const changed = () => send("valuesChanged", values()),
      released = () => send("sliderReleased", values());
    const dragLow = pointerRange(low, changed, released),
      dragHigh = pointerRange(high, changed, released);
    for (const input of [low, high])
      input.addEventListener("keydown", (e) => {
        const sign = {
          ArrowLeft: -1,
          ArrowDown: -1,
          ArrowRight: 1,
          ArrowUp: 1,
        }[e.key];
        if (sign) {
          e.preventDefault();
          let v = Number(input.value) + sign * (p.singleStep || 1);
          input.value = Math.max(
            p.minimum,
            Math.min(
              p.maximum,
              input === low
                ? Math.min(v, Number(high.value))
                : Math.max(v, Number(low.value)),
            ),
          );
          paint();
          changed();
        }
      });
    apply = () => {
      for (const input of [low, high]) {
        input.min = p.minimum;
        input.max = p.maximum;
        input.disabled = !p.enabled;
      }
      if (!dragLow() && !dragHigh()) {
        low.value = p.low;
        high.value = p.high;
      }
      root.classList.toggle("vertical-range", p.orientation === 2);
      paint();
    };
  } else if (type === "ProgressBar") {
    const progress = element("progress"),
      label = element("span", "progress-label");
    root.append(progress, label);
    apply = () => {
      progress.max = Math.max(1, p.maximum - p.minimum);
      if (p.maximum === 0 && p.minimum === 0) progress.removeAttribute("value");
      else progress.value = Math.max(0, p.value - p.minimum);
      label.textContent =
        p.value >= p.minimum
          ? `${Math.round(((p.value - p.minimum) / (p.maximum - p.minimum || 1)) * 100)}%`
          : "";
      label.hidden = !p.textVisible;
    };
  } else if (type === "Label") {
    const img = element("img", "icon"),
      text = element("span");
    root.append(img, text);
    let previous;
    apply = () => {
      if (previous !== p.text) {
        richText(text, p.text || "");
        previous = p.text;
      }
      img.hidden = !p.icon;
      if (p.icon) img.src = p.icon;
    };
  } else if (type === "GroupBox") {
    const legend = element("legend");
    root.append(legend);
    apply = () => {
      legend.textContent = p.title || "";
    };
  } else if (type === "ColorSwitch") {
    const bg = element("input", "color-background"),
      fg = element("input", "color-foreground");
    fg.type = bg.type = "color";
    fg.title = "Foreground color";
    bg.title = "Background color";
    const swap = element("button", "color-swap", "↔"),
      reset = element("button", "color-reset", "◩");
    swap.title = "Swap colors";
    reset.title = "Default colors";
    root.append(bg, fg, swap, reset);
    const rgba = (s) => [
      parseInt(s.slice(1, 3), 16),
      parseInt(s.slice(3, 5), 16),
      parseInt(s.slice(5, 7), 16),
      s.length === 9 ? parseInt(s.slice(7, 9), 16) : 255,
    ];
    const sendColors = () =>
      send("colorChanged", {
        foreground: rgba(fg.value),
        background: rgba(bg.value),
      });
    fg.addEventListener("change", sendColors);
    bg.addEventListener("change", sendColors);
    swap.addEventListener("click", () => {
      [fg.value, bg.value] = [bg.value, fg.value];
      sendColors();
    });
    reset.addEventListener("click", () => {
      fg.value = "#ffffff";
      bg.value = "#333333";
      sendColors();
    });
    apply = () => {
      fg.value = (p.foreground || "#000000").slice(0, 7);
      bg.value = (p.background || "#ffffff").slice(0, 7);
      for (const e of [fg, bg, swap, reset]) e.disabled = !p.enabled;
    };
  } else if (type === "Viewer") {
    root.setAttribute("aria-label", "3D viewport");
    viewer = createViewer(root, id, emit);
    apply = () => viewer.update(p);
    dispose = () => viewer.dispose();
  } else if (type === "ToolBar") {
    apply = () => {
      root.classList.toggle("vertical-toolbar", p.orientation === 2);
      root.style.setProperty("--icon-size", `${p.iconWidth || 16}px`);
    };
  } else if (type === "Splitter") {
    apply = (patch) => {
      content.classList.toggle("vertical-splitter", p.orientation === 2);
      if (patch.sizes) {
        const children = [...content.children].filter((e) =>
          e.classList.contains("widget"),
        );
        children.forEach((e, i) => (e.style.flex = `${p.sizes?.[i] || 1} 1 0`));
      }
    };
  }
  root.append(content);
  function update(patch, sequence = Number.MAX_SAFE_INTEGER) {
    canApply = sequence >= pendingSequence;
    Object.assign(p, patch);
    apply(patch);
    root.hidden = p.visible === false;
    root.dataset.widgetName = p.name || "";
    root.title = p.toolTip || "";
    root.setAttribute("aria-disabled", String(!p.enabled));
  }
  update({});
  return {
    id,
    type,
    root,
    content,
    update,
    dispose,
    properties: p,
    receiveBinary: (data) => viewer?.receiveBinary(data),
    resetPending: () => {
      viewer?.reconnect();
      pendingSequence = 0;
    },
  };
}
