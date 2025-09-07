import pandas as pd
import json
import src.io as io
import src.data_prep.species as spp
import src.colors2json as c2j

def write_json(original_path: str, df: pd.DataFrame, output_path: str = None) -> None:
    import json

    with open(original_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    id_map = df.set_index("id").to_dict("index")

    for segment in data.get("segments", []):
        sid = segment.get("id")
        if sid in id_map and "treeAttributes" in segment:
            row = id_map[sid]
            attr = segment["treeAttributes"]

            z = 0.0
            if isinstance(attr.get("position"), list) and len(attr["position"]) >= 3:
                z = attr["position"][2]

            if "x" in row and "y" in row:
                attr["position"] = [
                    float(row["x"]) ,
                    float(row["y"]) ,
                    z
                ]

            # Vyloučíme klíče, které už existují i ve vnějším segmentu
            skip_keys = {"id"}
            outer_keys = set(segment.keys())  # např. "label" na úrovni segmentu
            for key, value in row.items():
                if key not in skip_keys and key not in outer_keys:
                    attr[key] = value  # bezpečné přepsání nebo přidání

    output_path = output_path or original_path
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)


file_path = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v4.json"
out_file = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v10.json"
settings_file = "C:/Users/krucek/Documents/GitHub/3d-marteloscope/settings_with_colors.json"




trees = io.load_project_json(file_path)

trees


trees["dbh"] = trees["dbh_raycl"] * 100

trees["management_status"] = "Untouched"
trees.loc[trees.index[1:23], "management_status"] = "Target tree"
trees.loc[trees.index[25:100], "management_status"] = "Competition"
trees.loc[trees.index[101:123], "management_status"] = "Maturity"
trees.loc[trees.index[124:223], "management_status"] = "Sanitary"
trees.loc[trees.index[300:353], "management_status"] = "Promote rare species"
trees.loc[trees.index[354:400], "management_status"] = "Promote regeneration"

trees.to_feather("c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v10.feather")

trees

write_json(file_path,trees,out_file)


file_path2 = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v10.json"
out_file2 = "c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/_PokojnaHora_v11.json"
settings_file = "C:/Users/krucek/Documents/GitHub/3d-marteloscope/settings_with_colors.json"
c2j.add_palettes(file_path2,out_file2,settings_file)





