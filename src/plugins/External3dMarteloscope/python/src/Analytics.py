import streamlit as st
import pandas as pd
import src.io_utils as iou
from pygwalker.api.streamlit import StreamlitRenderer

# Import your data

with st.expander("### Interaktivní tvorba uživatelských grafů"):
    st.markdown("""
        #### Orientace v okně (co kam táhnout)
        Fields (sloupce) – app rozliší Dimension (kategorie, datum) vs Measure (číselné metriky), jde to ale měnit.
        X / Y – vodorovná a svislá osa.
        Color / Size / Shape – vizuální kódování další proměnné.
        Row / Column – facety (mřížka grafů) podle kategorií.
        Mark (typ grafu) – sloupcový, čárový, bodový, histogram, heatmapa…

        Aggregate – souhrny (Count, Sum, Avg, Min/Max…).
        Filters – výběr subsetu dat (práh, seznam hodnot, rozsahy).
        Data Table – náhled tabulky, rychlé profilování a změny typů.

        #### Rychlé recepty (tažení-a-pouštění)
        Sloupcový graf: počty podle kategorie
        Přetáhni kategorii na X, Count (nebo jinou metriku) na Y → „Bar“.

        Časová řada
        Přetáhni datum/čas na X, metriku na Y → „Line“.

        Histogram / Heatmapa
        Jedna číselná proměnná na X → „Histogram“. Dvě na X a Y → „Heatmap“.

        Bodový graf se třemi proměnnými
        X = metrika1, Y = metrika2, Color = kategorie (nebo Size = metrika3).
        Facety (mřížka)
        Táhni kategorii do Row nebo Column → jeden graf na každou hodnotu.
        Top N
        Seřaď Y a ve filtru nech „First N rows“ (nebo vyber konkrétní hodnoty).

        #### Uložení & sdílení
        Uložení konfigurace grafů (aby ses k nim vrátil později): parametr spec="moje_nastaveni.json" – stav uložíš z UI tlačítkem Save a příště načteš automaticky. 
        Export do HTML (samostatná stránka pro sdílení): HTML export přes API (např. to_html) – výsledek můžeš poslat kolegům. 

     """)

if "trees" not in st.session_state:
    file_path = ("c:/Users/krucek/OneDrive - vukoz.cz/DATA/_GS-LCR/SLP_Pokojna/PokojnaHora_3df/PokojnaHora.json")
    st.session_state.trees = iou.load_project_json(file_path)

df: pd.DataFrame = st.session_state.trees.copy()

# --- Vyber jen "ploché" sloupce (žádné list/dict/tuple/set) ---
def is_nested(val):
    return isinstance(val, (list, dict, tuple, set))

flat_columns = []
for col in df.columns:
    # bezpečně přeskoč NaN/None
    has_nested = df[col].dropna().apply(is_nested).any() if col in df else False
    if not has_nested:
        flat_columns.append(col)

# zajisti přítomnost id a management_status
must_have = [c for c in ["id", "management_status"] if c in df.columns and c not in flat_columns]
display_columns = flat_columns + must_have
df_display = df[display_columns].copy()
 
pyg_app = StreamlitRenderer(df_display,appearance="light")
 
pyg_app.explorer()