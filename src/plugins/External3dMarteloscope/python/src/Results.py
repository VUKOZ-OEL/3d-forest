import streamlit as st
from src.io import *
import pandas as pd

st.title("Analytics")
st.button("button")

if "trees" not in st.session_state:
    file_path = st.session_state.project_file
    st.session_state.trees = load_project_json(file_path)

st.session_state.trees.plot.scatter("dbh","height")