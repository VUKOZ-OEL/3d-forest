import streamlit as st

#from streamlit_multipage import MultiPage

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