import streamlit as st
import pandas as pd
import numpy as np
import re
import src.io_utils as iou

# stránka na šířku
st.set_page_config(layout="wide")

# --- Načtení dat do session_state ---
if "trees" not in st.session_state:
    file_path = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v11.json"
    st.session_state.trees = iou.load_project_json(file_path)

df = st.session_state.trees

# Přidej management_status, pokud chybí
if "management_status" not in df.columns:
    df["management_status"] = "none"

# --- Vyber jen "ploché" sloupce (žádné list/dict/tuple/set) ---
def is_nested(val):
    return isinstance(val, (list, dict, tuple, set))

flat_columns = []
for col in df.columns:
    has_nested = df[col].dropna().apply(is_nested).any() if col in df else False
    if not has_nested:
        flat_columns.append(col)

# zajisti přítomnost id a management_status (pokud tam jsou, ale vypadly z flat_columns)
must_have = [c for c in ["id", "management_status"] if c in df.columns and c not in flat_columns]
display_columns = flat_columns + must_have

# --- Kopie pro zobrazení + zaokrouhlení numerických hodnot na 1 desetinné místo ---
df_display = df[display_columns].copy()

num_cols = df_display.select_dtypes(include=[np.number]).columns
if len(num_cols) > 0:
    df_display[num_cols] = df_display[num_cols].round(1)

# --- Příprava zvýraznění řádků podle managementColorHex ---
# robustní parsování barvy (podporuje "#RRGGBB", "RRGGBB", "#RGB", "rgb(...)", "rgba(...)")
def parse_color_to_rgb(value: str):
    if not isinstance(value, str):
        return None
    s = value.strip()
    # rgb/rgba
    m = re.match(r"rgba?\s*\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)", s, flags=re.I)
    if m:
        r, g, b = (int(m.group(1)), int(m.group(2)), int(m.group(3)))
        r = max(0, min(255, r)); g = max(0, min(255, g)); b = max(0, min(255, b))
        return (r, g, b)
    # #RRGGBB nebo RRGGBB
    m = re.match(r"^#?([0-9a-fA-F]{6})$", s)
    if m:
        hex6 = m.group(1)
        r = int(hex6[0:2], 16)
        g = int(hex6[2:4], 16)
        b = int(hex6[4:6], 16)
        return (r, g, b)
    # #RGB
    m = re.match(r"^#?([0-9a-fA-F]{3})$", s)
    if m:
        h = m.group(1)
        r = int(h[0]*2, 16)
        g = int(h[1]*2, 16)
        b = int(h[2]*2, 16)
        return (r, g, b)
    return None

# map index -> rgba styl
alpha = 0.15  # "lehké" podbarvení
row_bg_cache = {}
if "managementColorHex" in df.columns:
    for idx, val in df["managementColorHex"].items():
        rgb = parse_color_to_rgb(val)
        if rgb:
            r, g, b = rgb
            row_bg_cache[idx] = f"background-color: rgba({r},{g},{b},{alpha})"
        else:
            row_bg_cache[idx] = ""  # bez zvýraznění
else:
    # sloupec není – bez zvýraznění
    row_bg_cache = {idx: "" for idx in df_display.index}

def style_row(row: pd.Series):
    # vrátí list stylů pro všechny sloupce v aktuálním řádku
    bg = row_bg_cache.get(row.name, "")
    return [bg] * len(row)

# vytvoř Styler; index držíme z původního DF, aby šel dohledat row_bg_cache
styler = df_display.style.apply(style_row, axis=1)

# --- Zobrazení nativní tabulky ---
st.dataframe(
    styler,
    use_container_width=True,
    height=600
)
