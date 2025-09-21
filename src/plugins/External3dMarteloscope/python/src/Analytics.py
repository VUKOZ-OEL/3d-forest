import streamlit as st
import pandas as pd
import src.io as io
from pygwalker.api.streamlit import StreamlitRenderer
 
# Adjust the width of the Streamlit page
st.set_page_config(
    page_title="Use Pygwalker In Streamlit",
)
# Import your data

if "trees" not in st.session_state:
    file_path = st.session_state["file_path"]
    st.session_state.trees = io.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()

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
df_display = df[display_columns].copy()
 
pyg_app = StreamlitRenderer(df_display,appearance="light")
 
pyg_app.explorer()