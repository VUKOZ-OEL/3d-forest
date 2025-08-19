/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#include "global.h"
#include "modelsettings.h"
#include "expression.h"

ModelSettings::ModelSettings()
{
}

void ModelSettings::loadModelSettings()
{
    XmlHelper xml(GlobalSettings::instance()->settings().node("model.settings"));
    growthEnabled = xml.valueBool("growthEnabled", true);
    mortalityEnabled = xml.valueBool("mortalityEnabled", true);
    lightExtinctionCoefficient = xml.valueDouble("lightExtinctionCoefficient", 0.5);
    lightExtinctionCoefficientOpacity=xml.valueDouble("lightExtinctionCoefficientOpacity", 0.5);
    temperatureTau=xml.valueDouble("temperatureTau",5);
    epsilon = xml.valueDouble("epsilon",1.8); // max light use efficiency (aka alpha_c)
    airDensity = xml.valueDouble("airDensity", 1.2);
    laiThresholdForClosedStands = xml.valueDouble("laiThresholdForClosedStands", 3.);
    boundaryLayerConductance = xml.valueDouble("boundaryLayerConductance", 0.2);
    XmlHelper world(GlobalSettings::instance()->settings().node("model.world"));
    latitude = RAD(world.valueDouble("latitude",48.));
    usePARFractionBelowGroundAllocation = xml.valueBool("usePARFractionBelowGroundAllocation", true);
    //useDynamicAvailableNitrogen = xml.valueBool("model.settings.soil.useDynamicAvailableNitrogen", false); // TODO: there is a bug in using a xml helper that whose top-node is set
    useDynamicAvailableNitrogen = GlobalSettings::instance()->settings().valueBool("model.settings.soil.useDynamicAvailableNitrogen", false);
    torusMode = GlobalSettings::instance()->settings().paramValueBool("torus", false);
    microclimateEnabled = GlobalSettings::instance()->settings().valueBool("model.climate.microclimate.enabled", false);
}

void ModelSettings::print()
{
    if (!logLevelInfo()) return;
    QStringList set=QStringList() << "Settings:";
    set << QString("growthEnabled=%1").arg(growthEnabled);
    set << QString("mortalityEnabled=%1").arg(mortalityEnabled);
    set << QString("lightExtinctionCoefficient=%1").arg(lightExtinctionCoefficient);
    set << QString("lightExtinctionCoefficientOpacity=%1").arg(lightExtinctionCoefficientOpacity);
    set << QString("temperatureTau=%1").arg(temperatureTau);
    set << QString("epsilon=%1").arg(epsilon);
    set << QString("airDensity=%1").arg(airDensity);
    set << QString("useDynamicAvailableNitrogen=%1").arg(useDynamicAvailableNitrogen);

    set << QString("latitude=%1").arg(GRAD(latitude));

    qDebug() << set.join("\n");
}
