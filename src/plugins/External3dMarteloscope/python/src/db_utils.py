# db_utils.py
from __future__ import annotations

import os
import sqlite3
from typing import Dict, Iterable, Optional, Tuple

import pandas as pd


# -----------------------------
# Path helpers
# -----------------------------
def get_sqlite_path_from_session(session_state) -> str:
    """
    Najde cestu k *.sqlite pro projekt.
    Zkouší několik typických klíčů v session_state.
    """
    candidates = [
        "project_file",
        "project_path",
        "project_file_path",
        "json_path",
        "file_path",
    ]
    json_path = None
    for k in candidates:
        if k in session_state and session_state[k]:
            json_path = session_state[k]
            break
    if not json_path:
        raise ValueError(
            "Project JSON path not found in st.session_state. "
            "Set st.session_state.project_json_path (or project_path)."
        )
    return os.path.splitext(str(json_path))[0] + ".sqlite"


# -----------------------------
# Schema
# -----------------------------
def ensure_mgmt_tables(sqlite_path: str) -> None:
    conn = sqlite3.connect(sqlite_path)
    try:
        conn.execute("PRAGMA foreign_keys=ON;")

        conn.execute(
            """
            CREATE TABLE IF NOT EXISTS managements (
              mgmt_id INTEGER PRIMARY KEY AUTOINCREMENT,
              label   TEXT NOT NULL
            );
            """
        )

        conn.execute(
            """
            CREATE TABLE IF NOT EXISTS management_values (
              tree_id INTEGER NOT NULL,
              mgmt_id INTEGER NOT NULL,
              value   TEXT,
              PRIMARY KEY (tree_id, mgmt_id),
              FOREIGN KEY (mgmt_id) REFERENCES managements(mgmt_id) ON DELETE CASCADE
            );
            """
        )

        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_management_values_mgmt ON management_values(mgmt_id);"
        )
        conn.execute(
            "CREATE INDEX IF NOT EXISTS idx_management_values_tree ON management_values(tree_id);"
        )

        conn.commit()
    finally:
        conn.close()


# -----------------------------
# Reads
# -----------------------------
def list_managements(sqlite_path: str) -> pd.DataFrame:
    """
    Vrátí DataFrame: mgmt_id, label (seřazeno od nejnovějších).
    """
    ensure_mgmt_tables(sqlite_path)

    conn = sqlite3.connect(sqlite_path)
    try:
        df = pd.read_sql("SELECT mgmt_id, label FROM managements ORDER BY mgmt_id DESC;", conn)
        return df
    finally:
        conn.close()


def load_management_map(sqlite_path: str, mgmt_id: int) -> Dict[int, Optional[str]]:
    """
    Vrátí dict: tree_id -> value pro daný mgmt_id.
    """
    ensure_mgmt_tables(sqlite_path)

    conn = sqlite3.connect(sqlite_path)
    try:
        df = pd.read_sql(
            "SELECT tree_id, value FROM management_values WHERE mgmt_id = ?;",
            conn,
            params=(int(mgmt_id),),
        )
        if df.empty:
            return {}
        df["tree_id"] = pd.to_numeric(df["tree_id"], errors="coerce").astype("Int64")
        df = df.dropna(subset=["tree_id"])
        df["tree_id"] = df["tree_id"].astype(int)
        return dict(zip(df["tree_id"].tolist(), df["value"].tolist()))
    finally:
        conn.close()


# -----------------------------
# Writes
# -----------------------------
def create_management(sqlite_path: str, label: str) -> int:
    """
    Vytvoří nový zásah/scénář a vrátí mgmt_id.
    """
    ensure_mgmt_tables(sqlite_path)

    conn = sqlite3.connect(sqlite_path)
    try:
        conn.execute("PRAGMA foreign_keys=ON;")
        cur = conn.execute("INSERT INTO managements(label) VALUES (?);", (str(label),))
        conn.commit()
        return int(cur.lastrowid)
    finally:
        conn.close()


def bulk_upsert_management_values(
    sqlite_path: str,
    mgmt_id: int,
    rows: Iterable[Tuple[int, Optional[str]]],
) -> None:
    """
    Bulk UPSERT do pivot tabulky.
    rows: iterable (tree_id, value)
    """
    ensure_mgmt_tables(sqlite_path)

    conn = sqlite3.connect(sqlite_path)
    try:
        conn.execute("PRAGMA foreign_keys=ON;")
        conn.executemany(
            """
            INSERT INTO management_values(tree_id, mgmt_id, value)
            VALUES (?, ?, ?)
            ON CONFLICT(tree_id, mgmt_id) DO UPDATE SET value = excluded.value;
            """,
            [(int(tree_id), int(mgmt_id), (None if value is None else str(value))) for tree_id, value in rows],
        )
        conn.commit()
    finally:
        conn.close()


def save_management_from_trees(
    sqlite_path: str,
    trees: pd.DataFrame,
    label: str,
    id_col: str = "id",
    value_col: str = "management_status",
    only_non_empty: bool = True,
) -> int:
    """
    Uloží aktuální user zásah z trees[value_col] jako nový scénář:
    - vytvoří managements řádek (label)
    - uloží pivot hodnoty (tree_id -> value)
    Vrátí new mgmt_id.
    """
    if id_col not in trees.columns:
        raise ValueError(f"trees missing column '{id_col}'")
    if value_col not in trees.columns:
        raise ValueError(f"trees missing column '{value_col}'")

    mgmt_id = create_management(sqlite_path, label=label)

    df = trees[[id_col, value_col]].copy()
    df[id_col] = pd.to_numeric(df[id_col], errors="coerce").astype("Int64")
    df = df.dropna(subset=[id_col]).copy()
    df[id_col] = df[id_col].astype(int)

    if only_non_empty:
        df[value_col] = df[value_col].astype("string")
        df = df[df[value_col].notna() & (df[value_col].str.len() > 0)]

    rows = list(zip(df[id_col].tolist(), df[value_col].tolist()))
    bulk_upsert_management_values(sqlite_path, mgmt_id=mgmt_id, rows=rows)

    return mgmt_id


def delete_management(sqlite_path: str, mgmt_id: int) -> None:
    """
    Smaže scénář + díky CASCADE i jeho pivot hodnoty.
    """
    ensure_mgmt_tables(sqlite_path)

    conn = sqlite3.connect(sqlite_path)
    try:
        conn.execute("PRAGMA foreign_keys=ON;")
        conn.execute("DELETE FROM managements WHERE mgmt_id = ?;", (int(mgmt_id),))
        conn.commit()
    finally:
        conn.close()
