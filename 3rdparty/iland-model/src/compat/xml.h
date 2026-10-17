#pragma once
#include "io.h"
#include <pugixml.hpp>
class QDomDocument;
class QDomNode {
protected:
  std::shared_ptr<pugi::xml_document> doc_;
  pugi::xml_node node_;

public:
  QDomNode() = default;
  QDomNode(std::shared_ptr<pugi::xml_document> d, pugi::xml_node n)
      : doc_(std::move(d)), node_(n) {}
  bool isNull() const { return !node_; }
  bool isText() const {
    return node_.type() == pugi::node_pcdata ||
           node_.type() == pugi::node_cdata;
  }
  bool isElement() const { return node_.type() == pugi::node_element; }
  bool hasChildNodes() const { return bool(node_.first_child()); }
  QString nodeName() const { return node_.name(); }
  QString nodeValue() const { return node_.value(); }
  QDomNode firstChild() const { return {doc_, node_.first_child()}; }
  QDomNode nextSibling() const { return {doc_, node_.next_sibling()}; }
  QDomNode parentNode() const { return {doc_, node_.parent()}; }
  QDomNode toElement() const { return isElement() ? *this : QDomNode(); }
  QDomNode toText() const { return *this; }
  void setData(const QString &s) { node_.set_value(s.toUtf8().constData()); }
  void setNodeValue(const QString &s) { setData(s); }
  QDomNode firstChildElement(const QString &name = {}) const {
    auto n = node_.first_child();
    while (n && (n.type() != pugi::node_element ||
                 (!name.isEmpty() && QString(n.name()) != name)))
      n = n.next_sibling();
    return {doc_, n};
  }
  QDomNode nextSiblingElement(const QString &name = {}) const {
    auto n = node_.next_sibling();
    while (n && (n.type() != pugi::node_element ||
                 (!name.isEmpty() && QString(n.name()) != name)))
      n = n.next_sibling();
    return {doc_, n};
  }
  QString text() const {
    QString s;
    std::function<void(pugi::xml_node)> walk = [&](auto n) {
      for (auto c : n.children()) {
        if (c.type() == pugi::node_pcdata || c.type() == pugi::node_cdata)
          s += QString(c.value());
        else
          walk(c);
      }
    };
    walk(node_);
    return s;
  }
  QDomNode appendChild(const QDomNode &n) {
    if (!node_ || !n.node_)
      return {};
    return {doc_, node_.append_copy(n.node_)};
  }
  QDomNode removeChild(const QDomNode &n) {
    node_.remove_child(n.node_);
    return n;
  }
  QString attribute(const QString &n, const QString &def = {}) const {
    auto a = node_.attribute(n.toUtf8().constData());
    return a ? QString(a.value()) : def;
  }
  bool hasAttribute(const QString &n) const {
    return bool(node_.attribute(n.toUtf8().constData()));
  }
  void setAttribute(const QString &n, const QString &v) {
    auto a = node_.attribute(n.toUtf8().constData());
    if (!a)
      a = node_.append_attribute(n.toUtf8().constData());
    a.set_value(v.toUtf8().constData());
  }
};
using QDomElement = QDomNode;
class QDomDocument {
  std::shared_ptr<pugi::xml_document> doc_ =
      std::make_shared<pugi::xml_document>();

public:
  void clear() { doc_ = std::make_shared<pugi::xml_document>(); }
  bool setContent(const QString &s, QString *error = nullptr,
                  int *line = nullptr, int *column = nullptr) {
    auto utf = s.toUtf8();
    auto r = doc_->load_buffer(utf.data(), utf.size(), pugi::parse_default);
    if (!r) {
      if (error)
        *error = r.description();
      if (line)
        *line =
            1 +
            static_cast<int>(std::count(
                utf.begin(),
                utf.begin() +
                    std::min(utf.size(), static_cast<size_t>(
                                             std::max<ptrdiff_t>(0, r.offset))),
                '\n'));
      if (column)
        *column = 1;
    }
    return bool(r);
  }
  QDomElement documentElement() const {
    return {doc_, doc_->document_element()};
  }
  QDomElement createElement(const QString &n) {
    auto temp = std::make_shared<pugi::xml_document>();
    return {temp, temp->append_child(n.toUtf8().constData())};
  }
  QDomNode createTextNode(const QString &s) {
    auto temp = std::make_shared<pugi::xml_document>();
    auto n = temp->append_child(pugi::node_pcdata);
    n.set_value(s.toUtf8().constData());
    return {temp, n};
  }
  void save(QTextStream &s, int indent) const {
    std::ostringstream out;
    std::string pad(std::max(0, indent), ' ');
    doc_->save(out, pad.c_str());
    s << out.str();
  }
};
