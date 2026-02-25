from datetime import datetime
import streamlit as st
import pandas as pd

import src.io_utils as iou
from src.i18n import t, t_help
from src.db_utils import (
    get_sqlite_path_from_session,
    ensure_mgmt_tables,
    list_managements,
    load_management_map,
    save_management_from_trees,
    delete_management,
)


from src.report_utils import generate_all_summary_figs, build_intervention_report_pdf, SummaryVariant

# ---------- DATA ----------
plot_info = st.session_state.plot_info
df: pd.DataFrame = st.session_state.trees.copy()

# ---------- DB PATH ----------
sqlite_path = get_sqlite_path_from_session(st.session_state)
ensure_mgmt_tables(sqlite_path)

# ---------- helper: user cache ----------
def _ensure_usr_cache():
    """
    Uloží aktuální user management_status do usr_mgmt_cache, pokud ještě není.
    Cache je "poslední uživatelský zásah" nezávislý na přepínání scénářů.
    """
    if "usr_mgmt_cache" not in st.session_state or st.session_state["usr_mgmt_cache"] is None:
        tmp = st.session_state.trees[["id", "management_status"]].copy()
        tmp["id"] = pd.to_numeric(tmp["id"], errors="coerce").astype("Int64")
        tmp = tmp.dropna(subset=["id"]).copy()
        tmp["id"] = tmp["id"].astype(int)
        st.session_state["usr_mgmt_cache"] = dict(zip(tmp["id"].tolist(), tmp["management_status"].tolist()))

def _restore_usr_cache():
    _ensure_usr_cache()
    tr = st.session_state.trees.copy()
    tr["id"] = pd.to_numeric(tr["id"], errors="coerce").astype("Int64")
    tr = tr.dropna(subset=["id"]).copy()
    tr["id"] = tr["id"].astype(int)

    cache = st.session_state.get("usr_mgmt_cache", {}) or {}
    # apply only where cache has value (and not None)
    tr["management_status"] = tr["id"].map(cache).fillna(tr["management_status"])
    # refresh colors
    palette = st.session_state.get("color_palette", {})
    tr = iou.refresh_management_colors(tr, palette)
    st.session_state.trees = tr

def _apply_saved_mgmt(mgmt_id: int):
    # before switching away, preserve current user intervention (once)
    _ensure_usr_cache()

    mp = load_management_map(sqlite_path, mgmt_id=int(mgmt_id))
    tr = st.session_state.trees.copy()

    tr["id"] = pd.to_numeric(tr["id"], errors="coerce").astype("Int64")
    tr = tr.dropna(subset=["id"]).copy()
    tr["id"] = tr["id"].astype(int)

    # apply scenario values only where present
    s = tr["id"].map(mp)
    mask = s.notna()
    tr.loc[mask, "management_status"] = s.loc[mask].astype(object)

    palette = st.session_state.get("color_palette", {})
    tr = iou.refresh_management_colors(tr, palette)
    st.session_state.trees = tr


# ---------- OVLÁDÁNÍ ----------
c1, c2 = st.columns([2, 2])

with c1:
    st.markdown(f"### :orange[**{plot_info['name'].iloc[0]}**]")
    st.markdown("###")

    st.markdown(f"##### {t('overview_header')}")
    st.markdown("")
    st.markdown(
        f"""
- **{t('forest_type')}** {plot_info['forest_type'].iloc[0]}
- **{t('number_of_trees_label')}** {plot_info['no_trees'].iloc[0]}
- **{t('wood_volume_label')}** {plot_info['volume'].iloc[0]} m³
- **{t('area')}** {plot_info['size_ha'].iloc[0]} ha
- **{t('altitude')}** {plot_info['altitude'].iloc[0]} m
- **{t('precipitation')}** {plot_info['precipitation'].iloc[0]} mm/year
- **{t('average_temperature')}** {plot_info['temperature'].iloc[0]} °C
- **{t('established')}** {plot_info['established'].iloc[0]}
- **{t('location')}** {plot_info['state'].iloc[0]}
- **{t('owner')}** {plot_info['owner'].iloc[0]}
- **{t('scan_date')}** {plot_info['scan_date'].iloc[0]}
"""
    )

with c2:
    st.markdown(f"##### {t('choose_existing_selection')}")
    st.markdown("")

    msg = st.session_state.pop("flash_success", None)
    if msg:
        st.success(msg)

    # --- dropdown items ---
    # special "user" option
    USER_KEY = "__USER__"

    mgmts_df = list_managements(sqlite_path)
    # make labels like: "Label (id: 12)"
    saved_items = []
    for _, r in mgmts_df.iterrows():
        mid = int(r["mgmt_id"])
        lbl = str(r["label"])
        saved_items.append((f"{lbl}  (id: {mid})", mid))

    # options mapping: display_label -> value (USER_KEY or mgmt_id)
    options = {t("usr_mgmt_unsaved"): USER_KEY}
    for disp, mid in saved_items:
        options[disp] = mid

    labels = list(options.keys())

    # current selection
    active = st.session_state.get("active_mgmt_selection", USER_KEY)
    # invert to label
    inv = {v: k for k, v in options.items()}
    current_label = inv.get(active, t("usr_mgmt_unsaved"))
    default_index = labels.index(current_label) if current_label in labels else 0

    selected_label = st.selectbox(
        f"**{t('management_examples')}**",
        labels,
        index=default_index,
        key="mgmt_selectbox",
    )
    selected_value = options[selected_label]  # USER_KEY or mgmt_id

    # ---- Load (switch displayed management) ----
    if st.button(
        f"**{t('load_example')}**",
        icon=":material/model_training:",
        use_container_width=True,
        key="btn_load_mgmt",
    ):
        if selected_value == USER_KEY:
            _restore_usr_cache()
            st.session_state.active_mgmt_selection = USER_KEY
            st.session_state.flash_success = t("success_load_usr_mgmt")
        else:
            _apply_saved_mgmt(int(selected_value))
            st.session_state.active_mgmt_selection = int(selected_value)
            st.session_state.flash_success = t("success_load_mgmt")
        st.session_state.flash_success_ts = pd.Timestamp.utcnow().isoformat()
        st.rerun()

    st.divider()

    # ---- Save current user intervention to SQLite ----
    st.markdown(f"##### {t('save_current_mgmt_header')}")
    label = st.text_input(
        f"**{t('mgmt_label_input')}**",
        value="",
        placeholder=t("mgmt_label_placeholder"),
        key="mgmt_label_input",
    )

    if st.button(
        f"**{t('save_current_mgmt_btn')}**",
        icon=":material/save:",
        use_container_width=True,
        key="btn_save_current_mgmt",
    ):
        if not str(label).strip():
            st.warning(t("mgmt_label_required"))
        else:
            # IMPORTANT: we save what's currently in trees[management_status] (user intervention)
            new_id = save_management_from_trees(
                sqlite_path=sqlite_path,
                trees=st.session_state.trees,
                label=str(label).strip(),
                id_col="id",
                value_col="management_status",
            )
            st.session_state.active_mgmt_selection = int(new_id)
            st.session_state.flash_success = t("success_save_mgmt")
            st.session_state.flash_success_ts = pd.Timestamp.utcnow().isoformat()
            st.rerun()

    # ---- Delete selected saved management from SQLite ----
    can_delete = selected_value != USER_KEY
    if st.button(
        f"**{t('delete_selected_mgmt_btn')}**",
        icon=":material/delete:",
        use_container_width=True,
        disabled=not can_delete,
        key="btn_delete_selected_mgmt",
        type="primary",
    ):
        if can_delete:
            delete_management(sqlite_path, int(selected_value))
            # after delete: go back to user intervention view
            st.session_state.active_mgmt_selection = USER_KEY
            _restore_usr_cache()
            st.session_state.flash_success = t("success_delete_mgmt")
            st.session_state.flash_success_ts = pd.Timestamp.utcnow().isoformat()
            st.rerun()

    st.divider()

    st.markdown(f"##### {t('project_controls')}")
   # --- Label zásahu (přizpůsob si podle svého mgmt logiky) ---
    active = st.session_state.get("active_mgmt_selection", "__USER__")
    if active == "__USER__":
        intervention_label = t("usr_mgmt_unsaved")  # např. "Uživatelský zásah (neuložený)"
    else:
        #intervention_label = f"{t('management_examples')} #{active}"
        active_label = next((lbl for lbl, mid in saved_items if mid == active), None)
        intervention_label = active_label if active_label else t("usr_mgmt_unsaved")

# --- 1) Render tlačítko ---
    if st.button(
        t("export_results"),
        icon=":material/file_save:",
        use_container_width=True,
    ):
        with st.spinner(t("export_rendering_pdf")):

            figs = generate_all_summary_figs(
                plot_info=st.session_state.plot_info,
                trees=st.session_state.trees,
                t=t,
            )

            pdf_bytes = build_intervention_report_pdf(
                plot_info=st.session_state.plot_info,
                trees=st.session_state.trees,
                figs=figs,
                intervention_label=intervention_label,
                t=t,
                language=st.session_state.get("lang", "cs"),
                created_dt=datetime.now(),
            )

            st.session_state["export_pdf_bytes"] = pdf_bytes
            st.session_state["export_pdf_name"] = f"report_{datetime.now().strftime('%Y%m%d_%H%M')}.pdf"
            st.session_state["export_pdf_ready_ts"] = datetime.now().isoformat()

    # --- 2) Download se ukáže až po renderu ---
    if st.session_state.get("export_pdf_bytes"):
        st.download_button(
            label=t("export_download_ready"),
            data=st.session_state["export_pdf_bytes"],
            file_name=st.session_state.get("export_pdf_name", "report.pdf"),
            mime="application/pdf",
            icon=":material/download:",
            use_container_width=True,
        )


with st.expander(label=t("expander_help_label"),icon=":material/help:"):
    st.markdown(t_help("dashboard_help"))