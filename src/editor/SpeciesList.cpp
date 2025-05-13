/*
    Copyright 2020 VUKOZ

    This file is part of 3D Forest.

    3D Forest is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    3D Forest is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file SpeciesList.cpp */

// Include 3D Forest.
#include <Error.hpp>
#include <SpeciesList.hpp>

// Include local.
#define LOG_MODULE_NAME "SpeciesList"
#include <Log.hpp>

SpeciesList::SpeciesList()
{
}

void SpeciesList::clear()
{
    LOG_DEBUG(<< "Clear.");
    data_.clear();
    index_.clear();
}

void SpeciesList::setDefault()
{
    LOG_DEBUG(<< "Set default.");

    clear();

    push_back(Species(0,
                      "unknown",
                      "unknown",
                      "unknown",
                      "unknown",
                      {0.8, 0.8, 0.8}));
    push_back(Species(1,
                      "Pinus",
                      "nigra",
                      "Pinus nigra",
                      "Borovice černá",
                      0xff0029U));
    push_back(Species(2,
                      "Pinus",
                      "sylvestris",
                      "Pinus sylvestris",
                      "Borovice lesní",
                      0x377eb8U));
    push_back(Species(3,
                      "Pinus",
                      "strobus",
                      "Pinus strobus",
                      "Borovice vejmutovka",
                      0x66a61eU));
    push_back(Species(4,
                      "Betula",
                      "pendula",
                      "Betula pendula",
                      "Bříza bělokorá",
                      0x984ea3U));
    push_back(Species(5,
                      "Betula",
                      "pubescens",
                      "Betula pubescens",
                      "Bříza pýřitá",
                      0x00d2d5U));
    push_back(Species(6,
                      "Fagus",
                      "sylvatica",
                      "Fagus sylvatica",
                      "Buk lesní",
                      0xff7f00U));
    push_back(Species(7,
                      "Pseudotsuga",
                      "menziesii",
                      "Pseudotsuga menziesii",
                      "Douglaska tisolistá",
                      0xaf8d00U));
    push_back(Species(8,
                      "Quercus",
                      "rubra",
                      "Quercus rubra",
                      "Dub červený",
                      0x7f80cdU));
    push_back(Species(9,
                      "Quercus",
                      "robur",
                      "Quercus robur",
                      "Dub letní",
                      0xb3e900U));
    push_back(Species(10,
                      "Quercus",
                      "petraea",
                      "Quercus petraea",
                      "Dub zimní",
                      0xc42e60U));
    push_back(Species(11,
                      "Carpinus",
                      "betulus",
                      "Carpinus betulus",
                      "Habr obecný",
                      0xa65628U));
    push_back(Species(12,
                      "Fraxinus",
                      "excelsior",
                      "Fraxinus excelsior",
                      "Jasan ztepilý",
                      0xf781bfU));
    push_back(Species(13,
                      "Acer",
                      "campestre",
                      "Acer campestre",
                      "Javor babyka",
                      0x8dd3c7U));
    push_back(Species(14,
                      "Acer",
                      "negundo",
                      "Acer negundo",
                      "Javor jasanolistý",
                      0xbebadaU));
    push_back(Species(15,
                      "Acer",
                      "pseudoplatanus",
                      "Acer pseudoplatanus",
                      "Javor klen",
                      0xfb8072U));
    push_back(Species(16,
                      "Acer",
                      "platanoides",
                      "Acer platanoides",
                      "Javor mléč",
                      0x80b1d3U));
    push_back(Species(17,
                      "Abies",
                      "alba",
                      "Abies alba",
                      "Jedle bělokorá",
                      0xfdb462U));
    push_back(Species(18,
                      "Abies",
                      "grandis",
                      "Abies grandis",
                      "Jedle obrovská",
                      0xfccde5U));
    push_back(Species(19,
                      "Sorbus",
                      "torminalis",
                      "Sorbus torminalis",
                      "Jeřáb břek",
                      0xbc80bdU));
    push_back(Species(20,
                      "Sorbus",
                      "domestica",
                      "Sorbus domestica",
                      "Jeřáb oskeruše",
                      0xffed6fU));
    push_back(Species(21,
                      "Sorbus",
                      "aucuparia",
                      "Sorbus aucuparia",
                      "Jeřáb ptačí",
                      0xc4eaffU));
    push_back(Species(22,
                      "Ulmus",
                      "glabra",
                      "Ulmus glabra",
                      "Jilm drsný",
                      0xcf8c00U));
    push_back(Species(23,
                      "Ulmus",
                      "minor",
                      "Ulmus minor",
                      "Jilm habrolistý",
                      0x1b9e77U));
    push_back(Species(24,
                      "Ulmus",
                      "pumila",
                      "Ulmus pumila",
                      "Jilm sibiřský",
                      0xd95f02U));
    push_back(
        Species(25, "Ulmus", "laevis", "Ulmus laevis", "Jilm vaz", 0xe7298aU));
    push_back(Species(26,
                      "Tilia",
                      "cordata",
                      "Tilia cordata",
                      "Lípa srdčitá",
                      0xe6ab02U));
    push_back(Species(27,
                      "Tilia",
                      "platyphyllos",
                      "Tilia platyphyllos",
                      "Lípa velkolistá",
                      0xa6761dU));
    push_back(Species(28,
                      "Larix",
                      "kaempferi",
                      "Larix kaempferi",
                      "Modřín japonský",
                      0x0097ffU));
    push_back(Species(29,
                      "Larix",
                      "decidua",
                      "Larix decidua",
                      "Modřín opadavý",
                      0x00d067U));
    push_back(Species(30,
                      "Alnus",
                      "glutinosa",
                      "Alnus glutinosa",
                      "Olše lepkavá",
                      0x252525U));
    push_back(
        Species(31, "Alnus", "incana", "Alnus incana", "Olše šedá", 0x525252U));
    push_back(Species(32,
                      "Juglans",
                      "nigra",
                      "Juglans nigra",
                      "Ořešák černý",
                      0x737373U));
    push_back(Species(33,
                      "Ailanthus",
                      "altissima",
                      "Ailanthus altissima",
                      "Pajasan žláznatý",
                      0x969696U));
    push_back(Species(34,
                      "Picea",
                      "pungens",
                      "Picea pungens",
                      "Smrk pichlavý",
                      0xbdbdbdU));
    push_back(Species(35,
                      "Picea",
                      "sitchensis",
                      "Picea sitchensis",
                      "Smrk sitka",
                      0xf43600U));
    push_back(Species(36,
                      "Picea",
                      "abies",
                      "Picea abies",
                      "Smrk ztepilý",
                      0x4ba93bU));
    push_back(Species(37,
                      "Rhus",
                      "typhina",
                      "Rhus typhina",
                      "Škumpa orobincová",
                      0x5779bbU));
    push_back(Species(38,
                      "Taxus",
                      "baccata",
                      "Taxus baccata",
                      "Tis červený",
                      0x927accU));
    push_back(Species(39,
                      "Populus",
                      "alba",
                      "Populus alba",
                      "Topol bílý",
                      0x97ee3fU));
    push_back(Species(40,
                      "Populus",
                      "nigra",
                      "Populus nigra",
                      "Topol černý",
                      0xbf3947U));
    push_back(Species(41,
                      "Populus",
                      "× canadensis",
                      "Populus × canadensis",
                      "Topol kanadský",
                      0x9f5b00U));
    push_back(Species(42,
                      "Populus",
                      "tremula",
                      "Populus tremula",
                      "Topol osika",
                      0xf48758U));
    push_back(Species(43,
                      "Populus",
                      "× canescens",
                      "Populus × canescens",
                      "Topol šedý",
                      0x8caed6U));
    push_back(Species(44,
                      "Robinia",
                      "pseudoacacia",
                      "Robinia pseudoacacia",
                      "Trnovník akát",
                      0xf2b94fU));
    push_back(Species(45,
                      "Prunus",
                      "avium",
                      "Prunus avium",
                      "Třešeň ptačí",
                      0xeff26eU));
    push_back(
        Species(46, "Salix", "caprea", "Salix caprea", "Vrba jíva", 0xe43872U));
}

void SpeciesList::push_back(const Species &species)
{
    LOG_DEBUG(<< "Append species <" << species << ">.");

    size_t id = species.id;
    size_t idx = data_.size();

    data_.push_back(species);

    index_[id] = idx;
}

void SpeciesList::erase(size_t pos)
{
    LOG_DEBUG(<< "Erase item <" << pos << ">.");

    if (data_.size() == 0)
    {
        return;
    }

    size_t key = id(pos);
    index_.erase(key);

    size_t n = data_.size() - 1;
    for (size_t i = pos; i < n; i++)
    {
        data_[i] = data_[i + 1];
        index_[data_[i].id] = i;
    }
    data_.resize(n);
}

size_t SpeciesList::index(size_t id, bool throwException) const
{
    const auto &it = index_.find(id);
    if (it != index_.end())
    {
        return it->second;
    }

    if (throwException)
    {
        THROW("Invalid species id");
    }
    else
    {
        return SIZE_MAX;
    }
}

bool SpeciesList::contains(size_t id) const
{
    const auto &it = index_.find(id);
    if (it != index_.end())
    {
        return true;
    }
    return false;
}

std::string SpeciesList::labelById(size_t id, bool throwException) const
{
    size_t idx = index(id, throwException);
    if (idx != SIZE_MAX)
    {
        return data_[idx].latin;
    }
    return "Unknown";
}

size_t SpeciesList::unusedId() const
{
    LOG_DEBUG(<< "Obtain unused id.");
    // Return minimum available id value.
    for (size_t rval = 0; rval < std::numeric_limits<size_t>::max(); rval++)
    {
        if (index_.find(rval) == index_.end())
        {
            return rval;
        }
    }

    THROW("New species identifier is not available.");
}

void fromJson(SpeciesList &out, const Json &in)
{
    out.clear();

    size_t i = 0;
    size_t n = in.array().size();

    out.data_.resize(n);

    for (auto const &it : in.array())
    {
        fromJson(out.data_[i], it);
        size_t id = out.data_[i].id;
        out.index_[id] = i;
        i++;
    }
}

void toJson(Json &out, const SpeciesList &in)
{
    size_t i = 0;

    for (auto const &it : in.data_)
    {
        toJson(out[i], it);
        i++;
    }
}

std::ostream &operator<<(std::ostream &out, const SpeciesList &in)
{
    Json json;
    toJson(json, in);
    return out << json.serialize();
}
