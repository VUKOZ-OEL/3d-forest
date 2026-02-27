# -*- coding: utf-8 -*-
import numpy as np
import pandas as pd
from shapely import wkt
from src.i18n import t, t_help, t_mgmt


# =========================================================
# PARSE WKT → coordinates + area
# =========================================================
def parse_polygon(wkt_str):
    """Parse WKT polygon into (x,y,area). Supports Polygon & MultiPolygon."""
    try:
        geom = wkt.loads(wkt_str)
    except Exception:
        return None, None, None

    # Merge MultiPolygon → Polygon (union)
    if geom.geom_type == "MultiPolygon":
        geom = geom.buffer(0)

    if geom.geom_type != "Polygon":
        return None, None, None

    area = float(geom.area)
    x, y = geom.exterior.xy
    return list(x), list(y), area


# =========================================================
# HEX → RGBA
# =========================================================
def hex_to_rgba(hex_color, alpha=0.3):
    if not isinstance(hex_color, str):
        return f"rgba(120,120,120,{alpha})"
    hex_color = hex_color.strip()
    if hex_color.startswith("#"):
        hex_color = hex_color[1:]
    if len(hex_color) != 6:
        return f"rgba(120,120,120,{alpha})"

    r = int(hex_color[0:2], 16)
    g = int(hex_color[2:4], 16)
    b = int(hex_color[4:6], 16)
    return f"rgba({r},{g},{b},{alpha})"


# =========================================================
# MASKS (before / after / removed)
# =========================================================
def make_masks(df):
    """Return dict of boolean masks for before / after / removed."""
    keep = {"Target tree", "Untouched"}
    ms = df["management_status"].astype(str)
    mask_after = ms.isin(keep)
    mask_removed = ~mask_after
    mask_before = pd.Series(True, index=df.index)
    return {
        "before": mask_before,
        "after": mask_after,
        "removed": mask_removed,
    }


# =========================================================
# HOVER DATA
# =========================================================
def make_hover_data(df: pd.DataFrame):
    """
    i18n-safe hover.
    Zachovává PŘESNĚ stejné customdata indexy jako původní verze.
    """

    # helpers
    def r1(col):
        if col not in df.columns:
            return np.full(len(df), np.nan)
        return pd.to_numeric(df[col], errors="coerce").round(1)

    # DBH jako celé číslo
    dbh_rounded = pd.to_numeric(df["dbh"], errors="coerce").round(0).astype("Int64")

    customdata = np.column_stack(
        [
            df["species"].astype(str),                    # 0
            df.get("label", "").astype(str),              # 1
            dbh_rounded.to_numpy(),                       # 2
            df["management_status"].map(t_mgmt),          # 3  ✅ i18n
            r1("height"),                                 # 4
            r1("Volume_m3"),                              # 5
            r1("crown_base_height"),                      # 6
            r1("crown_centroid_height"),                  # 7
            r1("crown_volume"),                           # 8
            r1("crown_surface"),                          # 9
            r1("horizontal_crown_proj"),                  # 10
            r1("vertical_crown_proj"),                    # 11
            r1("heightXdbh"),                             # 12
            r1("projection_exposure"),                    # 13
        ]
    )

    hovertemplate = (
        f"{t('species')}: %{{customdata[0]}}<br>"
        f"{t('label')}: %{{customdata[1]}}<br>"
        f"{t('dbh')}: %{{customdata[2]}} {t('unit_cm')}<br>"
        f"{t('management_label')}: %{{customdata[3]}}<br>"
        f"{t('tree_height')}: %{{customdata[4]}} {t('unit_m')}<br>"
        f"{t('value_volume')}: %{{customdata[5]}} {t('unit_m3')}<br>"
        f"{t('crown_base_height')}: %{{customdata[6]}} {t('unit_m')}<br>"
        f"{t('crown_centroid_height')}: %{{customdata[7]}} {t('unit_m')}<br>"
        f"{t('crown_volume')}: %{{customdata[8]}} {t('unit_m3')}<br>"
        f"{t('crown_surface')}: %{{customdata[9]}} {t('unit_m2')}<br>"
        f"{t('horizontal_crown_proj')}: %{{customdata[10]}} {t('unit_m2')}<br>"
        f"{t('vertical_crown_proj')}: %{{customdata[11]}} {t('unit_m2')}<br>"
        f"{t('height_dbh_ratio')}: %{{customdata[12]}}<br>"
        f"{t('projection_exposure')}: %{{customdata[13]}} {t('unit_percent')}"
        "<extra></extra>"
    )

    return customdata, hovertemplate



# =========================================================
# POINT SIZE SCALING BY DECILES
# =========================================================
def compute_point_sizes(df: pd.DataFrame, scale_var: str,
                        size_min: float, size_max: float,
                        default_var: str = "dbh") -> np.ndarray:
    """
    Compute point sizes based on deciles of scale_var.
    - scale_var: column name to scale by
    - size_min / size_max: min/max marker size
    - default_var: fallback column if scale_var not in df
    """
    if scale_var not in df.columns:
        scale_var = default_var

    values = pd.to_numeric(df[scale_var], errors="coerce")

    # pokud je vše NaN → vrátíme konstantní velikosti
    if values.notna().sum() == 0:
        return np.full(len(df), (size_min + size_max) / 2.0)

    v = values.fillna(values.min())
    # decily 10–90 (9 hranic → 10 bucketů)
    bins = np.percentile(v, np.linspace(10, 90, 9))

    # bucket 0–9
    decile_idx = np.digitize(v, bins, right=True)
    decile_idx = np.clip(decile_idx, 0, 9)

    sizes = size_min + (size_max - size_min) * (decile_idx / 9.0)
    return sizes
