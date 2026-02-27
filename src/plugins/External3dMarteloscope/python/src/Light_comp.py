# -*- coding: utf-8 -*-
# ------------------------------------------------------------
# Streamlit: Competition for Light (percent-based)
# 1) Bubble: Avg available light (%)
# 2) Bars by Species: who competes (sum of % shading by neighbors)
# 3) Bars by Management: who competes (sum of % shading by neighbors)
# Filters: Stand State + DBH + Height; Species & Management (multi-select) BELOW charts
# ------------------------------------------------------------

import json
import math
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import streamlit as st
import src.io_utils as iou

from src.i18n import t,t_help,t_mgmt

# --- panel names from session (already localized elsewhere) ---
Before = st.session_state.Before
After = st.session_state.After
Removed = st.session_state.Removed
stand_stat_all = [Before, After]

st.markdown(f"##### {t('light_comp_pg_title')}")

# ---------- DATA ----------
if "trees" not in st.session_state:
    file_path = st.session_state.project_file
    st.session_state.trees = iou.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()

# ---------- HELPERS ----------
def _is_num(x) -> bool:
    try:
        float(x)
        return True
    except Exception:
        return False


def _to_comp_map(v) -> dict:
    """Parse light_comp; supports dict, JSON-str, or None. Keys -> int(treeId), values -> float(%)."""
    if v is None or (isinstance(v, float) and np.isnan(v)):
        return {}
    if isinstance(v, dict):
        return {int(k): float(vv) for k, vv in v.items() if _is_num(vv)}
    if isinstance(v, str):
        try:
            parsed = json.loads(v)
            if isinstance(parsed, dict):
                return {int(k): float(vv) for k, vv in parsed.items() if _is_num(vv)}
        except Exception:
            return {}
    return {}


def _species_colors(d: pd.DataFrame) -> dict:
    if "species" not in d.columns or "speciesColorHex" not in d.columns:
        return {}
    return (
        d.assign(species=lambda x: x["species"].astype(str))
        .groupby("species")["speciesColorHex"]
        .first()
        .to_dict()
    )


def _management_colors(d: pd.DataFrame) -> dict:
    if "management_status" not in d.columns or "managementColorHex" not in d.columns:
        return {}
    cmap = (
        d.assign(ms=lambda x: x["management_status"].astype(str))
        .groupby("ms")["managementColorHex"]
        .first()
        .to_dict()
    )
    return {k: (v if isinstance(v, str) and v.strip() else "#AAAAAA") for k, v in cmap.items()}


def _make_masks(d: pd.DataFrame):
    """Stand state masks for focal-trees by their own management_status."""
    keep_status = {"Target tree", "Untouched"}
    mask_after = d.get("management_status", pd.Series(False, index=d.index)).isin(keep_status)
    mask_removed = ~mask_after if "management_status" in d.columns else pd.Series(False, index=d.index)
    mask_before = pd.Series(True, index=d.index)
    return {Before: mask_before, After: mask_after, Removed: mask_removed}


# ---------- STATIC LISTS ----------
sp_all = iou._unique_sorted(df.get("species", pd.Series(dtype=object)))
mg_all = iou._unique_sorted(df.get("management_status", pd.Series(dtype=object)))

# ---------- CANONICALIZE COLUMNS ----------
# Fallback pro starší export: 'alight_avail'
if "light_avail" not in df.columns and "alight_avail" in df.columns:
    df["light_avail"] = df["alight_avail"]

# Zajistit existenci a rozsah 0..100 pro light_avail
df["light_avail"] = (
    pd.to_numeric(df.get("light_avail", 0.0), errors="coerce")
    .fillna(0.0)
    .clip(lower=0.0, upper=100.0)
)

# Bezpečné sestavení mapy sousedů (light_comp_map)
_src_comp = df["light_comp"] if "light_comp" in df.columns else pd.Series([{}] * len(df), index=df.index)

def _to_comp_map_safe(v):
    try:
        return _to_comp_map(v)
    except Exception:
        return {}

df["light_comp_map"] = _src_comp.apply(_to_comp_map_safe)

# ---------- DBH & HEIGHT FILTERS (values for UI) ----------
dbh_series = pd.to_numeric(df.get("dbh", np.nan), errors="coerce").dropna()
DBHmax = int(dbh_series.max()) if len(dbh_series) > 0 and np.isfinite(dbh_series.max()) else 0
DBHmax = max(DBHmax, 0)

h_series = pd.to_numeric(df.get("height", np.nan), errors="coerce")
Hmax = int(np.nanmax(h_series)) if np.isfinite(np.nanmax(h_series)) else 0
Hmax = max(0, Hmax)

# ---------- TOP CONTROLS ----------
_, c1, _, c2, _, c3, _, c4, _ = st.columns([0.5, 3, 0.5, 3, 0.5, 2, 0.5, 2, 0.5])

with c1:
    stand_state = st.segmented_control(
        f"**{t('select_stand_state')}**",
        options=stand_stat_all,
        default=Before,
        selection_mode="single",
        width="stretch",
        help=t("help_select_stand_state_light"),  # NEW KEY
    )

with c2:
    chart_mode = st.segmented_control(
        f"**{t('show_mode')}**",
        options=[t("who_competes"), t("sky_view_values")],
        default=t("who_competes"),
        selection_mode="single",
        width="stretch",
        help=t("help_chart_mode_light"),  # NEW KEY
    )

with c3:
    dbh_min, dbh_max = st.slider(
        f"**{t('dbh_filter')}**",
        min_value=0,
        max_value=max(DBHmax, 0 if DBHmax > 0 else 1),
        value=(0, max(DBHmax, 0 if DBHmax > 0 else 1)),
        step=1,
        help=t("dbh_filter_help"),
    )

with c4:
    height_min, height_max = st.slider(
        f"**{t('height_filter')}**",
        min_value=0,
        max_value=Hmax,
        value=(0, Hmax),
        step=1,
        help=t("height_filter_help"),
    )

# ---------- LAYOUT: CHART + FILTERS BELOW ----------
c_bot1, c_bot2 = st.columns([2, 15])
with c_bot1:

    # Inicializace pouze při prvním běhu
    if "light_species_sel" not in st.session_state:
        st.session_state["light_species_sel"] = sp_all.copy()

    st.pills(
        f"**{t('filter_species')}:**",
        options=sp_all if sp_all else ["(none)"],
        selection_mode="multi",
        key="light_species_sel",
    )
    
with c_bot2:
    #st.plotly_chart(fig, use_container_width=True)
    chart_placeholder = st.empty()

c31, c32 = st.columns([2, 15])
with c32:

    # 1️⃣ Nejprve vytvořit mapování
    mgmt_label_map = {m: t_mgmt(m) for m in mg_all}
    mgmt_labels = list(mgmt_label_map.values())
    label_to_mgmt = {v: k for k, v in mgmt_label_map.items()}

    # 2️⃣ Inicializace jen při prvním běhu
    if "light_mgmt_sel_labels" not in st.session_state:
        st.session_state["light_mgmt_sel_labels"] = mgmt_labels.copy()
        st.session_state["light_mgmt_sel"] = mg_all.copy()

    # 3️⃣ Widget
    light_mgmt_sel_labels = st.pills(
        f"**{t('filter_management')}:**",
        options=mgmt_labels if mgmt_labels else ["(none)"],
        selection_mode="multi",
        help=t("filter_management_help"),
        key="light_mgmt_sel_labels",
    )

    # 4️⃣ Synchronizace interní hodnoty
    st.session_state["light_mgmt_sel"] = [
        label_to_mgmt[lbl] for lbl in light_mgmt_sel_labels if lbl in label_to_mgmt
    ]



# ---------- READ CURRENT FILTER VALUES FROM SESSION (species / mgmt) ----------
light_species_sel = st.session_state.get("light_species_sel", sp_all if sp_all else ["(none)"])
mgmt_sel = st.session_state.get("mgmt_sel", mg_all if mg_all else ["(none)"])

light_species_sel = list(light_species_sel) if isinstance(light_species_sel, (list, tuple, set)) else [light_species_sel]
mg_sel_set = set(mgmt_sel) if isinstance(mgmt_sel, (list, set, tuple)) else {mgmt_sel}

# ---------- BASE MASKS ----------
state_masks = _make_masks(df)
mask_state = state_masks.get(stand_state, pd.Series(True, index=df.index))

# DBH mask
mask_dbh = pd.Series(True, index=df.index)
if "dbh" in df.columns:
    d_vals = pd.to_numeric(df["dbh"], errors="coerce")
    mask_dbh = (d_vals >= dbh_min) & (d_vals <= dbh_max)

# HEIGHT mask
mask_height = pd.Series(True, index=df.index)
if "height" in df.columns:
    h_vals = pd.to_numeric(df["height"], errors="coerce")
    mask_height = (h_vals >= height_min) & (h_vals <= height_max)

# Species mask
mask_species = pd.Series(True, index=df.index)
if light_species_sel and "(none)" not in light_species_sel and "species" in df.columns:
    mask_species = df["species"].astype(str).isin(light_species_sel)

# Management mask
mask_mgmt = pd.Series(True, index=df.index)
if mg_sel_set and "(none)" not in mg_sel_set and "management_status" in df.columns:
    mask_mgmt = df["management_status"].astype(str).isin(mg_sel_set)

# Focal trees
focal_mask = mask_dbh & mask_height & mask_state & mask_species & mask_mgmt
df_focal = df.loc[focal_mask].copy()

# Sada ID odstraněných stromů (globálně podle masky Removed)
mask_removed_global = state_masks[Removed]
removed_ids = set(
    pd.to_numeric(df.loc[mask_removed_global, "id"], errors="coerce")
    .dropna().astype(int).tolist()
)

# ---------- RECOMPUTE light_avail / light_comp for After/Removed ----------
def _adjust_light(row, do_adjust: bool):
    """Return (adj_light_avail, adj_comp_map) for row based on stand_state."""
    la = float(row.get("light_avail", 0.0))
    cmap = row.get("light_comp_map", {})
    if not isinstance(cmap, dict):
        cmap = {}

    if not do_adjust or not removed_ids:
        return la, cmap

    add_back = 0.0
    kept = {}
    for nid, pct in cmap.items():
        try:
            nid_int = int(nid)
        except Exception:
            continue
        if nid_int in removed_ids:
            add_back += float(pct)
        else:
            kept[nid_int] = float(pct)
    la_new = min(100.0, max(0.0, la + add_back))
    return la_new, kept

do_adjust = stand_state in (After, Removed)

if not df_focal.empty:
    adj_vals = df_focal.apply(lambda r: _adjust_light(r, do_adjust), axis=1)
    df_focal["light_avail_adj"] = [v[0] for v in adj_vals]
    df_focal["light_comp_adj"] = [v[1] for v in adj_vals]
else:
    df_focal["light_avail_adj"] = []
    df_focal["light_comp_adj"] = []

# ---------- NEIGHBOR ATTRS ----------
neighbor_attrs = df[["id", "species", "management_status", "speciesColorHex", "managementColorHex"]].copy()
neighbor_attrs["id"] = pd.to_numeric(neighbor_attrs["id"], errors="coerce").astype("Int64")

# ---------- EXPLODE adjusted light_comp -> df_comp ----------
if not df_focal.empty:
    df_comp = (
        df_focal[["light_comp_adj"]]
        .assign(tmp=lambda tdf: tdf["light_comp_adj"].apply(lambda d: list(d.items())))
        .explode("tmp", ignore_index=True)
    )
    df_comp = df_comp.dropna(subset=["tmp"])
    df_comp[["neighbor_id", "shade_pct"]] = pd.DataFrame(df_comp["tmp"].tolist(), index=df_comp.index)
    df_comp = df_comp.drop(columns=["tmp"])
    df_comp["neighbor_id"] = pd.to_numeric(df_comp["neighbor_id"], errors="coerce").astype("Int64")
    df_comp["shade_pct"] = pd.to_numeric(df_comp["shade_pct"], errors="coerce").fillna(0.0).clip(lower=0.0)
else:
    df_comp = pd.DataFrame(columns=["neighbor_id", "shade_pct"])

df_comp = df_comp.merge(neighbor_attrs, left_on="neighbor_id", right_on="id", how="left")

# ---------- AGGREGATION (WHO COMPETES) ----------
spec_df = (
    df_comp.groupby("species", as_index=False)["shade_pct"].sum()
    if not df_comp.empty else pd.DataFrame(columns=["species", "shade_pct"])
).rename(columns={"shade_pct": "value"})

mgmt_df = (
    df_comp.groupby("management_status", as_index=False)["shade_pct"].sum()
    if not df_comp.empty else pd.DataFrame(columns=["management_status", "shade_pct"])
).rename(columns={"shade_pct": "value"})

species_all = sorted(df["species"].astype(str).unique().tolist()) if "species" in df.columns else []
mgmt_all = df["management_status"].astype(str).unique().tolist() if "management_status" in df.columns else []

if species_all:
    spec_df = spec_df.set_index("species").reindex(species_all, fill_value=0.0).reset_index()
if mgmt_all:
    mgmt_df = mgmt_df.set_index("management_status").reindex(mgmt_all, fill_value=0.0).reset_index()

# color maps
species_cmap = _species_colors(df)
mgmt_cmap = _management_colors(df)

# ---------- BUBBLE: Average available light ----------
avg_light = float(df_focal["light_avail_adj"].mean()) if not df_focal.empty else 0.0
p_light = max(0.0, min(100.0, avg_light)) / 100.0

bubble_title = t("avg_avail_light_label")

if chart_mode == t("who_competes"):
    spec_title = t("bars_by_species_title")
    mgmt_title = t("bars_by_management_title")
else:
    spec_title = t("sky_view_species_title")
    mgmt_title = t("sky_view_management_title")

fig = make_subplots(
    rows=1, cols=3,
    specs=[[{"type": "xy"}, {"type": "xy"}, {"type": "xy"}]],
    subplot_titles=(bubble_title, spec_title, mgmt_title),
    horizontal_spacing=0.06,
)

# === 1) BUBBLE (outer 100%, inner area ~ avg light) ===
R = 1.0
r = math.sqrt(p_light) * R
xg, yg = 0.0, 0.0
xr, yr = 0.0, 0.0

fig.add_shape(
    type="circle",
    xref="x1", yref="y1",
    x0=xg - R, x1=xg + R, y0=yg - R, y1=yg + R,
    line=dict(width=0),
    fillcolor="#06402B",
)
fig.add_shape(
    type="circle",
    xref="x1", yref="y1",
    x0=xr - r, x1=xr + r, y0=yr - r, y1=yr + r,
    line=dict(width=0),
    fillcolor="#87CEEB",
)


fig.add_annotation(
    x=xg, y=yg , xref="x1", yref="y1",
    text=f"<b>{t('available_light_label', value=f'{avg_light:.0f}')}</b>",
    showarrow=False,
    font=dict(size=20, color="#000000"),
)

fig.update_xaxes(row=1, col=1, visible=False, range=[-1.3, 1.3], scaleanchor="y1", scaleratio=1)
fig.update_yaxes(row=1, col=1, visible=False, range=[-1.3, 1.3])

# === 2 & 3) RIGHT CHARTS: MODE-DEPENDENT ===
if chart_mode == t("who_competes"):
    # 2) BARS by SPECIES (values in %)
    x_species = spec_df["species"].astype(str).tolist() if not spec_df.empty else (species_all or [])
    y_species = spec_df["value"].tolist() if not spec_df.empty else ([0.0] * len(x_species))
    colors_species = [species_cmap.get(s, "#AAAAAA") for s in x_species]
    fig.add_trace(
        go.Bar(
            x=x_species,
            y=y_species,
            marker_color=colors_species,
            hovertemplate=t("hover_species_shade") + "<extra></extra>",
            showlegend=False,
        ),
        row=1, col=2,
    )

    # 3) BARS by MANAGEMENT (values in %)
    #x_mgmt = mgmt_df["management_status"].astype(str).tolist() if not mgmt_df.empty else (mgmt_all or [])
    x_mgmt = mgmt_df["management_status"].astype(str).tolist()
    y_mgmt = mgmt_df["value"].tolist() if not mgmt_df.empty else ([0.0] * len(x_mgmt))
    colors_mgmt = [mgmt_cmap.get(m, "#AAAAAA") for m in x_mgmt]
    fig.add_trace(
        go.Bar(
            x = mgmt_df["management_status"].apply(t_mgmt),
            y=y_mgmt,
            marker_color=[mgmt_cmap.get(m, "#AAAAAA") for m in mgmt_df["management_status"]],
            hovertemplate=(
                f"{t('management_label')}: %{{x}}<br>"
                f"{t('value_label')}: %{{y:.1f}} %"
                "<extra></extra>"
            ),
            showlegend=False,
        ),
        row=1, col=3,
    )

    # Axes formatting for bars
    for c in (2, 3):
        fig.update_xaxes(title_text=None, tickangle=45, row=1, col=c)
        col_vals = y_species if c == 2 else y_mgmt
        ymax = float(max(col_vals)) if col_vals else 1.0
        if ymax <= 0:
            y_upper = 1.0
        else:
            magnitude = 10 ** int(np.floor(np.log10(ymax)))
            step = magnitude / 2
            y_upper = math.ceil(ymax / step) * step
        fig.update_yaxes(title_text=t("bar_ylabel_shade"), range=[0, y_upper], row=1, col=c)

else:
    # Sky View Stats: VIOLIN PLOTS of light_avail_adj
    if not df_focal.empty and "light_avail_adj" in df_focal.columns:
        # Species
        if "species" in df_focal.columns:
            for sp in df_focal["species"].astype(str).dropna().unique().tolist():
                vals = (
                    df_focal.loc[df_focal["species"].astype(str) == sp, "light_avail_adj"]
                    .dropna().tolist()
                )
                if not vals:
                    continue
                fig.add_trace(
                    go.Violin(
                        x=[sp] * len(vals),
                        y=vals,
                        name=str(sp),
                        box_visible=True,
                        meanline_visible=False,
                        points=False,
                        fillcolor=species_cmap.get(sp, "#AAAAAA"),
                        line=dict(color="black", width=1),
                        showlegend=False,
                    ),
                    row=1, col=2,
                )

        # Management
        if "management_status" in df_focal.columns:
            for mg in df_focal["management_status"].astype(str).dropna().unique().tolist():
                vals = (
                    df_focal.loc[
                        df_focal["management_status"].astype(str) == mg,
                        "light_avail_adj",
                    ]
                    .dropna()
                    .tolist()
                )
                if not vals:
                    continue

                mg_label = t_mgmt(mg)

                fig.add_trace(
                    go.Violin(
                        x=[mg_label] * len(vals),   # ✅ správná délka
                        y=vals,                     # ✅ DATA CHYBĚLA
                        name=mg_label,
                        box_visible=True,
                        meanline_visible=False,
                        points=False,
                        fillcolor=mgmt_cmap.get(mg, "#AAAAAA"),
                        line=dict(color="black", width=1),
                        showlegend=False,
                    ),
                    row=1,
                    col=3,
                )


    for c in (2, 3):
        fig.update_xaxes(title_text=None, tickangle=45, row=1, col=c)
        fig.update_yaxes(title_text=t("violin_ylabel_light"), range=[0, 100], row=1, col=c)

fig.update_layout(height=460, margin=dict(l=10, r=10, t=60, b=40))



chart_placeholder.plotly_chart(fig, use_container_width=True)

with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("light_comp_help"))