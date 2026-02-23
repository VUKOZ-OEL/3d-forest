# help_dict.py
from typing import Literal


Lang = Literal["cs", "en"]

HELP_I18N: dict[str, dict[Lang, str]] = {

    "dashboard_help": {
        "cs": """
#### Info & ovládání

Tato stránka slouží jako úvodní a řídicí rozhraní modulu 3D-Marteloscop.
Není to analytická stránka v užším smyslu, ale centrální místo, odkud
uživatel pracuje s projektem jako celkem a řídí průběh vyhodnocení
simulovaného pěstebního zásahu.

#### Přehled plochy a zásahu

V levé části jsou zobrazeny základní informace o hodnocené ploše a
aktuálním stavu simulovaného zásahu: identifikace plochy, popis porostu
a stav projektu z hlediska načtených dat a aktivního scénáře.
Tyto hodnoty se nemění přepínáním zásahů – slouží jako referenční kontext.

#### Typy zásahů

Zásah definuje pěstební volbu přiřazenou každému stromu (cílový strom,
ponechaný strom, odstraněný strom apod.).

- **Uživatelský zásah** – aktuálně editovaný stav, automaticky cachovaný
- **Uložené zásahy** – dříve uložené scénáře v projektové databázi SQLite

Mezi zásahy lze přepínat pomocí rozbalovacího seznamu. Všechny změny
se okamžitě promítají do ostatních analytických stránek modulu.

#### Správa zásahů

Stránka umožňuje:
- aktivovat demonstrační nebo referenční scénář zásahu
- uložit aktuální uživatelský zásah pod zvoleným názvem do projektové databáze
- uložený zásah znovu načíst nebo trvale smazat
- vymazat uživatelský zásah a vrátit se do čistého stavu

Smazání uloženého zásahu je nevratné.

#### Export výsledků

Tlačítkem **Exportovat výsledky** se vygeneruje souhrnný report
obsahující přehled plochy, aktivního zásahu a grafické výstupy.
Jazyk reportu odpovídá zvolenému jazyku aplikace.
Před exportem zkontrolujte, který zásah je aktivní.

#### Pěstební využití

Přepínání mezi variantami zásahu umožňuje jejich systematické porovnání.
Lze tak rozlišit varianty se stejnou celkovou intenzitou, ale odlišným
dopadem na prostorovou strukturu porostu, světelné podmínky nebo
stabilitu cílových stromů. Tento nástroj podporuje transparentní,
datově podložené pěstební rozhodování.
""",
        "en": """
#### Info & Management

This page serves as the introductory and control interface of the
3D-Marteloscop module. It is not an analytical page in the narrow sense,
but the central point from which the user manages the project as a whole
and controls the evaluation of the simulated silvicultural intervention.

#### Stand and intervention overview

The left panel shows basic information about the evaluated plot and the
current state of the simulated intervention: plot identification, stand
description, and project status in terms of loaded data and active scenario.
These values do not change when switching between interventions – they serve
as a reference context.

#### Intervention types

An intervention defines the silvicultural assignment of each tree (target tree,
retained tree, removed tree, etc.).

- **User intervention** – currently edited state, automatically cached
- **Saved interventions** – previously stored scenarios in the project SQLite database

Interventions can be switched using the dropdown menu. All changes are
immediately reflected in the other analytical pages of the module.

#### Intervention management

This page allows you to:
- activate a demonstration or reference intervention scenario
- save the current user intervention under a chosen name to the project database
- reload or permanently delete a saved intervention
- clear the user intervention to return to a clean state

Deleting a saved intervention is irreversible.

#### Export results

The **Export results** button generates a summary report containing the plot
overview, active intervention, and graphical outputs.
The report language matches the selected application language.
Always verify which intervention is active before exporting.

#### Silvicultural use

Switching between intervention variants enables systematic comparison.
This allows distinguishing variants with the same overall intensity but
different effects on stand spatial structure, light conditions, or stability
of target trees. This tool supports transparent, data-driven silvicultural
decision-making.
""",
    },

    "summary_help": {
        "cs": """
#### Souhrn

Stránka poskytuje základní souhrnné vyhodnocení porostu a simulovaného
pěstebního zásahu z hlediska tradičních porostních charakteristik. Je to
výchozí analytická stránka, která shrnuje klíčové parametry ve stavu
před a po zásahu a umožňuje jejich přímé porovnání.

#### Zobrazované charakteristiky

Výsledky jsou vyjádřeny na hektar a zahrnují:
- počet stromů
- objem hroubí
- výčetní kruhovou základnu
- zápoj porostu
- zakmenění (pokud je relevantní pro danou plochu)

Charakteristiky jsou členěny podle dřevin a rozděleny do tloušťkových
a výškových tříd.

#### Ovládání

Uživatel volí:
- sledovanou charakteristiku (počet stromů, objem, základna, zápoj, zakmenění)
- zobrazení pro stav před zásahem, po zásahu nebo pro odstraněné stromy
- barevné rozlišení podle dřevin nebo volby zásahu

Jednotlivé kategorie lze interaktivně skrývat prostřednictvím legendy.
Veškeré změny nastavení se okamžitě promítají do grafů.

#### Typy grafů

Koláčový graf znázorňuje celkové složení porostu. U zápojem a zakmenění
je dopočten prázdný podíl do 100 %. Sloupcové grafy zobrazují rozdělení
do tloušťkových a výškových tříd jako skládané sloupce.

#### Pěstební využití

Stránka odpovídá tradičnímu lesnickému vyhodnocení prováděnému v
provozní praxi. Umožňuje rychle ověřit, zda zásah odpovídá zamýšlenému
typu výchovy, zkontrolovat rozsah zásahu z hlediska zachování stability
porostu a posoudit změnu druhové skladby. Souhrn poskytuje referenční
rámec pro interpretaci všech dalších analytických výstupů modulu.
""",
        "en": """
#### Summary

This page provides a basic summary evaluation of the stand and the
simulated silvicultural intervention in terms of traditional stand
characteristics. It is the primary analytical page, summarising key
parameters in the state before and after intervention for direct comparison.

#### Displayed characteristics

Results are expressed per hectare and include:
- tree count
- stem volume
- basal area
- canopy cover
- stocking (if relevant for the given plot)

Characteristics are broken down by species and divided into diameter
and height classes.

#### Controls

The user selects:
- the characteristic to display (tree count, volume, basal area, canopy cover, stocking)
- the stand state: before intervention, after intervention, or removed trees
- colour coding by species or management category

Individual categories can be interactively hidden via the legend.
All setting changes are immediately reflected in the charts.

#### Chart types

The pie chart shows overall stand composition. For canopy cover and stocking,
the remaining share to 100 % is added as an empty segment. Bar charts display
the distribution across diameter and height classes as stacked bars.

#### Silvicultural use

The page corresponds to the traditional silvicultural evaluation used in
operational practice. It allows quick verification of whether the intervention
matches the intended tending type, checking the scope of the intervention in
terms of maintaining stand stability, and assessing changes in species
composition. The summary provides the reference frame for interpreting all
other analytical outputs of the module.
""",
    },

    "intensity_help": {
        "cs": """
#### Intenzita zásahu

Stránka zobrazuje intenzitu simulovaného pěstebního zásahu vyjádřenou
pomocí několika komplementárních porostních a korunových charakteristik.
Na rozdíl od souhrnného přehledu je zde intenzita interpretována
z hlediska skutečného biologického účinku zásahu, nikoli pouze jako
bilanční údaj.

#### Zobrazované charakteristiky

Intenzita zásahu je hodnocena podle:
- počtu stromů
- výčetní kruhové základny
- objemu hroubí
- objemu korun

Výsledky jsou vyjádřeny v procentech a prezentovány jako skládané grafy.

#### Ovládání

Uživatel volí:
- charakteristiku, podle které je intenzita počítána
- rozlišení výsledků podle dřevin nebo podle volby zásahu
- filtraci stromů podle výčetní tloušťky a výšky

Při rozlišení podle dřevin zobrazuje osa x celkovou intenzitu výběru
pro danou dřevinu, barevné plochy vyjadřují podíl jednotlivých kategorií
zásahu. Při rozlišení podle volby zásahu jsou na ose x vyneseny sumární
hodnoty pro každou kategorii, barevně rozdělené podle dřevin.

#### Typy grafů

**Intenzita z celku** – podíl odstraněné části porostu vůči celkovému
stavu před zásahem. Řádek Suma vyjadřuje celkovou intenzitu zásahu.

**Intenzita uvnitř skupiny** – jaký podíl každé skupiny byl odstraněn.
Referenční hodnotou je součet hodnot dané skupiny před zásahem.
Umožňuje posoudit selektivitu zásahu vůči konkrétní dřevině nebo kategorii.

#### Pěstební využití

Hodnocení intenzity podle objemu korun umožňuje lépe vystihnout, do jaké
míry byl porost skutečně uvolněn z hlediska růstového prostoru. Tento
přístup je obzvláště významný u porostů obhospodařovaných přírodě blízkým
způsobem, kde cílem zásahu není primárně snížení zásoby, ale regulace
konkurenčních vztahů a světelných podmínek. Stránka slouží ke kontrole
souladu zásahu s modelem hospodaření a k optimalizaci výběru cílových stromů.
""",
        "en": """
#### Intervention Intensity

This page displays the intensity of the simulated silvicultural intervention
using several complementary stand and crown characteristics. Unlike the
summary overview, intensity is interpreted here in terms of the actual
biological effect of the intervention, not merely as an accounting figure.

#### Displayed characteristics

Intervention intensity is assessed by:
- tree count
- basal area
- stem volume
- crown volume

Results are expressed in percent and presented as stacked bar charts.

#### Controls

The user selects:
- the characteristic used to compute intensity
- breakdown by species or by management category
- filtering of trees by diameter and height

When broken down by species, the x-axis shows the total selection intensity
for that species, with coloured areas representing the share of each management
category. When broken down by management category, the x-axis shows summary
values per category, colour-coded by species.

#### Chart types

**Removal from total** – the share of the removed portion of the stand relative
to the full state before intervention. The Sum row expresses overall intervention
intensity.

**Intensity within group** – the proportion removed within each group.
The reference value is the group total before intervention.
Allows assessing the selectivity of the intervention for a given species or category.

#### Silvicultural use

Assessing intensity by crown volume better captures the degree to which the stand
was actually released in terms of growing space. This approach is especially
relevant in close-to-nature managed stands, where the aim is not primarily to
reduce growing stock but to regulate competitive relationships and light conditions.
The page serves to verify alignment with the management model and to optimise
target tree selection.
""",
    },

    "tree_stats_help": {
        "cs": """
#### Statistiky stromů

Stránka poskytuje detailní statistický přehled atributů jednotlivých stromů
a jejich rozdělení v porostu. Zobrazení je zaměřeno na porovnání ponechaných
a odstraněných jedinců a na posouzení rozdílů mezi dřevinami a pěstebními
kategoriemi.

#### Zobrazované charakteristiky

Stránka umožňuje analyzovat:
- počet stromů
- výčetní tloušťku a kruhovou základnu
- objem hroubí s kůrou
- výšku stromu a výšku těžiště koruny
- objem a povrch koruny
- plochu vertikální a horizontální projekce koruny
- excentricitu koruny, štíhlostní koeficient
- dostupnost světla (zápoj stromu)

#### Ovládání

Uživatel volí zobrazovanou charakteristiku a způsob třídění výsledků:
- podle dřeviny nebo volby zásahu
- podle tloušťkových tříd (DBH)
- podle výškových tříd

Třídění do tříd je dostupné pouze pro počet stromů. Filtrace stromů je
možná podle dřeviny, tloušťkové a výškové třídy.

#### Typy grafů

Počet stromů je zobrazován jako skládané sloupcové grafy pro stav před
zásahem, po zásahu a pro odstraněné stromy. Ostatní charakteristiky jsou
zobrazeny jako houslové grafy, které znázorňují rozdělení hodnot – šířka
houslí odpovídá hustotě dat, box ukazuje medián a kvartily.

#### Pěstební využití

Stránka umožňuje posoudit, zda byly odstraňovány stromy tvarově nebo
zdravotně nežádoucí, zda byly ponechány perspektivní cílové stromy a zda
zásah směřuje k požadované tloušťkové a výškové struktuře. Analýza korunových
charakteristik je zvláště důležitá pro hodnocení ekofyziologické účinnosti
zásahu – porovnání objemu korun ponechaných a odstraněných jedinců ukazuje,
zda byl korunový prostor cílových stromů skutečně uvolněn.
""",
        "en": """
#### Tree Statistics

This page provides a detailed statistical overview of individual tree attributes
and their distribution within the stand. The focus is on comparing retained and
removed trees and assessing differences between species and management categories.

#### Displayed characteristics

The page allows analysis of:
- tree count
- diameter at breast height (DBH) and basal area
- stem volume over bark
- tree height and crown centre-of-gravity height
- crown volume and crown surface area
- vertical and horizontal crown projection area
- crown eccentricity, slenderness coefficient
- light availability (crown projection exposure)

#### Controls

The user selects the characteristic to display and the grouping method:
- by species or management category
- by diameter classes (DBH)
- by height classes

Class-based grouping is available only for tree count. Filtering is possible
by species, diameter class, and height class.

#### Chart types

Tree count is displayed as stacked bar charts for the state before intervention,
after intervention, and for removed trees. All other characteristics use violin plots,
showing the distribution of values – violin width reflects data density, the box
shows median and quartiles.

#### Silvicultural use

The page enables assessment of whether removed trees were those with poor form
or health, whether perspective target trees were retained, and whether the
intervention moves the stand towards the desired diameter and height structure.
Analysis of crown characteristics is especially important for assessing the
ecophysiological effectiveness – comparing crown volumes of retained and removed
trees shows whether growing space for target trees was genuinely released.
""",
    },

    "map_help": {
        "cs": """
#### Mapa plochy

Stránka zobrazuje půdorysnou stromovou mapu porostu, ve které je každý strom
reprezentován symbolem v poloze paty kmene. Mapové zobrazení poskytuje přehled
o prostorovém rozmístění jednotlivých jedinců a o jejich výběru v rámci
simulovaného zásahu.

#### Vizuální kódování

Stromy jsou v mapě rozlišeny pomocí:
- barevného kódování podle dřeviny nebo volby zásahu
- velikosti symbolu odpovídající zvolené stromové charakteristice
  (výčetní tloušťka, výška, objem, plocha koruny aj.)

Mapa může být doplněna o projekce korun stromů. Čtvercová síť 10 × 10 m
slouží k prostorové orientaci. Atributy stromů se zobrazují v popisku
po najetí myší.

#### Ovládání

Uživatel nastavuje:
- zobrazení stavu porostu před zásahem, po zásahu nebo odstraněných stromů
- barevné rozlišení stromů (dřevina, volba zásahu)
- atribut škálující velikost bodů a jeho rozsah (min / max)
- filtraci stromů podle dřevin, tloušťkových a výškových tříd
- zobrazení projekcí korun a jejich barevné rozlišení nezávisle na bodech
- textové popisky stromů (ID)

Mapa podporuje přiblížení, posun, fullscreen a uložení do formátu JPG.
Poměr os je fixní pro zachování skutečných vzdáleností.

#### Pěstební využití

Stránka je základním nástrojem pro prostorovou orientaci v porostu
a kontrolu prostorové logiky zásahu. Umožňuje posoudit:
- rovnoměrnost prostorového rozmístění zásahu v ploše
- distribuci a dostatečné uvolnění cílových stromů
- zachování skupin (hnízdní struktury) nebo vytvoření porostních mezer
- soulad zásahu s terénními a porostními podmínkami

Mapa propojuje individuální pěstební rozhodnutí s celkovou strukturou
porostu a ověřuje, že zásah je logický nejen statisticky, ale i prostorově.
""",
        "en": """
#### Stand Map

This page displays a plan-view tree map of the stand, with each tree represented
by a symbol at the base of the stem. The map provides an overview of the spatial
distribution of individual trees and their selection within the simulated intervention.

#### Visual encoding

Trees are distinguished by:
- colour coding by species or management category
- symbol size corresponding to a selected tree attribute
  (DBH, height, volume, crown area, etc.)

The map can be supplemented with crown projections. A 10 × 10 m grid
aids spatial orientation. Tree attributes are shown in a tooltip on hover.

#### Controls

The user configures:
- display of stand state: before intervention, after intervention, or removed trees
- colour coding (species, management category)
- the attribute scaling point size and its range (min / max)
- filtering by species, diameter class, and height class
- crown projection display and its colour coding independently of points
- tree labels (ID)

The map supports zoom, pan, fullscreen, and export to JPG format.
The axis ratio is fixed to preserve true distances.

#### Silvicultural use

The page is the primary tool for spatial orientation within the stand and for
verifying the spatial logic of the intervention. It enables assessment of:
- spatial uniformity of the intervention across the plot
- distribution and adequate release of target trees
- retention of tree clusters (nest structures) or creation of stand gaps
- consistency of the intervention with terrain and stand conditions

The map links individual silvicultural decisions to the overall stand structure
and verifies that the intervention is spatially coherent, not only statistically valid.
""",
    },

    "heatmap_help": {
        "cs": """
#### Heatmapy

Stránka zobrazuje prostorové rozložení vybraných charakteristik porostu
ve formě tematických rastrových map. Porost je v půdorysném pohledu
rozdělen na pravidelnou síť s rozlišením 1 m, každé buňce je přiřazena
hodnota odvozená z atributů stromů v jejím okolí. Mapy jsou zobrazeny
pro stav před zásahem, po zásahu a pro odstraněné stromy.

#### Zobrazované charakteristiky

Stránka umožňuje vizualizovat zejména:
- výčetní tloušťku a kruhovou základnu
- objem a výšku stromů
- výšku těžiště koruny
- objem, povrch a projekci koruny
- excentricitu koruny a štíhlostní koeficient

Barevná škála je společná pro všechny tři panely, což umožňuje přímé
porovnání prostorového účinku zásahu.

#### Ovládání

Uživatel volí zobrazovanou charakteristiku a může nastavovat:
- filtraci stromů podle dřevin a tloušťkových a výškových tříd
- překrytí rastru skutečnými polohami stromů obarvenými dle dřeviny nebo volby zásahu
- filtry specifické pro heatmapy: výběr dřevin a kategorií zásahu
  (tyto filtry se vztahují pouze na výpočet rastru, nikoli na překryvné body)

Mapové zobrazení podporuje plynulé přiblížení a posun pro přechod
mezi celkovým pohledem a detailní analýzou části plochy.

#### Pěstební využití

Heatmapy umožňují hodnotit horizontální prostorovou strukturu porostu
a její změnu v důsledku zásahu. Na rozdíl od souhrnných ukazatelů
poskytují informaci o tom, kde v ploše k těmto změnám dochází. Lze tak:
- identifikovat místa s intenzivním nebo naopak velmi slabým zásahem
- posoudit vznik a distribuci porostních mezer
- hodnotit mozaikovitost textury porostu žádoucí u přírodě blízkého hospodaření
- posoudit míru korunového uvolnění z hlediska objemu a plochy korun

Zobrazení korunových charakteristik je zvláště cenné – mapy objemu a plochy
korun vyjadřují funkční uvolnění korunového prostoru, nikoli pouze množství
odstraněné dřevní hmoty.
""",
        "en": """
#### Heatmaps

This page displays the spatial distribution of selected stand characteristics
as thematic raster maps. The stand is divided into a regular 1 m grid in plan
view, with each cell assigned a value derived from the attributes of nearby trees.
Maps are shown for the state before intervention, after intervention, and for
removed trees.

#### Displayed characteristics

The page enables visualisation of:
- DBH and basal area
- stem volume and tree height
- crown centre-of-gravity height
- crown volume, crown surface area, and crown projection
- crown eccentricity and slenderness coefficient

A shared colour scale is used for all three panels, enabling direct comparison
of the spatial effect of the intervention.

#### Controls

The user selects the characteristic to display and can configure:
- filtering by species, diameter class, and height class
- overlay of actual tree positions coloured by species or management category
- heatmap-specific filters: species selection and management category selection
  (these filters apply only to the raster computation, not to the overlaid points)

The map view supports smooth zoom and pan for transitioning between the overall
plot view and detailed analysis of specific areas.

#### Silvicultural use

Heatmaps allow assessment of the horizontal spatial structure of the stand and
how it changes as a result of the intervention. Unlike summary indicators, they
show where in the plot these changes occur. This enables:
- identification of areas with intensive or minimal intervention
- assessment of gap formation and its spatial distribution
- evaluation of stand texture mosaicity desirable in close-to-nature management
- assessment of crown release in terms of crown volume and projection area

Displaying crown characteristics is especially valuable – maps of crown volume
and projection area express functional crown space release, not merely the
quantity of removed timber.
""",
    },

    "canopy_help": {
        "cs": """
#### Vyplnění prostoru

Stránka zobrazuje vertikální rozložení objemu korun v porostu a jeho změnu
v důsledku simulovaného pěstebního zásahu. Základním výstupem jsou vertikální
profily objemu korun agregované po výškových metrech a přepočtené na hektar.

Výsledky jsou prezentovány pro stav porostu před zásahem, po zásahu
a pro odstraněné stromy, ve třech oddělených panelech.

#### Ovládání

Uživatel volí skupinování profilů: podle dřevin nebo volby zásahu.
Lze zobrazit jednu skupinu (samostatné křivky) nebo kombinaci dvou skupin
jako překryv. První skupina tvoří hlavní profily, druhá doplňkový překryv.

#### Struktura grafů

- Osa Y: výška nad terénem v metrech
- Osa X: objem korun ve výškové vrstvě (m³/ha)
- Barevné křivky: jednotlivé dřeviny nebo kategorie zásahu
- Šedá tečkovaná křivka Suma: celkový objem korun bez rozlišení skupin
- Titulek panelu: celkový objem korun na hektar pro daný stav

Pohybem myší se zobrazí výška vrstvy a hodnota objemu korun pro danou skupinu.

#### Pěstební využití

Profil výškového rozložení korun umožňuje hodnotit vertikální diferenciaci
porostu a míru uvolnění cílových stromů v jejich výškové úrovni. Stránka
slouží k posouzení:
- změny zastoupení korun v horním, středním a spodním patře porostu
- zda zásah vedl k žádoucímu prosvětlení porostu nebo nadměrnému narušení
  hlavní porostní úrovně
- způsobu práce s korunovým prostorem u dřevin s rozdílnými nároky na světlo

Analýza vertikální struktury korun je klíčová zejména u modelů přírodě blízkého
pěstování lesů, kde regulace světelných podmínek a maximální využití celého
vertikálního růstového prostoru tvoří podstatu pěstebního zásahu.
""",
        "en": """
#### Crown Space Occupancy

This page displays the vertical distribution of crown volume within the stand
and how it changes as a result of the simulated silvicultural intervention.
The primary output consists of vertical crown volume profiles aggregated per
height metre and normalised per hectare.

Results are presented for the stand state before intervention, after intervention,
and for removed trees, in three separate panels.

#### Controls

The user selects the grouping for profiles: by species or management category.
A single grouping (individual curves) or a combination of two groupings can be
displayed as an overlay. The first grouping forms the primary profiles, the second
the supplementary overlay.

#### Chart structure

- Y-axis: height above ground in metres
- X-axis: crown volume within the height layer (m³/ha)
- Coloured curves: individual species or management categories
- Grey dotted Sum curve: total crown volume without group breakdown
- Panel title: total crown volume per hectare for the given state

Hovering shows the layer height and crown volume value for the selected group.

#### Silvicultural use

The vertical crown distribution profile allows assessment of the vertical
differentiation of the stand and the degree of crown release for target trees
at their respective height levels. The page serves to assess:
- changes in crown representation in the upper, middle, and lower storeys
- whether the intervention led to desirable canopy opening or excessive
  disruption of the main stand level
- management of crown space for species with different light requirements

Vertical crown structure analysis is especially important in close-to-nature
silviculture, where regulating light conditions and maximising the use of the
full vertical growing space are central to the silvicultural intervention.
""",
    },

    "space_comp_help": {
        "cs": """
#### Prostorová konkurence

Stránka hodnotí prostorovou konkurenci mezi stromy o korunový prostor
prostřednictvím sdílených objemů korun (překryv korunových voxelů).
Výsledky jsou prezentovány pro stav porostu před a po zásahu.

#### Hodnocené ukazatele

- celkový objem sdíleného korunového prostoru v porostu
- rozdělení sdíleného prostoru mezi dřeviny
- podíl vnitrodruhové a mezidruhové konkurence
- srovnání míry konkurence před a po zásahu

Hodnoty lze vyjádřit jako procenta celkového sdíleného objemu nebo
jako absolutní objem v m³.

#### Vstupní množina stromů

Vybrané stromy tvoří fokální množinu. Do výpočtu jsou automaticky
zahrnuti jejich bezprostřední sousedé sdílející korunový prostor.
Filtry dřevin a volby zásahu určují, které stromy jsou považovány
za fokální. To zajišťuje správné vyhodnocení i na okrajích výběru.

#### Ovládání

Uživatel volí:
- stav porostu: před zásahem nebo po zásahu
- způsob vyjádření hodnot (procenta / absolutní objem)
- filtraci stromů podle výčetní tloušťky a výšky
- filtraci fokálních stromů podle dřevin a volby zásahu

#### Struktura grafů

**Sdílený vs. celkový objem korun** – porovnává celkový objem korun
fokálních stromů s objemem sdíleným se sousedy. Vyjadřuje podíl
korunového prostoru zatíženého konkurencí.

**Sdílený prostor podle dřevin** – jak se sdílený prostor rozděluje
mezi dřeviny.

**Sdílený prostor podle volby zásahu** – rozdělení sdíleného prostoru
podle kategorií managementu.

#### Pěstební využití

Hodnocení na základě překryvu korunových voxelů vychází z reálné prostorové
struktury korun, na rozdíl od tradičních ukazatelů vzdálenosti kmenů.
Stránka umožňuje posoudit, zda zásah vedl k reálnému snížení korunové
konkurence u cílových stromů, zda byla konkurence odstraněna správně
a zda přetrvává v klíčových výškových patrech. Pokles sdíleného prostoru
po zásahu objektivně dokládá uvolnění růstového prostoru.
""",
        "en": """
#### Crown Space Competition

This page evaluates spatial competition between trees for crown space through
shared crown volumes (overlap of crown voxels). Results are presented for the
stand state before and after intervention.

#### Evaluated indicators

- total shared crown space volume in the stand
- distribution of shared space among species
- share of intraspecific vs. interspecific competition
- comparison of competition level before and after intervention

Values can be expressed as percentages of the total shared volume or as
absolute volume in m³.

#### Input tree set

Selected trees form the focal set. Their immediate neighbours sharing crown
space are automatically included in the calculation. Species and management
filters define which trees are considered focal, ensuring correct evaluation
even at the edges of the selection.

#### Controls

The user selects:
- stand state: before or after intervention
- value expression mode (percentage / absolute volume)
- filtering by DBH and height
- filtering of focal trees by species and management category

#### Chart structure

**Shared vs. total crown volume** – compares the total crown volume of focal
trees with the volume shared with neighbours. Expresses the share of crown
space under competitive pressure.

**Shared space by species** – how shared space is distributed among species.

**Shared space by management category** – distribution of shared space across
management categories.

#### Silvicultural use

Evaluation based on crown voxel overlap uses the real spatial crown structure,
unlike traditional stem-distance indices. The page allows assessment of whether
the intervention genuinely reduced crown competition for target trees, whether
competition was removed selectively and correctly, and whether it persists in
key vertical layers. A decrease in shared crown space after intervention objectively
demonstrates the release of growing space.
""",
    },

    "light_comp_help": {
        "cs": """
#### Dostupnost světla

Stránka hodnotí světelné podmínky stromů a konkurenci o světlo způsobenou
okolními stromy. Dostupnost světla je vyjádřena jako podíl viditelné oblohy
z ohniskového bodu umístěného ve 2/3 výšky koruny každého stromu. Výpočet
probíhá metodou sledování paprsků (ray-tracing) nad voxelovou strukturou porostu
(5 000 paprsků na strom, rovnoměrné rozložení v kuželu s vrcholovým úhlem 60°).
Výsledky jsou vyjádřeny v procentech.

#### Ovládání

Uživatel volí:
- stav porostu: před zásahem nebo po zásahu
- režim zobrazení: Kdo stíní / Dostupnost světla (Sky view)
- filtraci stromů podle výčetní tloušťky a výšky

Filtry dřevin a volby zásahu v dolní části stránky určují hodnocenou množinu
stromů a ovlivňují průměrné dostupné světlo, rozdělení konkurence i statistiky
Sky view.

#### Struktura grafů

**Průměrné dostupné světlo** – kruhový diagram znázorňuje 100 % potenciálu
(vnější kruh) a průměrnou dostupnost světla stromů (vnitřní plocha). Větší
plocha = lepší světelné podmínky.

##### Režim „Kdo stíní"
- sloupcový graf stínění podle dřevin
- sloupcový graf stínění podle kategorií zásahu
- hodnoty vyjadřují součet procent stínění přes všechny hodnocené stromy

##### Režim „Sky view"
- houslové grafy dostupnosti světla rozdělené podle dřevin nebo volby zásahu
- zobrazuje typické světelné podmínky, variabilitu mezi stromy i extrémní jedince

#### Pěstební využití

Dostupnost světla je jedním z hlavních faktorů určujících růstovou dynamiku,
vitalitu a stabilitu stromů, zejména cílových jedinců. Stránka umožňuje:
- posoudit, zda zásah vedl k dostatečnému prosvětlení korun cílových stromů
- vyhodnotit světelné podmínky pro jednotlivé dřeviny s ohledem na jejich
  postavení v porostu
- identifikovat stromy, které i po zásahu zůstávají v silném zastínění
- posoudit vhodnost zásahu z hlediska podpory přirozené obnovy

Ve spojení se stránkami Vyplnění prostoru a Prostorová konkurence poskytuje
stránka komplexní pohled na vztah mezi strukturou korunového prostoru,
konkurenčním tlakem a světelným režimem porostu.
""",
        "en": """
#### Light Availability

This page evaluates light conditions for individual trees and competition for
light caused by neighbouring trees. Light availability is expressed as the
proportion of visible sky from a focal point placed at 2/3 of the crown height
of each tree. The calculation uses ray-tracing over the voxel structure of the
stand (5,000 rays per tree, uniform distribution within a cone with 60° apex angle).
Results are expressed in percent.

#### Controls

The user selects:
- stand state: before or after intervention
- display mode: Who shades / Light availability (Sky view)
- filtering by DBH and height

Species and management filters in the lower section define the evaluated tree set
and affect average available light, competition breakdown, and Sky view statistics.

#### Chart structure

**Average available light** – a circular diagram showing 100 % potential (outer circle)
and average tree light availability (inner area). Larger inner area = better light conditions.

##### "Who shades" mode
- bar chart of shading by species
- bar chart of shading by management category
- values represent the sum of shading percentages across all evaluated trees

##### "Sky view" mode
- violin plots of light availability grouped by species or management category
- shows typical light conditions, variability between trees, and extreme individuals

#### Silvicultural use

Light availability is one of the primary factors determining growth dynamics,
vitality, and stability of trees, especially target individuals. The page enables:
- assessment of whether the intervention led to sufficient crown illumination
  for target trees
- evaluation of light conditions for individual species relative to their stand position
- identification of trees still in strong shade after the intervention
- assessment of the intervention's suitability for promoting natural regeneration

Together with the Crown Space Occupancy and Crown Space Competition pages,
this page provides a comprehensive view of the relationship between crown space
structure, competitive pressure, and the light regime of the stand.
""",
    },

    "add_att_help": {
        "cs": """
#### Přidat atributy

Stránka slouží k rozšíření databáze stromů o uživatelsky definované atributy,
které nejsou součástí standardní datové struktury aplikace 3D Forest. Umožňuje
propojit výsledky terénních šetření, externích hodnocení nebo expertních
klasifikací s analytickými výstupy modulu 3D-Marteloscop.

Uživatelské atributy jsou ukládány do projektové databáze SQLite, jsou vázány
na jednoznačné identifikátory stromů a po importu jsou dostupné napříč celým
modulem. Základní data stromů zůstávají beze změny.

#### Import atributů

Základní funkcí stránky je import ze souboru CSV. Soubor musí splňovat:
- obsahovat sloupec `id` s číselnými identifikátory stromů bez chybějících hodnot
- hodnoty `id` musí přesně odpovídat ID stromů v projektu (žádné navíc ani chybějící)
- ostatní sloupce jsou považovány za atributy s číselnými (float) hodnotami
- žádné prázdné nebo nečíselné hodnoty nejsou přípustné

Pokud soubor obsahuje atributy, které v projektu již existují, je uživatel
explicitně upozorněn a import je povolen teprve po potvrzení přepisu.

Import se provede až po kliknutí na tlačítko **Import attributes**.

#### Správa atributů

V dolní části stránky lze zobrazit seznam uložených atributů, vybrat jeden nebo
více z nich a trvale je odebrat tlačítkem **Remove selected attributes**.
Odstranění je nevratné a projeví se okamžitě ve všech analytických výstupech.

#### Vazba na analytické stránky

Importované atributy jsou plně dostupné na stránce Statistiky stromů, kde lze
je zobrazovat a vyhodnocovat stejným způsobem jako standardní stromové
charakteristiky.

#### Doporučení

- Před importem vždy ověřte shodu ID stromů mezi CSV a projektem.
- Při přepisu existujících atributů pracujte s kopií projektu.
- Používejte jednoznačné a konzistentní názvy atributů.
""",
        "en": """
#### Add Attributes

This page allows the tree database to be extended with user-defined attributes
that are not part of the standard 3D Forest data structure. It enables linking
field survey results, external assessments, or expert classifications with the
analytical outputs of the 3D-Marteloscop module.

User-defined attributes are stored in the project SQLite database, bound to
unique tree identifiers, and available across the entire module after import.
Core tree data remain unchanged.

#### Attribute import

The primary function of the page is import from a CSV file. The file must meet:
- contain a column named `id` with numeric tree identifiers and no missing values
- `id` values must exactly match tree IDs in the project (no extra or missing IDs)
- all other columns are treated as attributes with numeric (float) values
- no missing or non-numeric values are allowed

If the file contains attributes that already exist in the project, the user is
explicitly warned and the import is only allowed after confirming the overwrite.

The import is executed only after clicking **Import attributes**.

#### Attribute management

The lower section of the page lists stored attributes. One or more can be selected
and permanently removed using **Remove selected attributes**.
Removal is irreversible and takes effect immediately across all analytical outputs.

#### Link to analytical pages

Imported attributes are fully available on the Tree Statistics page, where they
can be displayed and evaluated in the same way as standard tree characteristics.

#### Recommendations

- Always verify tree ID consistency between the CSV and the project before importing.
- Use a project copy when overwriting existing attributes.
- Use clear and consistent attribute names.
""",
    },

    "simulation_help": {
        "cs": """
#### Predikce

Stránka slouží k dlouhodobému vyhodnocení simulovaného pěstebního zásahu
pomocí procesně orientovaného růstového modelu iLand. Umožňuje kvantifikovat
očekávaný vývoj porostu v čase na základě aktuální struktury po zásahu
a zvolených simulačních parametrů. Jde o přechod od statického hodnocení
k dynamickému posouzení budoucí produkce a stability porostu.

Simulace je realizována jako vícenásobná (Monte Carlo) simulace, která zohledňuje
stochastické procesy mortality a přirozené obnovy. Aplikace 3D-Marteloscop
zajišťuje přípravu vstupních dat, opakované spouštění simulace a agregaci výsledků.

#### Nastavení simulace

Uživatel může nastavovat:
- délku simulačního období v letech
- počet opakování simulace (replikací) – ovlivňuje statistickou stabilitu výsledků
  a výpočetní náročnost
- zahrnutí mortality stromů
- zahrnutí přirozené obnovy porostu

Simulaci spustí tlačítko **Znovu spustit simulaci**. Průběh výpočtu je
indikován stavovým ukazatelem. Po dokončení jsou výsledky automaticky uloženy.

Pokud jsou oba přepínače (mortalita, obnova) vypnuty, je simulace deterministická
a všechny replikace jsou shodné – vějířové grafy se zobrazí jako jediná čára.
Pro smysluplnou analýzu nejistoty se doporučuje zapnout alespoň mortalitu.

#### Grafické výstupy

Výsledky jsou prezentovány jako vějířové grafy (fan charts) zobrazující vývoj
objemu živé dřevní hmoty v čase:
- **středová čára** – medián simulovaného vývoje
- **vnitřní pás** – 25–75 % kvantil
- **vnější pás** – 5–95 % kvantil

Zobrazeny jsou dva pohledy:
- vývoj objemu podle dřevin
- vývoj objemu podle volby zásahu (cílové stromy vs. ostatní ponechané)

#### Model iLand

iLand (*individual-based forest landscape model*) je procesně orientovaný
ekofyziologický model simulující růst jednotlivých stromů, konkurenci o světlo,
vodu a živiny, mortalitu a obnovu a reakci porostu na klimatické a stanovištní
podmínky. Model pracuje na úrovni jednotlivých stromů, výstupy jsou agregovány
na úroveň porostu.

Více informací: https://iland-model.org

#### Pěstební využití

Stránka umožňuje hodnotit zásah nikoli pouze z hlediska okamžitého dopadu,
ale především z pohledu budoucího vývoje porostu. Lze posoudit:
- zda zásah podporuje dlouhodobý růst objemu cílových stromů
- růstový potenciál jednotlivých dřevin po zásahu
- stabilitu porostu a riziko při zahrnutí mortality
- vliv zásahu na budoucí produkční potenciál plochy

Intervalové zobrazení výsledků reflektuje přirozenou nejistotu růstu
a umožňuje realistickou interpretaci bez fixace na jediný deterministický scénář.
""",
        "en": """
#### Prediction

This page supports long-term evaluation of the simulated silvicultural intervention
using the process-based ecophysiological growth model iLand. It quantifies the
expected stand development over time based on the current post-intervention structure
and selected simulation parameters. The page represents a transition from static
assessment to dynamic evaluation of future stand production and stability.

The simulation is implemented as a Monte Carlo simulation accounting for stochastic
processes of mortality and natural regeneration. The 3D-Marteloscop application
manages input data preparation, repeated simulation runs, and output aggregation.

#### Simulation settings

The user can configure:
- simulation period length in years
- number of simulation runs (replications) – affects statistical stability and
  computational demand
- inclusion of tree mortality
- inclusion of natural regeneration

The simulation is started with **Restart simulation**. Progress is indicated by
a status indicator. Results are automatically saved upon completion.

If both toggles (mortality, regeneration) are off, the simulation is deterministic
and all replications are identical – fan charts collapse to a single line.
Enabling at least mortality is recommended for meaningful uncertainty analysis.

#### Graphical outputs

Results are presented as fan charts showing the development of live stem volume over time:
- **central line** – median of simulated development
- **inner band** – 25–75 % quantile
- **outer band** – 5–95 % quantile

Two perspectives are displayed:
- volume development by species
- volume development by management category (target trees vs. other retained trees)

#### iLand model

iLand (*individual-based forest landscape model*) is a process-based ecophysiological
model simulating individual tree growth, competition for light, water, and nutrients,
mortality and regeneration, and stand response to climate and site conditions.
The model operates at the individual tree level; outputs are aggregated to stand level.

More information: https://iland-model.org

#### Silvicultural use

The page enables evaluation of the intervention not only in terms of its immediate
impact but primarily in terms of future stand development. It allows assessment of:
- whether the intervention supports long-term volume growth of target trees
- growth potential of individual species after intervention
- stand stability and development risk when mortality is included
- effect of the intervention on future production potential of the plot

Interval-based presentation of results reflects the natural uncertainty of stand
growth and enables realistic interpretation without fixation on a single deterministic scenario.
""",
    },

}
