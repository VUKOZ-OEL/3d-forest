import sqlite3
import pandas as pd


def ensure_usr_table(conn: sqlite3.Connection):
    conn.execute(
        """
        CREATE TABLE IF NOT EXISTS tree_usr_attr (
            tree_id INTEGER NOT NULL,
            attribute TEXT NOT NULL,
            value REAL NOT NULL,
            PRIMARY KEY (tree_id, attribute),
            FOREIGN KEY (tree_id) REFERENCES tree(id)
        )
        """
    )

# -------------------------------------------------
# DB helpers
# -------------------------------------------------
def get_tree_ids(sqlite_path: str) -> set[int]:
    with sqlite3.connect(sqlite_path) as conn:
        df = pd.read_sql("SELECT id FROM tree", conn)
    return set(df["id"].astype(int))


def get_existing_usr_attributes(sqlite_path: str) -> list[str]:
    with sqlite3.connect(sqlite_path) as conn:
        ensure_usr_table(conn)

        df = pd.read_sql(
            "SELECT DISTINCT attribute FROM tree_usr_attr ORDER BY attribute",
            conn,
        )

    return df["attribute"].tolist()


# -------------------------------------------------
# CSV validation
# -------------------------------------------------
def validate_csv(csv_df: pd.DataFrame, tree_ids: set[int]) -> dict:
    if "id" not in csv_df.columns:
        return {"ok": False, "error": "CSV must contain column 'id'"}

    # validate id
    ids = pd.to_numeric(csv_df["id"], errors="coerce")
    if ids.isna().any():
        return {"ok": False, "error": "Column 'id' must contain only numbers"}

    csv_ids = set(ids.astype(int))
    if csv_ids != tree_ids:
        return {
            "ok": False,
            "error": "CSV ids must exactly match project tree ids",
        }

    # attributes
    attributes = [c for c in csv_df.columns if c != "id"]
    if not attributes:
        return {"ok": False, "error": "No attributes found to import"}

    # numeric validation
    for col in attributes:
        values = pd.to_numeric(csv_df[col], errors="coerce")
        if values.isna().any():
            return {
                "ok": False,
                "error": f"Column '{col}' must contain only numeric values",
            }

    return {
        "ok": True,
        "attributes": attributes,
    }


# -------------------------------------------------
# Overwrite detection
# -------------------------------------------------
def detect_overwrite_attributes(
    new_attrs: list[str], existing_attrs: list[str]
) -> list[str]:
    return sorted(set(new_attrs) & set(existing_attrs))


# -------------------------------------------------
# INSERT / DELETE
# -------------------------------------------------
def insert_usr_attributes(sqlite_path: str, csv_df: pd.DataFrame):
    with sqlite3.connect(sqlite_path) as conn:
        ensure_usr_table(conn)
        cur = conn.cursor()

        for _, row in csv_df.iterrows():
            tree_id = int(row["id"])
            for col in csv_df.columns:
                if col == "id":
                    continue
                cur.execute(
                    """
                    INSERT OR REPLACE INTO tree_usr_attr
                    (tree_id, attribute, value)
                    VALUES (?, ?, ?)
                    """,
                    (tree_id, col, float(row[col])),
                )

        conn.commit()


def delete_usr_attributes(sqlite_path: str, attributes: list[str]):
    placeholders = ",".join("?" * len(attributes))
    with sqlite3.connect(sqlite_path) as conn:
        conn.execute(
            f"""
            DELETE FROM tree_usr_attr
            WHERE attribute IN ({placeholders})
            """,
            attributes,
        )
        conn.commit()


# -------------------------------------------------
# LOAD + PIVOT
# -------------------------------------------------
def load_usr_attributes_wide(sqlite_path: str) -> pd.DataFrame:
    with sqlite3.connect(sqlite_path) as conn:
        ensure_usr_table(conn)

        df = pd.read_sql(
            "SELECT tree_id, attribute, value FROM tree_usr_attr",
            conn,
        )

    if df.empty:
        return pd.DataFrame(columns=["id"])

    wide = (
        df.pivot(index="tree_id", columns="attribute", values="value")
        .reset_index()
        .rename(columns={"tree_id": "id"})
    )

    return wide
