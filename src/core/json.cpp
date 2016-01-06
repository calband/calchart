/*!
 * @file json.cpp
 * @brief Implements the classes defined in json.h for
 * representing and manipulating a JSON hierarchy.
 */

#include "json.h"

JSONData::JSONData() {
}

std::unique_ptr<JSONData> JSONData::makeNewNumber(double number) {
    return JSONDataNumber::makeNewNumber(number);
}

std::unique_ptr<JSONData> JSONData::makeNewString(std::string string) {
    return JSONDataString::makeNewString(string);
}

std::unique_ptr<JSONData> JSONData::makeNewBoolean(bool boolean) {
    return JSONDataBoolean::makeNewBoolean(boolean);
}

std::unique_ptr<JSONData> JSONData::makeNewNull() {
    return JSONDataNull::makeNewNull();
}

std::unique_ptr<JSONData> JSONData::makeNewObject(std::map<std::string, JSONElement> object) {
    return JSONDataObject::makeNewObject(object);
}

std::unique_ptr<JSONData> JSONData::makeNewArray(std::vector<JSONElement> array) {
    return JSONDataArray::makeNewArray(array);
}

JSONElement::JSONElement()
: JSONElement(makeNull()) {
}

JSONElement::JSONElement(const JSONElement& other)
: JSONElement(other.m_data->newCopy()) {
}

JSONElement::JSONElement(const double& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const int& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const unsigned& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const char* other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const std::string& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const bool& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const std::map<std::string, JSONElement>& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(const std::vector<JSONElement>& other)
: JSONElement() {
    *this = other;
}

JSONElement::JSONElement(std::unique_ptr<JSONData> data)
: m_data(std::move(data)) {
}

const JSONData* JSONElement::data() const {
    return m_data.get();
}

JSONData* JSONElement::data() {
    return m_data.get();
}

JSONElement& JSONElement::operator= (const JSONElement& other) {
    m_data = other.m_data->newCopy();
    return *this;
}

JSONElement& JSONElement::operator= (const double& other) {
    m_data = JSONData::makeNewNumber(other);
    return *this;
}

JSONElement& JSONElement::operator= (const int& other) {
    m_data = JSONData::makeNewNumber((double)other);
    return *this;
}

JSONElement& JSONElement::operator= (const unsigned& other) {
    m_data = JSONData::makeNewNumber((double)other);
    return *this;
}

JSONElement& JSONElement::operator= (const char* other) {
    m_data = JSONData::makeNewString(std::string(other));
    return *this;
}

JSONElement& JSONElement::operator= (const std::string& other) {
    m_data = JSONData::makeNewString(other);
    return *this;
}

JSONElement& JSONElement::operator= (const bool& other) {
    m_data = JSONData::makeNewBoolean(other);
    return *this;
}

JSONElement& JSONElement::operator= (const std::map<std::string, JSONElement>& other) {
    m_data = JSONData::makeNewObject(other);
    return *this;
}

JSONElement& JSONElement::operator= (const std::vector<JSONElement>& other) {
    m_data = JSONData::makeNewArray(other);
    return *this;
}

JSONData::JSONDataType JSONElement::type() const {
    return m_data->type();
}

JSONElement JSONElement::makeNumber(double number) {
    return JSONElement(JSONData::makeNewNumber(number));
}

JSONElement JSONElement::makeString(std::string string) {
    return JSONElement(JSONData::makeNewString(string));
}

JSONElement JSONElement::makeBoolean(bool boolean) {
    return JSONElement(JSONData::makeNewBoolean(boolean));
}

JSONElement JSONElement::makeNull() {
    return JSONElement(JSONData::makeNewNull());
}

JSONElement JSONElement::makeObject(std::map<std::string, JSONElement> object) {
    return JSONElement(JSONData::makeNewObject(object));
}

JSONElement JSONElement::makeArray(std::vector<JSONElement> array) {
    return JSONElement(JSONData::makeNewArray(array));
}

JSONData::JSONDataType JSONDataNumber::type() const {
    return JSONData::JSONDataTypeNumber;
}

std::unique_ptr<JSONData> JSONDataNumber::newCopy() const {
    return std::unique_ptr<JSONData>(new JSONDataNumber(m_val));
}

double JSONDataNumber::value() const {
    return m_val;
}

void JSONDataNumber::setValue(const double& val) {
    m_val = val;
}

void JSONDataNumber::setValue(const JSONDataNumber& val) {
    setValue(val.m_val);
}

JSONDataNumber& JSONDataNumber::operator= (const JSONDataNumber& other) {
    setValue(other);
    return *this;
}

JSONDataNumber& JSONDataNumber::operator= (const double& other) {
    setValue(other);
    return *this;
}

JSONDataNumber::JSONDataNumber(double val)
: m_val(val) {
    
}

std::unique_ptr<JSONDataNumber> JSONDataNumber::makeNewNumber(double number) {
    return std::unique_ptr<JSONDataNumber>(new JSONDataNumber(number));
}

JSONData::JSONDataType JSONDataString::type() const {
    return JSONData::JSONDataTypeString;
}

std::unique_ptr<JSONData> JSONDataString::newCopy() const {
    return std::unique_ptr<JSONData>(new JSONDataString(m_val));
}

std::string JSONDataString::value() const {
    return m_val;
}

void JSONDataString::setValue(const std::string& val) {
    m_val = val;
}

void JSONDataString::setValue(const JSONDataString& val) {
    setValue(val.m_val);
}

JSONDataString& JSONDataString::operator= (const JSONDataString& other) {
    setValue(other);
    return *this;
}

JSONDataString& JSONDataString::operator= (const std::string& other) {
    setValue(other);
    return *this;
}

JSONDataString::JSONDataString(std::string val)
: m_val(val) {
    
}

std::unique_ptr<JSONDataString> JSONDataString::makeNewString(std::string string) {
    return std::unique_ptr<JSONDataString>(new JSONDataString(string));
}

JSONData::JSONDataType JSONDataBoolean::type() const {
    return JSONData::JSONDataTypeBoolean;
}

std::unique_ptr<JSONData> JSONDataBoolean::newCopy() const {
    return std::unique_ptr<JSONData>(new JSONDataBoolean(m_val));
}

bool JSONDataBoolean::value() const {
    return m_val;
}

void JSONDataBoolean::setValue(const bool& val) {
    m_val = val;
}

void JSONDataBoolean::setValue(const JSONDataBoolean& val) {
    setValue(val.m_val);
}

JSONDataBoolean& JSONDataBoolean::operator= (const JSONDataBoolean& other) {
    setValue(other);
    return *this;
}

JSONDataBoolean& JSONDataBoolean::operator= (const bool& other) {
    setValue(other);
    return *this;
}

JSONDataBoolean::JSONDataBoolean(bool val)
: m_val(val) {
}

std::unique_ptr<JSONDataBoolean> JSONDataBoolean::makeNewBoolean(bool boolean) {
    return std::unique_ptr<JSONDataBoolean>(new JSONDataBoolean(boolean));
}

JSONData::JSONDataType JSONDataNull::type() const {
    return JSONData::JSONDataTypeNull;
}

std::unique_ptr<JSONData> JSONDataNull::newCopy() const {
    return std::unique_ptr<JSONData>(new JSONDataNull());
}

JSONDataNull::JSONDataNull() {
}

std::unique_ptr<JSONDataNull> JSONDataNull::makeNewNull() {
    return std::unique_ptr<JSONDataNull>(new JSONDataNull());
}

JSONData::JSONDataType JSONDataObject::type() const {
    return JSONData::JSONDataTypeObject;
}

std::unique_ptr<JSONData> JSONDataObject::newCopy() const {
    return std::unique_ptr<JSONData>(new JSONDataObject(m_elements));
}

unsigned long JSONDataObject::size() const {
    return m_elements.size();
}

JSONElement JSONDataObject::valueForKey(std::string key) const {
    return m_elements.at(key);
}

std::vector<std::pair<const std::string&, const JSONElement&>> JSONDataObject::items() const {
    std::vector<std::pair<const std::string&, const JSONElement&>> items;
    for (auto iter = itemsBegin(); iter != itemsEnd(); iter++) {
        items.push_back(*iter);
    }
    return items;
}

std::map<std::string, JSONElement>::const_iterator JSONDataObject::itemsBegin() const {
    return m_elements.cbegin();
}

std::map<std::string, JSONElement>::const_iterator JSONDataObject::itemsEnd() const {
    return m_elements.cend();
}

JSONDataObject::JSONDataObject(std::map<std::string, JSONElement> elements)
: m_elements(elements) {
}

std::unique_ptr<JSONDataObject> JSONDataObject::makeNewObject(std::map<std::string, JSONElement> object) {
    return std::unique_ptr<JSONDataObject>(new JSONDataObject(object));
}

void JSONDataObject::setValueForKey(const std::string& key, JSONElement& element) {
    m_elements[key] = element;
}

void JSONDataObject::erase(const std::string& key) {
    m_elements.erase(key);
}

void JSONDataObject::setValue(const JSONDataObject& other) {
    setValue(other.m_elements);
}

void JSONDataObject::setValue(const std::map<std::string, JSONElement>& other) {
    m_elements = other;
}

JSONDataObject& JSONDataObject::operator= (const JSONDataObject& other) {
    setValue(other);
    return *this;
}

JSONDataObject& JSONDataObject::operator= (const std::map<std::string, JSONElement>& other) {
    setValue(other);
    return *this;
}

const JSONElement& JSONDataObject::operator[] (const std::string& key) const {
    return m_elements.at(key);
}

JSONElement& JSONDataObject::operator[] (const std::string& key) {
    return m_elements[key];
}

JSONData::JSONDataType JSONDataArray::type() const {
    return JSONData::JSONDataTypeArray;
}

std::unique_ptr<JSONData> JSONDataArray::newCopy() const {
    return std::unique_ptr<JSONData>(new JSONDataArray(m_elements));
}

unsigned long JSONDataArray::size() const {
    return m_elements.size();
}

JSONElement JSONDataArray::valueAtIndex(unsigned i) const {
    return m_elements.at(i);
}

JSONElement& JSONDataArray::back() {
    return m_elements.back();
}

std::vector<std::reference_wrapper<const JSONElement>> JSONDataArray::values() const {
    std::vector<std::reference_wrapper<const JSONElement>> valuesVector;
    for (auto iter = valuesBegin(); iter != valuesEnd(); iter++) {
        valuesVector.push_back(std::ref(*iter));
    }
    return valuesVector;
}

std::vector<JSONElement>::const_iterator JSONDataArray::valuesBegin() const {
    return m_elements.cbegin();
}

std::vector<JSONElement>::const_iterator JSONDataArray::valuesEnd() const {
    return m_elements.cend();
}

JSONDataArray::JSONDataArray(std::vector<JSONElement> elements)
: m_elements(elements) {
}

std::unique_ptr<JSONDataArray> JSONDataArray::makeNewArray(std::vector<JSONElement> array) {
    return std::unique_ptr<JSONDataArray>(new JSONDataArray(array));
}

void JSONDataArray::insertValueAtIndex(unsigned i, const JSONElement& element) {
    m_elements.insert(m_elements.begin() + i, element);
}

void JSONDataArray::eraseValueAtIndex(unsigned i) {
    m_elements.erase(m_elements.begin() + i);
}

void JSONDataArray::pushBack(const JSONElement& element) {
    m_elements.push_back(element);
}

void JSONDataArray::setValue(const JSONDataArray& other) {
    setValue(other.m_elements);
}

void JSONDataArray::setValue(const std::vector<JSONElement>& other) {
    m_elements = other;
}

JSONDataArray& JSONDataArray::operator= (const JSONDataArray& other) {
    setValue(other);
    return *this;
}

JSONDataArray& JSONDataArray::operator= (const std::vector<JSONElement>& other) {
    setValue(other);
    return *this;
}

const JSONElement& JSONDataArray::operator[] (int index) const {
    return m_elements[index];
}

JSONElement& JSONDataArray::operator[] (int index) {
    return m_elements[index];
}

JSONDataNumberAccessor& JSONDataNumberAccessor::operator= (const JSONDataNumber& other) {
    dereference() = other;
    return *this;
}

JSONDataNumberAccessor& JSONDataNumberAccessor::operator= (const double& other) {
    dereference() = other;
    return *this;
}

JSONDataStringAccessor& JSONDataStringAccessor::operator= (const JSONDataString& other) {
    dereference() = other;
    return *this;
}

JSONDataStringAccessor& JSONDataStringAccessor::operator= (const std::string& other) {
    dereference() = other;
    return *this;
}

JSONDataBooleanAccessor& JSONDataBooleanAccessor::operator= (const JSONDataBoolean& other) {
    dereference() = other;
    return *this;
}

JSONDataBooleanAccessor& JSONDataBooleanAccessor::operator= (const bool& other) {
    dereference() = other;
    return *this;
}

JSONDataArrayAccessor& JSONDataArrayAccessor::operator= (const JSONDataArray& other) {
    dereference() = other;
    return *this;
}

JSONDataArrayAccessor& JSONDataArrayAccessor::operator= (const std::vector<JSONElement>& other) {
    dereference() = other;
    return *this;
}

const JSONElement& JSONDataArrayAccessor::operator[] (int index) const {
    return dereference()[index];
}

JSONElement& JSONDataArrayAccessor::operator[] (int index) {
    return dereference()[index];
}

const JSONElement& JSONDataArrayConstAccessor::operator[] (int index) const {
    return dereference()[index];
}

JSONDataObjectAccessor& JSONDataObjectAccessor::operator= (const JSONDataObject& other) {
    dereference() = other;
    return *this;
}

JSONDataObjectAccessor& JSONDataObjectAccessor::operator= (const std::map<std::string, JSONElement>& other) {
    dereference() = other;
    return *this;
}

const JSONElement& JSONDataObjectAccessor::operator[] (const std::string& key) const {
    return dereference()[key];
}

JSONElement& JSONDataObjectAccessor::operator[] (const std::string& key) {
    return dereference()[key];
}

const JSONElement& JSONDataObjectConstAccessor::operator[] (const std::string& key) const {
    return dereference()[key];
}


