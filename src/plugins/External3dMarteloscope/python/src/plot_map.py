import plotly.express as px
import streamlit as st

color_mapping = {
    "Target tree": "red",
    "Remove": "green",
    "Unselect": "blue",
    "Unknown": "gray",
}
data["color"] = data["status"].map(color_mapping)

# Vytvoření FigureWidget
fig = go.FigureWidget(
    data=go.Scatter(
        x=data["x"],
        y=data["y"],
        mode="markers",
        marker=dict(color=data["color"], size=10),
        text=data["id"],
        hoverinfo="text",
    )
)

# Funkce pro aktualizaci barev
def update_colors(status, new_color):
    # Aktualizace barev pro vybraný status
    data.loc[data["status"] == status, "color"] = new_color
    with fig.batch_update():
        fig.data[0].marker.color = data["color"]  # Aktualizace barvy bodů