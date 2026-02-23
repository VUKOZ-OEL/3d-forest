# -*- coding: utf-8 -*-
# ------------------------------------------------------------
# Streamlit: Crown volume profiles by height (i18n)
# ------------------------------------------------------------

import streamlit as st
import pandas as pd
import numpy as np
import math
import ast
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import src.io_utils as iou

from src.i18n import t, t_help, t_mgmt


# ------------------------------------------------------------
# PAGE TITLE
# ------------------------------------------------------------
st.markdown(f"#### {t('explore_canopy_stats')}")


# ------------------------------------------------------------
# DATA
# ------------------------------------------------------------
if "trees" not in st.session_state:
    file_path = (
        "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/"
        "PokojnaHora_3df/PokojnaHora.json"
    )
    st.session_state.trees = iou.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()

# ------------------------------------------------------------
# SETTINGS
# ------------------------------------------------------------
CHART_HEIGHT = 420

# panel labels (keep consistent with the rest of the app)
Before = t("label_before")
After = t("label_after")
Removed = t("label_removed")

# layer ids (internal) -> translated labels via format_func=t
LAYER_SPECIES = "species"
LAYER_MGMT = "management_label"  # i18n already uses this key as "Treatment selection"


# ------------------------------------------------------------
# UI CONTROLS
# ------------------------------------------------------------
c1, c2, c3 = st.columns([2, 2, 2])
with c2:
    layers_mode = st.segmented_control(
        f"**{t('show_values_by')}**",
        options=[LAYER_SPECIES, LAYER_MGMT],
        default=[LAYER_SPECIES],
        selection_mode="multi",
        width="stretch",
        format_func=lambda k: t(k),
        help=t("canopy_profiles_layers_help"),  # <- new key (see below)
    )

# ensure list + at least one layer
if not layers_mode:
    layers_mode = [LAYER_SPECIES]
elif isinstance(layers_mode, str):
    layers_mode = [layers_mode]

primary = layers_mode[0]
overlay = layers_mode[1] if len(layers_mode) > 1 else None


# ------------------------------------------------------------
# AREA (ha) for per-ha conversion
# ------------------------------------------------------------
try:
    area_ha = float(st.session_state.plot_info["size_ha"].iloc[0])
    if not np.isfinite(area_ha) or area_ha <= 0:
        area_ha = 1.0
except Exception:
    area_ha = 1.0


# ------------------------------------------------------------
# MASKS (after / removed)
# ------------------------------------------------------------
keep_status = {"Target tree", "Untouched"}
if "management_status" in df.columns:
    mask_after = df["management_status"].isin(keep_status)
    mask_removed = ~mask_after
else:
    mask_after = pd.Series(False, index=df.index)
    mask_removed = pd.Series(False, index=df.index)


# ------------------------------------------------------------
# COLOR MAPS
# ------------------------------------------------------------
def _species_colors(df_all: pd.DataFrame) -> dict:
    if "species" not in df_all.columns or "speciesColorHex" not in df_all.columns:
        return {}
    tmp = (
        df_all.assign(species=lambda d: d["species"].astype(str))
        .groupby("species")["speciesColorHex"]
        .first()
    )
    return tmp.to_dict()


def _management_colors(df_all: pd.DataFrame) -> dict:
    if "management_status" not in df_all.columns or "managementColorHex" not in df_all.columns:
        return {}
    tmp = (
        df_all.assign(mgmt=lambda d: d["management_status"].astype(str))
        .groupby("mgmt")["managementColorHex"]
        .first()
    )
    return tmp.to_dict()


def _nice_upper(value: float) -> float:
    if not np.isfinite(value) or value <= 0:
        return 500.0
    return math.ceil(value / 500.0) * 500.0


# ------------------------------------------------------------
# CROWN VOLUME HELPERS
# ------------------------------------------------------------
def _as_counts(seq) -> list[int]:
    if isinstance(seq, (list, tuple, np.ndarray, pd.Series)):
        return [int(x) if pd.notna(x) else 0 for x in seq]
    if isinstance(seq, str):
        try:
            return _as_counts(ast.literal_eval(seq))
        except Exception:
            return []
    return []


def crown_volume_per_tree(df_sub: pd.DataFrame) -> pd.Series:
    if "crownVoxelSize" not in df_sub.columns or "crownVoxelCountPerMeters" not in df_sub.columns:
        return pd.Series(0.0, index=df_sub.index, dtype=float)

    voxel_size_m = pd.to_numeric(df_sub["crownVoxelSize"], errors="coerce")
    if voxel_size_m.dropna().empty:
        return pd.Series(0.0, index=df_sub.index, dtype=float)

    voxel_vol = voxel_size_m.pow(3)
    counts_series = df_sub["crownVoxelCountPerMeters"].apply(_as_counts)
    counts_sum = counts_series.apply(lambda lst: float(np.nansum(lst)) if len(lst) else 0.0)
    vol = counts_sum * voxel_vol.fillna(0.0)
    return vol.fillna(0.0).astype(float)


def expand_crown_volume(df_sub: pd.DataFrame, group_col: str) -> pd.DataFrame:
    required = {"crownStartHeight", "crownVoxelCountPerMeters", "crownVoxelSize", group_col}
    missing = required - set(df_sub.columns)
    if missing:
        st.warning(t("missing_columns_crown", columns=", ".join(sorted(missing))))
        return pd.DataFrame(columns=["height_bin", group_col, "volume"])

    out_h, out_g, out_v = [], [], []
    for _, row in df_sub.iterrows():
        grp = str(row.get(group_col))
        h0 = pd.to_numeric(row.get("crownStartHeight"), errors="coerce")
        vs = pd.to_numeric(row.get("crownVoxelSize"), errors="coerce")
        seq = row.get("crownVoxelCountPerMeters")

        if not (np.isfinite(h0) and np.isfinite(vs) and vs > 0):
            continue

        counts = _as_counts(seq)
        if not counts:
            continue

        base = int(math.floor(float(h0)))
        voxel_vol = float(vs) ** 3

        for i, c in enumerate(counts):
            try:
                c_int = int(c)
            except Exception:
                continue
            if c_int <= 0:
                continue
            hb = base + i
            out_h.append(hb)
            out_g.append(grp)
            out_v.append(c_int * voxel_vol)

    if not out_h:
        return pd.DataFrame(columns=["height_bin", group_col, "volume"])

    long = pd.DataFrame({"height_bin": out_h, group_col: out_g, "volume": out_v})
    return long.groupby(["height_bin", group_col], as_index=False)["volume"].sum()


def profiles_by(df_sub: pd.DataFrame, H: int, group_col: str, color_map: dict) -> pd.DataFrame:
    long = expand_crown_volume(df_sub, group_col)

    if group_col not in df_sub.columns:
        groups = []
    else:
        groups = sorted(df_sub[group_col].astype(str).unique().tolist())

    if not groups:
        full_idx = pd.MultiIndex.from_product([range(0, H), []], names=["height_bin", group_col])
        return pd.DataFrame(index=full_idx).reset_index().assign(volume=0.0, color="#AAAAAA")

    full_idx = pd.MultiIndex.from_product([range(0, H), groups], names=["height_bin", group_col])
    prof = (
        long.set_index(["height_bin", group_col])["volume"]
        .reindex(full_idx, fill_value=0.0)
        .reset_index()
    )
    prof["color"] = prof[group_col].map(color_map).fillna("#AAAAAA")
    return prof


def sum_profile(df_sub: pd.DataFrame, H: int) -> np.ndarray:
    """Sum of volumes by height meter for the whole subset."""
    prof = expand_crown_volume(df_sub.assign(__sum__=t("sum_label")), "__sum__")
    if prof.empty:
        return np.zeros(H, dtype=float)
    d = (
        prof.groupby("height_bin")["volume"]
        .sum()
        .reindex(range(0, H), fill_value=0.0)
    )
    return d.to_numpy(dtype=float)


# ------------------------------------------------------------
# MAIN RENDER
# ------------------------------------------------------------
def render_crown_volume_profiles(df_all: pd.DataFrame, primary: str, overlay: str | None):
    need = {"height", "crownStartHeight", "crownVoxelCountPerMeters", "crownVoxelSize"}
    miss = need - set(df_all.columns)
    if miss:
        st.warning(t("missing_columns_crown", columns=", ".join(sorted(miss))))
        return

    hmax = pd.to_numeric(df_all["height"], errors="coerce").max()
    if not np.isfinite(hmax):
        st.warning(t("invalid_height_values"))
        return

    H = max(1, int(math.ceil(float(hmax))))
    y_centers = np.arange(0, H, 1) + 0.5

    species_cmap = _species_colors(df_all)
    mgmt_cmap = _management_colors(df_all)

    df_before = df_all
    df_after = df_all[mask_after]
    df_removed = df_all[mask_removed]

    def _colormap(col: str):
        return species_cmap if col == "species" else mgmt_cmap

    def _resolve_col(layer_key: str | None) -> str | None:
        if layer_key is None:
            return None
        return "species" if layer_key == LAYER_SPECIES else "management_status"

    primary_col = _resolve_col(primary)
    overlay_col = _resolve_col(overlay) if overlay else None
    if overlay_col == primary_col:
        overlay_col = None

    prof_before = profiles_by(df_before, H, primary_col, _colormap(primary_col))
    prof_after = profiles_by(df_after, H, primary_col, _colormap(primary_col))
    prof_removed = profiles_by(df_removed, H, primary_col, _colormap(primary_col))

    if overlay_col is not None:
        overlay_before = profiles_by(df_before, H, overlay_col, _colormap(overlay_col))
        overlay_after = profiles_by(df_after, H, overlay_col, _colormap(overlay_col))
        overlay_removed = profiles_by(df_removed, H, overlay_col, _colormap(overlay_col))
    else:
        overlay_before = overlay_after = overlay_removed = None

    def global_max_volume():
        vals = [
            float(prof_before["volume"].max() if not prof_before.empty else 0.0),
            float(prof_after["volume"].max() if not prof_after.empty else 0.0),
            float(prof_removed["volume"].max() if not prof_removed.empty else 0.0),
            float(sum_profile(df_before, H).max()),
            float(sum_profile(df_after, H).max()),
            float(sum_profile(df_removed, H).max()),
        ]
        if overlay_col is not None:
            vals.extend([
                float(overlay_before["volume"].max() if not overlay_before.empty else 0.0),
                float(overlay_after["volume"].max() if not overlay_after.empty else 0.0),
                float(overlay_removed["volume"].max() if not overlay_removed.empty else 0.0),
            ])
        return max(vals) if vals else 0.0

    x_upper = _nice_upper(global_max_volume())

    def _sum_per_ha(dfx: pd.DataFrame) -> float:
        denom = float(area_ha) if area_ha > 0 else 1.0
        return float(crown_volume_per_tree(dfx).sum()) / denom

    title_before = f"{Before} · Σ { _sum_per_ha(df_before):.0f} {t('m3_per_ha')}"
    title_after = f"{After} · Σ { _sum_per_ha(df_after):.0f} {t('m3_per_ha')}"
    title_removed = f"{Removed} · Σ { _sum_per_ha(df_removed):.0f} {t('m3_per_ha')}"

    fig = make_subplots(
        rows=1,
        cols=3,
        shared_yaxes=True,
        subplot_titles=(title_before, title_after, title_removed),
        horizontal_spacing=0.06,
    )

    if "plot_title_font" in st.session_state:
        fig.update_layout(
            annotations=[
                dict(
                    text=ann.text,
                    x=ann.x,
                    y=ann.y,
                    xref=ann.xref,
                    yref=ann.yref,
                    showarrow=False,
                    font=st.session_state.plot_title_font,
                )
                for ann in fig.layout.annotations
            ]
        )

    def _group_label_for(col_name: str) -> str:
        return t("species") if col_name == "species" else t("management_label")

    def _display_group_value(group_col: str, value: str) -> str:
        return t_mgmt(value) if group_col == "management_status" else value

    def add_panel(prof_df: pd.DataFrame, col: int, show_legend: bool, group_col: str):
        is_species = (group_col == "species")
        width = 6 if is_species else 4
        dash = None if is_species else "dash"
        opacity = 0.65 if is_species else 0.8

        group_label = _group_label_for(group_col)

        order = (
            prof_df.groupby(group_col)["volume"].sum().sort_values(ascending=False).index.tolist()
            if not prof_df.empty and group_col in prof_df.columns
            else []
        )

        for grp in order:
            grp_label = _display_group_value(group_col, grp)
            d = (
                prof_df[prof_df[group_col] == grp]
                .set_index("height_bin")
                .reindex(range(0, H), fill_value=0.0)
            )
            x_series = d["volume"].to_numpy(dtype=float)
            if np.allclose(x_series, 0.0):
                continue

            col_hex = d["color"].iloc[0] if "color" in d.columns and len(d) else "#AAAAAA"

            fig.add_trace(
                go.Scatter(
                    x=x_series,
                    y=y_centers,
                    mode="lines",
                    name=grp_label ,
                    legendgroup=f"{group_col}-{grp}",
                    showlegend=show_legend,
                    line=dict(shape="spline", width=width, dash=dash),
                    opacity=opacity,
                    marker_color=col_hex,
                    hovertemplate=(
                        f"{group_label}: {grp}<br>"
                        f"{t('axis_height_above')}: %{{y:.0f}}<br>"
                        f"{t('crown_volume_m3')}: %{{x:.0f}}<extra></extra>"
                    ),
                ),
                row=1,
                col=col,
            )

    # primary
    add_panel(prof_before, col=1, show_legend=True, group_col=primary_col)
    add_panel(prof_after, col=2, show_legend=False, group_col=primary_col)
    add_panel(prof_removed, col=3, show_legend=False, group_col=primary_col)

    # overlay
    if overlay_before is not None and overlay_col is not None:
        add_panel(overlay_before, col=1, show_legend=True, group_col=overlay_col)
        add_panel(overlay_after, col=2, show_legend=False, group_col=overlay_col)
        add_panel(overlay_removed, col=3, show_legend=False, group_col=overlay_col)

    # SUM curves
    sum_before = sum_profile(df_before, H)
    sum_after = sum_profile(df_after, H)
    sum_removed = sum_profile(df_removed, H)

    sum_name = t("sum_label")
    for col, arr in [(1, sum_before), (2, sum_after), (3, sum_removed)]:
        fig.add_trace(
            go.Scatter(
                x=arr,
                y=y_centers,
                mode="lines",
                name=sum_name,
                legendgroup=sum_name,
                showlegend=(col == 1),
                line=dict(color="#555555", width=4, dash="dot"),
                hovertemplate=(
                    f"{sum_name}<br>"
                    f"{t('axis_height_above')}: %{{y:.0f}}<br>"
                    f"{t('crown_volume_m3')}: %{{x:.0f}}<extra></extra>"
                ),
            ),
            row=1,
            col=col,
        )

    fig.update_layout(
        height=CHART_HEIGHT,
        margin=dict(l=10, r=10, t=60, b=60),
        legend=dict(orientation="h", yanchor="top", y=-0.2, xanchor="center", x=0.5),
        hovermode="y unified",
    )

    for c in (1, 2, 3):
        fig.update_xaxes(
            title_text=t("crown_volume_m3"),
            row=1,
            col=c,
            rangemode="tozero",
            range=[0, x_upper],
        )

    fig.update_yaxes(
        title_text=t("axis_height_above"),
        row=1,
        col=1,
        rangemode="tozero",
        range=[0, float(H)],
        tick0=0,
        dtick=5,
    )
    fig.update_yaxes(row=1, col=2, rangemode="tozero", range=[0, float(H)], tick0=0, dtick=5)
    fig.update_yaxes(row=1, col=3, rangemode="tozero", range=[0, float(H)], tick0=0, dtick=5)

    st.plotly_chart(fig, use_container_width=True)


render_crown_volume_profiles(df, primary=primary, overlay=overlay)

with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("canopy_help"))