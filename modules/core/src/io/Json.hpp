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

/**
    @file Json.hpp
*/

#ifndef JSON_HPP
#define JSON_HPP

#include <Error.hpp>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

/** JSON.

    example deserialize:
\code
    Json obj;
    obj.deserialize("{\"a\":5}");
    std::cout << obj["a"].isNumber() << "\n"; // 1
    std::cout << obj["a"].number() << "\n"; // 5
\endcode

    example number:
\code
    Json obj;
    obj["a"] = 5;
    obj["b"] = 3.14;
    std::cout << obj.serialize(0) << "\n"; // {"a":5,"b":3.14}
\endcode

    example string:
\code
    Json obj;
    obj["name"] = "John";

    std::cout << obj["name"].isString() << "\n"; // 1
    std::cout << obj["name"].string() << "\n"; // John
    std::cout << obj.serialize(0) << "\n"; // {"name":"John"}
\endcode

    example array:
\code
    Json obj;
    obj["dim"][0] = 10;
    obj["dim"][1] = 20;
    obj["scale"] = std::vector<double>{ 1, 2, 3 };

    std::cout << obj.isObject() << "\n"; // 1
    std::cout << obj["dim"].isArray() << "\n"; // 1
    std::cout << obj["dim"].size() << "\n"; // 2
    std::cout << obj["dim"][0].number() << "\n"; // 10
    std::cout << obj["dim"][1].number() << "\n"; // 20

    double area = 1.;
    for (auto const &it : obj["dim"].array())
    {
        area *= it.number();
    }
    std::cout << area << "\n"; // 200

    std::cout << obj.serialize(0) << "\n"; // {"dim":[10,20],"scale":[1,2,3]}
\endcode
*/
class Json
{
public:
    static const size_t DEFAULT_INDENT = 2;

    Json();
    ~Json() = default;

    Json(bool in);
    Json(int32_t in);
    Json(uint32_t in);
    Json(double in);
    Json(int64_t in);
    Json(uint64_t in);
    Json(const char *in);
    Json(const std::string &in);
    Json(const std::vector<double> &in);

    void clear();

    // object pair
    bool contains(const std::string &key) const;
    bool containsObject(const std::string &key) const;
    bool containsArray(const std::string &key) const;
    bool containsString(const std::string &key) const;
    bool containsNumber(const std::string &key) const;
    bool containsBool(const std::string &key) const;

    Json &operator[](const std::string &key);
    const Json &operator[](const std::string &key) const;

    // array element
    size_t size() const;
    Json &operator[](size_t index);
    const Json &operator[](size_t index) const;

    // value
    const std::map<std::string, Json> &object() const;
    const std::vector<Json> &array() const;
    const std::string &string() const;
    double number() const;

    // value conversion
    uint32_t uint32() const;
    uint64_t uint64() const;

    // type
    bool isObject() const { return type_ == TYPE_OBJECT; }
    bool isArray() const { return type_ == TYPE_ARRAY; }
    bool isString() const { return type_ == TYPE_STRING; }
    bool isNumber() const { return type_ == TYPE_NUMBER; }
    bool isTrue() const { return type_ == TYPE_TRUE; }
    bool isFalse() const { return type_ == TYPE_FALSE; }
    bool isNull() const { return type_ == TYPE_NULL; }

    // serialization
    std::string serialize(size_t indent = DEFAULT_INDENT) const;
    void deserialize(const std::string &in);
    void deserialize(const char *in, size_t n);

    void read(const std::string &fileName);
    void write(const std::string &fileName, size_t indent = DEFAULT_INDENT);

protected:
    /** JSON Data Type. */
    enum Type
    {
        TYPE_OBJECT,
        TYPE_ARRAY,
        TYPE_STRING,
        TYPE_NUMBER,
        TYPE_TRUE,
        TYPE_FALSE,
        TYPE_NULL
    };

    /** JSON Data Value. */
    class Data
    {
    public:
        std::shared_ptr<std::map<std::string, Json>> object;
        std::shared_ptr<std::vector<Json>> array;
        std::shared_ptr<std::string> string;
        double number;
    };

    Type type_;
    Data data_;

    void createObject();
    void createArray();
    void createArray(const std::vector<double> &in);
    void createString(const std::string &in);
    void createNumber(double in);
    void createType(Type t);
    void serialize(std::ostringstream &out) const;
    void serialize(std::ostringstream &out,
                   const std::string &indent,
                   const std::string &indentPlus) const;
    void deserialize(Json &obj, const char *in, size_t n, size_t &i);
};

inline Json::Json()
{
    createType(TYPE_NULL);
}

inline Json::Json(bool in)
{
    createType((in) ? TYPE_TRUE : TYPE_FALSE);
}

inline Json::Json(int32_t in)
{
    createNumber(static_cast<double>(in));
}

inline Json::Json(uint32_t in)
{
    createNumber(static_cast<double>(in));
}

inline Json::Json(double in)
{
    createNumber(in);
}

inline Json::Json(int64_t in)
{
    createNumber(static_cast<double>(in));
}

inline Json::Json(uint64_t in)
{
    createNumber(static_cast<double>(in));
}

inline Json::Json(const char *in)
{
    createString(in);
}

inline Json::Json(const std::string &in)
{
    createString(in);
}

inline Json::Json(const std::vector<double> &in)
{
    createArray(in);
}

inline void Json::createObject()
{
    type_ = TYPE_OBJECT;
    data_.object = std::make_shared<std::map<std::string, Json>>();
}

inline void Json::createArray()
{
    type_ = TYPE_ARRAY;
    data_.array = std::make_shared<std::vector<Json>>();
}

inline void Json::createArray(const std::vector<double> &in)
{
    createArray();
    data_.array->resize(in.size());
    for (size_t i = 0; i < in.size(); i++)
    {
        (*data_.array)[i] = in[i];
    }
}

inline void Json::createString(const std::string &in)
{
    type_ = TYPE_STRING;
    data_.string = std::make_shared<std::string>(in);
}

inline void Json::createNumber(double in)
{
    type_ = TYPE_NUMBER;
    data_.number = in;
}

inline void Json::createType(Type t)
{
    type_ = t;
}

inline bool Json::contains(const std::string &key) const
{
    return isObject() && data_.object->find(key) != data_.object->end();
}

inline bool Json::containsObject(const std::string &key) const
{
    if (isObject())
    {
        auto search = data_.object->find(key);
        if (search != data_.object->end())
        {
            return search->second.isObject();
        }
    }
    return false;
}

inline bool Json::containsArray(const std::string &key) const
{
    if (isObject())
    {
        auto search = data_.object->find(key);
        if (search != data_.object->end())
        {
            return search->second.isArray();
        }
    }
    return false;
}

inline bool Json::containsString(const std::string &key) const
{
    if (isObject())
    {
        auto search = data_.object->find(key);
        if (search != data_.object->end())
        {
            return search->second.isString();
        }
    }
    return false;
}

inline bool Json::containsNumber(const std::string &key) const
{
    if (isObject())
    {
        auto search = data_.object->find(key);
        if (search != data_.object->end())
        {
            return search->second.isNumber();
        }
    }
    return false;
}

inline bool Json::containsBool(const std::string &key) const
{
    if (isObject())
    {
        auto search = data_.object->find(key);
        if (search != data_.object->end())
        {
            return search->second.isTrue() || search->second.isFalse();
        }
    }
    return false;
}

inline Json &Json::operator[](const std::string &key)
{
    if (!isObject())
    {
        createObject();
    }

    auto search = data_.object->find(key);
    if (search != data_.object->end())
    {
        return search->second;
    }

    return (*data_.object)[key];
}

inline const Json &Json::operator[](const std::string &key) const
{
    if (!isObject())
    {
        THROW("JSON value is not object");
    }

    auto search = data_.object->find(key);
    if (search != data_.object->end())
    {
        return search->second;
    }

    THROW("JSON object pair name '" + key + "' not found");
}

inline size_t Json::size() const
{
    if (!isArray())
    {
        THROW("JSON value is not array");
    }

    return data_.array->size();
}

inline Json &Json::operator[](size_t index)
{
    if (!isArray())
    {
        createArray();
    }

    for (size_t i = data_.array->size(); i <= index; i++)
    {
        data_.array->push_back(Json());
    }

    return (*data_.array)[index];
}

inline const Json &Json::operator[](size_t index) const
{
    if (!isArray())
    {
        THROW("JSON value is not array");
    }

    if (index >= data_.array->size())
    {
        THROW("JSON array index is out of range");
    }

    return (*data_.array)[index];
}

inline const std::map<std::string, Json> &Json::object() const
{
    if (!isObject())
    {
        THROW("JSON value is not object");
    }

    return *data_.object;
}

inline const std::vector<Json> &Json::array() const
{
    if (!isArray())
    {
        THROW("JSON value is not array");
    }

    return *data_.array;
}

inline const std::string &Json::string() const
{
    if (!isString())
    {
        THROW("JSON value is not string");
    }

    return *data_.string;
}

inline double Json::number() const
{
    if (!isNumber())
    {
        THROW("JSON value is not number");
    }

    return data_.number;
}

inline uint32_t Json::uint32() const
{
    if (!isNumber())
    {
        THROW("JSON value is not number");
    }

    if (data_.number < 0.)
    {
        THROW("JSON number is out of range");
    }

    return static_cast<uint32_t>(data_.number);
}

inline uint64_t Json::uint64() const
{
    if (!isNumber())
    {
        THROW("JSON value is not number");
    }

    if (data_.number < 0.)
    {
        THROW("JSON number is out of range");
    }

    return static_cast<uint64_t>(data_.number);
}

#endif /* JSON_HPP */
