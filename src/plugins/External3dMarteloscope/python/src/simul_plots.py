from typing import Dict, Optional
import pandas as pd
import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots

def fig_sim_vol_height_ba(
    df: pd.DataFrame,
    code2color: Dict[str, str],
    code2label: Dict[str, str],
    smooth_window: int = 3,
    height_px: int = 420,
) -> go.Figure:
    """
    3 panely vedle sebe, sdílená legenda dole.
    Pro každou dřevinu kreslí 3 vrstvy linií:
        - tlustá (šířka 6, opacity 0.18) – nejprůhlednější
        - střední (šířka 4, opacity 0.35)
        - tenká (šířka 2, opacity 1.0) – nahoře, v legendě a s hovrem
    """

    need_cols = {"year", "species", "volume_m3", "height", "basalArea"}
    missing = need_cols - set(df.columns)
    if missing:
        raise ValueError(f"Chybí sloupce: {sorted(missing)}")

    d = df.copy()
    d["species"] = d["species"].astype(str).str.lower()

    # agregace
    g_vol = d.groupby(["year", "species"], as_index=False)["volume_m3"].sum()
    g_hgt = d.groupby(["year", "species"], as_index=False)["height"].mean()
    g_bas = d.groupby(["year", "species"], as_index=False)["basalArea"].sum()

    # wide + sjednocený index let (pro konzistentní X napříč metrikami)
    w_vol = g_vol.pivot(index="year", columns="species", values="volume_m3")
    w_hgt = g_hgt.pivot(index="year", columns="species", values="height")
    w_bas = g_bas.pivot(index="year", columns="species", values="basalArea")

    all_years = sorted(set(w_vol.index) | set(w_hgt.index) | set(w_bas.index))
    w_vol = w_vol.reindex(all_years).fillna(0.0)
    w_hgt = w_hgt.reindex(all_years)  # mean -> necháme NaN, smoothing je vyhladí
    w_bas = w_bas.reindex(all_years).fillna(0.0)

    # smoothing (rolling okno, centrované)
    def smooth(dfw: pd.DataFrame) -> pd.DataFrame:
        return dfw.rolling(window=smooth_window, min_periods=1, center=True).mean()

    w_vol_s = smooth(w_vol)
    w_hgt_s = smooth(w_hgt)
    w_bas_s = smooth(w_bas)

    species_all = sorted(set(w_vol_s.columns) | set(w_hgt_s.columns) | set(w_bas_s.columns))
    years = [int(y) for y in all_years]

    fig = make_subplots(
        rows=1, cols=3, shared_yaxes=False,
        horizontal_spacing=0.06,
        subplot_titles=("Σ Volume (m³)", "Mean Height (m)", "Σ Basal Area (m²)")
    )

    def color_for(code: str) -> str:
        return code2color.get(code, "#BBBBBB")

    def label_for(code: str) -> str:
        return code2label.get(code, code.title())

    # vrstvy (šířka ↗ => průhlednost ↗)
    LAYERS = [
        {"width": 20, "opacity": 0.2},  # thickest, most transparent
        {"width": 5, "opacity": 1.00},  # thinnest, opaque, hover+legend
    ]

    def add_series(col_df: pd.DataFrame, col_idx: int, unit: str):
        for sp in species_all:
            y_vals = col_df.get(sp)
            if y_vals is None:
                continue
            y = y_vals.values.tolist()

            # přidej 3 vrstvy v pořadí: tlustá -> střední -> tenká (tenká navrch)
            for li, layer in enumerate(LAYERS):
                showlegend = (layer["width"] == 5 and col_idx == 3)  # legenda jen u posledního panelu a tenké vrstvy
                hovertemplate = (
                    f"Year=%{{x}}<br>{label_for(sp)}: %{{y:.3f}} {unit}<extra></extra>"
                    if layer["width"] == 5 else None
                )
                hoverinfo = "skip" if layer["width"] != 2 else "text"

                fig.add_trace(
                    go.Scatter(
                        x=years, y=y,
                        mode="lines",
                        name=label_for(sp),
                        legendgroup=sp,
                        line=dict(width=layer["width"]),
                        marker=dict(color=color_for(sp)),
                        opacity=layer["opacity"],
                        showlegend=showlegend,
                        hovertemplate=hovertemplate,
                        hoverinfo=hoverinfo,
                    ),
                    row=1, col=col_idx
                )

    # Panel 1: volume
    add_series(w_vol_s, 1, "m³")
    # Panel 2: height
    add_series(w_hgt_s, 2, "m")
    # Panel 3: basal area
    add_series(w_bas_s, 3, "m²")

    # osy a legenda
    fig.update_xaxes(title_text="Year", row=1, col=1)
    fig.update_xaxes(title_text="Year", row=1, col=2)
    fig.update_xaxes(title_text="Year", row=1, col=3)
    fig.update_yaxes(title_text="Volume (m³)", row=1, col=1)
    fig.update_yaxes(title_text="Height (m)", row=1, col=2)
    fig.update_yaxes(title_text="Basal Area (m²)", row=1, col=3)

    fig.update_layout(
        height=height_px,
        legend=dict(
            orientation="h",
            yanchor="top", y=-0.18,   # pod grafikou
            x=0.5, xanchor="center",  # 3) vystředění
        ),
        margin=dict(l=10, r=10, t=60, b=90),
    )
    return fig