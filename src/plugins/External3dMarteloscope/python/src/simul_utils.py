# src/simul_utils.py
from pathlib import Path
import numpy as np
import pandas as pd
from pathlib import Path
import xml.etree.ElementTree as ET


# ============================================================================
# 1) ČTENÍ JEDNÉ SQLITE (POUZE LIVING)
# ============================================================================
def read_single_sqlite_living(path: Path | str, rep_id: str) -> pd.DataFrame:
    path = Path(path)
    if not path.exists():
        return pd.DataFrame()

    import sqlite3

    con = sqlite3.connect(str(path))
    try:
        df = pd.read_sql_query("SELECT * FROM tree", con)
    finally:
        con.close()

    if df.empty:
        return df

    df["replication"] = rep_id
    return df


# ============================================================================
# 2) MAPOVÁNÍ DRUHŮ (iLand kód -> plný název)
# ============================================================================
def map_species_label(sim_trees: pd.DataFrame, species_dict: dict) -> pd.DataFrame:
    """
    Mapuje iLand species kód (abbreviation, např. 'pini')
    -> plný latinský název (např. 'Pinus nigra')
    """
    out = sim_trees.copy()

    # vytvoř mapu: abbreviation -> species
    abbrev_to_species = {
        v.get("abbreviation"): v.get("species")
        for v in species_dict.values()
        if isinstance(v, dict) and "abbreviation" in v and "species" in v
    }

    out["species_label"] = (
        out["species"]
        .astype(str)
        .str.lower()
        .map(abbrev_to_species)
        .fillna("Unknown")
    )

    return out


# ============================================================================
# 3) OZNAČENÍ TARGET TREES (ID + X + Y)
# ============================================================================
def mark_target_trees(sim_trees: pd.DataFrame, trees: pd.DataFrame) -> pd.DataFrame:
    """
    Označí stromy v simulaci jako:
      - 'Target tree'
      - 'Untouched'

    Párování POUZE přes `id` (správné pro iLand).
    """
    out = sim_trees.copy()

    # fallback – žádné vstupní stromy
    if trees is None or trees.empty or "id" not in trees.columns:
        out["management_group"] = "Untouched"
        return out

    # target IDs ze vstupu
    target_ids = set(
        trees.loc[
            trees["management_status"] == "Target tree",
            "id"
        ]
        .dropna()
        .astype(int)
        .tolist()
    )

    # označení v simulaci
    out["management_group"] = np.where(
        out["id"].astype(int).isin(target_ids),
        "Target tree",
        "Untouched",
    )

    return out

# ============================================================================
# 4) AGREGACE PRO FAN CHART (KVANTILY)
# ============================================================================
def agg_fan(
    df: pd.DataFrame,
    group_cols: list[str],
    value_col: str = "volume_m3",
    rep_col: str = "replication",
    quantiles=(0.05, 0.25, 0.5, 0.75, 0.95),
) -> pd.DataFrame:
    """
    Vrátí tabulku:
      group_cols + q5, q25, q50, q75, q95, mean
    """

    d = df.copy()
    if rep_col not in d.columns:
        d[rep_col] = "rep_1"

    # 1) agregace per-rep (SUM)
    per_rep = (
        d.groupby([rep_col] + group_cols, as_index=False)[value_col]
        .sum()
        .rename(columns={value_col: "value"})
    )

    # 2) agregace přes replikace (fan chart)
    agg_dict = {
        "mean": ("value", "mean"),
    }
    for q in quantiles:
        agg_dict[f"q{int(q*100)}"] = ("value", lambda x, q=q: np.nanquantile(x, q))

    stats = (
        per_rep
        .groupby(group_cols, as_index=False)
        .agg(**agg_dict)
    )

    return stats

    #_--____------------------------
    import pandas as pd
from pathlib import Path

# ============================================================================
# 5) EXPORT Trees to CSV
# ============================================================================
def export_iland_trees_csv(
    trees: pd.DataFrame,
    species_dict: dict,
    project_file: str,
    filename: str = "trees.csv",
):
    """
    Vytvoří vstupní CSV pro iLand ze stromů v session_state.trees.

    - vybere pouze Target tree + Untouched
    - mapuje species -> abbreviation (iLand kód)
    - uloží CSV do složky s project JSON
    """

    if trees is None or trees.empty:
        raise ValueError("Input trees DataFrame is empty.")

    # --------------------------------------------------
    # 1) výběr stromů podle managementu
    # --------------------------------------------------
    trees_sel = trees.loc[
        trees["management_status"].isin(["Target tree", "Untouched"])
    ].copy()

    if trees_sel.empty:
        raise ValueError("No trees with management_status Target tree / Untouched.")

    # --------------------------------------------------
    # 2) mapování species -> abbreviation
    # --------------------------------------------------
    latin_to_abbrev = {
        v["species"]: v["abbreviation"]
        for v in species_dict.values()
        if "species" in v and "abbreviation" in v
    }

    trees_sel["species"] = (
        trees_sel["species"]
        .map(latin_to_abbrev)
        .fillna("none")   # fallback (unknown)
        .astype(str)
    )

    xmin = trees_sel["x"].min()
    ymin = trees_sel["y"].min()

    trees_sel["x"] = trees_sel["x"] - xmin
    trees_sel["y"] = trees_sel["y"] - ymin

    # --------------------------------------------------
    # 3) příprava povinných sloupců iLand
    # --------------------------------------------------
    out = pd.DataFrame({
        "id": trees_sel["id"].astype(int),
        "species": trees_sel["species"],
        "x": trees_sel["x"].astype(float),
        "y": trees_sel["y"].astype(float),
        "dbh": trees_sel["dbh"].astype(float),
        "height": trees_sel["height"].astype(float),
        "age": 0,   # iLand dovoluje age=0 (nebo můžeš dopočítat)
    })

    # --------------------------------------------------
    # 4) sanity check (doporučeno)
    # --------------------------------------------------
    if out[["id", "x", "y"]].isna().any().any():
        raise ValueError("NaN detected in id/x/y – invalid input for iLand.")

    if (out["dbh"] <= 0).any():
        raise ValueError("dbh <= 0 detected – invalid for iLand.")

    if (out["height"] <= 0).any():
        raise ValueError("height <= 0 detected – invalid for iLand.")

    # --------------------------------------------------
    # 5) uložení do složky projektu
    # --------------------------------------------------
    project_path = Path(project_file)
    project_dir = project_path.parent
    out_path = project_dir / filename

    out.to_csv(out_path, index=False, sep=";")

    return out_path

# ============================================================================
# 6) MODIFY iLand XML
# ============================================================================

def set_iland_mortality_regeneration(
    xml_path: str | Path,
    mortality_enabled: bool,
    regeneration_enabled: bool,
):
    """
    Upraví projektové iLand XML:
      - <mortalityEnabled>
      - <regenerationEnabled>

    Ostatní části XML zůstávají beze změny.
    """
    xml_path = Path(xml_path)

    if not xml_path.exists():
        raise FileNotFoundError(f"XML file not found: {xml_path}")

    tree = ET.parse(xml_path)
    root = tree.getroot()

    # najdi <settings> block
    settings = root.find(".//model/settings")
    if settings is None:
        raise RuntimeError("Could not find <model><settings> block in XML.")

    def _set_bool(tag: str, value: bool):
        el = settings.find(tag)
        if el is None:
            # pokud element chybí, vytvoříme ho
            el = ET.SubElement(settings, tag)
        el.text = "true" if value else "false"

    _set_bool("mortalityEnabled", mortality_enabled)
    _set_bool("regenerationEnabled", regeneration_enabled)

    tree.write(xml_path, encoding="utf-8", xml_declaration=True)
