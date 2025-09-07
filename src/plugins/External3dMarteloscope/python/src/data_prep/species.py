species_map = {
    "BK": "Fagus sylvatica",
    "SM": "Picea abies",
    "MD": "Larix decidua",
    "DB": "Quercus robur",
    "DBZ":"Quercus petraea",
    "HB": "Carpinus betulus",
    "JS": "Fraxinus excelsior",
    "KL": "Acer pseudoplatanus"
}


def map_species_codes2latin(label):
    if isinstance(label, str):
        code = label.split()[0]  # Vezmeme první část (např. 'BK' z 'BK 1')
        return species_map.get(code, "Unknown")
    return "Unknown"