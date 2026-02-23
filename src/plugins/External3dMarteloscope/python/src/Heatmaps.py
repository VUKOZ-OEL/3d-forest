# -*- coding: utf-8 -*-
# ------------------------------------------------------------
# XY Heatmaps (Before | After | Removed) — i18n version
# ------------------------------------------------------------

import streamlit as st
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import src.io_utils as iou
import re

from src.i18n import t, t_help, t_mgmt


# ------------------------------------------------------------
# PAGE TITLE
# ------------------------------------------------------------
st.markdown(f"### {t('page_title')}")  # "Heatmaps for selected Attribute" :contentReference[oaicite:0]{index=0}


# ------------------------------------------------------------
# LOAD + NORMALIZE DATA
# ------------------------------------------------------------
if "trees" not in st.session_state:
    file_path = (
        "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/"
        "SLP_Pokojna/PokojnaHora_3df/PokojnaHora.json"
    )
    st.session_state.trees = iou.load_project_json(file_path)

df0 = st.session_state.trees.copy()

# normalize XY (origin = min)
df0["x"] = df0["x"] - df0["x"].min()
df0["y"] = df0["y"] - df0["y"].min()


# ------------------------------------------------------------
# CONSTANTS
# ------------------------------------------------------------
CHART_HEIGHT = 500
HEATMAP_NBINS = 50
keep_status = {"Target tree", "Untouched"}

# labely z session_state (už přeložené UI)
#COLOR_SPP = st.session_state.Species
#COLOR_MGMT = st.session_state.Management

COLOR_SPP = "species"
COLOR_MGMT = "management"


# ------------------------------------------------------------
# VALUE MAPPING (stable IDs -> dataframe columns)
# ------------------------------------------------------------
VALUE_TREE_COUNT = "tree_count"  # exists :contentReference[oaicite:1]{index=1}

value_mapping = {
    VALUE_TREE_COUNT: None,
    "dbh": "dbh",                          # exists :contentReference[oaicite:2]{index=2}
    "basal_area_m2": "BasalArea_m2",       # exists :contentReference[oaicite:3]{index=3}
    "volume": "Volume_m3",                 # exists :contentReference[oaicite:4]{index=4}
    "tree_height": "height",               # exists :contentReference[oaicite:5]{index=5}
    "crown_base_height": "crown_base_height",          # exists :contentReference[oaicite:6]{index=6}
    "crown_centroid_height": "crown_centroid_height",  # exists :contentReference[oaicite:7]{index=7}
    "crown_volume_m3": "crown_volume",                 # exists :contentReference[oaicite:8]{index=8}
    "crown_surface": "crown_surface",                  # exists :contentReference[oaicite:9]{index=9}
    "horizontal_crown_proj": "horizontal_crown_proj",  # exists :contentReference[oaicite:10]{index=10}
    "vertical_crown_proj": "vertical_crown_proj",      # exists :contentReference[oaicite:11]{index=11}
    "crown_eccentricity": "crown_eccentricity",        # exists :contentReference[oaicite:12]{index=12}
    "height_dbh_ratio": "heightXdbh",                  # exists :contentReference[oaicite:13]{index=13}
}

value_options = list(value_mapping.keys())


# ------------------------------------------------------------
# HELPERS
# ------------------------------------------------------------
def _safe_num(s):
    return pd.to_numeric(s, errors="coerce")


def _normalize_list(lst):
    return [] if (not lst or "(none)" in lst) else lst


def _gaussian_kernel_1d(sigma_bins: float) -> np.ndarray:
    if sigma_bins <= 0 or not np.isfinite(sigma_bins):
        return np.array([1.0])
    r = int(max(1, round(3 * sigma_bins)))
    x = np.arange(-r, r + 1)
    k = np.exp(-0.5 * (x / sigma_bins) ** 2)
    k /= k.sum()
    return k


def _conv1d(A, k, axis):
    r = len(k) // 2
    pad = [(0, 0), (0, 0)]
    pad[axis] = (r, r)
    Ap = np.pad(A, pad, mode="reflect")
    if axis == 1:
        return np.apply_along_axis(lambda m: np.convolve(m, k, mode="valid"), 1, Ap)
    else:
        return np.apply_along_axis(lambda m: np.convolve(m, k, mode="valid"), 0, Ap)


def _blur2d(A, sx_bins, sy_bins):
    out = A
    if sx_bins > 0:
        out = _conv1d(out, _gaussian_kernel_1d(sx_bins), axis=1)
    if sy_bins > 0:
        out = _conv1d(out, _gaussian_kernel_1d(sy_bins), axis=0)
    return out


# ------------------------------------------------------------
# FILTER LISTS
# ------------------------------------------------------------
sp_all = sorted(df0["species"].astype(str).unique())
mg_all = sorted(df0["management_status"].astype(str).unique())
mgmt_label_map = {m: t_mgmt(m) for m in mg_all}
# labels pro UI
mgmt_labels = list(mgmt_label_map.values())

# ------------------------------------------------------------
# UI — TOP
# ------------------------------------------------------------
c1, c2, c3, c4 = st.columns([4, 4, 4, 4])

with c1:
    value_id = st.selectbox(
        f"**{t('heatmap_value_label')}**",  # exists :contentReference[oaicite:14]{index=14}
        options=value_options,
        format_func=lambda k: t(k),
        index=0,
    )
    z_col = value_mapping[value_id]  # None -> Count

with c2:
    dbh_vals = _safe_num(df0["dbh"]).dropna()
    if dbh_vals.empty:
        st.warning(t("warn_no_data_for_filters"))  # exists :contentReference[oaicite:15]{index=15}
        st.stop()

    dbh_range = st.slider(
        f"**{t('dbh_filter_heatmap')}**",  # NEW KEY (návrh níže)
        int(dbh_vals.min()),
        int(dbh_vals.max()),
        (int(dbh_vals.min()), int(dbh_vals.max())),
    )

with c3:
    hvals = _safe_num(df0["height"]).dropna()
    if hvals.empty:
        st.warning(t("warn_no_data_for_filters"))  # exists :contentReference[oaicite:16]{index=16}
        st.stop()

    height_range = st.slider(
        f"**{t('height_filter_heatmap')}**",  # exists :contentReference[oaicite:17]{index=17}
        int(hvals.min()),
        int(hvals.max()),
        (int(hvals.min()), int(hvals.max())),
    )

with c4:
    color_mode = st.segmented_control(
        f"**{t('color_by')}**",
        options=[COLOR_SPP, COLOR_MGMT],
        default=COLOR_SPP,
        format_func=lambda v: {
            COLOR_SPP: t("species"),
            COLOR_MGMT: t("management_label"),
        }.get(v, v),
    )

show_overlay = st.checkbox(
    f"**{t('show_tree_positions')}**",  # NEW KEY (návrh níže)
    value=True,
)


# ------------------------------------------------------------
# UI — BOTTOM (heatmap-only filters)
# ------------------------------------------------------------
cA, cB = st.columns([2, 15])
with cA:
    species_sel = st.pills(
        f"**{t('filter_species_heatmap')}**",  # exists :contentReference[oaicite:19]{index=19}
        sp_all,
        default=sp_all,
        selection_mode="multi",
    )

with cB:
    plot_container = st.container()

_, _, cC = st.columns([1, 1, 15])
with cC:
    mgmt_sel_labels = st.pills(
        f"**{t('filter_selection_heatmap')}**",
        options=mgmt_labels,
        default=mgmt_labels,
        selection_mode="multi",
    )

species_sel = _normalize_list(species_sel)
label_to_mgmt = {v: k for k, v in mgmt_label_map.items()}
mgmt_sel = [label_to_mgmt[lbl] for lbl in mgmt_sel_labels]


# ------------------------------------------------------------
# APPLY FILTERS FOR HEATMAP ONLY
# ------------------------------------------------------------
def _apply_filters_heatmap(df):
    d = df.copy()
    if species_sel:
        d = d[d["species"].astype(str).isin(species_sel)]
    if mgmt_sel:
        d = d[d["management_status"].astype(str).isin(mgmt_sel)]
    d = d[(d["dbh"] >= dbh_range[0]) & (d["dbh"] <= dbh_range[1])]
    d = d[(d["height"] >= height_range[0]) & (d["height"] <= height_range[1])]
    return d


df_f = _apply_filters_heatmap(df0)   # ONLY heatmap is filtered

# overlay dots ALWAYS use full dataset
df_overlay = df0.copy()


# ------------------------------------------------------------
# VALID XY FOR HEATMAP
# ------------------------------------------------------------
x_f = _safe_num(df_f["x"])
y_f = _safe_num(df_f["y"])
valid_xy = x_f.notna() & y_f.notna()

if not valid_xy.any():
    st.info(t("warn_no_data_for_filters"))  # NEW KEY (návrh níže)
    st.stop()


# ------------------------------------------------------------
# XY RANGE (square panels)
# ------------------------------------------------------------
buffer = 1.0
xmin = float(df0["x"].min()) - buffer
xmax = float(df0["x"].max()) + buffer
ymin = float(df0["y"].min()) - buffer
ymax = float(df0["y"].max()) + buffer


# ------------------------------------------------------------
# WEIGHTS
# ------------------------------------------------------------
if z_col is None:
    w = pd.Series(1.0, index=df_f.index)
    colorbar_title = t("tree_count")  # exists :contentReference[oaicite:21]{index=21}
else:
    w = _safe_num(df_f[z_col]).fillna(0)
    colorbar_title = t(value_id)


# ------------------------------------------------------------
# GRID
# ------------------------------------------------------------
x_edges = np.linspace(xmin, xmax, HEATMAP_NBINS + 1)
y_edges = np.linspace(ymin, ymax, HEATMAP_NBINS + 1)

x_cent = (x_edges[:-1] + x_edges[1:]) / 2
y_cent = (y_edges[:-1] + y_edges[1:]) / 2

bin_wx = (xmax - xmin) / HEATMAP_NBINS
bin_wy = (ymax - ymin) / HEATMAP_NBINS


# ------------------------------------------------------------
# PANEL BUILDER
# ------------------------------------------------------------
def _panel(mask):
    m = (mask.reindex(df_f.index, fill_value=False)) & valid_xy

    if not m.any():
        Z = np.zeros((HEATMAP_NBINS, HEATMAP_NBINS))
        H = np.full_like(Z, t("no_data"), dtype=object)  # NEW KEY (návrh níže)
        return Z, H

    Z, _, _ = np.histogram2d(
        x_f[m], y_f[m], bins=[x_edges, y_edges], weights=w[m]
    )
    Z = Z.T

    if m.sum() > 1:
        sigma = 3
        Z = _blur2d(Z, sigma / bin_wx, sigma / bin_wy)

    H = np.empty_like(Z, dtype=object)
    for iy in range(Z.shape[0]):
        for ix in range(Z.shape[1]):
            val = Z[iy, ix]
            if z_col is None:
                H[iy, ix] = f"{t('count_label')} {val:.0f}"  # exists :contentReference[oaicite:22]{index=22}
            else:
                H[iy, ix] = f"{colorbar_title}: {val:.2f}"

    return Z, H


# ------------------------------------------------------------
# COMPUTE PANELS
# ------------------------------------------------------------
m_before = pd.Series(True, index=df_f.index)
m_after = df_f["management_status"].isin(keep_status)
m_removed = ~m_after

Zb, Hb = _panel(m_before)
Za, Ha = _panel(m_after)
Zr, Hr = _panel(m_removed)

zmax = max(Zb.max(), Za.max(), Zr.max())


# ------------------------------------------------------------
# PLOT INIT
# ------------------------------------------------------------
fig = make_subplots(
    rows=1,
    cols=3,
    subplot_titles=(
        t("label_before"),
        t("label_after"),
        t("label_removed"),
    ),
    specs=[[{"type": "heatmap"}] * 3],
    horizontal_spacing=0.003,
)


def _add(Z, H, col):
    fig.add_trace(
        go.Heatmap(
            x=x_cent,
            y=y_cent,
            z=Z,
            text=H,
            hoverinfo="text",
            coloraxis="coloraxis",
        ),
        row=1,
        col=col,
    )


_add(Zb, Hb, 1)
_add(Za, Ha, 2)
_add(Zr, Hr, 3)


# ------------------------------------------------------------
# OVERLAY POINTS (UNFILTERED)
# ------------------------------------------------------------
if show_overlay:
    df_o = df_overlay.copy()
    ok = df_o["x"].notna() & df_o["y"].notna()

    if color_mode == COLOR_SPP:
        group_col = "species"
        color_col = "speciesColorHex"
    else:
        group_col = "management_status"
        color_col = "managementColorHex"

    valid_hex = re.compile(r"^#([0-9A-Fa-f]{6})$")
    df_o[color_col] = df_o[color_col].where(
        df_o[color_col].astype(str).str.match(valid_hex),
        "#000000",
    )

    for category, sub in df_o[ok].groupby(group_col):
        colvals = sub[color_col].tolist()
        legend_name = (
            t_mgmt(category)
            if group_col == "management_status"
            else str(category)
        )
        for c in (1, 2, 3):
            fig.add_trace(
                go.Scatter(
                    x=sub["x"],
                    y=sub["y"],
                    mode="markers",
                    name=legend_name,
                    hoverinfo="skip",
                    showlegend=(c == 1),
                    legendgroup=str(category),
                    marker=dict(
                        size=4,
                        color=colvals,
                        opacity=0.9,
                        line=dict(width=0),
                    ),
                ),
                row=1,
                col=c,
            )


# ------------------------------------------------------------
# AXES (SQUARE PANELS)
# ------------------------------------------------------------
x_title = f"x [{t('unit_m')}]"
y_title = f"y [{t('unit_m')}]"

for c in (1, 2, 3):
    fig.update_xaxes(
        range=[xmin, xmax],
        title=x_title,
        row=1,
        col=c,
        ticks="outside",
        ticklen=4,
        showgrid=False,
    )

    if c == 1:
        fig.update_yaxes(
            range=[ymin, ymax],
            title=y_title,
            showgrid=False,
            scaleanchor="x1",
            scaleratio=1,
            ticks="outside",
            ticklen=4,
            row=1,
            col=c,
        )
    else:
        fig.update_yaxes(
            range=[ymin, ymax],
            showticklabels=False,
            ticks="",
            title=None,
            showgrid=False,
            scaleanchor=f"x{c}",
            scaleratio=1,
            row=1,
            col=c,
        )


# ------------------------------------------------------------
# LAYOUT
# ------------------------------------------------------------
fig.update_layout(
    height=CHART_HEIGHT,
    margin=dict(l=4, r=4, t=50, b=150),
    legend=dict(
        orientation="h",
        x=0.5,
        y=-0.15,
        xanchor="center",
        yanchor="top",
        bgcolor="rgba(255,255,255,0.6)",
    ),
    coloraxis=dict(
        colorscale=[
            (0.00, "#ffffff"),
            (0.02, "#e8f5e9"),
            (0.55, "#66bb6a"),
            (1.00, "#2e7d32"),
        ],
        cmin=0,
        cmax=zmax,
        colorbar=dict(
            title=None,
            orientation="h",
            x=0.5,
            xanchor="center",
            y=-0.23,
            yanchor="top",
            thickness=12,
            len=0.70,
        ),
    ),
)


# ------------------------------------------------------------
# RENDER
# ------------------------------------------------------------
with plot_container:
    st.plotly_chart(fig, use_container_width=True)

with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("heatmap_help"))