# -*- coding: utf-8 -*-
from __future__ import annotations

import io
from dataclasses import dataclass
from datetime import datetime
from typing import Any, Callable, List, Optional, Sequence, Tuple

import numpy as np
import pandas as pd

from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.units import mm
from reportlab.pdfgen import canvas
from reportlab.lib.utils import ImageReader
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont

from matplotlib.figure import Figure as MplFigure

from src.summary_charts import (
    build_three_panel_figure,
    STATUS_BEFORE, STATUS_AFTER, STATUS_REMOVED,
    COLOR_BY_SPECIES, COLOR_BY_MANAGEMENT,
    METRIC_TREE_COUNT, METRIC_VOLUME,
    METRIC_BASAL_AREA, METRIC_CANOPY_COVER,
)

# ------------------------------------------------------------------
# FONT REGISTRATION (UNICODE SAFE)
# ------------------------------------------------------------------
FONT_REG = "DejaVu"
FONT_BOLD = "DejaVu-Bold"

pdfmetrics.registerFont(TTFont(FONT_REG, "assets/fonts/DejaVuSans.ttf"))
pdfmetrics.registerFont(TTFont(FONT_BOLD, "assets/fonts/DejaVuSans-Bold.ttf"))


# ------------------------------------------------------------------
# HELPERS
# ------------------------------------------------------------------
def mpl_fig_to_png_bytes(fig: MplFigure, dpi: int = 170) -> bytes:
    bio = io.BytesIO()
    fig.savefig(bio, format="png", dpi=dpi, bbox_inches="tight")
    bio.seek(0)
    return bio.read()


def _safe_str(x: Any, default: str = "") -> str:
    if x is None:
        return default
    s = str(x)
    return s if s.strip() else default


# ------------------------------------------------------------------
# SUMMARY VARIANTS
# ------------------------------------------------------------------
@dataclass(frozen=True)
class SummaryVariant:
    dist_mode: str
    metric_id: str
    color_mode: str


def default_summary_variants() -> List[SummaryVariant]:
    return [
        SummaryVariant(d, m, c)
        for d in (STATUS_BEFORE, STATUS_AFTER, STATUS_REMOVED)
        for m in (METRIC_TREE_COUNT, METRIC_VOLUME, METRIC_BASAL_AREA, METRIC_CANOPY_COVER)
        for c in (COLOR_BY_SPECIES, COLOR_BY_MANAGEMENT)
    ]


def make_variant_title(t: Callable[[str], str], v: SummaryVariant) -> str:
    return f"{t(v.dist_mode)} — {t(v.metric_id)} — {t(v.color_mode)}"


def generate_all_summary_figs(
    *,
    plot_info: pd.DataFrame,
    trees: pd.DataFrame,
    t: Callable[[str], str],
    variants: Optional[List[SummaryVariant]] = None,
    plot_title_font: Any = None,
) -> List[Tuple[str, MplFigure]]:
    variants = variants or default_summary_variants()
    out: List[Tuple[str, MplFigure]] = []

    for v in variants:
        fig = build_three_panel_figure(
            plot_info=plot_info,
            df=trees,
            dist_mode=v.dist_mode,
            metric_id=v.metric_id,
            color_mode=v.color_mode,
            t=t,
        )
        out.append((make_variant_title(t, v), fig))
    return out


# ------------------------------------------------------------------
# METRICS – JEDNOTNÁ LOGIKA (STEJNÁ JAKO V APP)
# ------------------------------------------------------------------
def _ensure_metrics(df: pd.DataFrame, area_ha: float) -> pd.DataFrame:
    d = df.copy()

    if "Volume_m3" in d.columns:
        d["volume"] = pd.to_numeric(d["Volume_m3"], errors="coerce").fillna(0.0)

    if "dbh" in d.columns:
        dbh_cm = pd.to_numeric(d["dbh"], errors="coerce")
        d["basal_area_m2"] = np.pi * (dbh_cm / 200.0) ** 2

    if "surfaceAreaProjection" in d.columns:
        sap = pd.to_numeric(d["surfaceAreaProjection"], errors="coerce").fillna(0.0)
        d["canopy_cover_pct"] = (sap / (area_ha * 10_000.0)) * 100.0

    return d


def _make_masks(df: pd.DataFrame) -> dict:
    keep = {"Target tree", "Untouched"}
    before = pd.Series(True, index=df.index)

    if "management_status" in df.columns:
        after = df["management_status"].astype(str).isin(keep)
        harvested = ~after
    else:
        after = harvested = pd.Series(False, index=df.index)

    return {
        "before": before,
        "after": after,
        "harvested": harvested,
    }


def compute_report_table(trees: pd.DataFrame, plot_info: pd.DataFrame) -> dict:
    try:
        area_ha = float(plot_info["size_ha"].iloc[0])
        if not np.isfinite(area_ha) or area_ha <= 0:
            area_ha = 1.0
    except Exception:
        area_ha = 1.0

    d = _ensure_metrics(trees, area_ha)
    masks = _make_masks(d)

    out = {"area_ha": area_ha}
    for k, m in masks.items():
        dd = d[m]
        out[(k, "tree_count")] = float(len(dd))
        out[(k, "volume")] = float(dd["volume"].sum())
        out[(k, "basal_area")] = float(dd["basal_area_m2"].sum())
        out[(k, "canopy_cover")] = float(dd["canopy_cover_pct"].sum())

    return out


# ------------------------------------------------------------------
# PDF GENERATOR
# ------------------------------------------------------------------
def build_intervention_report_pdf(
    *,
    plot_info: pd.DataFrame,
    trees: pd.DataFrame,
    figs: Sequence[Tuple[str, MplFigure]],
    intervention_label: str,
    t: Callable[[str], str],
    language: str = "cs",
    created_dt: Optional[datetime] = None,
    png_dpi: int = 170,
) -> bytes:

    created_dt = created_dt or datetime.now()

    def resolve_intervention_label(lbl: str) -> str:
        value = _safe_str(lbl)
        if value.startswith("#"):
            key = value.lstrip("#")
            translated = t(key)
            if translated == key:
                return t("usr_mgmt_unsaved")
            return translated
        return value or t("usr_mgmt_unsaved")

    buf = io.BytesIO()
    c = canvas.Canvas(buf, pagesize=landscape(A4))
    W, H = landscape(A4)

    M = 16 * mm
    y = H - M

    def new_page():
        nonlocal y
        c.showPage()
        y = H - M

    def header():
        nonlocal y
        c.setFont(FONT_BOLD, 18)
        c.drawString(M, y, t("report_title"))
        y -= 8 * mm

        c.setFont(FONT_REG, 10)
        c.drawString(M, y, f"{t('created')}: {created_dt:%Y-%m-%d %H:%M}")
        y -= 5 * mm
        c.drawString(M, y, f"{t('intervention')}: {resolve_intervention_label(intervention_label)}")
        y -= 8 * mm

        c.line(M, y, W - M, y)
        y -= 8 * mm

    # ---------- PAGE 1 ----------
    header()

    stats = compute_report_table(trees, plot_info)

    pi = plot_info.iloc[0] if isinstance(plot_info, pd.DataFrame) and not plot_info.empty else pd.Series(dtype=object)

    c.setFont(FONT_BOLD, 13)
    c.drawString(M, y, t("overview_header"))
    y -= 8 * mm

    c.setFont(FONT_REG, 10)
    overview_lines = [
        f"{t('forest_type')}: {_safe_str(pi.get('forest_type', '—'), '—')}",
        f"{t('number_of_trees_label')}: {_safe_str(pi.get('no_trees', '—'), '—')}",
        f"{t('wood_volume_label')}: {_safe_str(pi.get('volume', '—'), '—')} m³",
        f"{t('area')}: {_safe_str(pi.get('size_ha', '—'), '—')} ha",
        f"{t('altitude')}: {_safe_str(pi.get('altitude', '—'), '—')} m",
        f"{t('precipitation')}: {_safe_str(pi.get('precipitation', '—'), '—')} mm/year",
        f"{t('average_temperature')}: {_safe_str(pi.get('temperature', '—'), '—')} °C",
        f"{t('established')}: {_safe_str(pi.get('established', '—'), '—')}",
        f"{t('location')}: {_safe_str(pi.get('state', '—'), '—')}",
        f"{t('owner')}: {_safe_str(pi.get('owner', '—'), '—')}",
        f"{t('scan_date')}: {_safe_str(pi.get('scan_date', '—'), '—')}",
    ]

    for line in overview_lines:
        c.drawString(M, y, f"• {line}")
        y -= 4.5 * mm

    y -= 2 * mm

    c.setFont(FONT_BOLD, 13)
    c.drawString(M, y, t("report_intervention_summary"))
    y -= 8 * mm

    c.setFont(FONT_REG, 10)
    for mode in ("before", "after", "harvested"):
        c.drawString(
            M, y,
            f"{t(mode)} – {t('tree_count')}: {int(stats[(mode,'tree_count')])}, "
            f"{t('volume')}: {stats[(mode,'volume')]:.1f} m³"
        )
        y -= 5 * mm

    # ---------- CHARTS (ONLY IF EXISTS) ----------
    if figs:
        new_page()
        header()
        c.setFont(FONT_BOLD, 14)
        c.drawString(M, y, t("charts"))
        y -= 8 * mm

        max_w = W - 2 * M
        max_h = 155 * mm

        for title, fig in figs:
            png = mpl_fig_to_png_bytes(fig, dpi=png_dpi)
            img = ImageReader(io.BytesIO(png))
            iw, ih = img.getSize()

            scale = min(max_w / iw, max_h / ih)
            sw, sh = iw * scale, ih * scale

            if y - sh < M:
                new_page()
                header()

            c.setFont(FONT_BOLD, 11)
            c.drawString(M, y, title)
            y -= 6 * mm

            c.drawImage(img, M, y - sh, sw, sh, preserveAspectRatio=True)
            y -= sh + 8 * mm

    c.save()
    return buf.getvalue()
