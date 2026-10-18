import pandas as pd

def prepare_tree_dataframe(df: pd.DataFrame, colormap: dict) -> pd.DataFrame:
    """Připraví DataFrame stromů pro zobrazení v Bokeh grafu.
    
    - Barvy dle species
    - Obrys dle management_status
    - Velikost dle dbh
    """
    df = df.copy()

    # --- Barvy ---
    species_colormap = colormap.get("species", {})
    management_colormap = colormap.get("management", {})

    df["color"] = df["species"].map(species_colormap).fillna("#aaaaaa")

    # --- Obrys barva ---
    df["line_color"] = df["management_status"].apply(
        lambda status: management_colormap.get(status) if status in ["target", "remove"] else None
    )
    df["line_width"] = df["management_status"].apply(
        lambda status: 2 if status in ["target", "remove"] else 0
    )

    # --- Velikost dle dbh ---
    if "dbh" in df.columns:
        dbh_min, dbh_max = df["dbh"].min(), df["dbh"].max()
        size_min, size_max = 6, 14

        def scale_dbh(dbh):
            if pd.isna(dbh) or dbh_max == dbh_min:
                return 8
            return size_min + (dbh - dbh_min) / (dbh_max - dbh_min) * (size_max - size_min)

        df["size"] = df["dbh"].apply(scale_dbh)
    else:
        df["size"] = 8

    # --- Ostatní pro Bokeh ---
    df["alpha"] = 0.8
    df["id"] = df.index
    df["label"] = df["tree_id"].astype(str) if "tree_id" in df.columns else df.index.astype(str)

    return df

def update_trees(trees_json, updates):
    """
    Aplikuje změny management_status do seznamu stromů (JSON struktury).

    Args:
        trees_json (list[dict]): Původní seznam stromů, každý strom jako slovník.
        updates (dict[int, str]): Mapa indexů stromů na nový management_status.

    Returns:
        list[dict]: Nový seznam stromů s aplikovanými aktualizacemi.
    """
    # Pokud chcete zachovat původní, odkomentujte následující řádek:
    # trees_json = [tree.copy() for tree in trees_json]

    for idx, status in updates.items():
        if not isinstance(idx, int):
            continue
        if 0 <= idx < len(trees_json):
            trees_json[idx]['management_status'] = status
    return trees_json