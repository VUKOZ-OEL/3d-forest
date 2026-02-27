# src/Simulation.py
import ctypes
import os
from pathlib import Path
import shutil

import streamlit as st
import pandas as pd
import numpy as np
import plotly.graph_objects as go

import src.io_utils as iou
import src.simul_utils2 as sut
from src.i18n import t, t_help, t_mgmt
from src.species_dict import species_dict
from src.run_iland import run_iland


# =============================================================================
# HEADER
# =============================================================================
st.markdown(f"#### {t('simulation_header')}")

trees = st.session_state.trees
project_file = st.session_state.project_file

out_csv = sut.export_iland_trees_csv(
    trees=trees,
    species_dict=species_dict,
    project_file=project_file,
)



# =============================================================================
# CONFIG
# =============================================================================
# xml_path = Path("C:/Users/krucek/Documents/iLand/test/Pokojna_hora.xml")
# out_db = Path("C:/Users/krucek/Documents/iLand/test/output/output.sqlite")
# temp_db = Path("C:/Users/krucek/Documents/iLand/test/output/temp.sqlite")

xml_path = st.session_state.project_file.replace(".json", ".xml")
out_db = st.session_state.project_file.replace(".json", ".output.sqlite")
temp_db = st.session_state.project_file.replace(".json", ".temp.sqlite")

def _fan_label(g: str) -> str:
    if g == "SUM":
        return t("sum_label")
    return t_mgmt(g)

# =============================================================================
# UI
# =============================================================================
c1, _, c2, _, c3, _, c4 = st.columns([2, 0.25, 2, 0.25, 2, 0.25, 3])

scenario_labels = {
    "**RCP 4.5**": "rcp__4_5",
    "**RCP 8.5**": "rcp__8_5",
}

with c1:
    clima_label = st.segmented_control(
        f"**{t('clima_scenario')}**",
        options=list(scenario_labels.keys()),
        default="**RCP 4.5**",
        width="stretch",
    )
    clima_scenario = scenario_labels[clima_label]
    run_simul = st.button(
        f"**{t('button_run_simulation')}**",
        icon=":material/play_arrow:",
        width="stretch",
        type="primary",
    )

with c2:
    st.markdown(f"**{t('simulation_period')}**")
    years = st.slider("", min_value=0, max_value=100, value=30, step=5)

with c3:
    st.markdown(f"**{t('replications')}**")
    n_rep = st.slider("", min_value=1, max_value=100, value=10, step=1)

with c4:
    st.markdown(f"**{t('simulation_options')}**")
    c4l, c4r = st.columns([1, 1])

    with c4l:
        mortality = st.toggle(t("mortality_box"), value=True)
    with c4r:
        regeneration = st.toggle(t("regeneration_box"), value=True)

st.divider()


# =============================================================================
# RUN SIMULATION (MONTE CARLO)
# =============================================================================
if run_simul:
    all_living = []

    progress = st.progress(0.0, text=t("simulation_progress_running"))

    sut.set_iland_xml(xml_path, mortality, regeneration,clima_scenario,years)

    for i in range(1, n_rep + 1):
        percent = int(round((i-0.5) / n_rep * 100))

        progress.progress(
            (i-0.5) / n_rep,
            text=t("simulation_progress_replication").format(p=percent),
        )

        run_iland(str(xml_path).encode("utf-8"), years)
        shutil.copyfile(out_db, temp_db)

        living = sut.read_single_sqlite_living(
            temp_db,
            rep_id=f"rep_{i:03d}"
        )
        if not living.empty:
            all_living.append(living)

    progress.empty()

    if not all_living:
        st.error(t("simulation_no_output"))
        st.stop()

    sim_trees = pd.concat(all_living, ignore_index=True)

    trees = st.session_state.get("trees")
    sim_trees = sut.map_species_label(sim_trees, species_dict)
    sim_trees = sut.mark_target_trees(sim_trees, trees)
    sim_trees["year"] = pd.to_numeric(sim_trees["year"], errors="coerce")

    st.session_state.sim_trees = sim_trees


# =============================================================================
# STOP HERE if no simulation yet
# =============================================================================
if "sim_trees" in st.session_state:

    sim_trees = st.session_state.sim_trees

    # =============================================================================
    # COLORS
    # =============================================================================
    palette = st.session_state.get("color_palette") or iou.load_color_palette(
        st.session_state.get("project_file", "data/test_project.json")
    )

    species_colors = palette.get("species", {})
    management_colors = palette.get("management", {})

    SUM_COLOR = "#7A7A7A"

    mg_colors = {
        "Target tree": management_colors.get("Target tree", "#F4EE00"),
        "Untouched": management_colors.get("Untouched", "#A6A6A6"),
        "SUM": SUM_COLOR,
    }


    # =============================================================================
    # FAN CHART PLOTTER
    # =============================================================================
    def _hex_to_rgba(hex_color: str, alpha: float) -> str:
        h = hex_color.lstrip("#")
        r, g, b = (int(h[i:i + 2], 16) for i in (0, 2, 4))
        return f"rgba({r},{g},{b},{alpha})"


    def fig_fan(stats: pd.DataFrame, group_col: str, color_map: dict, title_key: str):
        fig = go.Figure()

        for g in stats[group_col].unique():
            s = stats[stats[group_col] == g].sort_values("year")
            color = color_map.get(g, SUM_COLOR)

            label = _fan_label(g)

            for lo, hi, a in [("q5", "q95", 0.15), ("q25", "q75", 0.25)]:
                fig.add_trace(go.Scatter(
                    x=s["year"],
                    y=s[hi],
                    mode="lines",
                    line=dict(width=0),
                    showlegend=False,
                    hoverinfo="skip",
                    legendgroup=g,
                ))
                fig.add_trace(go.Scatter(
                    x=s["year"],
                    y=s[lo],
                    mode="lines",
                    line=dict(width=0),
                    fill="tonexty",
                    fillcolor=_hex_to_rgba(color, a),
                    showlegend=False,
                    hoverinfo="skip",
                    legendgroup=g,
                ))

            fig.add_trace(go.Scatter(
                x=s["year"],
                y=s["q50"],
                mode="lines",
                name=label,                 # ✅ přeložený text
                legendgroup=g,              # ❗ interní ID zůstává
                showlegend=True,
                line=dict(color=color, width=4 if g == "SUM" else 2),
                hovertemplate=(
                    f"{label}<br>"
                    f"{t('chart_x_year')}: %{{x}}<br>"
                    f"{t('metric_volume_m3')}: %{{y:.0f}}"
                    "<extra></extra>"
                ),
            ))

        fig.update_layout(
            title=dict(
                text=t(title_key),
                x=0.5,
                xanchor="center",
            ),
            xaxis_title=t("chart_x_year"),
            yaxis_title=t("metric_volume_m3"),
            margin=dict(l=10, r=10, t=60, b=10),
            legend=dict(groupclick="togglegroup",),
            yaxis=dict(autorange=True,),
        )
        return fig


    # =============================================================================
    # BUILD FAN DATA
    # =============================================================================
    df = sim_trees[sim_trees["year"] <= years]

    fan_species = pd.concat([
        sut.agg_fan(df, ["year", "species_label"]).assign(group=lambda d: d["species_label"]),
        sut.agg_fan(df, ["year"]).assign(group="SUM"),
    ])

    fan_mgmt = pd.concat([
        sut.agg_fan(df, ["year", "management_group"]).assign(group=lambda d: d["management_group"]),
        sut.agg_fan(df, ["year"]).assign(group="SUM"),
    ])


    # =============================================================================
    # PLOTS
    # =============================================================================
    col1, col2 = st.columns(2, gap="large")

    with col1:
        st.plotly_chart(
            fig_fan(fan_species, "group", species_colors, "chart_volume_by_species"),
            width="stretch",
        )

    with col2:
        st.plotly_chart(
            fig_fan(fan_mgmt, "group", mg_colors, "chart_volume_by_management"),
            width="stretch",
        )

with st.expander(label=t("expander_help_label"), icon=":material/help:"):
    st.markdown(t_help("simulation_help"))
