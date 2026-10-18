# sun_access_cone_raytracing_project_json.py
# -*- coding: utf-8 -*-

import json
import math
import os
from collections import defaultdict, Counter
from typing import Dict

import numpy as np
import pandas as pd
from functools import lru_cache

# =============================================================================
# PARAMETRY
# =============================================================================
APEX_ANGLE_DEG = 60.0        # vrcholový úhel kužele (celkový), poloviční úhel = 30°
VOXEL_SIZE = 0.25            # [m] velikost voxelu
CONE_HEIGHT = 60.0           # [m] max délka paprsku / výška kužele
NEIGHBOR_RADIUS = 40.0       # [m] horizontální okno pro sousedy
Z_MARGIN_UP = 10.0           # [m] rezerva nad Z cílového bodu
Z_MIN = 0.0                  # [m] spodní mez
N_RAY_SAMPLES = 5000         # počet paprsků v kuželu (vyšší= přesnější, pomalejší)
RAY_T0 = 0 * VOXEL_SIZE    # malé odsazení od apexu

# --- VSTUP / VÝSTUP ---
INPUT_PROJECT_JSON = r"D:/GS_LCR_DELIVERABLE/Buchlovice/Buchlovice.json"
OUTPUT_PROJECT_JSON = r"D:/GS_LCR_DELIVERABLE/Buchlovice/Buchlovice.json"

# --- LAS složka ---
# Defaultně se použije složka, kde leží INPUT_PROJECT_JSON.
# Pokud chceš jinou, nastav LAS_DIR_OVERRIDE na existující cestu.
LAS_DIR_OVERRIDE = r""  # např. r"C:/data/las" nebo nech prázdné

# Název LAS se skládá: <label>.las
LAS_EXT = ".las"

# --- APEX posun v Z ---
# nový apex Z = crownCenterZ + 0.16 * crown_height, kde crown_height = height - crownStartHeight
APEX_CROWN_HEIGHT_SHIFT_FACTOR = 0.0

# --- Výstupní klíče do treeAttributes ---
OUT_LIGHT_AVAIL_KEY = "light_avail"   # % volného prostoru
OUT_LIGHT_COMP_KEY = "light_comp"     # dict {treeId: pct}

EXCLUDE_LABEL_SUBSTRINGS = ("ground", "unsegmented")  # case-insensitive

def _is_zero_center(c):
    try:
        return (
            isinstance(c, (list, tuple))
            and len(c) >= 3
            and abs(float(c[0])) < 1e-6
            and abs(float(c[1])) < 1e-6
            and abs(float(c[2])) < 1e-6
        )
    except Exception:
        return False

def compute_crown_center_from_las(
    las_path: str,
    voxel_size: float,
    z_base: float,
    crown_start_height: float
) -> np.ndarray | None:
    """
    Spočítá crownCenter jako průměr středů voxelů koruny.
    Koruna = body výše než z_base + crownStartHeight.
    """
    X, Y, Z = _load_las_xyz(las_path)

    z_min = z_base + crown_start_height
    mask = Z >= z_min

    if not np.any(mask):
        return None

    pts = np.vstack([X[mask], Y[mask], Z[mask]]).T.astype(np.float64)

    # voxelizace
    ijk = np.floor(pts / voxel_size).astype(np.int64)

    # unikátní voxely
    uniq = np.unique(ijk, axis=0)

    # střed voxelů
    centers = (uniq.astype(np.float64) + 0.5) * voxel_size

    return centers.mean(axis=0)

# =============================================================================
# WRITE JSON (tvá funkce, beze změny)
# =============================================================================
# =============================================================================
# WRITE JSON – STRIKTNĚ KONTROLOVANÝ ZÁPIS
# =============================================================================
def write_json(original_path: str, df: pd.DataFrame, output_path: str = None) -> None:
    with open(original_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    id_map = df.set_index("id").to_dict("index")

    for segment in data.get("segments", []):
        sid = segment.get("id")
        if sid not in id_map or "treeAttributes" not in segment:
            continue

        row = id_map[sid]
        attr = segment["treeAttributes"]

        # ---------------------------------------------------------------------
        # 1) light_avail + light_comp – VŽDY povoleno
        # ---------------------------------------------------------------------
        if "light_avail" in row:
            attr["light_avail"] = row["light_avail"]

        if "light_comp" in row:
            attr["light_comp"] = row["light_comp"]

        # ---------------------------------------------------------------------
        # 2) crownCenter – POUZE pokud neexistuje nebo je [0,0,0]
        # ---------------------------------------------------------------------
        if "crownCenter" in row and row["crownCenter"] is not None:
            existing = attr.get("crownCenter")

            def _is_zero_center(c):
                try:
                    return (
                        isinstance(c, list)
                        and len(c) >= 3
                        and abs(float(c[0])) < 1e-6
                        and abs(float(c[1])) < 1e-6
                        and abs(float(c[2])) < 1e-6
                    )
                except Exception:
                    return False

            if existing is None or _is_zero_center(existing):
                attr["crownCenter"] = row["crownCenter"]

        # ---------------------------------------------------------------------
        # 3) NIC JINÉHO SE NESMÍ MĚNIT
        # ---------------------------------------------------------------------

    output_path = output_path or original_path
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

# =============================================================================
# RAYTRACING HELPERS
# =============================================================================
def unit_vector(v: np.ndarray) -> np.ndarray:
    n = np.linalg.norm(v)
    if n == 0:
        return v
    return v / n


def sample_directions_in_cone(axis: np.ndarray, half_angle_rad: float, n_samples: int) -> np.ndarray:
    """
    Náhodné přibližně rovnoměrné směry uvnitř kužele o polovičním úhlu half_angle_rad kolem osy axis.
    """
    axis = unit_vector(axis)

    # ortonormální báze kolem axis
    if abs(axis[2]) < 0.999:
        x_axis = unit_vector(np.cross(axis, np.array([0, 0, 1.0])))
    else:
        x_axis = unit_vector(np.cross(axis, np.array([0, 1.0, 0])))
    y_axis = np.cross(axis, x_axis)

    cos_min = math.cos(half_angle_rad)
    u = np.random.rand(n_samples)
    cos_theta = 1 - u * (1 - cos_min)
    sin_theta = np.sqrt(1 - cos_theta**2)
    phi = 2 * np.pi * np.random.rand(n_samples)

    dirs_local = (
        (sin_theta * np.cos(phi))[:, None] * x_axis[None, :] +
        (sin_theta * np.sin(phi))[:, None] * y_axis[None, :] +
        (cos_theta)[:, None] * axis[None, :]
    )
    return dirs_local


def points_in_inverted_cone(apex: np.ndarray, axis: np.ndarray, half_angle_rad: float, height: float, pts: np.ndarray) -> np.ndarray:
    """
    Vrátí masku bodů v kuželu s vrcholem v apex a osou axis (nahoru), poloviční úhel half_angle_rad, délka height.
    """
    v = pts - apex
    proj_len = np.dot(v, axis)
    inside_height = (proj_len >= 0) & (proj_len <= height)

    v_norm = np.linalg.norm(v, axis=1)
    safe = v_norm > 0
    cosang = np.zeros_like(v_norm)
    cosang[safe] = (np.dot(v[safe], axis)) / v_norm[safe]

    inside_angle = np.zeros_like(inside_height, dtype=bool)
    inside_angle[safe] = cosang[safe] >= math.cos(half_angle_rad)

    return inside_height & inside_angle


def voxel_hash(points: np.ndarray, voxel_size: float, extra_payload=None, reducer: str = "first"):
    """
    Založí obsazenost voxelů.
    Vrací:
      - set klíčů voxelů (i,j,k)
      - map voxel -> payload (např. id stromu)
    """
    ijk = np.floor(points / voxel_size).astype(np.int64)
    keys = [tuple(x) for x in ijk]
    occ = {}

    if extra_payload is None:
        for k in keys:
            if k not in occ:
                occ[k] = None
        return set(occ.keys()), occ

    if reducer == "first":
        for k, p in zip(keys, extra_payload):
            if k not in occ:
                occ[k] = p
    elif reducer == "mode":
        bags = defaultdict(list)
        for k, p in zip(keys, extra_payload):
            bags[k].append(p)
        for k, bag in bags.items():
            occ[k] = Counter(bag).most_common(1)[0][0] if bag else None
    else:
        for k in keys:
            if k not in occ:
                occ[k] = None

    return set(occ.keys()), occ


def raytrace_voxels(
    apex: np.ndarray,
    dirs: np.ndarray,
    voxel_size: float,
    height: float,
    occ_set: set,
    occ_payload: dict,
    t0: float = 0.0,
):
    """
    Jednoduchý voxel stepping (krok ~ voxel_size).
    Pro každý směr vrací tuple (hit_bool, hit_payload_or_None).
    """
    results = []
    for d in dirs:
        d = unit_vector(d)
        t = float(t0)
        hit_payload = None
        hit = False
        while t <= height:
            p = apex + d * t
            k = tuple(np.floor(p / voxel_size).astype(np.int64))
            if k in occ_set:
                hit = True
                hit_payload = occ_payload.get(k, None)
                break
            t += voxel_size
        results.append((hit, hit_payload))
    return results

@lru_cache(maxsize=1000)
def _load_las_xyz(las_path: str):
    import laspy
    with laspy.open(las_path) as f:
        las = f.read()

    X = las.X * las.header.scale[0] + las.header.offsets[0]
    Y = las.Y * las.header.scale[1] + las.header.offsets[1]
    Z = las.Z * las.header.scale[2] + las.header.offsets[2]
    return X, Y, Z

def laspy_read_points_window(las_path: str, x: float, y: float, z_min: float, z_max: float, radius: float) -> np.ndarray:
    X, Y, Z = _load_las_xyz(las_path)

    dx = X - x
    dy = Y - y
    r2 = dx * dx + dy * dy

    mask = (r2 <= radius * radius) & (Z >= z_min) & (Z <= z_max)
    if not np.any(mask):
        return np.empty((0, 3), dtype=np.float32)

    return np.vstack([X[mask], Y[mask], Z[mask]]).T.astype(np.float32)


# =============================================================================
# PROJECT JSON LOADER (minimal) + LAS path + apex Z shift
# =============================================================================
def _sanitize_label_for_filename(label: str) -> str:
    """
    Pokud label obsahuje znaky, které na Windows dělají problémy,
    nahradíme je podtržítkem.
    """
    if not isinstance(label, str):
        return ""
    bad = ['<', '>', ':', '"', '/', '\\', '|', '?', '*']
    out = label.strip()
    for ch in bad:
        out = out.replace(ch, "_")
    return out

def _is_excluded_label(label: str) -> bool:
    if not isinstance(label, str):
        return False
    s = label.lower()
    return any(sub in s for sub in EXCLUDE_LABEL_SUBSTRINGS)

def _compute_apex_z(z_center: float, height, crown_start) -> float:
    """
    Posun apexu: z_center + 0.16 * crown_height, crown_height = height - crown_start.
    Když výšky chybí, vrátí původní z_center.
    """
    try:
        if height is None or crown_start is None:
            return float(z_center)
        crown_h = float(height) - float(crown_start)
        if not np.isfinite(crown_h) or crown_h <= 0:
            return float(z_center)
        return float(z_center) + APEX_CROWN_HEIGHT_SHIFT_FACTOR * crown_h
    except Exception:
        return float(z_center)


def load_project_json_light(file_path: str, las_dir_override: str = "") -> pd.DataFrame:
    """
    Načte projektový JSON a vrátí DF s minimem pro výpočet světla:
    - id, label
    - apex (x,y) z crownCenter (fallback position)
    - apex z = crownCenterZ + 0.16*(height-crownStartHeight)
    - height, crownStartHeight, crown_height
    - las_file = join(las_dir, f"{label}.las")
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(file_path)

    with open(file_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    project_dir = os.path.dirname(os.path.abspath(file_path))
    las_dir = (las_dir_override.strip() if isinstance(las_dir_override, str) else "")
    if not las_dir:
        las_dir = project_dir

    rows = []
    for seg in data.get("segments", []):
        tid = seg.get("id")
        label = seg.get("label", "")

       # sestavení LAS: <las_dir>/<label>.las
        safe_label = _sanitize_label_for_filename(str(label))
        las_file = os.path.join(las_dir, safe_label + LAS_EXT)

        if _is_excluded_label(label):
            continue

        attrs = seg.get("treeAttributes", {}) or {}
#################
        crown_center = attrs.get("crownCenter")
        pos = attrs.get("position")

        height = attrs.get("height", None)
        crown_start = attrs.get("crownStartHeight", None)

        apex = None
        used_computed_crown = False

        # základní Z (pro výpočet koruny)
        z_base = None
        if isinstance(pos, list) and len(pos) >= 3:
            z_base = float(pos[2])
        elif isinstance(crown_center, list) and len(crown_center) >= 3:
            z_base = float(crown_center[2])

        # 1) crownCenter existuje a není nulový
        if isinstance(crown_center, list) and len(crown_center) >= 3 and not _is_zero_center(crown_center):
            apex = crown_center

        # 2) crownCenter = 0,0,0 → dopočítat
        elif _is_zero_center(crown_center) and z_base is not None and crown_start is not None:
            computed = compute_crown_center_from_las(
                las_path=las_file,
                voxel_size=VOXEL_SIZE,
                z_base=z_base,
                crown_start_height=float(crown_start)
            )
            if computed is not None:
                apex = computed.tolist()
                used_computed_crown = True

        # 3) fallback
        if apex is None and isinstance(pos, list) and len(pos) >= 3:
            apex = pos

###################
        if apex is None or tid is None:
            continue

        height = attrs.get("height", None)
        crown_start = attrs.get("crownStartHeight", None)

        crown_height = None
        try:
            if height is not None and crown_start is not None:
                crown_height = float(height) - float(crown_start)
        except Exception:
            crown_height = None



        # posun apex Z
        z_center = float(apex[2])
        z_apex = _compute_apex_z(z_center, height, crown_start)

        rows.append({
            "id": int(tid),
            "label": str(label),
            "x": float(apex[0]),
            "y": float(apex[1]),
            "z": float(z_apex),
            "height": float(height) if height is not None else np.nan,
            "crownStartHeight": float(crown_start) if crown_start is not None else np.nan,
            "crown_height": float(crown_height) if crown_height is not None else np.nan,
            "las_file": las_file,
            "crownCenter": apex if used_computed_crown else None
        })

    df = pd.DataFrame(rows)
    if df.empty:
        return df

    df["id"] = df["id"].astype(int)
    return df.reset_index(drop=True)

# =====================================================
def compute_light_competition(
    project_json_path: str,
    voxel_size: float,
    las_dir_override: str = "",
    progress_callback=None,
) -> pd.DataFrame:
    """
    Spustí raytracing výpočet světla nad projektovým JSON.

    Parameters
    ----------
    project_json_path : str
        Cesta k projektovému JSON.
    voxel_size : float
        Velikost voxelu [m].
    las_dir_override : str
        Volitelná složka s LAS soubory.
    progress_callback : callable
        Funkce(progress: float, message: str) – pro Streamlit progress bar.

    Returns
    -------
    pd.DataFrame
        Výsledný DataFrame s light_avail a light_comp.
    """

    global VOXEL_SIZE
    VOXEL_SIZE = float(voxel_size)

    df = load_project_json_light(project_json_path, las_dir_override=las_dir_override)

    if df.empty:
        raise RuntimeError("V JSONu nebyly nalezeny validní segmenty.")

    # kontrola LAS
    missing = df[~df["las_file"].apply(os.path.exists)]
    if not missing.empty:
        raise FileNotFoundError(
            "Chybí LAS soubory:\n"
            + "\n".join(missing["las_file"].tolist())
        )

    xs = df["x"].to_numpy(float)
    ys = df["y"].to_numpy(float)
    zs = df["z"].to_numpy(float)
    ids = df["id"].to_numpy(int)
    las_files = df["las_file"].astype(str).to_numpy()

    half_angle = math.radians(APEX_ANGLE_DEG / 2.0)
    axis = np.array([0.0, 0.0, 1.0], dtype=np.float64)

    results_rows = []

    total_trees = len(df)

    for i in range(total_trees):

        if progress_callback:
            progress_callback(i / total_trees, f"{i/total_trees*100:.1f} %")

        tree_id = int(ids[i])
        x, y, z = float(xs[i]), float(ys[i]), float(zs[i])

        dx = xs - x
        dy = ys - y
        r2 = dx * dx + dy * dy
        neighbor_mask = (r2 <= NEIGHBOR_RADIUS ** 2)
        neighbor_mask[i] = False
        neighbor_idx = np.where(neighbor_mask)[0]

        if neighbor_idx.size == 0:
            results_rows.append({
                "id": tree_id,
                OUT_LIGHT_AVAIL_KEY: 100.0,
                OUT_LIGHT_COMP_KEY: {}
            })
            continue

        z_min = Z_MIN
        z_max = z + CONE_HEIGHT + Z_MARGIN_UP

        pts_list = []
        payload_list = []

        for j in neighbor_idx:
            pts_j = laspy_read_points_window(
                las_files[j], x, y, z_min, z_max, NEIGHBOR_RADIUS
            )
            if pts_j.shape[0] == 0:
                continue
            pts_list.append(pts_j)
            payload_list.append(
                np.full(pts_j.shape[0], int(ids[j]), dtype=np.int64)
            )

        if not pts_list:
            results_rows.append({
                "id": tree_id,
                OUT_LIGHT_AVAIL_KEY: 100.0,
                OUT_LIGHT_COMP_KEY: {}
            })
            continue

        pts = np.vstack(pts_list)
        payload_all = np.concatenate(payload_list)

        apex = np.array([x, y, z], dtype=np.float64)
        mask_cone = points_in_inverted_cone(apex, axis, half_angle, CONE_HEIGHT, pts)

        pts_cone = pts[mask_cone]
        payload_cone = payload_all[mask_cone]

        if pts_cone.shape[0] == 0:
            results_rows.append({
                "id": tree_id,
                OUT_LIGHT_AVAIL_KEY: 100.0,
                OUT_LIGHT_COMP_KEY: {}
            })
            continue

        occ_set, occ_payload = voxel_hash(
            pts_cone,
            VOXEL_SIZE,
            extra_payload=payload_cone,
            reducer="mode"
        )

        dirs = sample_directions_in_cone(axis, half_angle, N_RAY_SAMPLES)
        hits = raytrace_voxels(
            apex, dirs, VOXEL_SIZE, CONE_HEIGHT,
            occ_set, occ_payload, t0=RAY_T0
        )

        total = len(hits)
        blocked = sum(1 for h, _ in hits if h)
        free_pct = 100.0 * (total - blocked) / total if total else 100.0

        by_id = Counter()
        for h, pid in hits:
            if h:
                by_id[int(pid) if pid is not None else -1] += 1

        breakdown = {
            ("unknown" if k == -1 else str(k)):
            round(100.0 * v / total, 3)
            for k, v in by_id.items()
        }

        results_rows.append({
            "id": tree_id,
            OUT_LIGHT_AVAIL_KEY: round(free_pct, 3),
            OUT_LIGHT_COMP_KEY: breakdown
        })

    df_res = pd.DataFrame(results_rows)
    df_out = df.merge(df_res, on="id", how="left")

    write_json(project_json_path, df_out)

    if progress_callback:
        progress_callback(1.0, "100 %")

    return df_out



# =============================================================================
# MAIN
# =============================================================================
def main():
    df = load_project_json_light(INPUT_PROJECT_JSON, las_dir_override=LAS_DIR_OVERRIDE)

    if df.empty:
        raise RuntimeError("V JSONu jsem nenašel žádné segmenty s crownCenter/position.")

    # fail-fast kontrola LASů (skládá se jako <las_dir>/<label>.las)
    missing = df[~df["las_file"].apply(lambda p: os.path.exists(p))][["id", "label", "las_file"]]
    if not missing.empty:
        bad = "\n".join([f"- id={r.id}, label={r.label}, las='{r.las_file}'" for r in missing.itertuples(index=False)])
        raise FileNotFoundError(
            "Některé LAS soubory neexistují (skládám cestu jako <las_dir>/<label>.las).\n"
            f"{bad}\n"
            "Zkontroluj label v JSONu (musí odpovídat názvu LAS), nebo nastav LAS_DIR_OVERRIDE."
        )

    xs = df["x"].to_numpy(dtype=float)
    ys = df["y"].to_numpy(dtype=float)
    zs = df["z"].to_numpy(dtype=float)
    ids = df["id"].to_numpy(dtype=int)
    las_files = df["las_file"].astype(str).to_numpy()

    half_angle = math.radians(APEX_ANGLE_DEG / 2.0)
    axis = np.array([0.0, 0.0, 1.0], dtype=np.float64)  # kužel nahoru

    print("[INFO] Běží čtení LAS přes laspy (bez PDAL).")
    print(f"[INFO] Stromů v JSONu: {len(df)}")
    print(f"[INFO] LAS dir: {os.path.dirname(las_files[0]) if len(las_files) else ''}")

    results_rows = []

    for i in range(len(df)):

        print(i)

        tree_id = int(ids[i])
        x = float(xs[i])
        y = float(ys[i])
        z = float(zs[i])

        # sousedi podle vzdálenosti v XY
        dx = xs - x
        dy = ys - y
        r2 = dx * dx + dy * dy
        neighbor_mask = (r2 <= NEIGHBOR_RADIUS * NEIGHBOR_RADIUS)
        neighbor_mask[i] = False

        neighbor_idx = np.where(neighbor_mask)[0]
        if neighbor_idx.size == 0:
            results_rows.append({
                "id": tree_id,
                OUT_LIGHT_AVAIL_KEY: 100.0,
                OUT_LIGHT_COMP_KEY: {}
            })
            continue

        z_min = Z_MIN
        z_max = z + CONE_HEIGHT + Z_MARGIN_UP

        pts_list = []
        payload_list = []

        # Načteme body ze všech sousedů (z jejich <label>.las)
        for j in neighbor_idx:
            las_path = las_files[j]
            neighbor_id = int(ids[j])

            pts_j = laspy_read_points_window(
                las_path=las_path,
                x=x, y=y,
                z_min=z_min, z_max=z_max,
                radius=NEIGHBOR_RADIUS
            )
            if pts_j.shape[0] == 0:
                continue

            pts_list.append(pts_j)
            payload_list.append(np.full(pts_j.shape[0], neighbor_id, dtype=np.int64))

        if not pts_list:
            results_rows.append({
                "id": tree_id,
                OUT_LIGHT_AVAIL_KEY: 100.0,
                OUT_LIGHT_COMP_KEY: {}
            })
            continue

        pts = np.vstack(pts_list).astype(np.float32)
        payload_all = np.concatenate(payload_list).astype(np.int64)

        # omez na kužel
        apex = np.array([x, y, z], dtype=np.float64)
        mask_cone = points_in_inverted_cone(apex, axis, half_angle, CONE_HEIGHT, pts)
        pts_cone = pts[mask_cone]
        payload_cone = payload_all[mask_cone]

        if pts_cone.shape[0] == 0:
            results_rows.append({
                "id": tree_id,
                OUT_LIGHT_AVAIL_KEY: 100.0,
                OUT_LIGHT_COMP_KEY: {}
            })
            continue

        # voxelizace
        occ_set, occ_payload = voxel_hash(pts_cone, VOXEL_SIZE, extra_payload=payload_cone, reducer="mode")

        # ray tracing
        dirs = sample_directions_in_cone(axis, half_angle, N_RAY_SAMPLES)
        hits = raytrace_voxels(apex, dirs, VOXEL_SIZE, CONE_HEIGHT, occ_set, occ_payload, t0=RAY_T0)

        total = len(hits)
        blocked = sum(1 for h, _ in hits if h)
        free = total - blocked
        free_pct = 100.0 * free / total if total > 0 else 100.0

        by_id = Counter()
        for h, pid in hits:
            if h:
                by_id[int(pid) if pid is not None else -1] += 1

        breakdown_pct: Dict[str, float] = {}
        for k, v in by_id.items():
            label = "unknown" if k == -1 else str(k)
            breakdown_pct[label] = round(100.0 * v / total, 3)

        results_rows.append({
            "id": tree_id,
            OUT_LIGHT_AVAIL_KEY: round(free_pct, 3),
            OUT_LIGHT_COMP_KEY: breakdown_pct
        })

    df_res = pd.DataFrame(results_rows)
    df_out = df.merge(df_res, on="id", how="left")

    # zapiš zpět do JSON (treeAttributes)
    df_out = df_out.drop(columns=["las_file"], errors="ignore")
    write_json(INPUT_PROJECT_JSON, df_out, OUTPUT_PROJECT_JSON)
    print(f"[OK] Uloženo: {OUTPUT_PROJECT_JSON}")


if __name__ == "__main__":
    main()
