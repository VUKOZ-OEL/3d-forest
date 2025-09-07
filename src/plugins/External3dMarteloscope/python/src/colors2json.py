import json
from pathlib import Path
from typing import Dict, Tuple

__all__ = ["add_palettes"]


def add_palettes(input_path: str, output_path: str, settings_path: str) -> None:
    """
    Načte JSON, přidá palety mgmt_cols a species_cols v RGB (list) a uloží nový soubor.

    Args:
        input_path: cesta k vstupnímu JSON souboru
        output_path: cesta k výstupnímu JSON souboru
    """
    input_file = Path(input_path)
    output_file = Path(output_path)
    settings_file = Path(settings_path)

    # načtení JSONu
    with input_file.open("r", encoding="utf-8") as f:
        data = json.load(f)

    with settings_file.open("r", encoding="utf-8") as fs:
        cols = json.load(fs)

    # přidání do dat
    data["species_colors"] = cols["tree_species"]
    data["managementStatus"] = cols["managementStatus"]

    # uložení
    with output_file.open("w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

    print(f"Palety přidány do {output_file}")
