import streamlit as st
import sys
import ctypes, os
from pathlib import Path

#from streamlit_multipage import MultiPage

if len(sys.argv) > 1:
    file_path = sys.argv[1]
else:
    file_path = "c:/default.json"

st.session_state["file_path"] = file_path

# ------------------------------------------------------------------------------
# Get directory of the running script
base_dir = Path(__file__).resolve().parent

# Build a relative path to the DLL
lib_path = base_dir / "../../../ILandModel.dll"
os.add_dll_directory(str(lib_path.parent))

# Normalize (resolve) the path
lib_path = lib_path.resolve()

# Load the shared library
iland = ctypes.CDLL(str(lib_path))

# Define argument and return types
iland.runilandmodel.argtypes = [ctypes.c_char_p, ctypes.c_int]
iland.runilandmodel.restype = ctypes.c_int

# Call the function
xml_path = b"C:\\Users\\vbubn\\Documents\\3d-forest\\bin\\iland2\\Pokojna_hora.xml"
years = 1
result = iland.runilandmodel(xml_path, years)
# print("Result:", result)

# ------------------------------------------------------------------------------
st.set_page_config(page_title="3D-Marteloscope", page_icon=":material/nature_people:",layout="wide")

dash_page = st.Page("src/Dashboard.py", title="Plot Info", icon=":material/dashboard:")

tree_page = st.Page("src/Tree_stats.py", title="Tree Statistics", icon=":material/monitoring:")
canopy_page = st.Page("src/Canopy_stats.py", title="Canopy Space", icon=":material/forest:")

map_page = st.Page("src/Map.py", title="Plot Map", icon=":material/map:")
att_page = st.Page("src/Attributes.py", title="Attribute Table", icon=":material/data_table:")
analytics_page = st.Page("src/Analytics.py", title="Analytics", icon=":material/addchart:")

results_page = st.Page("src/Results.py", title="Future Outlook", icon=":material/clock_arrow_up:")
test_page = st.Page("src/sandbox.py", title="_SANDBOX_")

#pg = st.navigation([dash_page, map_page,att_page,analytics_page,])

pages = {
    "Summary": [
        dash_page,
    ],
    "Results:": [
        tree_page,
        canopy_page,
        map_page,
        att_page,
        analytics_page,
    ],
    "Simulation": [
        results_page,
        test_page,
    ],
}

pg = st.navigation(pages)

# Spuštění aplikace
pg.run()