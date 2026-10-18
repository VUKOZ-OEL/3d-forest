# i18n.py
from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Iterable, Literal
import re

import streamlit as st

from src.help_dict import HELP_I18N

Lang = Literal["cs", "en"]
DEFAULT_LANG: Lang = "en"

MGMT_STATUS_KEY_MAP: dict[str, str] = {
    "Competition": "mgmt_status_competition",
    "Maturity": "mgmt_status_maturity",
    "Promote rare species": "mgmt_status_promote_rare",
    "Promote regeneration": "mgmt_status_promote_regeneration",
    "Sanitary": "mgmt_status_sanitary",
    "Target tree": "mgmt_status_target_tree",
    "Technical": "mgmt_status_technical",
    "Untouched": "mgmt_status_untouched",
}


I18N: dict[str, dict[Lang, str]] = {
    "mgmt_status_competition": {"cs": "Konkurence", "en": "Competition"},
    "mgmt_status_maturity": {"cs": "Zralost", "en": "Maturity"},
    "mgmt_status_promote_rare": {"cs": "Uvolnění vzácných druhů", "en": "Promote rare species"},
    "mgmt_status_promote_regeneration": {"cs": "Uvolnění obnovy", "en": "Promote regeneration"},
    "mgmt_status_sanitary": {"cs": "Zdravotní výběr", "en": "Sanitary"},
    "mgmt_status_target_tree": {"cs": "Cílový strom", "en": "Target tree"},
    "mgmt_status_technical": {"cs": "Technický zásah", "en": "Technical"},
    "mgmt_status_untouched": {"cs": "Bez zásahu", "en": "Untouched"},

    "altitude": {"cs": "Nadmořská výška:", "en": "Altitude:"},
    "app_localization": {"cs": "Jazyk", "en": "Language:"},
    "area": {"cs": "Rozloha:", "en": "Area:"},
    "available_light_label": {"cs": "{value} %", "en": "{value} %"},
  #  "available_light_label": {"cs": "Dostupné: {value} %", "en": "Available: {value} %"},
    "average_temperature": {"cs": "Průměrná teplota:", "en": "Average temperature:"},
    "avg_avail_light_label": {"cs": "Průměrně dostupné světlo", "en": "Average Available Light"},
    "axis_height_above": {"cs": "Výška nad povrchem [m]", "en": "Height above ground [m]"},
    "axis_x": {"cs": "Osa x [m]", "en": "x [m]"},
    "axis_y": {"cs": "Osa y [m]", "en": "y [m]"},
    "bar_ylabel_shade": {"cs": "Příspěvek ke stínění [%]", "en": "Shade contribution [%]"},
    "bars_by_management_title": {"cs": "Kdo soutěží podle výběru", "en": "Who Competes by Selection"},
    "bars_by_species_title": {"cs": "Kdo soutěží podle dřeviny", "en": "Who Competes by Species"},
    "basal_area": {"cs": "Výčetní kruhová základna", "en": "Basal Area"},
    "basal_area_m2": {"cs": "Výčetní kruhová základna [m²]", "en": "Basal Area [m²]"},
    "btn_import_attributes": {"cs": "Importovat atributy", "en": "Import attributes"},
    "btn_save_colors": {"cs": "Uložit barvy do projektu", "en": "Save colors to project"},
    "btn_save_project": {"cs": "Uložit projekt", "en": "Save project"},
    "btn_save_project_as": {"cs": "Uložit projekt jako", "en": "Save project as"},
    "btn_set_selection_color_default": {
        "cs": "Nastavit výchozí barvy výběru",
        "en": "Set management colors to default",
    },
    "btn_set_species_color_default": {
        "cs": "Nastavit výchozí barvy druhů",
        "en": "Set species colors to default",
    },
    "btn_start_simulation": {"cs": "Spustit simulaci", "en": "Start simulation"},
    "cestina": {"cs": "Čeština", "en": "Czech"},
    "clear_selection": {"cs": "Vymazat výběr", "en": "Clear Selection"},
    "color_by": {"cs": "Barvy podle", "en": "Color by"},
    "color_by_help": {
        "cs": "Vyberte kategorii (druh / management), podle které se budou barvy přiřazovat.",
        "en": "Select category (Species / Management) to color plots by.",
    },
    "colorbar_title": {"cs": "Legenda barev", "en": "Color scale"},
    "colors": {"cs": "Barvy", "en": "Colors"},
    "count_label": {"cs": "Počet:", "en": "Count:"},
    "crown_base_height": {"cs": "Výška nasazení koruny [m]", "en": "Crown Base Height [m]"},
    "crown_centroid_height": {"cs": "Výška těžiště koruny", "en": "Crown Centroid Height"},
    "crown_eccentricity": {"cs": "Excentricita koruny", "en": "Crown Eccentricity"},
    "crown_projections": {"cs": "Projekce korun", "en": "Crown Projections"},
    "crown_surface": {"cs": "Povrch koruny [m²]", "en": "Crown Surface Area [m²]"},
    "crown_volume_m3": {"cs": "Objem koruny [m³]", "en": "Crown Volume [m³]"},
    "crown_volume": {"cs": "Objem koruny [m³]", "en": "Crown Volume"},
    "dbh": {"cs": "Výčetní tloušťka", "en": "DBH"},
    "dbh_class": {"cs": "Tloušťková třída", "en": "DBH class"},
    "dbh_class_range": {"cs": "Šířka tloušťkových tříd [cm]", "en": "DBH class range [cm]"},
    "dbh_filter": {"cs": "Filtr tlouštěk [cm]", "en": "DBH filter [cm]"},
    "dbh_filter_help": {"cs": "Filtrovat stromy podle výčetní tloušťky.", "en": "Filter trees by DBH."},
    "english": {"cs": "Angličtina", "en": "English"},
    "error_missing_ids": {
        "cs": "{count} ID chybí v CSV (budou zobrazeny jako <NA>):",
        "en": "{count} IDs missing in CSV (will show <NA>):",
    },
    "established": {"cs": "Založeno:", "en": "Established:"},
    "explore_canopy_stats": {
        "cs": "Využití nadzemního prostoru: Profil objemu koruny podle výšky nad terénem",
        "en": "Explore canopy statistics: Crown volume profiles by height above the ground",
    },
    "explore_tree_stats": {"cs": "Statistika jednotlivých stromů:", "en": "Explore tree statistics:"},
    "export_results": {"cs": "Exportovat výsledky", "en": "Export results"},
    "file_uploader_details": {
        "cs": "Soubor musí obsahovat hlavičku. Pokud sloupec ID není přítomen, první sloupec je považován za ID stromů. Používejte tečku jako desetinnou čárku.",
        "en": "The file must contain a header. If the ID column is not present, the first column is considered as Tree IDs. Use . as decimal separator.",
    },
    "file_uploader_help": {"cs": "Formát souboru:", "en": "File format:"},
    "filter_management_help": {"cs": "Vyber jednu nebo více kategorií výběru.", "en": "Select one or more selection categories."},
    "filter_selection": {"cs": "Filtr výběru", "en": "Filter Selection"},
    "filter_selection_heatmap": {"cs": "Filtr výběru (pouze heatmapa):", "en": "Filter Selection (heatmap only):"},
    "filter_species": {"cs": "Filtr druhů", "en": "Filter Species"},
    "filter_species_heatmap": {"cs": "Filtr druhů (pouze heatmapa):", "en": "Filter Species (heatmap only):"},
    "filter_species_help": {"cs": "Vyber jeden nebo více dřevin.", "en": "Pick one or more species."},
    "filter_values_help": {
        "cs": "Omezit rozsah hodnot proměnné. Ovlivní, která data se zobrazí a jak bude škálovaná osa Y.",
        "en": "Filter range of metric values. This affects which data are shown and how Y-axis is scaled.",
    },
    "filter_values_label": {"cs": "Filtr hodnot:", "en": "Filter values:"},
    "forest_type": {"cs": "Typ lesa:", "en": "Forest type:"},
    "grid_label": {"cs": "Síť 10 m", "en": "Grid 10 m"},
    "group_label": {"cs": "Skupina", "en": "Group"},
    "heatmap_value_label": {"cs": "Hodnota:", "en": "Value:"},
    "height_class": {"cs": "Výšková třída", "en": "Height class"},
    "height_class_range": {"cs": "Šířka výškových tříd [m]", "en": "Height class range [m]"},
    "height_dbh_ratio": {"cs": "Štíhlostní koeficient", "en": "Height-DBH Ratio"},
    "height_filter": {"cs": "Filtr výšky [m]", "en": "Filter Height [m]"},
    "height_filter_heatmap": {"cs": "Filtr výšky [m] (pouze heatmapa)", "en": "Height filter [m] | Heatmap only"},
    "height_filter_help": {"cs": "Zobrazí pouze stromy v zadaném intervalu výšek.", "en": "Filter trees by height interval."},
    "horizontal_crown_proj": {"cs": "Horizontální projekce koruny [m²]", "en": "Horizontal Crown Projection [m²]"},
    "hover_management": {"cs": "Management: %{x}<br>Hodnota: %{y:.2f}", "en": "Management: %{x}<br>Value: %{y:.2f}"},
    "hover_management_pct": {"cs": "Management: %{x}<br>Hodnota: %{y:.2f} %", "en": "Management: %{x}<br>Value: %{y:.2f} %"},
    "hover_management_shade": {"cs": "Výběr: %{x}<br>Stínění: %{y:.2f} %", "en": "Selection: %{x}<br>Shade: %{y:.2f} %"},
    "hover_primary_line": {
        "cs": "{group}: {value}\nVýška: {height} m\nObjem: {volume} m³",
        "en": "{group}: {value}\nHeight: {height} m\nVolume: {volume} m³",
    },
    "hover_species": {"cs": "Dřevina: %{x}<br>Hodnota: %{y:.2f}", "en": "Species: %{x}<br>Value: %{y:.2f}"},
    "hover_species_pct": {"cs": "Dřevina: %{x}<br>Hodnota: %{y:.2f} %", "en": "Species: %{x}<br>Value: %{y:.2f} %"},
    "hover_species_shade": {"cs": "Dřevina: %{x}<br>Stínění: %{y:.2f} %", "en": "Species: %{x}<br>Shade: %{y:.2f} %"},
    "hover_sum_line": {"cs": "Suma\nVýška: {height} m\nObjem: {volume} m³", "en": "Sum\nHeight: {height} m\nVolume: {volume} m³"},
    "choose_existing_selection": {"cs": "Výběr existujícího zásahu", "en": "Choose existing selection:"},
    "import_ext_selection": {"cs": "Importovat zásah z externího souboru:", "en": "Import Selection from external file:"},
    "import_help_text": {
        "cs": "Načíst soubor exportovaný z mobilní aplikace (ForDil), pokud jste vytvořili management výběr v terénu.",
        "en": "Load file exported from mobile app (ForDil) if you created management selection in field.",
    },
    "in_dbh_class": {"cs": "V tloušťkových třídách", "en": "In DBH class"},
    "in_height_class": {"cs": "Ve výškových třídách", "en": "In Height class"},
    "info_no_categories_color_mode_simple": {
        "cs": "Nebyly nalezeny žádné kategorie pro zvolený způsob barvení.",
        "en": "No categories found for selected color mode.",
    },
    "info_no_columns_to_import": {"cs": "Žádné nové ani překrývající se sloupce k importu.", "en": "No new or overlapping columns to import."},
    "info_no_skyview_data": {"cs": "Žádná dostupná data pro Sky View.", "en": "No data for Sky View values."},
    "info_no_valid_values": {"cs": "Žádné platné hodnoty pro „{column}“.", "en": "No valid values for '{column}'."},
    "intensity_based_on": {"cs": "Intenzita podle:", "en": "Intensity based on:"},
    "intensity_header": {"cs": "Intenzita pěstebních zásahů", "en": "Management Intensity"},
    "intensity_in_group_title": {"cs": "Intenzita výběru ve skupině", "en": "Intensity of Selection in Group"},
    "invalid_height_values": {"cs": "Neplatné hodnoty ve sloupci „height“.", "en": "Invalid values in 'height'."},
    "invert_crown_colors": {"cs": "Prohodit barvy koruny", "en": "Invert Crown Colors"},
    "project_controls": {"cs": "Ovládání projektu:", "en": "Project controls:"},
    "label_after": {"cs": "Po zásahu", "en": "After Cut"},
    "label_before": {"cs": "Před zásahem", "en": "Before Cut"},
    "label_options": {"cs": "Možnosti", "en": "Options"},
    "label_removed": {"cs": "Vytěženo", "en": "Harvested"},
    "label_selection": {"cs": "Výběr", "en": "Selection"},
    "label_set_length": {"cs": "Nastav délku simulace", "en": "Set length of simulation"},
    "legend_crown": {"cs": "Koruna – {value}", "en": "Crown – {value}"},
    "legend_title": {"cs": "Legenda", "en": "Legend"},
    "light_comp_pg_title": {"cs": "Konkurence o světlo", "en": "Competition for Light"},
    "load_example": {"cs": "Načíst příklad", "en": "Load example"},
    "location": {"cs": "Poloha:", "en": "Location:"},
    "m2_per_ha": {"cs": "m²/ha", "en": "m²/ha"},
    "m3_per_ha": {"cs": "m³/ha", "en": "m³/ha"},
    "management_examples": {"cs": "Příklady zásahů:", "en": "Selection examples:"},
    "management_label": {"cs": "Volba zásahu", "en": "Treatment selection"},
    "menu_basic": {"cs": "Základní výsledky", "en": "Basic Results"},
    "menu_expert": {"cs": "Rozšířené výsledky", "en": "Expert Results"},
    "menu_growth": {"cs": "Simulace růstu", "en": "Growth Simulation"},
    "menu_main": {"cs": "Hlavní", "en": "Main"},
    "menu_settings": {"cs": "Nastavení", "en": "Settings"},
    "mgmt_status_target_tree": {"cs": "Cílový strom", "en": "Target tree"},
    "mgmt_status_untouched": {"cs": "Bez zásahu", "en": "Untouched"},
    "missing_column": {"cs": "Chybí sloupec", "en": "Missing column"},
    "missing_columns_crown": {
        "cs": "Chybí požadované sloupce pro výpočet profilu objemu koruny: {columns}",
        "en": "Missing columns for crown volume profile: {columns}",
    },
    "msg_colors_saved": {"cs": "Barvy byly uloženy do projektu.", "en": "Colors saved into project."},
    "no_valid_xy": {"cs": "Nebyly nalezeny platné souřadnice po filtrování.", "en": "No valid coordinates after filtering."},
    "overlay_color_by": {"cs": "Barvit překrytí podle", "en": "Overlay color by"},
    "overlay_show_positions": {"cs": "Zobrazit pozice stromů (body)", "en": "Show tree positions (small dots)"},
    "overview_header": {"cs": "Přehled:", "en": "Overview:"},
    "owner": {"cs": "Vlastník:", "en": "Owner:"},
    "page_add_attributes": {"cs": "Přidat atributy", "en": "Add attributes"},
    "page_add_attributes_title": {"cs": "Přidat atributy do aktuálního projektu", "en": "Add attributes to current project"},
    "page_canopy_occupancy": {"cs": "Vyplnění prostoru", "en": "Canopy Occupancy"},
    "page_colors_selection_title": {"cs": "Barvy výběru", "en": "Selection colors"},
    "page_colors_species_title": {"cs": "Barvy dřevin", "en": "Species colors"},
    "page_detailed_view": {"cs": "Detailní pohled", "en": "Detailed view"},
    "page_heatmaps": {"cs": "Heatmapy", "en": "Heatmaps"},
    "page_info_controls": {"cs": "Info & ovládání", "en": "Info & controls"},
    "page_intensity": {"cs": "Intenzita", "en": "Intensity"},
    "page_plot_map": {"cs": "Mapa plochy", "en": "Plot Map"},
    "page_prediction": {"cs": "Predikce", "en": "Prediction"},
    "page_simulation_title": {"cs": "Simulace růstu lesa", "en": "Forest Growth Simulation"},
    "page_sky_view_factor": {"cs": "Dostupnost světla", "en": "Sky View Factor"},
    "page_space_competition": {"cs": "Prostorová konkurence", "en": "Space Competition"},
    "page_summary": {"cs": "Souhrn", "en": "Summary"},
    "page_title": {"cs": "Heatmapy vybraného atributu", "en": "Heatmaps for selected Attribute"},
    "page_tree_statistics": {"cs": "Statistiky stromů", "en": "Tree Statistics"},
    "percent_label": {"cs": "Procenta", "en": "Percent"},
    "percentage_mode": {"cs": "Procenta (%)", "en": "Percentage (%)"},
    "plot_by": {"cs": "Vykreslit podle:", "en": "Plot by:"},
    "plot_by_help_category_only": {
        "cs": "Pro zvolenou proměnnou je k dispozici pouze zobrazení podle kategorie.",
        "en": "For selected variable only display by Category is allowed.",
    },
    "plot_by_help_tree_count": {
        "cs": "Zobrazit počet stromů podle:\n- **DBH** třídy\n- **Výškové** třídy\n- **Kategorie** (druh / management).",
        "en": "Show tree count according to:\n- **DBH** class\n- **Height** class\n- **Category** (Species/Management).",
    },
    "plot_summary": {"cs": "Souhrn: Hodnoty na hektar", "en": "Plot summary: Per Hectare Values"},
    "precipitation": {"cs": "Srážky:", "en": "Precipitation:"},
    "preview_full": {"cs": "Zobrazit celý náhled", "en": "Show full preview"},
    "preview_title": {"cs": "Náhled atributů k importu (první 3 řádky)", "en": "Preview of attributes to be imported (first 3 rows)"},
    "projection_exposure": {"cs": "Zápoj stromu", "en": "Projection Exposure"},
    "removal_percent_axis_title": {"cs": "Odstranění [%]", "en": "Removal [%]"},
    "removed_label": {"cs": "Odstraněno", "en": "Removed"},
    "scale_horizontal_proj": {"cs": "Horizontální projekce", "en": "Horizontal Projection"},
    "scale_max_point_size": {"cs": "Maximální velikost bodu", "en": "Scale Max Point Size"},
    "scale_min_point_size": {"cs": "Minimální velikost bodu", "en": "Scale Min Point Size"},
    "scale_point_size_by": {"cs": "Měřítko velikosti bodů podle:", "en": "Scale point size by:"},
    "scale_vertical_proj": {"cs": "Vertikální projekce", "en": "Vertical Projection"},
    "select_stand_state": {"cs": "Zvolte stav porostu:", "en": "Select Stand State:"},
    "shared_label": {"cs": "Sdíleno: {value} m³", "en": "Shared: {value} m³"},
    "shared_label_pct": {"cs": "Sdíleno: {value} %", "en": "Shared: {value} %"},
    "shared_vs_total_title": {"cs": "Sdílený vs. celkový objem koruny", "en": "Shared vs Total Crown Volume"},
    "show_data_for": {"cs": "Zobrazit data pro:", "en": "Show Data for:"},
    "show_label": {"cs": "Popisky", "en": "Label"},
    "show_mode": {"cs": "Zobrazit:", "en": "Show:"},
    "show_values_as": {"cs": "Zobrazit hodnoty jako:", "en": "Show Values as:"},
    "show_values_by": {"cs": "Zobrazit hodnoty podle:", "en": "Show Values by:"},
    "show_values_by_help": {
        "cs": "Vyberte jednu nebo obě možnosti. Pokud jsou vybrány obě, obě budou vykresleny (Druh = plná čára, Management = čárkovaná).",
        "en": "Select one or both. When both are selected, both are shown (Species = solid, Management = dashed).",
    },
    "sky_view_management_title": {"cs": "Hodnoty dostupného světla podle zásahu", "en": "Sky View Values by Management"},
    "sky_view_species_title": {"cs": "Hodnoty dostupného světla podle druhu", "en": "Sky View Values by Species"},
    "sky_view_values": {"cs": "Hodnoty dostupného světla", "en": "Sky View Values"},
    "space_comp_pg_title": {"cs": "Konkurence o prostor – Sdílený objem korun", "en": "Competition for Space – Shared Volume of Crowns"},
    "species": {"cs": "Dřevina", "en": "Species"},
    "spinner_loading_sim": {
        "cs": "Načítání a zpracování výsledků simulace růstu lesa, prosím čekejte.",
        "en": "Loading and processing outcomes of forest growth simulation, please wait.",
    },
    "stacked_bars_help": {"cs": "Pouze pro počet stromů: přepíná mezi skládaným a seskupeným režimem.", "en": "Only for Tree Count, switch between stacked and grouped mode."},
    "stacked_bars_label": {"cs": "Skládané sloupce", "en": "Stacked bars"},
    "stand_composition": {"cs": "Složení porostu", "en": "Stand Composition"},
    "stocking": {"cs": "Zastoupení", "en": "Stocking"},
    "styler_duplicate_column": {"cs": "Duplicitní sloupec – bude přepsán", "en": "Duplicate column – will be overwritten"},
    "styler_missing_id": {"cs": "Hodnota chybí – ID není v importovaném CSV", "en": "Missing value – ID not present in imported CSV"},
    "success_import": {"cs": "Importováno/aktualizováno {count} sloupců: {cols}", "en": "Imported/updated {count} column(s): {cols}"},
    "sum_label": {"cs": "Suma", "en": "Sum"},
    "sum_symbol": {"cs": "Σ", "en": "Σ"},
    "sum_values_by": {"cs": "Součty podle:", "en": "Sum values by:"},
    "sum_volume_title": {"cs": "{title} · Σ {value} m³/ha", "en": "{title} · Σ {value} m³/ha"},
    "summary_total_selection_intensity": {"cs": "Celková intenzita výběru: {value} %", "en": "Total Selection Intensity: {value} %"},
    "toggle_mortality": {"cs": "Mortalita", "en": "Mortality"},
    "toggle_regeneration": {"cs": "Obnova", "en": "Regeneration"},
    "total_crown_label": {"cs": "Celkový objem koruny: {value} m³", "en": "Total crown: {value} m³"},
    "total_crown_label_pct": {"cs": "Celkový objem koruny: 100 %", "en": "Total crown: 100 %"},
    "total_label": {"cs": "Celkem", "en": "Total"},
    "total_light_label": {"cs": "Celkem: 100 %", "en": "Total: 100 %"},
    "tree_count": {"cs": "Počet stromů", "en": "Tree Count"},
    "tree_height": {"cs": "Výška stromu [m]", "en": "Tree Height [m]"},
    "trees": {"cs": "Stromy", "en": "Trees"},
    "unit_trees": {"cs": "Stromů", "en": "Trees"},
    "trees_per_ha": {"cs": "Stromů/Ha", "en": "Trees/Ha"},
    "unit_cm": {"cs": "cm", "en": "cm"},
    "unit_m": {"cs": "m", "en": "m"},
    "unit_m2": {"cs": "m²", "en": "m²"},
    "unit_m3": {"cs": "m³", "en": "m³"},
    "unit_percent": {"cs": "%", "en": "%"},
    "value_horizontal_crown_projection": {"cs": "Horizontální projekce koruny", "en": "Horizontal Crown Projection Area"},
    "value_to_display": {"cs": "Zobrazená hodnota", "en": "Value to display"},
    "value_vertical_crown_projection": {"cs": "Vertikální projekce koruny", "en": "Vertical Crown Projection Area"},
    "value_volume": {"cs": "Objem", "en": "Volume"},
    "values_to_plot": {"cs": "Hodnoty k vykreslení:", "en": "Values to plot:"},
    "values_to_plot_help": {
        "cs": "Zvolte proměnnou k zobrazení:\n- **Počet stromů** se vykreslí jako sloupcový graf.\n- Ostatní proměnné se zobrazí jako violin graf.\n- **Projection Exposure** používá speciální rozvržení se třemi violin grafy.",
        "en": "Choose variable to display:\n- **Tree count** is plotted as barplot.\n- Other variables are shown as violin plot.\n- **Projection Exposure** uses a dedicated triple violin layout.",
    },
    "vertical_crown_proj": {"cs": "Vertikální projekce koruny [m²]", "en": "Vertical Crown Projection [m²]"},
    "violin_ylabel_light": {"cs": "Dostupné světlo [%]", "en": "Available light [%]"},
    "volume": {"cs": "Objem [m³]", "en": "Volume [m³]"},
    "warn_cannot_aggregate_non_numeric": {"cs": "Nelze {stat} nenumerický sloupec „{column}“.", "en": "Cannot {stat} non-numeric variable '{column}'."},
    "warn_invalid_crown_voxel_size": {"cs": "Neplatná hodnota crownVoxelSize.", "en": "Invalid crownVoxelSize value."},
    "warn_invalid_light_data": {"cs": "Neplatná nebo chybějící data o světle.", "en": "Invalid or missing light data."},
    "warn_missing_column": {"cs": "Chybí sloupec „{column}“.", "en": "Missing column '{column}'."},
    "warn_no_focal_data": {"cs": "Žádná data po aplikaci filtrů.", "en": "No data after applying filters."},
    "warn_overwrite_columns": {"cs": "Existující sloupce budou přepsány:", "en": "Existing columns will be overwritten:"},
    "who_competes": {"cs": "Kdo soutěží", "en": "Who competes"},
    "y_title_shared_space_m3": {"cs": "Sdílený prostor [m³]", "en": "Shared space [m³]"},
    "y_title_shared_space_pct": {"cs": "Podíl sdíleného prostoru [%]", "en": "Share of shared space [%]"},
    # --------------------------------------------- added later
    # DASHBOARD
    "number_of_trees_label": {"cs": "Počet stromů:", "en": "Number of trees:"},
    "wood_volume_label": {"cs": "Zásoba dříví:", "en": "Wood volume:"},
    "btn_clear_management": {"cs": "Vymazat zásah", "en": "Clear management"},
    "success_load_mgmt": {"cs": "Zásah načten", "en": "Selection loaded"},
    # management examples keys - aliases of column name
    "usr_mgmt": {"cs": "Výběr definovaný uživatelem", "en": "Users defined selection"},
    "ph_mgmt_ex_1": {"cs": "Pokojná hora - modelový zásah", "en": "Pokojná hora - example"},
    "scan_date": {"cs": "Lidarová data sebrána v:", "en": "Lidar data collection:"},

    # Summary
    "metric_tree_count": {"cs": "Počet stromů", "en": "Tree count"},
    "metric_volume_m3": {"cs": "Objem (m³)", "en": "Volume (m³)"},
    "metric_basal_area_m2": {"cs": "Výčetní kruhová základna (m²)", "en": "Basal area (m²)"},
    "metric_canopy_cover_pct": {"cs": "Zápoj porostu (%)", "en": "Canopy cover (%)"},
    "metric_canopy_cover": {"cs": "Zápoj porostu", "en": "Canopy cover"},
    "no_data": {"cs": "Žádná data", "en": "No data"},
    "uncovered": {"cs": "Nezakryto", "en": "Uncovered"},
    "column_sum": {"cs": "Suma v kategorii", "en": "Category sum"},
    "in_class": {"cs": "V kategorii", "en": "In class"},

    "warn_no_data_for_filters": {
        "cs": "Pro zvolenou kombinaci filtrů nejsou k dispozici žádná data. Upravte prosím filtry a zkuste to znovu.",
        "en": "No data available for the selected filters. Please adjust the filters and try again.",
    },

    "dbh_filter_heatmap": {
        "cs": "Filtr tlouštěk [cm] (pouze heatmapa)",
        "en": "DBH filter [cm] | Heatmap only",
    },

    "show_tree_positions": {
        "cs": "Zobrazit pozice stromů (body)",
        "en": "Show tree positions (points)",
    },

    "no_data": {
        "cs": "Žádná data",
        "en": "No data",
    },

    ## Tree statistics
    "explore_tree_statistics": {
        "cs": "Statistiky stromů",
        "en": "Explore tree statistics",
    },

    "values_to_plot": {
        "cs": "Zobrazované hodnoty",
        "en": "Values to plot",
    },
    "values_to_plot_help": {
        "cs": "Vyber proměnnou k zobrazení:\n- Počet stromů se zobrazuje jako sloupcový graf.\n- Ostatní proměnné jako violin plot.\n- Projection Exposure používá speciální rozložení.",
        "en": "Choose variable to display:\n- Tree count is shown as a bar plot.\n- Other variables are shown as violin plot.\n- Projection Exposure uses a dedicated layout.",
     },

    "plot_by_help_count": {
        "cs": "Zobrazí počet stromů podle:\n- tříd DBH\n- tříd výšky\n- kategorií (dřevina / zásah).",
        "en": "Show tree count by:\n- DBH classes\n- Height classes\n- Category (Species/Management).",
    },
    "plot_by_help_category_only": {
        "cs": "Pro tuto proměnnou je povoleno pouze zobrazení podle kategorií.",
        "en": "For this variable only Category display is allowed.",
    },
    "color_by_help": {
        "cs": "Zvol, podle čeho se mají grafy barevně rozlišovat (dřevina / zásah).",
        "en": "Select how to color plots (Species / Management).",
    },

    "stacked_bars": {
        "cs": "Skládané sloupce",
        "en": "Stacked bars",
    },
    "stacked_bars_help": {
        "cs": "Pouze pro Počet stromů: přepíná mezi skládaným a seskupeným zobrazením.",
        "en": "Only for Tree Count: switch between stacked and grouped mode.",
    },

    "dbh_class_range": {
        "cs": "Šířka tříd DBH [cm]",
        "en": "DBH class range [cm]",
    },
    "dbh_class_range_help": {
        "cs": "Šířka intervalů DBH pro třídění do sloupců.",
        "en": "Width of DBH bands for class-based bars.",
    },
    "height_class_range": {
        "cs": "Šířka tříd výšky [m]",
        "en": "Height class range [m]",
    },
    "height_class_range_help": {
        "cs": "Šířka intervalů výšky pro třídění do sloupců.",
        "en": "Width of height bands for class-based bars.",
    },

    "dbh_filter_help": {
        "cs": "Filtrovat stromy podle DBH.",
        "en": "Filter trees by DBH.",
    },
    "height_filter_help": {
        "cs": "Filtrovat stromy podle výšky.",
        "en": "Filter trees by height.",
    },

    "filter_values": {
        "cs": "Filtr hodnot",
        "en": "Filter values",
    },
    "filter_values_help": {
        "cs": "Nastaví rozsah hodnot proměnné. Ovlivní, která data se zobrazí a také měřítko osy Y.",
        "en": "Set the metric value range. Affects which data are shown and Y-axis scaling.",
    },

    "warn_no_categories": {
        "cs": "Pro zvolený režim barvení nebyly nalezeny žádné kategorie.",
        "en": "No categories found for the selected color mode.",
    },

    "warn_non_numeric_stat": {
        "cs": "Nelze vypočítat {stat} pro nečíselnou proměnnou „{var}“.",
        "en": "Cannot compute {stat} for non-numeric variable '{var}'.",
    },

    "warn_no_valid_values": {
        "cs": "Pro „{column}“ nejsou k dispozici platné hodnoty.",
        "en": "No valid values for '{column}'.",
    },

    "stat_of_variable": {
        "cs": "{stat} z {var}",
        "en": "{stat} of {var}",
    },

    "category": {
        "cs": "Kategorie",
        "en": "Category",
    },

    "warn_projection_exposure_special": {
        "cs": "Projection Exposure používá speciální vykreslení (triple violin).",
        "en": "Projection Exposure uses a dedicated rendering (triple violin).",
    },

    "canopy_profiles_layers_help": {
        "cs": "Vyber jednu nebo obě vrstvy. Pokud zvolíš obě, druhá se vykreslí jako překryv (čárkovaná).",
        "en": "Select one or both layers. If both are selected, the second one is drawn as an overlay (dashed).",
    },

    "filter_management": {
        "cs": "Filtr volby zásahu",
        "en": "Filter Treatment Selection"
    },

    "help_select_stand_state_light": {
        "cs": "Vyber, pro jaký stav porostu se mají data zobrazit.",
        "en": "Choose which stand state to display."
    },
    "help_chart_mode_light": {
        "cs": "Přepíná mezi sloupci konkurence (kdo stíní) a houslovým grafem hodnt Sky View.",
        "en": "Switch between competition bars (who shades) and Sky View violin statistics."
    },

    # DASHBOARD:
    
    "usr_mgmt_unsaved": {
        "cs": "Uživatelský zásah (lokální)",
        "en": "User intervention (local)"
    },

    "success_load_usr_mgmt": {
        "cs": "Načten uživatelský zásah.",
        "en": "User intervention loaded."
    },

    "save_current_mgmt_header": {
        "cs": "Uložit aktuální zásah",
        "en": "Save current intervention"
    },

    "mgmt_label_input": {
        "cs": "Název zásahu",
        "en": "Intervention name"
    },

    "mgmt_label_placeholder": {
        "cs": "Zadej název zásahu…",
        "en": "Enter intervention name…"
    },

    "mgmt_label_required": {
        "cs": "Zadej název zásahu.",
        "en": "Please enter an intervention name."
    },

    "save_current_mgmt_btn": {
        "cs": "Uložit zásah do projektu",
        "en": "Save intervention to project"
    },

    "success_save_mgmt": {
        "cs": "Zásah byl uložen.",
        "en": "Intervention has been saved."
    },

    "delete_selected_mgmt_btn": {
        "cs": "Smazat vybraný zásah",
        "en": "Delete selected intervention"
    },

    "success_delete_mgmt": {
        "cs": "Zásah byl smazán.",
        "en": "Intervention has been deleted."
    },

    "success_clear_usr_mgmt": {
        "cs": "Uživatelský zásah byl vymazán.",
        "en": "User intervention has been cleared."
    },
    "export_rendering_pdf": {
        "cs": "Generuji PDF report…",
        "en": "Generating PDF report…"
    },
    "export_download_ready": {
        "cs": "Stáhnout vygenerovaný PDF report",
        "en": "Download generated PDF report"
    },
    "usr_mgmt_unsaved": {
        "cs": "Uživatelský zásah (neuložený)",
        "en": "User intervention (unsaved)"
    },

    "report_title": { "cs": "Report zásahu", "en": "Intervention report" },
    "plot": { "cs": "Plocha", "en": "Plot" },
    "created": { "cs": "Vytvořeno", "en": "Created" },
    "intervention": { "cs": "Zásah", "en": "Intervention" },
    "site_overview": { "cs": "Přehled lokality", "en": "Site overview" },
    "report_intervention_summary": { "cs": "Souhrn zásahu", "en": "Intervention summary" },
    "metric": { "cs": "Metrika", "en": "Metric" },
    "before": { "cs": "Před", "en": "Before" },
    "after": { "cs": "Po", "en": "After" },
    "harvested": { "cs": "Vytěženo", "en": "Harvested" },
    "per_ha": { "cs": "Na ha (před)", "en": "Per ha (before)" },
    "charts": { "cs": "Grafy", "en": "Charts" },
    "chart": { "cs": "Graf", "en": "Chart" },
    "report_footer_note": { "cs": "report_footer_note", "en": "report_footer_note" },

    "expander_help_label": { "cs": "Nápověda:", "en": "Help:" },

    "dashboard_help": { "cs": "Nápověda:", "en": "Help:" },
    "summary_help": { "cs": "Nápověda:", "en": "Help:" },
    "intensity_help": { "cs": "Nápověda:", "en": "Help:" },
    "map_help": { "cs": "Nápověda:", "en": "Help:" },
    "heatmap_help": { "cs": "Nápověda:", "en": "Help:" },
    "tree_stats_help": { "cs": "Nápověda:", "en": "Help:" },

    "canopy_help": { "cs": "Nápověda:", "en": "Help:" },
    "space_comp_help": { "cs": "Nápověda:", "en": "Help:" },
    "light_comp_help": { "cs": "Nápověda:", "en": "Help:" },

    "prediction_help": { "cs": "Nápověda:", "en": "Help:" },

    "add_att_help": { "cs": "Nápověda:", "en": "Help:" },

    #SIMULATION
    "simulation_header": { "cs": "Simulace růstu lesního porostu:", "en": "Forest Growth Simulation:" },
    "button_run_simulation": { "cs": "Spustit simulaci:", "en": "Start simulation" },
    "simulation_options": { "cs": "Možnosti simulace:", "en": "Simulation options:" },
    "simulation_period": { "cs": "Simulavané období:", "en": "Simulation period:" },
    "mortality_box": { "cs": "Mortalita", "en": "Mortality" },
    "regeneration_box": { "cs": "Obnova", "en": "Regeneration" },
    "replications": { "cs": "Počet replikací modelu:", "en": "Model replications:" },
    "simul_progress": { "cs": "Probíhá simulace růstu porostu pomocí modelu iLand…", "en": "Running iLand model simulations…" },


    "chart_volume_by_species": { "cs": "Objem porostu podle dřevin", "en": "Stand volume by species" },
    "chart_volume_by_management": { "cs": "Objem porostu podle volby zásahu", "en": "Stand volume by treatment selection" },

    "chart_x_year": { "cs": "Rok", "en": "Year" },

    "simulation_progress_running": { "cs": "Probíhá simulace růstu", "en": "Growth simulation running" },
    "simulation_progress_replication": { "cs": "Průběh: {p} %", "en": "Progress: {p} %" },

    "simulation_no_output": { "cs": "Žádné výstupy simulace.", "en": "Simulation produced no outputs." },

# Import/Remove
    "import_label": { "cs": "Import atributů", "en": "Import attributes" },
    "remove_label": { "cs": "Odstranění atributů", "en": "Remove attributes" },
    "uploader_label": { "cs": "Nahraj CSV", "en": "Upload CSV" },
    "simulation_no_output": { "cs": "Žádné výstupy simulace.", "en": "Simulation produced no outputs." },

    "csv_ok": { "cs": "CSV prošlo validací", "en": "CSV validation passed" },
    "detected_attributes_label": { "cs": "Detekované atributy:", "en": "Detected attributes:" },

    "existing_att_warn": { "cs": "Následující atributy existují a budou přepsány:\n\n", "en": "The following attributes already exist and will be overwritten:\n\n" },
    "confirm_overwrite": { "cs": "Rozumím a chci pokračovat", "en": "I understand and want to overwrite existing attributes" },

    "import_btn": { "cs": "Importovat atributy", "en": "Import attributes" },
    "remove_btn": { "cs": "Vymaž atributy", "en": "Remove attributes" },

    "import_sucess": { "cs": "Atributy úspěšně importovány", "en": "Attributes successfully imported" },

    "no_usr_att": { "cs": "nenalezeny žádné uživatelské atributy", "en": "No user attributes found" },
    "import_sucess": { "cs": "Vyber atributy k odstranění", "en": "Select attributes to remove" },
    "remove_sucess": { "cs": "Vybrané atributy odstraněny", "en": "Selected attributes removed" },

    # Tree stats
    "stat_count": {"cs": "Počet", "en": "Count"},
    "stat_sum": {"cs": "Suma", "en": "Sum"},
    "stat_mean": {"cs": "Průměr", "en": "Mean"},
    "stat_median": {"cs": "Medián", "en": "Median"},
    "stat_min": {"cs": "Minimum", "en": "Min"},
    "stat_max": {"cs": "Maximum", "en": "Max"},
    "stocking": {"cs": "Zakmenění", "en": "Stocking"},
    "metric_stocking": {"cs": "Zakmenění", "en": "Stocking"},
    
    "warn_missing_required_columns": {
        "cs": "Chybí požadované sloupce: {columns}",
        "en": "Missing required columns: {columns}",
    },

    "clima_scenario": {"cs": "Klimatický scénář:", "en": "Climate scenario:"},

    "rerun_light_label": {"cs": "Spust výpočet dostupného světla:", "en": "Re-run Sky View Factor computation:"},
    "set_voxel_size": {"cs": "Velikost voxelu:", "en": "Voxel size:"},
    "run_light_btn": {"cs": "Start výpočtu:", "en": "Start computation:"},
    
}



# -----------------------------
# i18n state + translation
# -----------------------------
def init_i18n(default_lang: Lang = DEFAULT_LANG) -> None:
    """Call once early in the app (e.g., top of app.py)."""
    if "lang" not in st.session_state:
        st.session_state.lang = default_lang


def set_lang(lang: Lang) -> None:
    st.session_state.lang = lang


def get_lang() -> Lang:
    lang = st.session_state.get("lang", DEFAULT_LANG)
    return lang if lang in ("cs", "en") else DEFAULT_LANG


def t(key: str, **kwargs: Any) -> str:
    """
    Translate using global st.session_state.lang.
    - fallback: en -> key
    - safe formatting (won't crash on missing placeholders)
    """
    lang = get_lang()

    entry = I18N.get(key)
    if not entry:
        return key

    text = entry.get(lang) or entry.get("en") or key

    if kwargs:
        try:
            return text.format(**kwargs)
        except KeyError:
            return text

    return text


def t_help(key: str, **kwargs: Any) -> str:
    """
    Translate using global st.session_state.lang.
    - fallback: en -> key
    - safe formatting (won't crash on missing placeholders)
    """
    lang = get_lang()

    entry = HELP_I18N.get(key)
    if not entry:
        return key

    text = entry.get(lang) or entry.get("en") or key

    if kwargs:
        try:
            return text.format(**kwargs)
        except KeyError:
            return text

    return text


def t_mgmt(status: str) -> str:
    """
    Přeloží management_status (EN hodnota v datech) do aktuálního jazyka.
    Fallback: vrátí původní hodnotu.
    """
    key = MGMT_STATUS_KEY_MAP.get(status)
    return t(key) if key else status

# -----------------------------
# Validation
# -----------------------------
_PLACEHOLDER_RE = re.compile(r"\{([a-zA-Z_][a-zA-Z0-9_]*)\}")


@dataclass(frozen=True)
class I18nValidationResult:
    ok: bool
    warnings: list[str]


def _placeholders(s: str) -> set[str]:
    return set(_PLACEHOLDER_RE.findall(s))


def validate_i18n(i18n: dict[str, dict[Lang, str]] | None = None) -> I18nValidationResult:
    """
    Validates:
    - each key has both 'cs' and 'en'
    - values are non-empty strings
    - placeholder sets match between cs/en (e.g. {value} present in both)
    - warns on suspicious keys (spaces/colon), which often indicates a bad key
    """
    i18n = i18n or I18N
    warnings: list[str] = []

    # dict already can't contain duplicate keys; but we can still validate structure + placeholders
    for key, entry in i18n.items():
        if not isinstance(key, str) or not key:
            raise ValueError(f"Invalid i18n key: {key!r}")

        if ":" in key or " " in key:
            warnings.append(f"Suspicious key (contains space/colon): {key!r}")

        if not isinstance(entry, dict):
            raise ValueError(f"Key {key!r} value must be dict, got {type(entry)}")

        if "cs" not in entry or "en" not in entry:
            raise ValueError(f"Key {key!r} must contain both 'cs' and 'en' translations.")

        cs = entry["cs"]
        en = entry["en"]
        if not isinstance(cs, str) or not cs.strip():
            raise ValueError(f"Key {key!r} cs translation is empty or not a string.")
        if not isinstance(en, str) or not en.strip():
            raise ValueError(f"Key {key!r} en translation is empty or not a string.")

        # placeholder consistency: cs and en must require the same named placeholders
        ph_cs = _placeholders(cs)
        ph_en = _placeholders(en)
        if ph_cs != ph_en:
            raise ValueError(
                f"Placeholder mismatch for key {key!r}: cs={sorted(ph_cs)} vs en={sorted(ph_en)}"
            )

    return I18nValidationResult(ok=True, warnings=warnings)
