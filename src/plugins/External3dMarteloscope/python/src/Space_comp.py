# -*- coding: utf-8 -*-
# -------------------------------------------------------------------
# Streamlit: Competition for Space (Shared Crown Voxels) — i18n version
# -------------------------------------------------------------------

import json
import math
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import streamlit as st
import src.io_utils as iou

from src.i18n import t, t_help, t_mgmt


# -------------------------------------------------------------------
# STABLE IDs (i18n keys)
# -------------------------------------------------------------------
STATE_BEFORE = "label_before"
STATE_AFTER = "label_after"
STATE_REMOVED = "label_removed"  # (nepoužívá se v segmented, ale využívá se v logice)

SHOW_PCT = "percentage_mode"     # existuje ve slovníku
SHOW_VOL = "metric_volume_m3"         

# Stand states shown in UI
stand_state_options = [STATE_BEFORE, STATE_AFTER]


st.markdown(f"### **{t('page_space_competition')}**")


# -------------------------------------------------------------------
# LOAD DATA
# -------------------------------------------------------------------
if "trees" not in st.session_state:
    file_path = (
        "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/"
        "PokojnaHora_3df/PokojnaHora.json"
    )
    st.session_state.trees = iou.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()


# -------------------------------------------------------------------
# HELPER FUNCTIONS
# -------------------------------------------------------------------
def _to_list_of_dicts(v):
    """Safely parse the crownVoxelCountShared field into a list of dicts."""
    if v is None or (isinstance(v, float) and np.isnan(v)):
        return []
    if isinstance(v, list):
        return [x for x in v if isinstance(x, dict) and "treeId" in x]
    if isinstance(v, str):
        try:
            parsed = json.loads(v)
            if isinstance(parsed, list):
                return [x for x in parsed if isinstance(x, dict) and "treeId" in x]
        except Exception:
            return []
    return []


def _species_colors(d):
    """Map species → speciesColorHex."""
    if "species" not in d.columns or "speciesColorHex" not in d.columns:
        return {}
    return (
        d.assign(species=lambda x: x["species"].astype(str))
        .groupby("species")["speciesColorHex"]
        .first()
        .to_dict()
    )


def _management_colors(d):
    """Map management_status → managementColorHex."""
    if "management_status" not in d.columns or "managementColorHex" not in d.columns:
        return {}
    cmap = (
        d.assign(ms=lambda x: x["management_status"].astype(str))
        .groupby("ms")["managementColorHex"]
        .first()
        .to_dict()
    )
    return {k: (v if isinstance(v, str) and v.strip() else "#AAAAAA") for k, v in cmap.items()}


def _crown_volume_series(d):
    """Compute total crown volume from voxel count and voxel size."""
    cnt = pd.to_numeric(d.get("crownVoxelCount", 0), errors="coerce").fillna(0)
    vsz = pd.to_numeric(d.get("crownVoxelSize", 0), errors="coerce").fillna(0)
    return (cnt * (vsz ** 3)).astype(float)


def _shared_volume_for_tree_row(row):
    """Compute total shared volume for a tree from shared voxel list."""
    vsz = pd.to_numeric(row.get("crownVoxelSize"), errors="coerce")
    if not np.isfinite(vsz) or vsz <= 0:
        return 0.0
    voxel_vol = float(vsz) ** 3
    items = _to_list_of_dicts(row.get("crownVoxelCountShared"))
    total_shared_vox = float(sum(max(0, int(it.get("count", 0))) for it in items))
    return total_shared_vox * voxel_vol


def _make_masks(d):
    """Return masks for stand-state logic: Before / After / Removed."""
    keep = {"Target tree", "Untouched"}
    mask_after = d["management_status"].astype(str).isin(keep)
    mask_removed = ~mask_after
    mask_before = pd.Series(True, index=d.index)
    return {STATE_BEFORE: mask_before, STATE_AFTER: mask_after, STATE_REMOVED: mask_removed}


# -------------------------------------------------------------------
# STATIC SELECTION LISTS
# -------------------------------------------------------------------
sp_all = iou._unique_sorted(df.get("species", pd.Series(dtype=object)))
mg_all = iou._unique_sorted(df.get("management_status", pd.Series(dtype=object)))


# -------------------------------------------------------------------
# TOP FILTERS (stand state, show mode, DBH, Height)
# -------------------------------------------------------------------
c1, c2, c3, c4, c5, c6, c7, c8, c9 = st.columns([0.5, 3, 0.5, 2, 0.5, 2, 0.5, 2, 0.5])

with c2:
    stand_state = st.segmented_control(
        f"**{t('select_stand_state')}**",
        options=stand_state_options,
        default=STATE_BEFORE,
        width="stretch",
        format_func=lambda k: t(k),
    )

with c4:
    show_mode = st.segmented_control(
        f"**{t('show_values_as')}**",
        options=[SHOW_PCT, SHOW_VOL],
        default=SHOW_PCT,
        width="stretch",
        format_func=lambda k: t(k),
    )

with c6:
    dbh_vals = pd.to_numeric(df.get("dbh", pd.Series(dtype=float)), errors="coerce").dropna()
    if dbh_vals.empty:
        dbh_min, dbh_max = 0, 1
    else:
        dbh_min, dbh_max = float(dbh_vals.min()), float(dbh_vals.max())
        dbh_min = math.floor(dbh_min)
        dbh_max = math.ceil(dbh_max)
        if dbh_max <= dbh_min:
            dbh_max = dbh_min + 1

    dbh_range = st.slider(
        f"**{t('dbh_filter')}**",
        dbh_min,
        dbh_max,
        (dbh_min, dbh_max),
    )

with c8:
    h_vals = pd.to_numeric(df.get("height", pd.Series(dtype=float)), errors="coerce").dropna()
    if h_vals.empty:
        h_min, h_max = 0, 1
    else:
        h_min, h_max = float(h_vals.min()), float(h_vals.max())
        h_min = math.floor(h_min)
        h_max = math.ceil(h_max)
        if h_max <= h_min:
            h_max = h_min + 1

    height_range = st.slider(
        f"**{t('height_filter')}**",
        h_min,
        h_max,
        (h_min, h_max),
    )

# -------------------------------------------------------------------
# BOTTOM FILTERS
# -------------------------------------------------------------------
c_bot1, c_bot2 = st.columns([2, 15])

with c_bot1:
    if "species_sel" not in st.session_state:
        st.session_state["species_sel"] = sp_all.copy()

    st.pills(
        f"**{t('filter_species')}**",
        options=sp_all,
        selection_mode="multi",
        key="species_sel",
    )

with c_bot2:
    #st.plotly_chart(fig, use_container_width=True)
    chart_placeholder = st.empty()

c31, c32 = st.columns([2, 15])

with c32:

    mgmt_label_map = {m: t_mgmt(m) for m in mg_all}
    mgmt_labels = list(mgmt_label_map.values())
    label_to_mgmt = {v: k for k, v in mgmt_label_map.items()}

    # ✅ Inicializace pouze při prvním běhu
    if "mgmt_sel_labels" not in st.session_state:
        st.session_state["mgmt_sel_labels"] = mgmt_labels.copy()
        st.session_state["mgmt_sel"] = mg_all.copy()

    mgmt_sel_labels = st.pills(
        f"**{t('filter_management')}**",
        options=mgmt_labels,
        selection_mode="multi",
        key="mgmt_sel_labels",
    )

    # vždy synchronizujeme interní hodnoty
    st.session_state["mgmt_sel"] = [
        label_to_mgmt[lbl] for lbl in mgmt_sel_labels
    ]

# -------------------------------------------------------------------
# READ SPECIES & MANAGEMENT SELECTIONS FROM SESSION
# -------------------------------------------------------------------
species_sel = st.session_state.get("species_sel", sp_all)
mg_sel = st.session_state.get("mgmt_sel", mg_all)

species_sel = list(species_sel) if species_sel is not None else []
mg_sel_set = set(mg_sel) if mg_sel is not None else set()


# -------------------------------------------------------------------
# DERIVED COLUMNS
# -------------------------------------------------------------------
df["crown_volume_ref"] = _crown_volume_series(df)

neighbor_attrs = df[["id", "species", "management_status", "speciesColorHex", "managementColorHex"]].copy()
neighbor_attrs["id"] = pd.to_numeric(neighbor_attrs["id"], errors="coerce").astype("Int64")


# -------------------------------------------------------------------
# BUILD FOCAL SUBSET (with DBH/Height filtering + neighbor preservation)
# -------------------------------------------------------------------
state_masks = _make_masks(df)

mask_dbh = (pd.to_numeric(df.get("dbh"), errors="coerce") >= dbh_range[0]) & (pd.to_numeric(df.get("dbh"), errors="coerce") <= dbh_range[1])
mask_h = (pd.to_numeric(df.get("height"), errors="coerce") >= height_range[0]) & (pd.to_numeric(df.get("height"), errors="coerce") <= height_range[1])

mask_species = df.get("species", pd.Series("", index=df.index)).astype(str).isin(species_sel) if species_sel else pd.Series(True, index=df.index)
mask_mgmt = df.get("management_status", pd.Series("", index=df.index)).astype(str).isin(mg_sel_set) if mg_sel_set else pd.Series(True, index=df.index)
mask_state = state_masks.get(stand_state, pd.Series(True, index=df.index))

# Trees that directly pass filters
focal_mask_base = mask_species & mask_mgmt & mask_state & mask_dbh & mask_h
focal_ids_base = set(pd.to_numeric(df.loc[focal_mask_base, "id"], errors="coerce").dropna().astype(int).tolist())

# Collect neighbors directly (without df_shared)
neighbor_ids = set()
for lst in df.loc[focal_mask_base, "crownVoxelCountShared"].apply(_to_list_of_dicts):
    for item in lst:
        tid = item.get("treeId")
        if tid is not None:
            try:
                neighbor_ids.add(int(tid))
            except Exception:
                pass

# Final set of trees to keep
all_kept_ids = focal_ids_base.union(neighbor_ids)

# Final focal subset
df_id_int = pd.to_numeric(df.get("id"), errors="coerce")
focal_mask = df_id_int.isin(all_kept_ids)
df_focal = df.loc[focal_mask].copy()

# Compute baseline shared volume for focal trees
df_focal["shared_volume_base"] = df_focal.apply(_shared_volume_for_tree_row, axis=1)


# -------------------------------------------------------------------
# EXPLODE SHARED LIST → df_shared
# -------------------------------------------------------------------
df_focal["_shared_list"] = df_focal["crownVoxelCountShared"].apply(_to_list_of_dicts)

df_shared = (
    df_focal[["id", "crownVoxelSize", "_shared_list"]]
    .rename(columns={"id": "focal_id"})
    .explode("_shared_list", ignore_index=True)
)
df_shared = df_shared.dropna(subset=["_shared_list"])

df_shared["neighbor_id"] = (
    pd.to_numeric(df_shared["_shared_list"].apply(lambda d: d.get("treeId")), errors="coerce")
    .astype("Int64")
)
df_shared["count"] = (
    pd.to_numeric(df_shared["_shared_list"].apply(lambda d: d.get("count")), errors="coerce")
    .fillna(0).astype(int).clip(lower=0)
)
df_shared["voxel_vol"] = pd.to_numeric(df_shared["crownVoxelSize"], errors="coerce").pow(3)
df_shared["shared_volume"] = (df_shared["count"] * df_shared["voxel_vol"]).fillna(0)

# Attach neighbor attributes
df_shared = df_shared.merge(
    neighbor_attrs,
    left_on="neighbor_id",
    right_on="id",
    how="left",
    suffixes=("", "_nbr"),
)


# -------------------------------------------------------------------
# REMOVED TREES HANDLING (adjust shared volume)
# -------------------------------------------------------------------
removed_ids = set(
    pd.to_numeric(df.loc[state_masks[STATE_REMOVED], "id"], errors="coerce")
    .dropna().astype(int).tolist()
)

df_shared["neighbor_removed"] = df_shared["neighbor_id"].isin(removed_ids)

removed_shared_by_focal = (
    df_shared[df_shared["neighbor_removed"]]
    .groupby("focal_id")["shared_volume"]
    .sum()
    .rename("removed_shared")
)

df_focal = df_focal.merge(removed_shared_by_focal, left_on="id", right_index=True, how="left")
df_focal["removed_shared"] = df_focal["removed_shared"].fillna(0)

# Adjust shared volume depending on stand state
if stand_state in (STATE_AFTER, STATE_REMOVED):
    df_focal["shared_volume_adj"] = (df_focal["shared_volume_base"] - df_focal["removed_shared"]).clip(lower=0)
else:
    df_focal["shared_volume_adj"] = df_focal["shared_volume_base"]

df_focal["free_volume_adj"] = (df_focal["crown_volume_ref"] - df_focal["shared_volume_adj"]).clip(lower=0)


# -------------------------------------------------------------------
# AGGREGATE FOR BAR CHARTS
# -------------------------------------------------------------------
df_shared_kept = (
    df_shared[~df_shared["neighbor_removed"]] if stand_state in (STATE_AFTER, STATE_REMOVED)
    else df_shared.copy()
)

spec_df = (
    df_shared_kept.groupby("species", as_index=False)["shared_volume"].sum()
    if not df_shared_kept.empty else pd.DataFrame(columns=["species", "value"])
).rename(columns={"shared_volume": "value"})

mgmt_df = (
    df_shared_kept.groupby("management_status", as_index=False)["shared_volume"].sum()
    if not df_shared_kept.empty else pd.DataFrame(columns=["management_status", "value"])
).rename(columns={"shared_volume": "value"})

# Reindex to include missing categories
if sp_all:
    spec_df = spec_df.set_index("species").reindex(sp_all, fill_value=0).reset_index()
if mg_all:
    mgmt_df = mgmt_df.set_index("management_status").reindex(mg_all, fill_value=0).reset_index()

# Color maps
species_cmap = _species_colors(df)
mgmt_cmap = _management_colors(df)


# -------------------------------------------------------------------
# TOTALS FOR BUBBLE
# -------------------------------------------------------------------
total_ref_vol = float(df_focal["crown_volume_ref"].sum())
total_shared_vol = float(df_focal["shared_volume_adj"].sum())
_ = max(0.0, total_ref_vol - total_shared_vol)


# -------------------------------------------------------------------
# NORMALIZATION FOR PLOTS
# -------------------------------------------------------------------
if show_mode == SHOW_PCT:
    denom = total_shared_vol if total_shared_vol > 0 else 1.0
    spec_plot_vals = spec_df.assign(plot_val=lambda tdf: (tdf["value"] / denom) * 100.0)
    mgmt_plot_vals = mgmt_df.assign(plot_val=lambda tdf: (tdf["value"] / denom) * 100.0)
    y_title_bars = t("y_title_shared_space_pct")
    total_label = t("total_crown_label_pct")
    pct = (total_shared_vol / total_ref_vol * 100.0) if total_ref_vol else 0.0
    shared_label = t("shared_label_pct", value=f"{pct:.0f}")
    unit_suffix = " %"
else:
    spec_plot_vals = spec_df.assign(plot_val=lambda tdf: tdf["value"].astype(float))
    mgmt_plot_vals = mgmt_df.assign(plot_val=lambda tdf: tdf["value"].astype(float))
    y_title_bars = t("y_title_shared_space_m3")
    total_label = t("total_crown_label", value=f"{total_ref_vol:.0f}")
    shared_label = t("shared_label", value=f"{total_shared_vol:.0f}")
    unit_suffix = " m³"


# -------------------------------------------------------------------
# PLOTTING
# -------------------------------------------------------------------
fig = make_subplots(
    rows=1,
    cols=3,
    specs=[[{"type": "xy"}, {"type": "xy"}, {"type": "xy"}]],
    subplot_titles=(
        t("shared_vs_total_title"),
        t("bars_by_species_title"),
        t("bars_by_management_title"),
    ),
    horizontal_spacing=0.06,
)

# 1) BUBBLE DIAGRAM
p_share = (total_shared_vol / total_ref_vol) if total_ref_vol > 0 else 0.0
R = 1.0
r = math.sqrt(max(0.0, min(1.0, p_share))) * R
xg, yg = 0.0, 0.0
xr, yr = (R - r), 0.0

fig.add_shape(
    type="circle",
    xref="x1",
    yref="y1",
    x0=xg - R,
    x1=xg + R,
    y0=yg - R,
    y1=yg + R,
    fillcolor="rgba(60,141,47,0.25)",
    line=dict(width=0),
)
fig.add_shape(
    type="circle",
    xref="x1",
    yref="y1",
    x0=xr - r,
    x1=xr + r,
    y0=yr - r,
    y1=yr + r,
    fillcolor="rgba(255,40,40,0.45)",
    line=dict(width=2, color="rgba(255,40,40,1)"),
)

fig.add_annotation(x=xg, y=yg + R + 0.15, xref="x1", yref="y1", text=total_label, showarrow=False)
fig.add_annotation(x=xr, y=yr - r - 0.15, xref="x1", yref="y1", text=shared_label, showarrow=False)

fig.update_xaxes(visible=False, range=[-1.3, 1.3], row=1, col=1, scaleanchor="y1")
fig.update_yaxes(visible=False, range=[-1.3, 1.3], row=1, col=1)

# 2) BAR CHART – SPECIES
fig.add_trace(
    go.Bar(
        x=spec_plot_vals["species"],
        y=spec_plot_vals["plot_val"],
        marker_color=[species_cmap.get(s, "#AAAAAA") for s in spec_plot_vals["species"]],
        hovertemplate=(
            f"{t('management_label')}: %{{x}}<br>"
            f"{t('value_label')}: %{{y:.2f}}{unit_suffix}"
            "<extra></extra>"
        ),
        showlegend=False,
    ),
    row=1,
    col=2,
)

# 3) BAR CHART – MANAGEMENT
mgmt_plot_vals["management_label"] = mgmt_plot_vals["management_status"].apply(t_mgmt)
fig.add_trace(
    go.Bar(
        x=mgmt_plot_vals["management_label"],   # ✅ přeloženo
        y=mgmt_plot_vals["plot_val"],
        marker_color=[mgmt_cmap.get(m, "#AAAAAA") for m in mgmt_plot_vals["management_status"]],
        hovertemplate=(t("hover_management") + unit_suffix + "<extra></extra>"),
        showlegend=False,
    ),
    row=1,
    col=3,
)

# Axis formatting
for c in (2, 3):
    fig.update_xaxes(tickangle=45, row=1, col=c)
    vals = spec_plot_vals["plot_val"] if c == 2 else mgmt_plot_vals["plot_val"]
    ymax = float(np.nanmax(vals.values)) if len(vals) else 1.0
    if not np.isfinite(ymax) or ymax <= 0:
        y_upper = 1.0
    else:
        magnitude = 10 ** int(np.floor(np.log10(ymax)))
        step = magnitude / 2
        y_upper = math.ceil(ymax / step) * step
    fig.update_yaxes(title_text=y_title_bars, range=[0, y_upper], row=1, col=c)

fig.update_layout(height=460, margin=dict(l=10, r=10, t=60, b=40))



chart_placeholder.plotly_chart(fig, use_container_width=True)

with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("space_comp_help"))