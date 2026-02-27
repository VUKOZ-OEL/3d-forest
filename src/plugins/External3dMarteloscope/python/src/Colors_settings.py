import streamlit as st
import pandas as pd
import src.io_utils as iou


df = st.session_state.trees  # zkratka

c1, c2, c3 = st.columns([1, 1, 1])

with c1:
    st.markdown("#### Species colors")

    # unikátní druhy + první barva
    sp_unique = df.drop_duplicates("species")[["species", "speciesColorHex"]]
    sp_colors = dict(zip(sp_unique["species"], sp_unique["speciesColorHex"]))

    # ovládací prvky (unikátní klíč!)
    for sp, col in sp_colors.items():
        sp_colors[sp] = st.color_picker(sp, col, key=f"cp_species_{sp}")

    # přepis do DF (bez chained assignment) a vrať zpět do session_state
    df.loc[:, "speciesColorHex"] = df["species"].map(sp_colors)
    st.session_state.trees = df

with c2:
    st.markdown("#### Management colors")

    mg_unique = df.drop_duplicates("management_status")[["management_status", "managementColorHex"]]
    mg_colors = dict(zip(mg_unique["management_status"], mg_unique["managementColorHex"]))

    for mg, col in mg_colors.items():
        mg_colors[mg] = st.color_picker(str(mg), col, key=f"cp_mgmt_{mg}")

    df.loc[:, "managementColorHex"] = df["management_status"].map(mg_colors)
    st.session_state.trees = df

with c3:
    if st.button("**Save colors to project**", width="stretch",type="primary", icon=":material/save:"):
        iou.save_project_json(st.session_state.project_file, st.session_state.trees, "data/test_save_project.json")
        iou.show_success("Colors saved into project.")

    st.markdown("###")
    st.button("Set species colors to default", use_container_width=True, icon=":material/temp_preferences_eco:")
    st.button("Set management colors to default", use_container_width=True, icon=":material/approval_delegation:")

    
    

print(st.session_state.trees["speciesColorHex"])