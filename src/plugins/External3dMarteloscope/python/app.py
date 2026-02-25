import streamlit as st
import src.io_utils as iou
from src.i18n import (
    I18N,
    init_i18n,
    set_lang,
    get_lang,
    t,
    validate_i18n,
)
from src.import_utils import (
    load_usr_attributes_wide,
)

# Set Page Title
st.set_page_config(page_title="3D-Marteloscope", page_icon=":material/nature_people:", layout="wide")
# Init dictionary
init_i18n()
validate_i18n()

# --------- Pages ---------
dash_page = st.Page("src/Dashboard.py", title=t("page_info_controls"), icon=":material/dashboard:")
# Basic results
summary_page = st.Page("src/Summary.py", title=t("page_summary"), icon=":material/info:")
intensity_page = st.Page("src/Intensity.py", title=t("page_intensity"), icon=":material/percent:")
map_page = st.Page("src/Map.py", title=t("page_plot_map"), icon=":material/map:")
heatmap_page = st.Page("src/Heatmaps.py", title=t("page_heatmaps"), icon=":material/blur_on:")
tree_page = st.Page("src/Tree_stats.py", title=t("page_tree_statistics"), icon=":material/nature:")
# Expert results
canopy_page = st.Page("src/Canopy_stats.py", title=t("page_canopy_occupancy"), icon=":material/forest:")
space_page = st.Page("src/Space_comp.py", title=t("page_space_competition"), icon=":material/join:")
light_page = st.Page("src/Light_comp.py", title=t("page_sky_view_factor"), icon=":material/light_mode:")
# Simulation
simul_page = st.Page("src/Simulation.py", title=t("page_prediction"), icon=":material/clock_arrow_up:")
# Settings
add_atts_page = st.Page("src/Add_attributes_prj.py", title=t("page_add_attributes"), icon=":material/list_alt_add:")
colors_page = st.Page("src/Colors_settings.py", title=t("colors"), icon=":material/colors:")
# Temp tests
sandbox_page = st.Page("src/sandbox.py", title=t("Sandbox"), icon=":material/thumb_up:")

#file_path = "d:/GS_LCR_DELIVERABLE/Buchlovice/Buchlovice.json"
#file_path = "d:/GS_LCR_DELIVERABLE/Klepacov/Klepacov.json"
#file_path = "d:/GS_LCR_DELIVERABLE/Krivoklat/Krivoklat.json"

if len(sys.argv) > 1:
    file_path = sys.argv[1]
else:
    file_path = "c:/default.json"

# Init data
if not st.session_state.get("data_initialized"):
    st.session_state.project_file = file_path
    st.session_state.sqlite_path = st.session_state.project_file.replace(".json", ".sqlite")
    st.session_state.trees = iou.load_project_json(file_path)
    st.session_state.mgmt_example = iou.load_mgmt_example_sqlite(file_path,"mgmt_example")
    st.session_state.plot_info = iou.load_plot_info(file_path)
    st.session_state.color_palette = iou.load_color_palette(file_path)
    st.session_state.data_initialized = True

    if "mgmt_example" in st.session_state and "trees" in st.session_state:
        st.session_state.mgmt_example = iou._ensure_usr_mgmt_column(
            st.session_state.trees, st.session_state.mgmt_example
        )

    # default volba
    if "active_mgmt_selection" not in st.session_state:
        st.session_state.active_mgmt_selection = "usr_mgmt"

    # vždy zajisti snapshot user atributů
    if "user_attributes" not in st.session_state:
        st.session_state.user_attributes = load_usr_attributes_wide(st.session_state.sqlite_path)

    if "stocking_reference" not in st.session_state:
        st.session_state.stocking_reference = iou.load_stocking_reference(st.session_state.sqlite_path)
    




# Define common labels:
# --------- Common labels (use keys) ---------
st.session_state.Before = t("label_before")
st.session_state.After = t("label_after")
st.session_state.Removed = t("label_removed")

# tyhle dvě položky ve slovníku zatím nemáš jako "Cutting purpose" a "Species"
# -> viz návrhy níže
st.session_state.Management = t("management_label")
st.session_state.Species = t("species")

st.session_state.plot_title_font = dict(size=18, color="#33343f", weight="bold")

# --------- Navigation ---------
pages = {
    t("menu_main"): [
        dash_page,
    ],
    t("menu_basic"): [
        summary_page,
        intensity_page,
        map_page,
        heatmap_page,
        tree_page,
    ],
    t("menu_expert"): [
        canopy_page,
        space_page,
        light_page,
    ],
    t("menu_growth"): [
        simul_page,
    ],
    t("menu_settings"): [
        add_atts_page,
       # colors_page,
       # sandbox_page,
    ],

}

pg = st.navigation(pages)

def _on_lang_change():
    set_lang(st.session_state.lang)  # key="lang" -> en/cs


with st.sidebar:
    st.segmented_control(
        f"**{t('app_localization')}**",
        options=["en", "cs"],
        format_func=lambda x: t("english") if x == "en" else t("cestina"),
        key="lang",
        on_change=_on_lang_change,
    )


# Spuštění aplikace
pg.run()
