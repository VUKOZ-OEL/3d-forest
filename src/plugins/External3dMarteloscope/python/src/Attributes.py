import streamlit as st
import pandas as pd
from st_aggrid import AgGrid, GridOptionsBuilder, DataReturnMode, GridUpdateMode
from src.io import load_project_json
import numpy as np

# stránka na šířku
st.set_page_config(layout="wide")

# --- Načtení dat do session_state ---
if "trees" not in st.session_state:
    file_path = st.session_state["file_path"]
    st.session_state.trees = load_project_json(file_path)

# Přidej management_status, pokud chybí
if "management_status" not in st.session_state.trees.columns:
    st.session_state.trees["management_status"] = "none"

df = st.session_state.trees

# --- Vyber jen "ploché" sloupce (žádné list/dict/tuple/set) ---
def is_nested(val):
    return isinstance(val, (list, dict, tuple, set))

flat_columns = []
for col in df.columns:
    # bezpečně přeskoč NaN/None
    has_nested = df[col].dropna().apply(is_nested).any() if col in df else False
    if not has_nested:
        flat_columns.append(col)

# zajisti přítomnost id a management_status
must_have = [c for c in ["id", "management_status"] if c in df.columns and c not in flat_columns]
display_columns = flat_columns + must_have

# --- Kopie pro zobrazení + zaokrouhlení numerických hodnot na 1 desetinné místo ---
df_display = df[display_columns].copy()

# zaokrouhlení všech numerických sloupců (vč. int -> float s 1 des. místem)
num_cols = df_display.select_dtypes(include=[np.number]).columns
if len(num_cols) > 0:
    df_display[num_cols] = df_display[num_cols].round(1)

# --- Konfigurace AgGrid (needitovatelná, bez výběru/checkboxů) ---
gb = GridOptionsBuilder.from_dataframe(df_display)

# žádné checkboxy, žádný výběr
# (když nena-konfigurujeme selection, ag-Grid výběr neumožní a nevkládá checkboxový sloupec)
gb.configure_grid_options(
    domLayout="normal",
    suppressRowClickSelection=True,    # jistota, že nepůjde vybírat klikem
    suppressHorizontalScroll=False,
)

# širší defaultní sloupce + resizable
# (nastavíme v defaultColDef; minWidth zajistí větší výchozí šířku)
grid_options = gb.build()
grid_options.setdefault("defaultColDef", {})
grid_options["defaultColDef"].update({
    "resizable": True,
    "minWidth": 100,     # zvětšená minimální šířka sloupců
})

# zapnout sidebar s filtry
grid_options["sideBar"] = {
    "toolPanels": [
        {"id": "filters", "labelDefault": "Filters", "labelKey": "filters", "iconKey": "filter", "toolPanel": "agFiltersToolPanel"},
    ],
    "defaultToolPanel": "filters",
}

# nechceme auto-fit sloupců (ať zůstanou širší a jde scrollovat)
# fit_columns_on_grid_load=False -> zachovává šířky a nechává horizontální posuvník
GRID_HEIGHT = 600  # dostatečně vysoké, aby bylo maximum v rámci „wide“ layoutu; posuvník řeší zbytek

# --- Zobrazení AgGrid ---
AgGrid(
    df_display,
    gridOptions=grid_options,
    update_mode=GridUpdateMode.NO_UPDATE,              # nic needitujeme, tak není třeba update eventů
    data_return_mode=DataReturnMode.FILTERED_AND_SORTED,
    theme="streamlit",
    enable_enterprise_modules=False,
    fit_columns_on_grid_load=False,                    # důležité: nezmenšuj sloupce na šířku containeru
    height=GRID_HEIGHT,                                # velké, ale s vertikálním scrollbarem, takže nepřetéká stránku
    key="trees_grid",
)
