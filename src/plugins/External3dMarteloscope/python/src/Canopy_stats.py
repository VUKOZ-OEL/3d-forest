import streamlit as st
import pandas as pd
import json
import src.io as io
import numpy as np
import plotly.graph_objects as go

# --- Načtení a inicializace ---
if "trees" not in st.session_state:
    file_path = st.session_state["file_path"]
    st.session_state.trees = io.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()