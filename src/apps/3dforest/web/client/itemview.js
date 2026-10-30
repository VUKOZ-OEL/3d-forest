const el = (tag, cls, text) => {
  const e = document.createElement(tag);
  if (cls) e.className = cls;
  if (text !== undefined) e.textContent = text;
  return e;
};
export function createItemView(type, p, send) {
  const isTree = type === "TreeWidget",
    root = el("div", "item-view");
  root.tabIndex = 0;
  root.setAttribute("role", isTree ? "treegrid" : "grid");
  const table = el("table"),
    head = el("thead"),
    body = el("tbody");
  table.append(head, body);
  root.append(table);
  let rows = [],
    selection = new Set(),
    anchor = null,
    current = null,
    modelKey = "",
    renderKey = "",
    frame = 0;
  const collapsed = new Set();
  const key = (r, c) => (isTree ? String(r) : `${r}:${c}`);
  const sendEvent = (name, extra) =>
    send(name, { modelVersion: p.modelVersion, ...extra });
  const selected = (r, c) => selection.has(key(r, c));
  function selectionUpdate() {
    for (const cell of body.querySelectorAll("td[data-col]"))
      cell.classList.toggle(
        "selected",
        selected(cell.dataset.row, Number(cell.dataset.col)),
      );
  }
  function publishSelection() {
    if (isTree) sendEvent("selection", { paths: [...selection] });
    else if (p.selectionBehavior === 1)
      sendEvent("selection", {
        rows: [...new Set([...selection].map((s) => Number(s.split(":")[0])))],
      });
    else if (p.selectionBehavior === 2)
      sendEvent("selection", {
        columns: [
          ...new Set([...selection].map((s) => Number(s.split(":")[1]))),
        ],
      });
    else
      sendEvent("selection", {
        cells: [...selection].map((s) => s.split(":").map(Number)),
      });
  }
  function choose(row, column, event = {}) {
    if (!p.enabled || p.selectionMode === 0) return;
    const rid = row.id,
      idx = rows.findIndex((r) => r.id === rid),
      cols = isTree ? p.columns : p.model.headers.length;
    const ctrl = event.ctrlKey || event.metaKey,
      shift =
        event.shiftKey &&
        anchor &&
        (p.selectionMode === 3 || p.selectionMode === 4);
    let targets = [];
    function add(r, c) {
      if (isTree) targets.push(key(r.id, c));
      else {
        const flags = r.cells[c]?.flags || 0;
        if (flags & 1 && flags & 32) targets.push(key(r.id, c));
      }
    }
    if (shift) {
      const ai = Math.max(
        0,
        rows.findIndex((r) => r.id === anchor.row),
      );
      for (let n = Math.min(ai, idx); n <= Math.max(ai, idx); ++n) {
        if (isTree || p.selectionBehavior === 1) {
          for (let c = 0; c < cols; ++c) add(rows[n], c);
        } else if (p.selectionBehavior === 2) {
          for (const rr of rows)
            for (
              let c = Math.min(anchor.column, column);
              c <= Math.max(anchor.column, column);
              ++c
            )
              add(rr, c);
        } else
          for (
            let c = Math.min(anchor.column, column);
            c <= Math.max(anchor.column, column);
            ++c
          )
            add(rows[n], c);
      }
    } else {
      anchor = { row: rid, column };
      if (isTree) add(row, column);
      else if (p.selectionBehavior === 1)
        for (let c = 0; c < cols; ++c) add(row, c);
      else if (p.selectionBehavior === 2) for (const r of rows) add(r, column);
      else add(row, column);
    }
    const toggle =
      !shift && (p.selectionMode === 2 || (ctrl && p.selectionMode === 3));
    if (!toggle && !(ctrl && shift && p.selectionMode === 3)) selection.clear();
    const remove = toggle && targets.every((k) => selection.has(k));
    for (const k of targets) {
      if (remove) selection.delete(k);
      else selection.add(k);
    }
    current = { row: rid, column };
    selectionUpdate();
    publishSelection();
  }
  function edit(td, row, c) {
    const cell = row.cells[c];
    if (
      !p.enabled ||
      (isTree ? !row.editable : !(cell.flags & 2) || !(cell.flags & 32))
    )
      return;
    if (td.querySelector("input[type=text]")) return;
    const input = el("input");
    input.type = "text";
    input.value = cell.text;
    td.replaceChildren(input);
    input.focus();
    input.select();
    let done = false;
    const commit = () => {
      if (done) return;
      done = true;
      if (input.value !== cell.text)
        sendEvent("edit", {
          ...(isTree ? { path: row.id } : { row: Number(row.id) }),
          column: c,
          text: input.value,
        });
      renderKey = "";
      renderRows();
      root.focus();
    };
    input.addEventListener("blur", commit);
    input.addEventListener("keydown", (e) => {
      e.stopPropagation();
      if (e.key === "Enter") commit();
      if (e.key === "Escape") {
        done = true;
        renderKey = "";
        renderRows();
        root.focus();
      }
    });
  }
  function rebuildRows() {
    rows = [];
    if (isTree) {
      const visit = (items, depth) => {
        for (const item of items) {
          rows.push({ ...item, id: item.path, depth });
          if (!collapsed.has(item.path)) visit(item.children, depth + 1);
        }
      };
      visit(p.rows || [], 0);
    } else
      rows = (p.model?.rows || []).map((cells, i) => ({
        id: String(i),
        cells,
        depth: 0,
      }));
  }
  function renderHeaders() {
    const headers = isTree ? p.headers : p.model?.headers || [],
      tr = el("tr");
    if (!isTree && p.rowHeaderVisible) tr.append(el("th", "row-number"));
    headers.forEach((title, c) => {
      const th = el("th", "", title);
      th.scope = "col";
      if (p.sorting) {
        const sort = el(
          "span",
          "sort-indicator",
          p.sortColumn === c ? (p.sortOrder ? "▾" : "▴") : "",
        );
        th.append(sort);
        th.addEventListener("click", () =>
          sendEvent("sort", {
            column: c,
            order: p.sortColumn === c ? 1 - p.sortOrder : 0,
          }),
        );
        th.tabIndex = 0;
        th.addEventListener("keydown", (e) => {
          if (e.key === "Enter") th.click();
        });
      }
      if (!isTree) {
        const width = p.columnWidths?.[c] ?? p.defaultColumnWidth;
        if (width > 0) {
          th.style.width = `${width}px`;
          th.style.minWidth = `${width}px`;
        } else if (width === -1) {
          let len = title.length;
          for (const r of p.model.rows.slice(0, 500))
            len = Math.max(len, (r[c]?.text || "").length);
          th.style.minWidth = `${Math.min(500, len * 7 + 14)}px`;
        }
        const grip = el("span", "column-resizer");
        th.append(grip);
        grip.addEventListener("click", (e) => e.stopPropagation());
        grip.addEventListener("pointerdown", (e) => {
          e.preventDefault();
          e.stopPropagation();
          const x = e.clientX,
            w = th.getBoundingClientRect().width;
          grip.setPointerCapture(e.pointerId);
          const move = (e) => {
            th.style.width =
              th.style.minWidth = `${Math.max(20, w + e.clientX - x)}px`;
          };
          const up = (e) => {
            grip.removeEventListener("pointermove", move);
            grip.removeEventListener("pointerup", up);
            sendEvent("columnWidth", {
              column: c,
              width: Math.round(Math.max(20, w + e.clientX - x)),
            });
          };
          grip.addEventListener("pointermove", move);
          grip.addEventListener("pointerup", up);
        });
      }
      tr.append(th);
    });
    head.replaceChildren(tr);
    head.hidden = !isTree && !p.headerVisible;
  }
  function renderRows() {
    const height = p.rowHeight > 0 ? p.rowHeight : 30;
    const virtual = rows.length > 300;
    const first = virtual
      ? Math.max(0, Math.floor(root.scrollTop / height) - 8)
      : 0;
    const last = virtual
      ? Math.min(
          rows.length,
          first + Math.ceil((root.clientHeight || 500) / height) + 20,
        )
      : rows.length;
    const keyNow = `${modelKey}:${first}:${last}:${[...collapsed]}`;
    if (renderKey === keyNow) {
      selectionUpdate();
      return;
    }
    if (
      body.contains(document.activeElement) &&
      document.activeElement.tagName === "INPUT" &&
      document.activeElement.type === "text"
    )
      return;
    renderKey = keyNow;
    const fragment = document.createDocumentFragment();
    const spacer = (h) => {
      const tr = el("tr", "virtual-spacer"),
        td = el("td");
      td.colSpan = (isTree ? p.columns : p.model.headers.length) + 1;
      td.style.height = `${h}px`;
      tr.append(td);
      fragment.append(tr);
    };
    if (first) spacer(first * height);
    for (let n = first; n < last; ++n) {
      const row = rows[n],
        tr = el("tr");
      tr.style.height = `${height}px`;
      tr.dataset.row = row.id;
      if (!isTree && p.rowHeaderVisible) {
        const th = el("th", "row-number", String(Number(row.id) + 1));
        th.addEventListener("click", (e) => choose(row, 0, e));
        tr.append(th);
      }
      row.cells.forEach((cell, c) => {
        const td = el("td");
        td.dataset.row = row.id;
        td.dataset.col = c;
        td.tabIndex = -1;
        td.classList.toggle("selected", selected(row.id, c));
        if (isTree && c === 0) {
          td.style.paddingLeft = `${row.depth * 16 + 3}px`;
          const exp = el(
            "button",
            "tree-expander",
            collapsed.has(row.id) ? "▸" : "▾",
          );
          exp.hidden = !row.children.length;
          exp.tabIndex = -1;
          exp.addEventListener("click", (e) => {
            e.stopPropagation();
            if (collapsed.has(row.id)) collapsed.delete(row.id);
            else collapsed.add(row.id);
            rebuildRows();
            renderKey = "";
            renderRows();
          });
          td.append(exp);
        }
        if (cell.check >= 0) {
          const checkCell = el("span", "cell-check");
          const check = el("input");
          check.type = "checkbox";
          check.checked = cell.check === 2;
          check.indeterminate = cell.check === 1;
          check.disabled =
            !p.enabled ||
            (!isTree && (!(cell.flags & 16) || !(cell.flags & 32)));
          check.addEventListener("click", (e) => e.stopPropagation());
          check.addEventListener("change", () =>
            sendEvent("check", {
              ...(isTree ? { path: row.id } : { row: Number(row.id) }),
              column: c,
              state: check.checked ? 2 : 0,
            }),
          );
          checkCell.append(check);
          td.append(checkCell);
        }
        const label = el("span", "cell-label", cell.text);
        if (cell.background) {
          td.style.background = cell.background;
        }
        td.append(label);
        td.addEventListener("click", (e) => {
          root.focus({ preventScroll: true });
          choose(row, c, e);
          if (p.enabled)
            sendEvent("itemClicked", {
              ...(isTree ? { path: row.id } : { row: Number(row.id) }),
              column: c,
            });
        });
        td.addEventListener("dblclick", () => {
          if (isTree)
            sendEvent("itemDoubleClicked", { path: row.id, column: c });
          else edit(td, row, c);
        });
        tr.append(td);
      });
      fragment.append(tr);
    }
    if (last < rows.length) spacer((rows.length - last) * height);
    body.replaceChildren(fragment);
    root.classList.toggle("fit-rows", isTree && p.fitRowsToContents);
  }
  root.addEventListener("scroll", () => {
    cancelAnimationFrame(frame);
    frame = requestAnimationFrame(renderRows);
  });
  root.addEventListener("keydown", (e) => {
    if (e.target !== root) return;
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "a") {
      e.preventDefault();
      if (p.selectionMode === 0) return;
      selection.clear();
      const selectedRows = p.selectionMode === 1 ? rows.slice(0, 1) : rows;
      for (const row of selectedRows)
        for (let c = 0; c < row.cells.length; ++c)
          if (isTree || (row.cells[c].flags & 1 && row.cells[c].flags & 32))
            selection.add(key(row.id, c));
      selectionUpdate();
      publishSelection();
      return;
    }
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === "c") {
      e.preventDefault();
      const text = rows
        .filter((r) => r.cells.some((_, c) => selected(r.id, c)))
        .map((r) =>
          r.cells
            .filter((_, c) => selected(r.id, c))
            .map((c) => c.text)
            .join("\t"),
        )
        .join("\n");
      navigator.clipboard?.writeText(text).catch(() => {});
      return;
    }
    if (e.key === "F2" && current) {
      const td = Array.from(body.querySelectorAll("td")).find(
        (c) =>
          c.dataset.row === current.row &&
          Number(c.dataset.col) === current.column,
      );
      if (td)
        edit(
          td,
          rows.find((r) => r.id === current.row),
          current.column,
        );
      return;
    }
    if (
      [
        "ArrowDown",
        "ArrowUp",
        "ArrowLeft",
        "ArrowRight",
        "Home",
        "End",
      ].includes(e.key)
    ) {
      e.preventDefault();
      let r = current ? rows.findIndex((r) => r.id === current.row) : 0,
        c = current?.column || 0;
      r = Math.max(0, r);
      if (e.key === "ArrowDown") r++;
      if (e.key === "ArrowUp") r--;
      if (e.key === "ArrowLeft") c--;
      if (e.key === "ArrowRight") c++;
      if (e.key === "Home") r = 0;
      if (e.key === "End") r = rows.length - 1;
      r = Math.max(0, Math.min(rows.length - 1, r));
      c = Math.max(
        0,
        Math.min((isTree ? p.columns : p.model.headers.length) - 1, c),
      );
      if (rows[r]) {
        choose(rows[r], c, e);
        const h = p.rowHeight > 0 ? p.rowHeight : 30;
        if (r * h < root.scrollTop) root.scrollTop = r * h;
        else if ((r + 2) * h > root.scrollTop + root.clientHeight)
          root.scrollTop = (r + 2) * h - root.clientHeight;
      }
    }
  });
  function update() {
    const next = JSON.stringify([
      isTree ? p.rows : p.model,
      p.columnWidths,
      p.headerVisible,
      p.rowHeaderVisible,
      p.sortColumn,
      p.sortOrder,
      p.defaultColumnWidth,
      p.rowHeight,
    ]);
    selection = new Set(
      isTree ? [] : (p.selection || []).map(([r, c]) => key(r, c)),
    );
    if (isTree) {
      const visit = (rs) => {
        for (const r of rs) {
          if (r.selected) selection.add(r.path);
          visit(r.children);
        }
      };
      visit(p.rows || []);
    }
    root.classList.toggle("alternating", !!p.alternating);
    if (next !== modelKey) {
      modelKey = next;
      rebuildRows();
      renderHeaders();
      renderKey = "";
      renderRows();
    } else selectionUpdate();
  }
  return { root, update, dispose: () => cancelAnimationFrame(frame) };
}
