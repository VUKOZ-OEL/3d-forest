import streamlit as st
import pandas as pd
import plotly.graph_objects as go
import src.io as io
import src.utils as utils

# --- Konstanty ---
FILE_PATH   = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/LS-Krivoklat/3df_project/Krivoklat_local_xy.json"
UI_REV_KEY  = "map-ui"  # klíč pro uirevision

# --- Session state init ---
if "trees" not in st.session_state:
    st.session_state.trees        = io.load_project_json(FILE_PATH)
    st.session_state.colormap     = io.load_colormap(FILE_PATH)
if "tree_updates" not in st.session_state:
    st.session_state.tree_updates = {}
if "selected_points" not in st.session_state:
    st.session_state.selected_points = set()

# --- Připrav DataFrame ---
df = utils.prepare_tree_dataframe(st.session_state.trees, st.session_state.colormap)
for idx, status in st.session_state.tree_updates.items():
    if idx in df.index:
        df.at[idx, "management_status"] = status

species_map    = st.session_state.colormap.get("species", {})   or {}
management_map = st.session_state.colormap.get("management", {}) or {}

df["color"]      = df["species"].map(species_map).fillna("#aaaaaa")
df["line_color"] = df["management_status"].map(
    lambda s: management_map.get(s) if s in ["target", "remove"] else None
)
df["line_width"] = df["management_status"].map(
    lambda s: 2 if s in ["target", "remove"] else 0
)

# zvýraznění výběru
hc, hw = [], []
for _, row in df.iterrows():
    if row["id"] in st.session_state.selected_points:
        hc.append("#000000"); hw.append(2)
    else:
        hc.append(row["line_color"]); hw.append(row["line_width"])
df["line_color"] = hc
df["line_width"] = hw

# žádné None
df["line_color"] = df["line_color"].fillna("rgba(0,0,0,0)")
df["alpha"]      = 0.8
df["size"]       = 12

# Python listy pro Plotly
xs         = df["x"].tolist()
ys         = df["y"].tolist()
labels     = df["label"].tolist()
colors     = df["color"].tolist()
line_cols  = df["line_color"].tolist()
line_wds   = df["line_width"].tolist()
sizes      = df["size"].tolist()
opacities  = df["alpha"].tolist()

# --- UI tlačítka ---
col_buttons, col_map = st.columns([1, 5], gap="small")
with col_buttons:
    st.markdown("__Select management__")
    if st.button("Target tree"):
        for idx in st.session_state.selected_points:
            st.session_state.tree_updates[idx] = "target"
            st.session_state.trees[idx]["management_status"] = "target"
        st.session_state.selected_points.clear()
        st.rerun()
    if st.button("Remove"):
        for idx in st.session_state.selected_points:
            st.session_state.tree_updates[idx] = "remove"
            st.session_state.trees[idx]["management_status"] = "remove"
        st.session_state.selected_points.clear()
        st.rerun()
    if st.button("Unselect"):
        st.session_state.selected_points.clear()
        st.rerun()
    st.markdown("---")
    if st.button("Exportovat změny"):
        updated = utils.update_trees(st.session_state.trees, st.session_state.tree_updates)
        io.save_project_json(FILE_PATH, updated)
        st.success("Změny byly úspěšně exportovány.")

# --- Statická Plotly mapa vedle tlačítek ---
with col_map:
    fig = go.Figure(go.Scattergl(
        x=xs,
        y=ys,
        mode="markers",
        marker=dict(
            color=colors,
            size=sizes,
            line=dict(color=line_cols, width=line_wds),
            opacity=opacities
        ),
        text=labels,
        hovertemplate="Tree: %{text}<extra></extra>"
    ))
    fig.update_layout(
        title="Pick trees on map",
        height=600,
        margin=dict(l=0, r=0, t=30, b=0),
        dragmode="select",
        clickmode="event+select",
        uirevision=UI_REV_KEY
    )

    # nativní vykreslení
    st.plotly_chart(fig, use_container_width=True)
