# -*- coding: utf-8 -*-
# ------------------------------------------------------------
# Intensity of Silvicultural Intervention — i18n version
# - Robust to empty / non-plottable filter results
# - Shows empty charts + warning when nothing meaningful to plot
# ------------------------------------------------------------

import streamlit as st
import pandas as pd
import numpy as np
import plotly.graph_objects as go
import src.io_utils as iou
import math

from src.i18n import t, t_help, t_mgmt


# ------------------------------------------------------------
# LOAD DATA
# ------------------------------------------------------------
if "trees" not in st.session_state:
    file_path = st.session_state.project_file
    st.session_state.trees = iou.load_project_json(file_path)

df0 = st.session_state.trees.copy()

# normalize strings (important for keep_status + grouping)
df0["species"] = df0["species"].astype(str).str.strip()
df0["management_status"] = df0["management_status"].astype(str).str.strip()

# numeric fix
for col in ["dbh", "height", "Volume_m3", "BA_m2", "crown_volume"]:
    if col in df0:
        df0[col] = pd.to_numeric(df0[col], errors="coerce")

# basal area if missing
if "BA_m2" not in df0:
    df0["BA_m2"] = np.pi * (df0["dbh"] / 200.0) ** 2

# crown volume placeholder if missing
if "crown_volume" not in df0:
    df0["crown_volume"] = np.nan


# ------------------------------------------------------------
# UI
# ------------------------------------------------------------
st.markdown(f"### **{t('intensity_header')}**")

c1, c2, c3, c4, c5, c6, c7, c8, c9 = st.columns([0.5, 2, 0.5, 2, 0.5, 2, 0.5, 2, 0.5])

METRIC_TREE_COUNT = "metric_tree_count"
METRIC_VOLUME_M3 = "metric_volume_m3"
METRIC_BASAL_AREA_M2 = "metric_basal_area_m2"
CROWN_VOLUME_M3 = "crown_volume_m3"

PLOT_BY_SPECIES = "species"
PLOT_BY_MANAGEMENT = "management_label"

with c2:
    metric_id = st.selectbox(
        f"**{t('intensity_based_on')}**",
        options=[METRIC_TREE_COUNT, METRIC_VOLUME_M3, METRIC_BASAL_AREA_M2, CROWN_VOLUME_M3],
        format_func=lambda k: t(k),
    )

with c4:
    group_by_id = st.segmented_control(
        f"**{t('plot_by')}**",
        options=[PLOT_BY_SPECIES, PLOT_BY_MANAGEMENT],
        format_func=lambda k: t(k),
        default=PLOT_BY_SPECIES,
        width="stretch",
    )

with c6:
    dbh_vals = df0["dbh"].dropna()
    if dbh_vals.empty:
        dbh_min, dbh_max = 0, 1
    else:
        dbh_min, dbh_max = float(dbh_vals.min()), float(dbh_vals.max())
        dbh_min = math.floor(dbh_min)
        dbh_max = math.ceil(dbh_max)
        if dbh_max <= dbh_min:
            dbh_max = dbh_min + 1
    dbh_range = st.slider(f"**{t('dbh_filter')}**", dbh_min, dbh_max, (dbh_min, dbh_max))

with c8:
    h_vals = df0["height"].dropna()
    if h_vals.empty:
        h_min, h_max = 0, 1
    else:
        h_min, h_max = float(h_vals.min()), float(h_vals.max())
        h_min = math.floor(h_min)
        h_max = math.ceil(h_max)
        if h_max <= h_min:
            h_max = h_min + 1
    height_range = st.slider(f"**{t('height_filter')}**", h_min, h_max, (h_min, h_max))


# ------------------------------------------------------------
# FILTER
# ------------------------------------------------------------
df = df0.copy()
df = df[(df["dbh"] >= dbh_range[0]) & (df["dbh"] <= dbh_range[1])]
df = df[(df["height"] >= height_range[0]) & (df["height"] <= height_range[1])]


# ------------------------------------------------------------
# METRIC COLUMN
# ------------------------------------------------------------
if metric_id == METRIC_TREE_COUNT:
    df["metric_value"] = 1.0
elif metric_id == METRIC_VOLUME_M3:
    df["metric_value"] = df.get("Volume_m3", pd.Series(index=df.index, dtype=float)).fillna(0.0)
elif metric_id == METRIC_BASAL_AREA_M2:
    df["metric_value"] = df.get("BA_m2", pd.Series(index=df.index, dtype=float)).fillna(0.0)
else:
    df["metric_value"] = df.get("crown_volume", pd.Series(index=df.index, dtype=float)).fillna(0.0)


# ------------------------------------------------------------
# REMOVAL MASK
# ------------------------------------------------------------
keep_status = {"Target tree", "Untouched"}
df["is_removed"] = ~df["management_status"].isin(keep_status)


# ------------------------------------------------------------
# GROUPING LOGIC
# ------------------------------------------------------------
if group_by_id == PLOT_BY_SPECIES:
    main_group = "species"
    stack_group = "management_status"
    color_col = "managementColorHex"
else:
    main_group = "management_status"
    stack_group = "species"
    color_col = "speciesColorHex"

if color_col not in df.columns:
    df[color_col] = "#AAAAAA"
else:
    df[color_col] = df[color_col].fillna("#AAAAAA").astype(str)


# ------------------------------------------------------------
# SAFE PIVOTS (FULL)
# ------------------------------------------------------------
def _empty_pivot() -> pd.DataFrame:
    return pd.DataFrame()


if df.empty:
    pivot_total_all = _empty_pivot()
    pivot_removed_all = _empty_pivot()
else:
    pivot_total_all = df.pivot_table(
        index=main_group,
        columns=stack_group,
        values="metric_value",
        aggfunc="sum",
        fill_value=0.0,
    )
    pivot_removed_all = df[df["is_removed"]].pivot_table(
        index=main_group,
        columns=stack_group,
        values="metric_value",
        aggfunc="sum",
        fill_value=0.0,
    )


# ------------------------------------------------------------
# GROUPS (SAFE)
# ------------------------------------------------------------
if pivot_total_all.empty:
    all_main_groups = []
    stack_groups = []
else:
    if group_by_id == PLOT_BY_SPECIES:
        all_main_groups = sorted(pivot_total_all.index.tolist())
    else:
        # v management módu nechceme ukazovat "Target tree" a "Untouched" jako "zásah"
        all_main_groups = sorted([g for g in pivot_total_all.index if g not in keep_status])

    if group_by_id == PLOT_BY_SPECIES:
        stack_groups = [c for c in pivot_total_all.columns if c not in keep_status and pivot_total_all[c].sum() > 0]
    else:
        stack_groups = [c for c in pivot_total_all.columns if pivot_total_all[c].sum() > 0]


# ------------------------------------------------------------
# SAFE REINDEXED PIVOTS
# ------------------------------------------------------------
if pivot_total_all.empty or not all_main_groups or not stack_groups:
    pivot_total = pd.DataFrame(index=all_main_groups, columns=stack_groups)
    pivot_removed = pd.DataFrame(index=all_main_groups, columns=stack_groups)
else:
    pivot_total = pivot_total_all.reindex(index=all_main_groups, columns=stack_groups, fill_value=0.0)
    pivot_removed = pivot_removed_all.reindex(index=all_main_groups, columns=stack_groups, fill_value=0.0)


# ------------------------------------------------------------
# PROCENTA (SAFE)
# ------------------------------------------------------------
total_all = float(pivot_total_all.values.sum()) if not pivot_total_all.empty else 0.0

if total_all > 0 and (not pivot_removed.empty) and (len(pivot_removed.columns) > 0):
    pct_from_total = (pivot_removed / total_all * 100.0)
else:
    pct_from_total = pivot_removed.copy()
    if not pct_from_total.empty and len(pct_from_total.columns) > 0:
        pct_from_total.loc[:, :] = 0.0

if not pivot_total_all.empty and all_main_groups:
    group_sums = pivot_total_all.reindex(all_main_groups).sum(axis=1)
else:
    group_sums = pd.Series(dtype=float)

if (not group_sums.empty) and (not pivot_removed.empty) and (len(pivot_removed.columns) > 0):
    group_sums_safe = group_sums.replace(0.0, np.nan)
    pct_in_group = (pivot_removed.div(group_sums_safe, axis=0) * 100.0).fillna(0.0)
else:
    pct_in_group = pivot_removed.copy()
    if not pct_in_group.empty and len(pct_in_group.columns) > 0:
        pct_in_group.loc[:, :] = 0.0


# ------------------------------------------------------------
# TOTAL řádek (Sum) — SAFE bez .loc při DF bez sloupců
# ------------------------------------------------------------
sum_label = t("sum_label")

if stack_groups and total_all > 0 and (not pivot_removed_all.empty):
    total_row_stack = (
        pivot_removed_all.reindex(index=pivot_total_all.index, columns=stack_groups, fill_value=0.0).sum(axis=0)
        / total_all
        * 100.0
    )
else:
    total_row_stack = pd.Series(0.0, index=stack_groups, dtype=float)

pct_from_total_plot = pct_from_total.copy()

# ✅ Kritická oprava: když DF nemá sloupce, nepoužívej .loc[...] = ...
if len(stack_groups) == 0:
    # nech jen index se sum řádkem, aby graf byl prázdný, ale layout fungoval
    pct_from_total_plot = pd.DataFrame(index=[sum_label])
else:
    # zajisti, že DF má přesně stack_groups jako sloupce
    if pct_from_total_plot.empty or list(pct_from_total_plot.columns) != list(stack_groups):
        pct_from_total_plot = pd.DataFrame(index=pct_from_total_plot.index, columns=stack_groups).fillna(0.0)

    # přidej sum řádek bezpečně přes concat
    sum_df = pd.DataFrame([total_row_stack.reindex(stack_groups).to_numpy()], index=[sum_label], columns=stack_groups)
    pct_from_total_plot = pd.concat([pct_from_total_plot, sum_df], axis=0)

summary_intensity = float(total_row_stack.sum()) if len(total_row_stack) else 0.0
summary_title = t("summary_total_selection_intensity", value=f"{summary_intensity:.1f}")


# ------------------------------------------------------------
# BARVY PRO STACK (SAFE)
# ------------------------------------------------------------
stack_colors = {}
for sg in stack_groups:
    row = df[df[stack_group] == sg]
    stack_colors[sg] = row[color_col].iloc[0] if len(row) else "#777777"


# ------------------------------------------------------------
# WARNING LOGIC (robust)
# ------------------------------------------------------------
def _has_any_positive_values(pv: pd.DataFrame) -> bool:
    if pv is None or pv.empty or len(pv.columns) == 0:
        return False
    arr = pv.to_numpy(dtype=float)
    return np.isfinite(arr).any() and float(np.nansum(arr)) > 0.0

plot_matrix = (
    pivot_total_all.reindex(index=all_main_groups, columns=stack_groups, fill_value=0.0)
    if (not pivot_total_all.empty and all_main_groups and stack_groups)
    else pd.DataFrame()
)

no_data_for_plot = (
    df.empty
    or pivot_total_all.empty
    or (len(all_main_groups) == 0)
    or (len(stack_groups) == 0)
    or (not _has_any_positive_values(plot_matrix))
)


# ------------------------------------------------------------
# LAYOUT – DVA GRAFY
# ------------------------------------------------------------
left, right = st.columns([1, 1])

if no_data_for_plot:
    st.warning(t("warn_no_data_for_filters"))

def display_group_label(g: str) -> str:
    return t_mgmt(g) if main_group == "management_status" else str(g)

# ------------------------------------------------------------
# 🔵 GRAF 1 — REMOVAL FROM TOTAL
# ------------------------------------------------------------
with left:
    fig1 = go.Figure()

    if (not no_data_for_plot) and stack_groups and (len(stack_groups) > 0):
        for sg in stack_groups:
            cd_rows = []
            for g in pct_from_total_plot.index:
                if g == sum_label:
                    removed_val = float(
                        pivot_removed_all.reindex(columns=[sg], fill_value=0.0)[sg].sum()
                    ) if not pivot_removed_all.empty else 0.0
                    total_val = float(pivot_total_all.values.sum()) if not pivot_total_all.empty else 0.0
                else:
                    if pivot_removed_all.empty or pivot_total_all.empty:
                        removed_val, total_val = 0.0, 0.0
                    else:
                        removed_val = float(
                            pivot_removed_all.reindex(index=[g], columns=[sg], fill_value=0.0).iloc[0, 0]
                        )
                        total_val = float(pivot_total_all.reindex(index=[g], fill_value=0.0).loc[g, :].sum())
                cd_rows.append([removed_val, total_val])

            custom = np.array(cd_rows, dtype=float) if cd_rows else np.empty((0, 2), dtype=float)

            label = t_mgmt(sg) if stack_group == "management_status" else str(sg)

            fig1.add_trace(
                go.Bar(
                    y=[display_group_label(g) for g in pct_from_total_plot.index],
                    x=pct_from_total_plot[sg] if (sg in pct_from_total_plot.columns) else [0.0] * len(pct_from_total_plot.index),
                    name=label,
                    orientation="h",
                    marker_color=stack_colors.get(sg, "#777777"),
                    customdata=custom,
                    hovertemplate=(
                        f"{t('group_label')}: %{{y}}<br>"
                        f"{t('removed_label')}: %{{customdata[0]:.1f}}<br>"
                        f"{t('total_label')}: %{{customdata[1]:.1f}}<br>"
                        f"{t('percent_label')}: %{{x:.1f}}%<extra></extra>"
                    ),
                )
            )

    fig1.update_layout(
        height=400,
        barmode="stack",
        title={"text": summary_title, "x": 0.5, "xanchor": "center"},
        xaxis_title=t("removal_percent_axis_title"),
        yaxis_title="",
        legend_title=t("legend_title"),
    )
    fig1.update_xaxes(range=[0, 100])
    st.plotly_chart(fig1, use_container_width=True)


# ------------------------------------------------------------
# 🔵 GRAF 2 — INSIDE GROUP INTENSITY
# ------------------------------------------------------------
with right:
    fig2 = go.Figure()

    if (not no_data_for_plot) and stack_groups and (len(stack_groups) > 0):
        for sg in stack_groups:
            cd_rows = []
            for g in pct_in_group.index:
                if pivot_removed_all.empty or pivot_total_all.empty:
                    removed_val, total_val = 0.0, 0.0
                else:
                    removed_val = float(pivot_removed_all.reindex(index=[g], columns=[sg], fill_value=0.0).iloc[0, 0])
                    total_val = float(pivot_total_all.reindex(index=[g], fill_value=0.0).loc[g, :].sum())
                cd_rows.append([removed_val, total_val])

            custom = np.array(cd_rows, dtype=float) if cd_rows else np.empty((0, 2), dtype=float)
            label = t_mgmt(sg) if stack_group == "management_status" else str(sg)

            fig2.add_trace(
                go.Bar(
                    y=[display_group_label(g) for g in pct_in_group.index],
                    x=pct_in_group[sg] if (sg in pct_in_group.columns) else [0.0] * len(pct_in_group.index),
                    name=label,
                    orientation="h",
                    marker_color=stack_colors.get(sg, "#777777"),
                    customdata=custom,
                    hovertemplate=(
                        f"{t('group_label')}: %{{y}}<br>"
                        f"{t('removed_label')}: %{{customdata[0]:.1f}}<br>"
                        f"{t('total_label')}: %{{customdata[1]:.1f}}<br>"
                        f"{t('percent_label')}: %{{x:.1f}}%<extra></extra>"
                    ),
                )
            )

    fig2.update_layout(
        height=400,
        barmode="stack",
        title={"text": t("intensity_in_group_title"), "x": 0.5, "xanchor": "center"},
        xaxis_title=t("removal_percent_axis_title"),
        yaxis_title="",
        legend_title=t("legend_title"),
    )
    fig2.update_xaxes(range=[0, 100])
    st.plotly_chart(fig2, use_container_width=True)

with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("intensity_help"))