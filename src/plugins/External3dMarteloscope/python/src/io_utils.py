import json
import pandas as pd
from typing import Dict, List, Union, Any, Optional
import streamlit as st
from pathlib import Path
import sqlite3
import html
import os
import numpy as np
import re
import math
from shapely.geometry import Point, Polygon, MultiPoint, LineString
import shapely
from shapely.ops import triangulate, unary_union, polygonize
from shapely import wkt as shapely_wkt


__all__ = [
    "load_project_json",
    "save_project_json",
    "load_color_palette",
    "load_plot_info",
    "load_simulation_results",
    "heading_centered",
    "_unique_sorted",
    "show_success",
    "load_stocking_reference",
]


# --- Pomocné funkce (samostatné a robustní) ----------------------------------


def load_stocking_reference(db_path: str):
    """
    Načte tabulku stocking_reference (species, ref_volume) ze SQLite DB.

    Vrací:
    - pd.DataFrame, pokud:
        - tabulka existuje
        - a alespoň jedna hodnota ref_volume > 0
    - None, pokud:
        - DB neexistuje
        - tabulka neexistuje
        - tabulka je prázdná
        - nebo všechny ref_volume <= 0
    """
    try:
        conn = sqlite3.connect(db_path)
    except Exception:
        return None

    try:
        df = pd.read_sql(
            "SELECT species, ref_volume FROM stocking_reference",
            conn,
        )
    except Exception:
        return None
    finally:
        conn.close()

    if df.empty:
        return None

    # bezpečná konverze
    df["ref_volume"] = pd.to_numeric(df["ref_volume"], errors="coerce").fillna(0)

    # klíčová kontrola
    if (df["ref_volume"] > 0).any():
        return df

    return None

def alpha_shape(points, alpha=0.3):
    """
    Vypočítá concave hull pomocí alpha shape (Shapely).
    Čím menší alpha → tím detailnější obal.
    """
    if len(points) < 4:
        return MultiPoint(points).convex_hull

    pts = MultiPoint(points)
    triangles = triangulate(pts)

    edges = []

    for tri in triangles:
        coords = list(tri.exterior.coords)
        a, b, c = coords[0], coords[1], coords[2]

        # délky stran
        da = MultiPoint([a, b]).length
        db = MultiPoint([b, c]).length
        dc = MultiPoint([c, a]).length

        # poloměr opsané kružnice
        s = (da + db + dc) / 2.0
        area = tri.area
        if area == 0:
            continue

        R = da * db * dc / (4.0 * area)

        # filtr podle alpha parametru
        if R < 1.0 / alpha:
            edges.append((a, b))
            edges.append((b, c))
            edges.append((c, a))

    if not edges:
        return pts.convex_hull

    # vytvořit LineStringy
    lines = [shapely.geometry.LineString(e) for e in edges]

    # sjednotit
    merged = unary_union(lines)

    # polygonizace
    polygons = list(polygonize(merged))
    if not polygons:
        return pts.convex_hull

    return unary_union(polygons)


def ply_to_wkt_polygon(ply_path: str, alpha=0.3, simplify_tolerance=0.5) -> str:

    if not os.path.exists(ply_path):
        return None

    xs, ys = [], []
    header = True

    with open(ply_path, "r") as f:
        for line in f:
            if header:
                if line.strip().lower() == "end_header":
                    header = False
                continue

            parts = line.strip().split()
            if len(parts) < 2:
                continue

            try:
                x, y = float(parts[0]) * 0.0001, float(parts[1]) * 0.0001

                # odstranit artefakt 0,0
                if x == 0 and y == 0:
                    continue

                xs.append(x)
                ys.append(y)
            except:
                continue

    if len(xs) < 3:
        return None

    points = list(zip(xs, ys))
    
    points = [
        (x, y) for x, y in points
        if not (-0.1 < x < 0.1 or -0.1 < y < 0.1)
    ]

    # --- 1) Vytvoř concave hull ---
    hull = alpha_shape(points, alpha=alpha)

    if hull is None or hull.is_empty:
        return None

    # --- 2) Zjednodušení polygonu ---
    hull = hull.simplify(simplify_tolerance, preserve_topology=True)

    # --- 3) Pokud je to MultiPolygon → vezmeme největší ---
    if isinstance(hull, shapely.geometry.MultiPolygon):
        hull = max(hull.geoms, key=lambda p: p.area)

    # --- 4) Pokud to není Polygon → convex hull fallback ---
    if not isinstance(hull, Polygon):
        hull = hull.convex_hull

    return hull.wkt

def norm(s: str) -> str:
    return (s or "").strip().lower()


def _is_hex_color(x: Any) -> bool:
    return isinstance(x, str) and len(x) == 7 and x.startswith("#")


def _rgb_to_hex01(c01: List[float]) -> str:
    """c01 = [r,g,b] v rozsahu 0..1 → '#RRGGBB'"""
    r = max(0, min(255, int(round(c01[0] * 255))))
    g = max(0, min(255, int(round(c01[1] * 255))))
    b = max(0, min(255, int(round(c01[2] * 255))))
    return f"#{r:02X}{g:02X}{b:02X}"


def _to_color01(value: Any) -> List[float] | None:
    """
    Přijme '#RRGGBB' nebo [r,g,b] v 0..1 či 0..255 a vrátí [r,g,b] v 0..1.
    """
    if value is None:
        return None

    # hex řetězec
    if _is_hex_color(value):
        hs = value[1:]
        try:
            r = int(hs[0:2], 16) / 255.0
            g = int(hs[2:4], 16) / 255.0
            b = int(hs[4:6], 16) / 255.0
            return [r, g, b]
        except Exception:
            return None

    # seznam/tuple čísel
    if isinstance(value, (list, tuple)) and len(value) == 3:
        try:
            r, g, b = float(value[0]), float(value[1]), float(value[2])
        except Exception:
            return None
        # 0..1 nebo 0..255?
        if max(r, g, b) <= 1.0:
            return [max(0.0, min(1.0, r)), max(0.0, min(1.0, g)), max(0.0, min(1.0, b))]
        else:
            return [
                max(0.0, min(1.0, r / 255.0)),
                max(0.0, min(1.0, g / 255.0)),
                max(0.0, min(1.0, b / 255.0)),
            ]
    return None


def _to_hex(value: Any) -> str | None:
    """Vrátí '#RRGGBB' nebo None."""
    if _is_hex_color(value):
        return value.upper()
    c01 = _to_color01(value)
    return _rgb_to_hex01(c01) if c01 else None


def _unique_sorted(series: pd.Series) -> list[str]:
    return sorted(series.dropna().astype(str).unique().tolist())


def heading_centered(text: str, color: str = "#2E7D32", level: int = 5):
    """
    Render a centered heading in Streamlit with custom color and level (h1–h6).

    Parameters
    ----------
    text : str
        Heading text.
    color : str, optional
        CSS color (e.g., '#2E7D32' or 'darkgreen'). Default '#2E7D32'.
    level : int, optional
        Heading level 1–6 (maps to <h1>..</h6>). Default 5.
    """
    # clamp level to 1..6
    try:
        lvl = int(level)
    except Exception:
        lvl = 5
    lvl = min(max(lvl, 1), 6)

    safe_text = html.escape(str(text))
    st.markdown(
        f"<h{lvl} style='text-align:center; color:{color}; margin:0;'>{safe_text}</h{lvl}>",
        unsafe_allow_html=True,
    )


# --- Hlavní funkce ------------------------------------------------------------
def load_color_palette(file_path: str) -> Dict[str, Dict[str, str]]:
    """
    Načte barvy ze секcí 'species' a 'managementStatus' v projektovém JSON.
    Vrací dict:
      {
        "species": { "<latin>": "#RRGGBB", ... },
        "management": { "<label>": "#RRGGBB", ... }
      }
    """
    with open(file_path, "r", encoding="utf-8") as f:
        data: Dict[str, Any] = json.load(f)

    sp_map: Dict[str, str] = {}
    mg_map: Dict[str, str] = {}

    # --- species: latin -> color ---
    for item in data.get("species", []) or []:
        if not isinstance(item, dict):
            continue
        latin = item.get("latin")
        col = item.get("color")
        hexc = _to_hex(col)
        if latin and hexc:
            sp_map[str(latin)] = hexc

    # --- managementStatus: label -> color ---
    for item in data.get("managementStatus", []) or []:
        if not isinstance(item, dict):
            continue
        label = item.get("label")
        col = item.get("color")
        hexc = _to_hex(col)
        if label and hexc:
            mg_map[str(label)] = hexc

    return {"species": sp_map, "management": mg_map}

def load_plot_info(file_path: str) -> pd.DataFrame:
    with open(file_path, "r", encoding="utf-8") as f:
        data: Dict[str, Union[Dict, List]] = json.load(f)

    pi = data.get("plot_info") or []

    return pd.DataFrame(pi)


def load_simulation_results(db_path: str | Path, table: str = "tree") -> pd.DataFrame:
    """
    Load a table (default: 'tree') from a SQLite database into a pandas DataFrame.
    """
    db_path = Path(db_path)
    if not db_path.exists():
        raise FileNotFoundError(f"Database not found: {db_path}")

    with sqlite3.connect(db_path) as con:
        # check table exists
        cur = con.cursor()
        cur.execute(
            "SELECT name FROM sqlite_master WHERE type='table' AND name=?;", (table,)
        )
        if cur.fetchone() is None:
            raise ValueError(f"Table '{table}' not found in {db_path}")

        # load via pandas
        df = pd.read_sql_query(f"SELECT * FROM {table};", con)

    st.session_state.simulation = df
    return df




def show_success(message: str, timeout: int = 2000):
    """
    Zobrazí úspěchovou zprávu na pár vteřin.

    Args:
        message: text zprávy
        timeout: doba zobrazení v ms (2000 = 2 vteřiny)
    """
    placeholder = st.empty()
    placeholder.markdown(
        f"""
        <div id="tmp-success" style="background:#d4edda;
             color:#155724;
             border:1px solid #c3e6cb;
             padding:0.75rem 1rem;
             border-radius:0.25rem;
             margin:0.5rem 0;">
          ✅ {message}
        </div>
        <script>
        setTimeout(function(){{
            var el = document.getElementById("tmp-success");
            if(el) el.style.display = 'none';
        }}, {timeout});
        </script>
        """,
        unsafe_allow_html=True,
    )


def save_project_json(
    original_path: str, df: pd.DataFrame, output_path: str = None
) -> None:
    with open(original_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    # mapování řádek podle ID
    id_map = df.set_index("id").to_dict("index")

    for segment in data.get("segments", []):
        sid = segment.get("id")
        if sid in id_map and "treeAttributes" in segment:
            row = id_map[sid]
            attr = segment["treeAttributes"]
            print("a")
            if "speciesId" in row:
                segment["speciesId"] = int(row["speciesId"])
                print(segment["speciesId"])
                print(row["speciesId"])

            # zachovat původní Z, ale aktualizovat X,Y pokud jsou v DF
            z = 0.0
            if isinstance(attr.get("position"), list) and len(attr["position"]) >= 3:
                z = attr["position"][2]

            if "x" in row and "y" in row:
                attr["position"] = [float(row["x"]), float(row["y"]), z]

            # --- převod HEX -> RGB (0..1) a zapsání do speciesColor/managementColor ---
            if "speciesColorHex" in row and isinstance(row["speciesColorHex"], str):
                rgb = _to_color01(row["speciesColorHex"])
                if rgb is not None:
                    attr["speciesColor"] = rgb

            if "managementColorHex" in row and isinstance(
                row["managementColorHex"], str
            ):
                rgb = _to_color01(row["managementColorHex"])
                if rgb is not None:
                    attr["managementColor"] = rgb

            # --- kopírování dalších atributů do treeAttributes ---
            # vynecháme klíče, které nechceme vkládat (HEX, id) a ty, co existují ve vnějším segmentu
            skip_keys = {"id", "speciesColorHex", "managementColorHex"}
            outer_keys = set(segment.keys())
            for key, value in row.items():
                if key in skip_keys or key in outer_keys:
                    continue
                # speciesColor / managementColor už jsme nastavili výše z HEX – nepřepisovat z DF,
                # pokud by tam náhodou byly i floatové sloupce stejného jména
                if key in {"speciesColor", "managementColor"}:
                    continue
                attr[key] = value

    output_path = output_path or original_path
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


def load_project_json(file_path: str, exclude_from_sql_update: List[str] = None) -> pd.DataFrame:
    """
    Načte projektový JSON 3DForest, vytvoří DataFrame stromů, sloučí ho se SQLite databází,
    doplní chybějící 2D projekce koruny (PLY → WKT) do SQLite přes UPDATE
    a spočte metriky projection_exposure a projection_exposure_after_mgmt.
    """
    # --------------------------------------------------------------------------------------
    # 0) Sloupce, které SQL nesmí nikdy přepsat
    # --------------------------------------------------------------------------------------
    if exclude_from_sql_update is None:
        exclude_from_sql_update = []

    protected_cols = [
        "species",
        "speciesColorHex",
        "management_status",
        "managementColorHex",
        "label",
    ]

    exclude_final = list(set(exclude_from_sql_update + protected_cols))

    # --------------------------------------------------------------------------------------
    # 1) Načtení JSON souboru
    # --------------------------------------------------------------------------------------
    if not os.path.exists(file_path):
        raise FileNotFoundError(file_path)

    with open(file_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    # --------------------------------------------------------------------------------------
    # 2) Lookup tabulky species + management
    # --------------------------------------------------------------------------------------
    sp_id_map = {}
    for item in data.get("species", []):
        try:
            sid = int(item.get("id"))
            sp_id_map[sid] = {
                "latin": item.get("latin", "Unknown"),
                "color": _to_hex(item.get("color")),
            }
        except:
            pass

    mg_id_map = {}
    for item in data.get("managementStatus", []):
        try:
            mid = int(item.get("id"))
            mg_id_map[mid] = {
                "label": item.get("label", "Unknown"),
                "color": _to_hex(item.get("color")),
            }
        except:
            pass
    
    # --------------------------------------------------------------------------------------
    # 3) Segmenty → DataFrame
    # --------------------------------------------------------------------------------------
    rows = []

    for seg in data.get("segments", []):
        base = {}

        base["id"] = seg.get("id")
        base["label"] = seg.get("label", "")

        # Lookup IDs
        try:
            s_id = int(seg.get("speciesId"))
        except:
            s_id = -1
        try:
            m_id = int(seg.get("managementStatusId"))
        except:
            m_id = -1

        # Lookup data
        sp = sp_id_map.get(s_id, {})
        mg = mg_id_map.get(m_id, {})

        base["species"] = sp.get("latin", "Unknown")
        base["speciesColorHex"] = sp.get("color")
        base["management_status"] = mg.get("label", "Unknown")
        base["managementColorHex"] = mg.get("color")

        # Atributy stromu
        SKIP_ATTRS = {"speciesColorHex", "managementColorHex", "species", "management_status"}

        attrs = seg.get("treeAttributes", {}) or {}
        for k, v in attrs.items():
            if k == "position":
                continue
            if k in SKIP_ATTRS:
                continue
            base[k] = v

        # Pozice
        pos = attrs.get("position")
        if isinstance(pos, list) and len(pos) >= 2:
            base["x"] = float(pos[0])
            base["y"] = float(pos[1])
            if len(pos) > 2:
                base["z"] = float(pos[2])
        else:
            base["x"], base["y"] = 0.0, 0.0

        # Crown centroid height = crownCenter.z - position.z
        crown_center = attrs.get("crownCenter")
        if (
            isinstance(crown_center, list) and len(crown_center) >= 3
            and isinstance(pos, list) and len(pos) >= 3
        ):
            try:
                base["crown_centroid_height"] = float(crown_center[2]) - float(pos[2])
            except (TypeError, ValueError):
                base["crown_centroid_height"] = np.nan
        else:
            base["crown_centroid_height"] = np.nan

        rows.append(base)


    df_json = pd.DataFrame(rows)
    df_json["dbh"] = df_json["dbh"] * 100 

    if df_json.empty:
        return df_json

    df_json["id"] = df_json["id"].astype(int)
    df_json.set_index("id", inplace=True, drop=False)

    # Odstranit ground/unsegmented
    if "label" in df_json.columns:
        df_json = df_json[
            ~df_json["label"].astype(str).str.contains("ground|unsegmented", case=False, na=False)
        ]

    # --------------------------------------------------------------------------------------
    # 4) SQLite MERGE
    # --------------------------------------------------------------------------------------
    sqlite_path = os.path.splitext(file_path)[0] + ".sqlite"
    table_name = "tree"

    # A) Pokud SQLite neexistuje → vytvořit základní tabulku
    if not os.path.exists(sqlite_path):
        try:
            conn = sqlite3.connect(sqlite_path)
            df_json[["id", "label", "species"]].to_sql(
                table_name, conn, if_exists="replace", index=False
            )
            conn.close()
        except Exception as e:
            print("Chyba při tvorbě SQLite:", e)

    # B) SQLite existuje → merge
    if os.path.exists(sqlite_path):
        try:
            conn = sqlite3.connect(sqlite_path)

            exists = not pd.read_sql(
                f"SELECT name FROM sqlite_master WHERE type='table' AND name='{table_name}';",
                conn
            ).empty

            if exists:
                df_sql = pd.read_sql(f"SELECT * FROM {table_name}", conn)

                if not df_sql.empty:
                    df_sql["id"] = df_sql["id"].astype(int)
                    df_sql.set_index("id", inplace=True, drop=False)

                    # Sloupce, které smíme přepsat
                    update_cols = [
                        c for c in df_sql.columns
                        if c in df_json.columns and c not in exclude_final
                    ]

                    if update_cols:
                        df_json.update(df_sql[update_cols])

                    # Nové sloupce z SQL → přidat
                    new_cols = [c for c in df_sql.columns if c not in df_json.columns]
                    if new_cols:
                        df_json = df_json.join(df_sql[new_cols])

            conn.close()

        except Exception as e:
            print("Chyba merge SQLite:", e)

    # --------------------------------------------------------------------------------------
    # 5) POLYGONY (PLY → WKT → SQLite UPDATE)
    # --------------------------------------------------------------------------------------
    project_dir = os.path.dirname(file_path)
    project_name = os.path.splitext(os.path.basename(file_path))[0]

    # Sloupec v DF
    if "planar_projection_poly" not in df_json.columns:
        df_json["planar_projection_poly"] = None

    # Načíst existující polygony ze SQL, pokud jsou
    try:
        conn = sqlite3.connect(sqlite_path)
        sql_cols = pd.read_sql(f"PRAGMA table_info({table_name});", conn)["name"].tolist()

        if "planar_projection_poly" in sql_cols:
            df_poly = pd.read_sql(
                f"SELECT id, planar_projection_poly FROM {table_name}", conn
            )
            df_poly["id"] = df_poly["id"].astype(int)
            df_poly.set_index("id", inplace=True, drop=False)
            df_json.update(df_poly[["planar_projection_poly"]])

        conn.close()
    except:
        pass

    # Najít chybějící WKT polygony
    missing = df_json[df_json["planar_projection_poly"].isna()]

    if not missing.empty:
        # dopočítat WKT z PLY
        for tid in missing["id"]:
            
            ply_name = f"{project_name}.{tid}.concaveHullProjection.ply"
            ply_path = os.path.join(project_dir, ply_name)
            print(ply_path)
            wkt_poly = ply_to_wkt_polygon(ply_path)
            if wkt_poly:
                df_json.loc[tid, "planar_projection_poly"] = wkt_poly

        # uložit do SQL pomocí UPDATE
        try:
            conn = sqlite3.connect(sqlite_path)

            # přidat sloupec, pokud neexistuje
            sql_cols = pd.read_sql(f"PRAGMA table_info({table_name});", conn)["name"].tolist()
            if "planar_projection_poly" not in sql_cols:
                conn.execute(f"ALTER TABLE {table_name} ADD COLUMN planar_projection_poly TEXT;")
                conn.commit()

            # UPDATE řádek po řádku
            for tid, row in df_json.iterrows():
                poly = row.get("planar_projection_poly")
                if poly and isinstance(poly, str):
                    conn.execute(
                        f"UPDATE {table_name} SET planar_projection_poly = ? WHERE id = ?",
                        (poly, int(tid))
                    )

            conn.commit()
            conn.close()

        except Exception as e:
            print("Chyba ukládání polygonů do SQLite:", e)

    # --------------------------------------------------------------------------------------
    # 6) PROJECTION EXPOSURE (jen když máme height a polygon)
    # --------------------------------------------------------------------------------------
    # inicializace sloupců
    if "projection_exposure" not in df_json.columns:
        df_json["projection_exposure"] = np.nan
    if "projection_exposure_after_mgmt" not in df_json.columns:
        df_json["projection_exposure_after_mgmt"] = np.nan

    # zkusíme najít název výškového sloupce
    height_col = None
    for cand in ["height", "tree_height", "h"]:
        if cand in df_json.columns:
            height_col = cand
            break

    # --- nejdřív zjistíme, jestli už projection_exposure existuje v SQLite ---
    has_projection_exposure_in_sql = False
    if os.path.exists(sqlite_path):
        try:
            conn = sqlite3.connect(sqlite_path)
            sql_cols = pd.read_sql(f"PRAGMA table_info({table_name});", conn)["name"].tolist()
            has_projection_exposure_in_sql = "projection_exposure" in sql_cols

            if has_projection_exposure_in_sql:
                # jen načteme existující hodnoty
                df_proj = pd.read_sql(
                    f"SELECT id, projection_exposure FROM {table_name}", conn
                )
                if not df_proj.empty:
                    df_proj["id"] = df_proj["id"].astype(int)
                    df_proj.set_index("id", inplace=True, drop=False)
                    df_json.update(df_proj[["projection_exposure"]])

            conn.close()
        except Exception as e:
            print("Chyba čtení projection_exposure ze SQLite:", e)
            has_projection_exposure_in_sql = False

    # Pokud height nebo polygony chybí, není co počítat → jen vrátíme (basal_area dopočítáme níže)
    can_compute_exposure = (
        height_col is not None
        and "planar_projection_poly" in df_json.columns
    )

    # Budeme potřebovat geometrie i pro after_mgmt, tak je připravíme jen jednou
    geoms = {}
    areas = {}
    heights = {}

    if can_compute_exposure:
        for idx, row in df_json.iterrows():
            wkt_str = row.get("planar_projection_poly")
            if not isinstance(wkt_str, str) or not wkt_str.strip():
                continue
            try:
                g = shapely_wkt.loads(wkt_str)
                if g.is_empty or not g.is_valid:
                    continue
                # pokud MultiPolygon → vezmeme největší část
                if isinstance(g, shapely.geometry.MultiPolygon):
                    g = max(g.geoms, key=lambda p: p.area)
                a = g.area
                if a <= 0:
                    continue
                geoms[idx] = g
                areas[idx] = a
                h_val = row.get(height_col)
                try:
                    heights[idx] = float(h_val) if pd.notna(h_val) else None
                except:
                    heights[idx] = None
            except Exception:
                continue

    # --- 6a) projection_exposure: počítáme jen když není v SQL ---
    if can_compute_exposure and not has_projection_exposure_in_sql and geoms:
        ids = list(geoms.keys())

        for i in ids:
            h_i = heights.get(i)
            geom_i = geoms.get(i)
            area_i = areas.get(i)
            if geom_i is None or area_i is None or area_i <= 0 or h_i is None or math.isnan(h_i):
                continue

            exp_geom = geom_i
            # okolní vyšší stromy
            for j in ids:
                if j == i:
                    continue
                h_j = heights.get(j)
                if h_j is None or math.isnan(h_j) or h_j <= h_i:
                    continue
                gj = geoms.get(j)
                if gj is None:
                    continue
                if not exp_geom.intersects(gj):
                    continue
                try:
                    exp_geom = exp_geom.difference(gj)
                    if exp_geom.is_empty:
                        break
                except Exception:
                    # když se něco rozbije, prostě skončíme s tím, co máme
                    break

            exposed_area = exp_geom.area if (exp_geom is not None and not exp_geom.is_empty) else 0.0
            df_json.at[i, "projection_exposure"] = 100.0 * exposed_area / area_i

        # uložíme do SQLite
        try:
            conn = sqlite3.connect(sqlite_path)
            sql_cols = pd.read_sql(f"PRAGMA table_info({table_name});", conn)["name"].tolist()
            if "projection_exposure" not in sql_cols:
                conn.execute(f"ALTER TABLE {table_name} ADD COLUMN projection_exposure REAL;")
                conn.commit()

            for tid, row in df_json.iterrows():
                val = row.get("projection_exposure")
                if pd.isna(val):
                    continue
                conn.execute(
                    f"UPDATE {table_name} SET projection_exposure = ? WHERE id = ?",
                    (float(val), int(row["id"]))
                )

            conn.commit()
            conn.close()
        except Exception as e:
            print("Chyba ukládání projection_exposure do SQLite:", e)

    # --- 6b) projection_exposure_after_mgmt: jen v DF, bez SQL logiky ---
    if can_compute_exposure and geoms and "management_status" in df_json.columns:
        # povolené management statusy
        allowed_status = {"Target tree", "Untouched"}

        # indexy stromů, které do výpočtu vstupují
        mgmt_ids = [
            idx for idx, row in df_json.iterrows()
            if row.get("management_status") in allowed_status and idx in geoms
        ]

        # ostatní mají NA
        df_json["projection_exposure_after_mgmt"] = np.nan

        for i in mgmt_ids:
            h_i = heights.get(i)
            geom_i = geoms.get(i)
            area_i = areas.get(i)
            if geom_i is None or area_i is None or area_i <= 0 or h_i is None or math.isnan(h_i):
                continue

            exp_geom = geom_i
            # pouze okolní stromy, které také mají management_status v allowed_status
            for j in mgmt_ids:
                if j == i:
                    continue
                h_j = heights.get(j)
                if h_j is None or math.isnan(h_j) or h_j <= h_i:
                    continue
                gj = geoms.get(j)
                if gj is None:
                    continue
                if not exp_geom.intersects(gj):
                    continue
                try:
                    exp_geom = exp_geom.difference(gj)
                    if exp_geom.is_empty:
                        break
                except Exception:
                    break

            exposed_area = exp_geom.area if (exp_geom is not None and not exp_geom.is_empty) else 0.0
            df_json.at[i, "projection_exposure_after_mgmt"] = 100.0 * exposed_area / area_i

    # --------------------------------------------------------------------------------------
    # 7) dopočet bazální plochy [m²] z DBH [cm]  -> BA = π * (dbh_cm / 200)^2
    # --------------------------------------------------------------------------------------
    if "dbh" in df_json.columns:
        dbh_cm = pd.to_numeric(df_json["dbh"], errors="coerce")
        df_json["basal_area_m2"] = np.pi * (dbh_cm / 200.0) ** 2
    else:
        df_json["basal_area_m2"] = np.nan
    
    # --------------------------------------------------------------------------------------
    # 8) Štíhlostní koeficient heightXdbh = height / (dbh/100)  [height v m, dbh v cm → m]
    # --------------------------------------------------------------------------------------
    df_json["heightXdbh"] = np.nan

    if height_col is not None and "dbh" in df_json.columns:
        try:
            h = pd.to_numeric(df_json[height_col], errors="coerce")
            dbh_cm = pd.to_numeric(df_json["dbh"], errors="coerce")

            # přepočet dbh cm → m
            dbh_m = dbh_cm / 100.0

            df_json["heightXdbh"] = h / dbh_m
        except Exception as e:
            print("Chyba výpočtu heightXdbh:", e)

    # --------------------------------------------------------------------------------------
    return df_json.reset_index(drop=True)

def load_mgmt_example_sqlite(file_path: str, table_name: str = "mgmt_example") -> pd.DataFrame:
    """
    Načte tabulku `mgmt_example` ze SQLite databáze, která patří k projektu (stejný základ názvu jako JSON).
    Vrací obsah tabulky jako pandas DataFrame.
    """
    sqlite_path = os.path.splitext(file_path)[0] + ".sqlite"

    print(sqlite_path)
    print(table_name)

    if not os.path.exists(sqlite_path):
        raise FileNotFoundError(f"SQLite DB neexistuje: {sqlite_path}")

    conn = sqlite3.connect(sqlite_path)
    try:
        # ověřit existenci tabulky
        exists = not pd.read_sql(
            f"SELECT name FROM sqlite_master WHERE type='table' AND name='{table_name}';",
            conn
        ).empty

        if not exists:
            # když tabulka není, vrátíme prázdný DF
            return pd.DataFrame()

        df = pd.read_sql(f"SELECT * FROM {table_name}", conn)
        print(df)
        return df

    finally:
        conn.close()

def _ensure_usr_mgmt_column(trees: pd.DataFrame, mgmt_example: pd.DataFrame) -> pd.DataFrame:
    """
    Zajistí, že mgmt_example má sloupec 'usr_mgmt' (a je naplněn z trees.management_status podle ID).
    Vrací mgmt_example (může být kopie).
    """
    if mgmt_example is None or mgmt_example.empty:
        # Když tabulka neexistuje, vytvoříme minimální s id + usr_mgmt
        out = trees[["id", "management_status"]].copy()
        out.rename(columns={"management_status": "usr_mgmt"}, inplace=True)
        return out

    out = mgmt_example.copy()

    # sjednotit id jako int a mít index
    if "id" not in out.columns:
        raise ValueError("mgmt_example musí mít sloupec 'id'")

    out["id"] = pd.to_numeric(out["id"], errors="coerce").astype("Int64")
    out = out.dropna(subset=["id"]).copy()
    out["id"] = out["id"].astype(int)
    out.set_index("id", inplace=True, drop=False)

    # trees id/index
    tr = trees.copy()
    if "id" not in tr.columns:
        raise ValueError("trees musí mít sloupec 'id'")
    tr["id"] = pd.to_numeric(tr["id"], errors="coerce").astype("Int64")
    tr = tr.dropna(subset=["id"]).copy()
    tr["id"] = tr["id"].astype(int)
    tr.set_index("id", inplace=True, drop=False)

    if "usr_mgmt" not in out.columns:
        out["usr_mgmt"] = pd.NA

    # doplnit usr_mgmt jen tam, kde je NA (abychom nepřepsali dřívější user volby)
    missing_mask = out["usr_mgmt"].isna()
    if missing_mask.any() and "management_status" in tr.columns:
        out.loc[missing_mask, "usr_mgmt"] = tr.loc[out.index[missing_mask], "management_status"].reindex(out.index[missing_mask])

    return out.reset_index(drop=True)

def apply_mgmt_selection(
    selection_key: str,
    trees_key: str = "trees",
    mgmt_key: str = "mgmt_example",
    mgmt_status_col: str = "management_status",
    usr_col: str = "usr_mgmt",
) -> None:
    """
    Přepne trees[management_status] podle selection_key:
    - 'usr_mgmt' => obnoví z mgmt_example['usr_mgmt']
    - jinak => uloží aktuální trees management_status do mgmt_example['usr_mgmt'] a přepíše z mgmt_example[selection_key]
    """
    trees = st.session_state[trees_key]
    mgmt_example = st.session_state[mgmt_key]

    # zajistit usr_mgmt
    mgmt_example = _ensure_usr_mgmt_column(trees, mgmt_example)

    # připravit indexy
    tr = trees.copy()
    tr["id"] = pd.to_numeric(tr["id"], errors="coerce").astype("Int64")
    tr = tr.dropna(subset=["id"]).copy()
    tr["id"] = tr["id"].astype(int)
    tr.set_index("id", inplace=True, drop=False)

    mg = mgmt_example.copy()
    mg["id"] = pd.to_numeric(mg["id"], errors="coerce").astype("Int64")
    mg = mg.dropna(subset=["id"]).copy()
    mg["id"] = mg["id"].astype(int)
    mg.set_index("id", inplace=True, drop=False)

    # jen společná ID
    common_ids = tr.index.intersection(mg.index)

    if selection_key == usr_col:
        # restore user-defined
        if usr_col not in mg.columns:
            # nic k obnovení
            st.session_state[mgmt_key] = mg.reset_index(drop=True)
            st.session_state[trees_key] = tr.reset_index(drop=True)
            return

        restored = mg.loc[common_ids, usr_col]
        # přepis jen tam, kde restored není NA
        mask = restored.notna()
        tr.loc[common_ids[mask], mgmt_status_col] = restored.loc[common_ids[mask]].astype(object)

    else:
        # switching to example -> uložit current do usr_mgmt
        if mgmt_status_col in tr.columns:
            mg.loc[common_ids, usr_col] = tr.loc[common_ids, mgmt_status_col].astype(object)

        # aplikovat example sloupec
        if selection_key not in mg.columns:
            st.warning(f"Sloupec '{selection_key}' není v mgmt_example.")
        else:
            ex = mg.loc[common_ids, selection_key]
            mask = ex.notna()
            tr.loc[common_ids[mask], mgmt_status_col] = ex.loc[common_ids[mask]].astype(object)

    # Refresh colors
    palette = st.session_state.get("color_palette", {})
    tr = refresh_management_colors(tr, palette)
    # uložit zpět
    st.session_state[mgmt_key] = mg.reset_index(drop=True)
    st.session_state[trees_key] = tr.reset_index(drop=True)
    st.session_state["active_mgmt_selection"] = selection_key

def refresh_management_colors(
    trees: pd.DataFrame,
    color_palette: dict,
    status_col: str = "management_status",
    color_col: str = "managementColorHex",
    default_color: str = "#A6A6A6",  # fallback, kdyby label nebyl v paletě
) -> pd.DataFrame:
    """
    Aktualizuje trees[managementColorHex] podle trees[management_status] a palety.
    Vrací kopii DF.
    """
    out = trees.copy()

    mg_map = (color_palette or {}).get("management", {}) if isinstance(color_palette, dict) else {}

    if status_col not in out.columns:
        return out

    out[color_col] = out[status_col].astype(str).map(mg_map).fillna(default_color)
    return out