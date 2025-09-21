import streamlit as st
import pandas as pd
import numpy as np
import math
import plotly.express as px
import src.io as io
from plotly.subplots import make_subplots
import plotly.graph_objects as go

# --- Data ---
if "trees" not in st.session_state:
    file_path = st.session_state["file_path"]
    st.session_state.trees = io.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()

# ========== UI HLAVIČKA ==========

st.markdown("### Marteloscope site: XXX")
st.markdown("new line")
st.markdown("new line 2")

# Přepínač typu distribuce (segmented_control pokud je k dispozici)
dist_options = ["Species Distribution", "DBH Distribution", "Height Distribution"]
c_left, c_mid, c_right = st.columns([1, 2, 1])

with c_left:
    st.markdown("#### Show Management by:")

with c_mid:
    dist_mode = st.segmented_control("", options=dist_options, default="Species Distribution")

# ========== SPOLEČNÉ POMŮCKY ==========
CHART_HEIGHT = 200  # výška každého grafu

df = df.copy()
if "speciesColorHex" in df.columns:
    df["speciesColorHex"] = df["speciesColorHex"]
if "species" in df.columns:
    df["species"] = df["species"].astype(str)

keep_status = {"Target tree", "Untouched"}
mask_after   = df.get("management_status", pd.Series(False, index=df.index)).isin(keep_status)
mask_removed = ~mask_after if "management_status" in df.columns else pd.Series(False, index=df.index)

# ========== SPECIES DISTRIBUTION ==========
def render_species_distribution(df_all: pd.DataFrame):
    required = {"species", "management_status", "speciesColorHex"}
    missing = required - set(df_all.columns)
    if missing:
        st.warning("Chybí sloupce pro Species distribution: " + ", ".join(sorted(missing)))
        return

    def counts_by_species(sub: pd.DataFrame) -> pd.DataFrame:
        return (sub.groupby("species", as_index=False)
                    .agg(count=("species", "size"),
                         speciesColorHex=("speciesColorHex", "first")))

    before_df  = counts_by_species(df_all)
    after_df   = counts_by_species(df_all[mask_after])
    removed_df = counts_by_species(df_all[mask_removed])

    # Pořadí dle Before
    species_order = before_df.sort_values("count", ascending=False)["species"].tolist()

    # Doplň chybějící druhy na 0 a barvy z before_df
    def ensure_all_species(sub_counts: pd.DataFrame) -> pd.DataFrame:
        base = pd.DataFrame({"species": species_order})
        sub  = base.merge(sub_counts, on="species", how="left")
        sub["count"] = sub["count"].fillna(0).astype(int)
        color_map_full = dict(zip(before_df["species"], before_df["speciesColorHex"]))
        sub["speciesColorHex"] = sub["species"].map(color_map_full).fillna("#AAAAAA")
        return sub

    before_df  = ensure_all_species(before_df)
    after_df   = ensure_all_species(after_df)
    removed_df = ensure_all_species(removed_df)

    # Barvy per druh do px.bar
    discrete_map = dict(zip(before_df["species"], before_df["speciesColorHex"]))

    # Fix Y (zaokrouhlení nahoru na 50)
    y_max_before = int(before_df["count"].max() if len(before_df) else 0)
    y_upper = int(max(50, math.ceil(y_max_before / 50.0) * 50))

    def bar(fig_df: pd.DataFrame, title: str):
        fig = px.bar(
            fig_df,
            x="species",
            y="count",
            color="species",
            color_discrete_map=discrete_map,
            category_orders={"species": species_order},
            title=title
        )
        fig.update_layout(
            showlegend=False,
            height=CHART_HEIGHT + 100,
            margin=dict(l=10, r=10, t=40, b=10),
        )
        fig.update_xaxes(title=None, tickangle=45)
        fig.update_yaxes(title="Trees", range=[0, y_upper], tick0=0, dtick=50)
        return fig

    c1, c2, c3 = st.columns(3)
    with c1:
        st.plotly_chart(bar(before_df, "Before"), use_container_width=True)
    with c2:
        st.plotly_chart(bar(after_df,  "After"), use_container_width=True)
    with c3:
        st.plotly_chart(bar(removed_df, "Removed"), use_container_width=True)

# ========== OBECNÁ BINNERKA PRO DBH / HEIGHT ==========
# --- helper: barvy druhů z celého df (konzistence napříč grafy) ---


def _species_colors(df_all: pd.DataFrame) -> dict:
    if "species" not in df_all.columns or "speciesColorHex" not in df_all.columns:
        return {}
    tmp = (df_all.assign(species=lambda d: d["species"].astype(str),
                         speciesColorHex=lambda d: d["speciesColorHex"])
                 .groupby("species")["speciesColorHex"].first())
    return tmp.to_dict()

# --- helper: společné biny a popisky z CELÉHO df (stejná X-osa pro všechny tři grafy) ---
def _make_bins_labels(df_all: pd.DataFrame, value_col: str, bin_size: float, unit_label: str):
    vals = pd.to_numeric(df_all.get(value_col), errors="coerce")
    vals_ok = vals.dropna()
    if vals_ok.empty:
        return None, None
    vmin = float(np.floor(vals_ok.min() / bin_size) * bin_size)
    vmax = float(np.ceil (vals_ok.max()  / bin_size) * bin_size)
    if vmax <= vmin:
        vmax = vmin + bin_size
    bins = np.arange(vmin, vmax + bin_size, bin_size, dtype=float)
    labels = [f"{int(b)}–{int(b + bin_size)} {unit_label}" for b in bins[:-1]]
    return bins, labels


# --- robustní masky (nepočítat NaN jako Removed) ---
def _make_masks(df: pd.DataFrame):
    keep_status = {"Target tree", "Untouched"}
    mask_after   = df.get("management_status", pd.Series(False, index=df.index)).isin(keep_status)
    mask_removed = ~mask_after if "management_status" in df.columns else pd.Series(False, index=df.index)
    print(mask_removed)
    return mask_after, mask_removed


# --- helper pro výšku osy Y: max počet stromů v binu (z daného sloupce) ---
def _y_upper_for(df_all: pd.DataFrame, value_col: str, bins: np.ndarray, labels: list[str]) -> int:
    vals = pd.to_numeric(df_all.get(value_col), errors="coerce")
    cats = pd.cut(vals, bins=bins, labels=labels, include_lowest=True, right=False, ordered=True)
    vc = cats.value_counts()
    if vc.empty:
        return 10
    return int(max(10, math.ceil(vc.max() / 10.0) * 10))

def _compute_binned_counts(df_sub: pd.DataFrame,
                           value_col: str,
                           bins: np.ndarray,
                           color_map_species: dict,
                           labels: list[str]) -> pd.DataFrame:
    """
    Vrátí long DF: ['bin','species','count'] se zaručenými nulami pro chybějící kombinace
    a stabilním pořadím tříd (labels). Ošetří i prázdné/NaN hodnoty.
    """
    d = df_sub.copy()

    cat = pd.Categorical(pd.cut(d[value_col], bins=bins, labels=labels,
                                include_lowest=True, right=False, ordered=True),
                         categories=labels, ordered=True)
    d = d.assign(bin=cat)

    # pivot = čisté počty řádků (kusy)
    species_all = sorted(pd.Index(d["species"]).union(pd.Index(color_map_species.keys())))
    idx = pd.MultiIndex.from_product([labels, species_all], names=["bin", "species"])


    pv = (d.pivot_table(index="bin", columns="species", aggfunc="size", fill_value=0))
    print(pv)
    long = pv.stack().rename("count").reset_index()

    # barvy
    discrete_map = {sp: color_map_species.get(sp, "#AAAAAA") for sp in long["species"].unique()}
    long["count"] = long["count"].astype(int)
    print(long)
    return long


def build_three_panel_shared_legend(df_all: pd.DataFrame,
                                    value_col: str,
                                    bin_size: float,
                                    unit_label: str,
                                    color_map: dict,
                                    m_after: pd.Series,
                                    m_removed: pd.Series,
                                    y_upper: int):
    """Vytvoří 3 podgrafy vedle sebe + jednu sdílenou legendu dole."""
    # společné biny a štítky
    bins, labels = _make_bins_labels(df_all, value_col, bin_size, unit_label)
    if bins is None:
        return None

    # počty v long formátu pro každou fázi
    long_before  = _compute_binned_counts(df_all,              value_col, bins, color_map,labels)
    long_after   = _compute_binned_counts(df_all[m_after],     value_col, bins, color_map,labels)
    long_removed = _compute_binned_counts(df_all[m_removed],   value_col, bins, color_map,labels)

    # sjednocený seznam druhů kvůli barvám + stabilní pořadí
    species_all = sorted(pd.Index(long_before["species"]).astype(str).str.strip().unique())
    print(species_all)

    # default barvy pro chybějící druhy
    def color_for(sp):
        return color_map.get(sp, "#AAAAAA")

    # vytvoření subplots
    fig = make_subplots(
        rows=1, cols=3,
        subplot_titles=("Before", "After", "Removed"),
        horizontal_spacing=0.06
    )

    # přidej stackované bary po druzích do každého subgrafu
    # aby se legenda nedublovala: showlegend=True pouze při prvním přidání daného druhu (v 1. podgrafu)
    def add_traces(long_df: pd.DataFrame, col: int, show_legend_for_species: set[str]):
        for sp in species_all:
            y_vals = (long_df[long_df["species"] == sp]
                      .set_index("bin")
                      .reindex(labels)["count"]
                      .fillna(0)
                      .tolist())
            fig.add_trace(
                go.Bar(
                    x=labels,
                    y=y_vals,
                    name=sp,
                    marker_color=color_for(sp),
                    showlegend=(sp in show_legend_for_species),
                    hovertemplate=f"%{{x}}<br>Species: {sp}<br>Trees: %{{y}}<extra></extra>",
                ),
                row=1, col=col
            )

    # legenda jen v prvním panelu
    add_traces(long_before,  col=1, show_legend_for_species=set(species_all))
    add_traces(long_after,   col=2, show_legend_for_species=set())  # bez legendy
    add_traces(long_removed, col=3, show_legend_for_species=set())  # bez legendy

    # layout
    fig.update_layout(
        barmode="stack",
        height=CHART_HEIGHT + 140,  # trochu místa navíc na legendu
        margin=dict(l=10, r=10, t=60, b=80),
        legend=dict(
            orientation="h",
            yanchor="top",
            y=-0.35,     # pod grafy
            xanchor="center",
            x=0.5,
            title="Species"
        )
    )
    # osy
    fig.update_xaxes(title_text=None, tickangle=45, categoryorder="array", categoryarray=labels)
    # titul pouze u 1. panelu
    fig.update_yaxes(title_text="Trees", row=1, col=1, tick0=0, dtick=25, range=[0, y_upper])
    fig.update_yaxes(title_text=None, row=1, col=2, tick0=0, dtick=25, range=[0, y_upper])
    fig.update_yaxes(title_text=None, row=1, col=3, tick0=0, dtick=25, range=[0, y_upper])

    return fig

# === Nahraďte vaši původní funkci render_dbh_distribution tímto ===
def render_dbh_distribution(df_all: pd.DataFrame):
    bins, labels = _make_bins_labels(df_all, "dbh", 10, "cm")
    if bins is None:
        st.info("Žádné platné DBH hodnoty.")
        return

    color_map = _species_colors(df_all)
    m_after, m_removed = _make_masks(df_all)
    y_upper = _y_upper_for(df_all, "dbh", bins, labels)

    fig = build_three_panel_shared_legend(
        df_all=df_all,
        value_col="dbh",
        bin_size=10,
        unit_label="cm",
        color_map=color_map,
        m_after=m_after,
        m_removed=m_removed,
        y_upper=y_upper
    )
    if fig is None:
        st.info("Žádná data pro DBH graf.")
        return

    st.plotly_chart(fig, use_container_width=True)

# === Nahraďte vaši původní funkci render_height_distribution tímto ===
def render_height_distribution(df_all: pd.DataFrame):
    if "height" not in df_all.columns:
        st.warning("Chybí sloupec 'height' pro Height distribution.")
        return

    bins, labels = _make_bins_labels(df_all, "height", 5, "m")
    if bins is None:
        st.info("Žádné platné Height hodnoty.")
        return

    color_map = _species_colors(df_all)
    m_after, m_removed = _make_masks(df_all)
    y_upper = _y_upper_for(df_all, "height", bins, labels)

    fig = build_three_panel_shared_legend(
        df_all=df_all,
        value_col="height",
        bin_size=5,
        unit_label="m",
        color_map=color_map,
        m_after=m_after,
        m_removed=m_removed,
        y_upper=y_upper
    )
    if fig is None:
        st.info("Žádná data pro Height graf.")
        return

    st.plotly_chart(fig, use_container_width=True)


# ========== RENDER PODLE PŘEPÍNAČE ==========
if dist_mode == "Species Distribution":
    render_species_distribution(df)
elif dist_mode == "DBH Distribution":
    render_dbh_distribution(df)
else:  # Height distribution
    render_height_distribution(df)
