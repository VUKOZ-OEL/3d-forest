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

/** @file Json.cpp */

// Include std.
#include <cmath>
#include <iomanip>

// Include 3D Forest.
#include <File.hpp>
#include <Json.hpp>

// Include local.
#define LOG_MODULE_NAME "Json"
#include <Log.hpp>

void Json::clear()
{
    type_ = TYPE_NULL;
    data_.object.reset();
    data_.array.reset();
    data_.string.reset();
}

void Json::read(const std::string &fileName)
{
    std::string data = File::read(fileName);
    deserialize(data);
}

void Json::write(const std::string &fileName, size_t indent)
{
    std::string data = serialize(indent);
    std::string writePath = File::tmpname(fileName, fileName);
    File::write(writePath, data);
    File::move(fileName, writePath);
}

std::string Json::serialize(size_t indent) const
{
    std::ostringstream out;

    if (indent == 0)
    {
        serialize(out);
    }
    else
    {
        std::string str;
        str.resize(indent);
        for (size_t i = 0; i < str.size(); i++)
        {
            str[i] = ' ';
        }
        serialize(out, "", str);
    }

    return out.str();
}

void Json::deserialize(const std::string &in)
{
    deserialize(in.c_str(), in.size());
}

void Json::deserialize(const char *in, size_t n)
{
    size_t i = 0;
    deserialize(*this, in, n, i);
}

void Json::serialize(std::ostringstream &out) const
{
    size_t i = 0;
    size_t n;

    uint64_t num;
    double numd;
    constexpr double e = std::numeric_limits<double>::epsilon();

    switch (type_)
    {
        case TYPE_OBJECT:
            out << "{";
            n = data_.object->size();
            for (auto const &it : *data_.object)
            {
                out << "\"" << it.first << "\": ";
                it.second.serialize(out);
                i++;
                if (i < n)
                {
                    out << ",";
                }
            }
            out << "}";
            break;

        case TYPE_ARRAY:
            out << "[";
            n = data_.array->size();
            for (auto const &it : *data_.array)
            {
                it.serialize(out);
                i++;
                if (i < n)
                {
                    out << ",";
                }
            }
            out << "]";
            break;

        case TYPE_STRING:
            out << "\"" << *data_.string << "\"";
            break;

        case TYPE_NUMBER:
            num = static_cast<uint64_t>(data_.number);
            numd = static_cast<double>(num);
            if (::fabs(data_.number - numd) > e)
            {
                out << std::fixed << std::setprecision(15) << data_.number
                    << std::defaultfloat;
            }
            else
            {
                out << num;
            }
            break;

        case TYPE_TRUE:
            out << "true";
            break;

        case TYPE_FALSE:
            out << "false";
            break;

        case TYPE_NULL:
        default:
            out << "null";
            break;
    }
}

void Json::serialize(std::ostringstream &out,
                     const std::string &indent,
                     const std::string &indentPlus) const
{
    std::string indent2;
    size_t i = 0;
    size_t n;
    bool container;
    uint64_t num;
    double numd;
    constexpr double e = std::numeric_limits<double>::epsilon();

    switch (type_)
    {
        case TYPE_OBJECT:
            out << "{\n";
            indent2 = indent + indentPlus;
            n = data_.object->size();
            for (auto const &it : *data_.object)
            {
                out << indent2 << "\"" << it.first << "\": ";
                it.second.serialize(out, indent2, indentPlus);
                i++;
                if (i < n)
                {
                    out << ",\n";
                }
            }
            out << "\n" << indent << "}";
            break;

        case TYPE_ARRAY:
            out << "[";
            n = data_.array->size();
            if ((n > 0) && ((data_.array->at(0).typeObject()) ||
                            (data_.array->at(0).typeArray())))
            {
                indent2 = indent + indentPlus;
                out << "\n" << indent2;
                container = true;
            }
            else
            {
                container = false;
            }
            for (auto const &it : *data_.array)
            {
                it.serialize(out, indent2, indentPlus);
                i++;
                if (i < n)
                {
                    out << ",";
                    if (container)
                    {
                        out << "\n" << indent2;
                    }
                }
            }
            if (container)
            {
                out << "\n" << indent;
            }
            out << "]";
            break;

        case TYPE_STRING:
            out << "\"" << *data_.string << "\"";
            break;

        case TYPE_NUMBER:
            num = static_cast<uint64_t>(data_.number);
            numd = static_cast<double>(num);
            if (::fabs(data_.number - numd) > e)
            {
                out << std::fixed << std::setprecision(15) << data_.number
                    << std::defaultfloat;
            }
            else
            {
                out << num;
            }
            break;

        case TYPE_TRUE:
            out << "true";
            break;

        case TYPE_FALSE:
            out << "false";
            break;

        case TYPE_NULL:
        default:
            out << "null";
            break;
    }
}

void Json::deserialize(Json &obj, const char *in, size_t n, size_t &i)
{
    size_t str_start;
    size_t str_len;
    size_t array_index;
    std::string str;

    enum State
    {
        STATE_VALUE,
        STATE_OBJECT,
        STATE_OBJECT_PAIR,
        STATE_ARRAY,
        STATE_STRING,
        STATE_STRING_NAME,
        STATE_STRING_VALUE,
        STATE_NUMBER,
        STATE_TYPE
    };
    int state = STATE_VALUE;
    int next;

    while (i < n)
    {
        switch (state)
        {
            case STATE_VALUE:
                if (in[i] == '{')
                {
                    obj.createObject();
                    state = STATE_OBJECT;
                }
                else if (in[i] == '[')
                {
                    obj.createArray();
                    state = STATE_ARRAY;
                    array_index = 0;
                }
                else if (in[i] == '\"')
                {
                    str_start = i + 1;
                    state = STATE_STRING;
                    next = STATE_STRING_VALUE;
                }
                else if (in[i] == '-' || (in[i] > 47 && in[i] < 58))
                {
                    str_start = i;
                    state = STATE_NUMBER;
                }
                else if (in[i] > 96 && in[i] < 123)
                {
                    str_start = i;
                    state = STATE_TYPE;
                }
                break;

            case STATE_OBJECT:
                if (in[i] == '\"')
                {
                    str_start = i + 1;
                    state = STATE_STRING;
                    next = STATE_STRING_NAME;
                }
                else if (in[i] == '}')
                {
                    i++;
                    return;
                }
                break;

            case STATE_STRING_NAME:
                if (in[i] == ':')
                {
                    state = STATE_OBJECT_PAIR;
                }
                break;

            case STATE_OBJECT_PAIR:
                // Parse object pair value.
                deserialize(obj[str], in, n, i);
                i--;
                state = STATE_OBJECT;
                break;

            case STATE_STRING:
                /** @todo Extra characters. */
                if (in[i] == '\"')
                {
                    str_len = i - str_start;
                    str.resize(str_len);
                    for (size_t j = 0; j < str_len; j++)
                    {
                        str[j] = in[str_start + j];
                    }
                    state = next;
                }
                break;

            case STATE_STRING_VALUE:
                obj.createString(str);
                return;

            case STATE_NUMBER:
                /** @todo Number format. */
                if (!(in[i] == '.' || (in[i] > 47 && in[i] < 58)))
                {
                    str_len = i - str_start;
                    str.resize(str_len);
                    for (size_t j = 0; j < str_len; j++)
                    {
                        str[j] = in[str_start + j];
                    }
                    obj.createNumber(std::stod(str));
                    return;
                }
                break;

            case STATE_ARRAY:
                if (in[i] > 32 && in[i] != ']')
                {
                    // Parse next array element.
                    deserialize(obj[array_index], in, n, i);
                    array_index++;
                }

                if (in[i] == ']')
                {
                    i++;
                    return;
                }
                break;

            case STATE_TYPE:
                if (!(in[i] > 96 && in[i] < 123))
                {
                    str_len = i - str_start;
                    str.resize(str_len);
                    for (size_t j = 0; j < str_len; j++)
                    {
                        str[j] = in[str_start + j];
                    }

                    if (str == "null")
                    {
                        obj.createType(TYPE_NULL);
                    }
                    else if (str == "false")
                    {
                        obj.createType(TYPE_FALSE);
                    }
                    else
                    {
                        obj.createType(TYPE_TRUE);
                    }

                    return;
                }
                break;

            default:
                break;
        }

        i++;
    }
}
