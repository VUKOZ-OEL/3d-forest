# src/summary_charts.py
from __future__ import annotations

from typing import Callable, Optional, Tuple, Dict, List

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.figure import Figure

from src.i18n import t_mgmt

# ---------- Stable IDs ----------
STATUS_BEFORE = "label_before"
STATUS_AFTER = "label_after"
STATUS_REMOVED = "label_removed"

COLOR_BY_SPECIES = "species"
COLOR_BY_MANAGEMENT = "management_label"

METRIC_TREE_COUNT = "metric_tree_count"
METRIC_VOLUME = "metric_volume_m3"
METRIC_BASAL_AREA = "metric_basal_area_m2"
METRIC_CANOPY_COVER = "metric_canopy_cover_pct"


# ---------- Masks ----------
def _make_masks(d: pd.DataFrame) -> Dict[str, pd.Series]:
    keep = {"Target tree", "Untouched"}
    if "management_status" in d.columns:
        after = d["management_status"].astype(str).isin(keep)
        removed = ~after
    else:
        after = pd.Series(False, index=d.index)
        removed = pd.Series(False, index=d.index)
    before = pd.Series(True, index=d.index)
    return {
        STATUS_BEFORE: before,
        STATUS_AFTER: after,
        STATUS_REMOVED: removed,
    }


# ---------- Colors ----------
def _safe_hex(x: object, default: str = "#AAAAAA") -> str:
    if isinstance(x, str) and x.startswith("#") and len(x) == 7:
        return x
    return default


def _species_colors(d: pd.DataFrame) -> Dict[str, str]:
    if "species" not in d.columns or "speciesColorHex" not in d.columns:
        return {}
    return (
        d.assign(
            species=lambda x: x["species"].astype(str),
            col=lambda x: x["speciesColorHex"].map(_safe_hex),
        )
        .groupby("species")["col"]
        .first()
        .to_dict()
    )


def _management_colors(d: pd.DataFrame) -> Dict[str, str]:
    if "management_status" not in d.columns or "managementColorHex" not in d.columns:
        return {}
    return (
        d.assign(
            mgmt=lambda x: x["management_status"].astype(str),
            col=lambda x: x["managementColorHex"].map(_safe_hex),
        )
        .groupby("mgmt")["col"]
        .first()
        .to_dict()
    )


# ---------- Metric metadata ----------
def _metric_meta(
    t: Callable[[str], str],
    metric_id: str,
) -> Tuple[Optional[str], str, str]:
    if metric_id == METRIC_TREE_COUNT:
        return None, t("metric_tree_count"), t("unit_trees")

    if metric_id == METRIC_VOLUME:
        return "volume", t("metric_volume_m3"), "m³"

    if metric_id == METRIC_BASAL_AREA:
        return "basal_area_m2", t("metric_basal_area_m2"), "m²"

    if metric_id == METRIC_CANOPY_COVER:
        return "canopy_cover_pct", t("metric_canopy_cover_pct"), "%"

    return None, t("metric_tree_count"), t("unit_trees")


# ---------- Binning ----------
def _make_bins(values: pd.Series, step: float) -> np.ndarray:
    v = pd.to_numeric(values, errors="coerce").dropna()
    if v.empty:
        return np.array([0.0, step])
    lo = np.floor(v.min() / step) * step
    hi = np.ceil(v.max() / step) * step
    return np.arange(lo, hi + step, step)


def _agg_stacked(
    d: pd.DataFrame,
    base_col: str,
    bins: np.ndarray,
    hue_col: str,
    value_col: Optional[str],
    categories: List[str],
) -> Tuple[np.ndarray, List[str]]:
    if base_col not in d.columns:
        return np.zeros((len(categories), 0)), []

    cut = pd.cut(pd.to_numeric(d[base_col], errors="coerce"), bins, right=False)
    dd = d.assign(_bin=cut).dropna(subset=["_bin"])

    if dd.empty:
        return np.zeros((len(categories), len(bins) - 1)), []

    if value_col is None:
        pv = dd.pivot_table(index="_bin", columns=hue_col, aggfunc="size", fill_value=0)
    else:
        pv = dd.pivot_table(
            index="_bin",
            columns=hue_col,
            values=value_col,
            aggfunc="sum",
            fill_value=0,
        )

    pv = pv.reindex(columns=categories, fill_value=0)
    labels = [f"{int(b.left)}–{int(b.right)}" for b in pv.index]

    return pv.to_numpy().T, labels


# ---------- MAIN FIGURE ----------
def build_three_panel_figure(
    *,
    plot_info: pd.DataFrame,
    df: pd.DataFrame,
    dist_mode: str,
    metric_id: str,
    color_mode: str,
    t: Callable[[str], str],
) -> Figure:
    d = df.copy()

    # Derived columns
    if "Volume_m3" in d.columns:
        d["volume"] = pd.to_numeric(d["Volume_m3"], errors="coerce")

    if "dbh" in d.columns:
        dbh = pd.to_numeric(d["dbh"], errors="coerce")
        d["basal_area_m2"] = np.pi * (dbh / 200.0) ** 2

    # Canopy cover contribution per tree (relative to plot area)
    try:
        area_ha = float(plot_info["size_ha"].iloc[0])
        area_ha = area_ha if area_ha > 0 else 1.0
    except Exception:
        area_ha = 1.0

    if "surfaceAreaProjection" in d.columns:
        sap = pd.to_numeric(d["surfaceAreaProjection"], errors="coerce").fillna(0.0)
        d["canopy_cover_pct"] = (sap / (area_ha * 10_000.0)) * 100.0

    masks = _make_masks(d)
    dsel = d[masks.get(dist_mode, pd.Series(True, index=d.index))].copy()

    # Hue
    if color_mode == COLOR_BY_MANAGEMENT:
        hue_col = "management_status"
        categories = sorted(d[hue_col].astype(str).dropna().unique())
        cmap = _management_colors(d)
        hue_title = t("management_label")
        legend_labels = [t_mgmt(cat) for cat in categories]
    else:
        hue_col = "species"
        categories = sorted(d[hue_col].astype(str).dropna().unique())
        cmap = _species_colors(d)
        hue_title = t("species")
        legend_labels = categories

    value_col, y_title, unit = _metric_meta(t, metric_id)

    fig = plt.figure(figsize=(13.4, 4.4), dpi=160)
    gs = fig.add_gridspec(1, 3, width_ratios=[1.05, 1.85, 1.85], wspace=0.25)

    ax0 = fig.add_subplot(gs[0, 0])
    ax1 = fig.add_subplot(gs[0, 1])
    ax2 = fig.add_subplot(gs[0, 2])

    # ===== PIE =====
    pie = (
        dsel.groupby(hue_col)[value_col].sum()
        if value_col
        else dsel.groupby(hue_col).size()
    ).reindex(categories, fill_value=0)

    ax0.pie(
        pie.values,
        colors=[cmap.get(k, "#AAAAAA") for k in pie.index],
        startangle=90,
        counterclock=False,
        wedgeprops=dict(width=0.5, edgecolor="white"),
    )

    total = pie.sum()
    txt = f"Σ\n{int(total)}\n{unit}" if value_col is None else f"Σ\n{total:,.1f}\n{unit}".replace(",", " ")
    ax0.text(0, 0, txt, ha="center", va="center", fontsize=12, fontweight="bold")
    ax0.set_title(t("stand_canopy_cover") if metric_id == METRIC_CANOPY_COVER else t("stand_composition"), fontsize=11)

    # ===== STACKED =====
    dbh_bins = _make_bins(dsel.get("dbh", pd.Series()), 10)
    h_bins = _make_bins(dsel.get("height", pd.Series()), 5)

    dbh_mat, dbh_lbl = _agg_stacked(dsel, "dbh", dbh_bins, hue_col, value_col, categories)
    h_mat, h_lbl = _agg_stacked(dsel, "height", h_bins, hue_col, value_col, categories)

    for ax, mat, lbls, unit_lbl in [
        (ax1, dbh_mat, dbh_lbl, t("unit_cm")),
        (ax2, h_mat, h_lbl, t("unit_m")),
    ]:
        x = np.arange(len(lbls))
        bottom = np.zeros(len(lbls))
        for i, cat in enumerate(categories):
            if mat.shape[1] == 0:
                continue
            y = mat[i]
            if np.allclose(y, 0):
                continue
            ax.bar(x, y, bottom=bottom, color=cmap.get(cat, "#AAAAAA"), label=cat)
            bottom += y

        ax.set_xticks(x)
        ax.set_xticklabels([f"{l} {unit_lbl}" for l in lbls], rotation=35, ha="right", fontsize=8)
        ax.set_ylabel(y_title)
        ax.grid(True, axis="y", alpha=0.25)

    handles = [
        plt.Rectangle((0, 0), 1, 1, facecolor=cmap.get(cat, "#AAAAAA"), edgecolor="none")
        for cat in categories
    ]

    fig.legend(
        handles,
        legend_labels,
        loc="lower center",
        ncol=min(6, len(categories)),
        frameon=False,
        bbox_to_anchor=(0.5, -0.01),
        title=hue_title,
        fontsize=8,
        title_fontsize=9,
    )

    fig.suptitle(f"{t(dist_mode)} — {t(metric_id)} — {t(color_mode)}", fontsize=12, fontweight="bold")
    fig.subplots_adjust(bottom=0.33, top=0.88)

    return fig
