import streamlit as st
import pandas as pd

from src.i18n import t, t_help

from src.import_utils import (
    get_tree_ids,
    get_existing_usr_attributes,
    validate_csv,
    detect_overwrite_attributes,
    insert_usr_attributes,
    delete_usr_attributes,
    load_usr_attributes_wide,
)

from src.data_prep.simulate_light import compute_light_competition

# =================================================
# INIT
# =================================================
sqlite_path = st.session_state.project_file.replace(".json", ".sqlite")

# vždy zajisti snapshot user atributů
if "user_attributes" not in st.session_state:
    st.session_state.user_attributes = load_usr_attributes_wide(sqlite_path)

tree_ids = get_tree_ids(sqlite_path)

c1, _ ,c2, _, c3 = st.columns([5, 1, 5,1,5])

with c1:
    st.markdown(f"### {t('import_label')}")

    # =================================================
    # IMPORT ATTRIBUTES
    # =================================================
    uploaded_file = st.file_uploader(
        label=t("uploader_label"),
        type="csv",
    )

    validation_ok = False
    overwrite_confirmed = False
    csv_df = None
    attributes = []

    if uploaded_file is not None:
        csv_df = pd.read_csv(uploaded_file)

        validation = validate_csv(csv_df, tree_ids)

        if not validation["ok"]:
            st.error(validation["error"])
        else:
            validation_ok = True
            attributes = validation["attributes"]

            st.success(t("csv_ok"))
            label = t("detected_attributes_label")
            st.caption(f"{label} {', '.join(attributes)}")

            existing_attrs = get_existing_usr_attributes(sqlite_path)
            overwrite_attrs = detect_overwrite_attributes(attributes, existing_attrs)

            if overwrite_attrs:
                st.warning(
                    t("existing_att_warn")
                    + ", ".join(overwrite_attrs)
                )
                overwrite_confirmed = st.checkbox(
                    t("confirm_overwrite")
                )
            else:
                overwrite_confirmed = True

    can_import = validation_ok and overwrite_confirmed

    if st.button(
        t("import_btn"),
        type="primary",
        disabled=not can_import,
    ):
        insert_usr_attributes(sqlite_path, csv_df)

        # vždy refresh snapshotu
        st.session_state.user_attributes = load_usr_attributes_wide(sqlite_path)

        st.success("Attributes successfully imported")

# =================================================
# REMOVE ATTRIBUTES
# =================================================
with c2:
    st.markdown(f"### {t("remove_label")}")

    existing_attrs = get_existing_usr_attributes(sqlite_path)

    if not existing_attrs:
        st.info(t("no_usr_att"))
    else:
        to_remove = st.multiselect(
            t("import_sucess"),
            options=existing_attrs,
        )

        if st.button(
            t("remove_btn"),
            type="secondary",
            disabled=not to_remove,
        ):
            delete_usr_attributes(sqlite_path, to_remove)

            # refresh snapshotu
            st.session_state.user_attributes = load_usr_attributes_wide(sqlite_path)

            st.success(t("remove_sucess"))

with c3:

    st.markdown(f"#### **{t('rerun_light_label')}**")
    st.markdown(f"**{t('set_voxel_size')}**")
    json_path = st.session_state.project_file

    c31,_, c32 = st.columns([3,1,4])

    
    with c31:
        voxel = st.number_input(
            f"**{t('set_voxel_size')}**",
            min_value=0.05,
            max_value=1.01,
            value=0.25,
            step=0.05,
            format="%.2f",
            label_visibility="collapsed",
        )

    with c32:
        if st.button(f"### **{t('run_light_btn')}**", type = "primary"):
            progress = st.progress(0)
            status = st.empty()

            def update_progress(p, msg):
                progress.progress(p)
                status.text(msg)

            df_result = compute_light_competition(
                json_path,
                voxel,
                progress_callback=update_progress
            )

with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("add_att_help"))