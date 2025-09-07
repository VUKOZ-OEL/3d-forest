import streamlit as st
from src.io import *
import pandas as pd

st.title("Analytics")
st.button("button")

if "trees" not in st.session_state:
    #file_path = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/LS-Krivoklat/3df_project/Krivoklat_test_SAVE.json"
    file_path = ("c:/data/2.json")
    st.session_state.trees = load_project_json(file_path)

st.session_state.trees.plot.scatter("dbh","height")