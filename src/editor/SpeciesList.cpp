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
                      "none",
                      "unknown",
                      "unknown",
                      "unknown",
                      "unknown",
                      {0.8, 0.8, 0.8}));
    push_back(Species(1,
                      "pini",
                      "Pinus",
                      "nigra",
                      "Pinus nigra",
                      "Borovice černá",
                      {0.384314, 0.85098, 0.819608}));
    push_back(Species(2,
                      "pisy",
                      "Pinus",
                      "sylvestris",
                      "Pinus sylvestris",
                      "Borovice lesní",
                      {0.501961, 0.501961, 0.0}));
    push_back(Species(3,
                      "pist",
                      "Pinus",
                      "strobus",
                      "Pinus strobus",
                      "Borovice vejmutovka",
                      {0.733333, 0.384314, 0.85098}));
    push_back(Species(4,
                      "bepe",
                      "Betula",
                      "pendula",
                      "Betula pendula",
                      "Bříza bělokorá",
                      {0.956863, 0.933333, 0.0}));
    push_back(Species(5,
                      "bepu",
                      "Betula",
                      "pubescens",
                      "Betula pubescens",
                      "Bříza pýřitá",
                      {0.552941, 0.384314, 0.85098}));
    push_back(Species(6,
                      "fasy",
                      "Fagus",
                      "sylvatica",
                      "Fagus sylvatica",
                      "Buk lesní",
                      {0.2, 0.8, 0.2}));
    push_back(Species(7,
                      "psme",
                      "Pseudotsuga",
                      "menziesii",
                      "Pseudotsuga menziesii",
                      "Douglaska tisolistá",
                      {0.827451, 0.384314, 0.85098}));
    push_back(Species(8,
                      "quru",
                      "Quercus",
                      "rubra",
                      "Quercus rubra",
                      "Dub červený",
                      {0.85098, 0.384314, 0.733333}));
    push_back(Species(9,
                      "quro",
                      "Quercus",
                      "robur",
                      "Quercus robur",
                      "Dub letní",
                      {0.85098, 0.419608, 0.384314}));
    push_back(Species(10,
                      "qupe",
                      "Quercus",
                      "petraea",
                      "Quercus petraea",
                      "Dub zimní",
                      {0.592157, 0.278431, 0.023529}));
    push_back(Species(11,
                      "cabe",
                      "Carpinus",
                      "betulus",
                      "Carpinus betulus",
                      "Habr obecný",
                      {0.192157, 0.52549, 0.607843}));
    push_back(Species(12,
                      "frex",
                      "Fraxinus",
                      "excelsior",
                      "Fraxinus excelsior",
                      "Jasan ztepilý",
                      {0.8, 0.8, 0.0}));
    push_back(Species(13,
                      "acca",
                      "Acer",
                      "campestre",
                      "Acer campestre",
                      "Javor babyka",
                      {0.85098, 0.384314, 0.392157}));
    push_back(Species(14,
                      "acne",
                      "Acer",
                      "negundo",
                      "Acer negundo",
                      "Javor jasanolistý",
                      {0.639216, 0.85098, 0.384314}));
    push_back(Species(15,
                      "acps",
                      "Acer",
                      "pseudoplatanus",
                      "Acer pseudoplatanus",
                      "Javor klen",
                      {0.886275, 0.419608, 0.039216}));
    push_back(Species(16,
                      "acpl",
                      "Acer",
                      "platanoides",
                      "Acer platanoides",
                      "Javor mléč",
                      {1.0, 0.752941, 0.0}));
    push_back(Species(17,
                      "abal",
                      "Abies",
                      "alba",
                      "Abies alba",
                      "Jedle bělokorá",
                      {0.0, 0.4, 0.0}));
    push_back(Species(18,
                      "abgr",
                      "Abies",
                      "grandis",
                      "Abies grandis",
                      "Jedle obrovská",
                      {0.384314, 0.419608, 0.85098}));
    push_back(Species(19,
                      "soto",
                      "Sorbus",
                      "torminalis",
                      "Sorbus torminalis",
                      "Jeřáb břek",
                      {0.85098, 0.384314, 0.662745}));
    push_back(Species(20,
                      "sodo",
                      "Sorbus",
                      "domestica",
                      "Sorbus domestica",
                      "Jeřáb oskeruše",
                      {0.482353, 0.384314, 0.85098}));
    push_back(Species(21,
                      "soau",
                      "Sorbus",
                      "aucuparia",
                      "Sorbus aucuparia",
                      "Jeřáb ptačí",
                      {0.615686, 0.384314, 0.85098}));
    push_back(Species(22,
                      "ulgl",
                      "Ulmus",
                      "glabra",
                      "Ulmus glabra",
                      "Jilm drsný",
                      {0.384314, 0.85098, 0.717647}));
    push_back(Species(23,
                      "ulmi",
                      "Ulmus",
                      "minor",
                      "Ulmus minor",
                      "Jilm habrolistý",
                      {0.85098, 0.384314, 0.623529}));
    push_back(Species(24,
                      "ulpu",
                      "Ulmus",
                      "pumila",
                      "Ulmus pumila",
                      "Jilm sibiřský",
                      {0.694118, 0.85098, 0.384314}));
    push_back(Species(25,
                      "ulla",
                      "Ulmus",
                      "laevis",
                      "Ulmus laevis",
                      "Jilm vaz",
                      {0.717647, 0.85098, 0.384314}));
    push_back(Species(26,
                      "tico",
                      "Tilia",
                      "cordata",
                      "Tilia cordata",
                      "Lípa srdčitá",
                      {0.6, 0.0, 1.0}));
    push_back(Species(27,
                      "tipl",
                      "Tilia",
                      "platyphyllos",
                      "Tilia platyphyllos",
                      "Lípa velkolistá",
                      {1.0, 0.0, 1.0}));
    push_back(Species(28,
                      "laka",
                      "Larix",
                      "kaempferi",
                      "Larix kaempferi",
                      "Modřín japonský",
                      {0.384314, 0.741176, 0.85098}));
    push_back(Species(29,
                      "lade",
                      "Larix",
                      "decidua",
                      "Larix decidua",
                      "Modřín opadavý",
                      {0.6, 1.0, 0.2}));
    push_back(Species(30,
                      "algl",
                      "Alnus",
                      "glutinosa",
                      "Alnus glutinosa",
                      "Olše lepkavá",
                      {0.45098, 0.384314, 0.85098}));
    push_back(Species(31,
                      "alin",
                      "Alnus",
                      "incana",
                      "Alnus incana",
                      "Olše šedá",
                      {0.85098, 0.384314, 0.615686}));
    push_back(Species(32,
                      "juni",
                      "Juglans",
                      "nigra",
                      "Juglans nigra",
                      "Ořešák černý",
                      {0.803922, 0.384314, 0.85098}));
    push_back(Species(33,
                      "aial",
                      "Ailanthus",
                      "altissima",
                      "Ailanthus altissima",
                      "Pajasan žláznatý",
                      {0.85098, 0.384314, 0.796078}));
    push_back(Species(34,
                      "pipu",
                      "Picea",
                      "pungens",
                      "Picea pungens",
                      "Smrk pichlavý",
                      {0.384314, 0.85098, 0.4}));
    push_back(Species(35,
                      "pisi",
                      "Picea",
                      "sitchensis",
                      "Picea sitchensis",
                      "Smrk sitka",
                      {0.384314, 0.85098, 0.427451}));
    push_back(Species(36,
                      "piab",
                      "Picea",
                      "abies",
                      "Picea abies",
                      "Smrk ztepilý",
                      {1.0, 0.0, 0.0}));
    push_back(Species(37,
                      "rhty",
                      "Rhus",
                      "typhina",
                      "Rhus typhina",
                      "Škumpa orobincová",
                      {0.85098, 0.384314, 0.733333}));
    push_back(Species(38,
                      "taba",
                      "Taxus",
                      "baccata",
                      "Taxus baccata",
                      "Tis červený",
                      {0.384314, 0.85098, 0.45098}));
    push_back(Species(39,
                      "poal",
                      "Populus",
                      "alba",
                      "Populus alba",
                      "Topol bílý",
                      {0.85098, 0.490196, 0.384314}));
    push_back(Species(40,
                      "poni",
                      "Populus",
                      "nigra",
                      "Populus nigra",
                      "Topol černý",
                      {0.694118, 0.85098, 0.384314}));
    push_back(Species(41,
                      "none",
                      "Populus",
                      "× canadensis",
                      "Populus × canadensis",
                      "Topol kanadský",
                      {0.85098, 0.384314, 0.458824}));
    push_back(Species(42,
                      "potr",
                      "Populus",
                      "tremula",
                      "Populus tremula",
                      "Topol osika",
                      {0.384314, 0.733333, 0.85098}));
    push_back(Species(43,
                      "none",
                      "Populus",
                      "× canescens",
                      "Populus × canescens",
                      "Topol šedý",
                      {0.85098, 0.384314, 0.584314}));
    push_back(Species(44,
                      "rops",
                      "Robinia",
                      "pseudoacacia",
                      "Robinia pseudoacacia",
                      "Trnovník akát",
                      {0.384314, 0.85098, 0.498039}));
    push_back(Species(45,
                      "prav",
                      "Prunus",
                      "avium",
                      "Prunus avium",
                      "Třešeň ptačí",
                      {0.85098, 0.384314, 0.639216}));
    push_back(Species(46,
                      "saca",
                      "Salix",
                      "caprea",
                      "Salix caprea",
                      "Vrba jíva",
                      {0.85098, 0.490196, 0.384314}));
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
