import streamlit as st
import pandas as pd
import json
import src.io as io
import numpy as np
import plotly.graph_objects as go

# --- Načtení a inicializace ---
if "trees" not in st.session_state:
    #file_path = ("c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v11.json")
    file_path = ("c:/data/1.json")
    st.session_state.trees = io.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()

# Požadované sloupce
required = {"x", "y", "dbh", "label", "speciesColorHex", "managementColorHex"}
missing = required - set(df.columns)
if missing:
    st.error(f"Chybí sloupce: {', '.join(sorted(missing))}")
    st.stop()

# ---------------- UI ----------------
c1, c2, c3, c4 = st.columns([1.4, 1, 1, 1.2])
with c1:
    color_mode = st.radio(
        "Color by",
        options=["Species", "Management"],
        horizontal=True,
        index=0
    )
with c2:
    size_min = st.slider("Min Point Size (px)", 1, 20, 6, 1)
with c3:
    size_max = st.slider("Max Point Size (px)", 20, 60, 28, 1)
with c4:
    show_text = st.checkbox("Show Tree ID", value=False)

# ---------------- Data prep ----------------
color_col = "speciesColorHex" if color_mode == "Species" else "managementColorHex"

def sanitize_hex(s):
    if isinstance(s, str) and s.startswith("#") and len(s) == 7:
        return s.upper()
    return "#AAAAAA"

colors = df[color_col].astype(str).apply(sanitize_hex)

# Velikosti dle DBH
dbh = pd.to_numeric(df["dbh"], errors="coerce")
dbh = dbh.fillna(dbh.median() if not np.isnan(dbh.median()) else 1.0)
if dbh.min() == dbh.max():
    sizes = np.full(len(df), (size_min + size_max) / 2.0)
else:
    sizes = np.interp(dbh, (dbh.min(), dbh.max()), (size_min, size_max))

# --- customdata: drž konzistentní pořadí: label, dbh, species, management_status
cd_parts = [df["label"].astype(str), dbh.astype(float)]
if "species" in df.columns:
    cd_parts.append(df["species"].astype(str))
if "management_status" in df.columns:
    cd_parts.append(df["management_status"].astype(str))

customdata = np.column_stack(cd_parts)

# Hover template přesně podle pořadí v customdata
hover_lines = [
    "label: %{customdata[0]}",
    "DBH: %{customdata[1]} cm",
]
idx = 2
if "species" in df.columns:
    hover_lines.append("Species: %{customdata[" + str(idx) + "]}")
    idx += 1
if "management_status" in df.columns:
    hover_lines.append("Management: %{customdata[" + str(idx) + "]}")

hovertemplate = "<br>".join(hover_lines) + "<br><extra></extra>"

# ---------------- Plot ----------------
# Příprava grupování pro legendu
if color_mode == "Species":
    group_col = "species" if "species" in df.columns else None
else:
    group_col = "management_status" if "management_status" in df.columns else None

scatter_cls = go.Scatter if show_text else go.Scattergl
mode = "markers+text" if show_text else "markers"

fig = go.Figure()

# Klíč ke skupinám (název do legendy) a barva z příslušného hex sloupce
group_keys = df[group_col] if group_col else df[color_col]
group_colors = colors  # z dřívějška

# Vytvoř mapu: (legend_key, hex_color) -> indexy řádků
# (kombinace zajistí, že druh se stejným názvem ale jinou barvou skončí odděleně)
from collections import defaultdict
buckets = defaultdict(list)
for i, (gk, col) in enumerate(zip(group_keys, group_colors)):
    buckets[(str(gk), col)].append(i)

for (legend_name, hex_color), idxs in buckets.items():
    # vyber subset
    xs = df["x"].iloc[idxs]
    ys = df["y"].iloc[idxs]
    txt = df["label"].iloc[idxs] if show_text else None
    subset_sizes = sizes[idxs]
    subset_custom = customdata[idxs, :] if customdata.ndim == 2 else customdata

    fig.add_trace(
        scatter_cls(
            x=xs,
            y=ys,
            mode=mode,
            name=legend_name,          # ← položka do legendy
            showlegend=True,
            text=txt,
            textposition="top center",
            marker=dict(
                size=subset_sizes,
                color=hex_color,       # jedna barva na trace (pro legendu)
                line=dict(width=0),
                sizemode="diameter",
            ),
            customdata=subset_custom,
            hovertemplate=hovertemplate,
        )
    )

# Mapový poměr stran 1:1 (případně odkomentuj autoreversed pokud máš y obráceně)
fig.update_yaxes(scaleanchor="x", scaleratio=1)
# fig.update_yaxes(autorange="reversed")

fig.update_layout(
    height=600,
    dragmode="pan",
    margin=dict(l=10, r=10, t=30, b=10),
    legend_title_text=color_mode,  # "Species" nebo "Management"
    legend=dict(itemsizing="trace", title_font=dict(size=12)),
)

st.plotly_chart(fig, use_container_width=True)
