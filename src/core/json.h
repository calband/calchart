//
//  json.hpp
//  CalChart
//
//  Created by Kevin Durand on 11/16/15.
//
//

#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>

class JSONElement;
class JSONData;
class JSONDataNumber;
class JSONDataString;
class JSONDataBoolean;
class JSONDataNull;
class JSONDataObject;
class JSONDataArray;

/*!
 @class      JSONData
 @abstract   A JSON file is represented as a hierarchical
 structure of JSONElement objects that wrap
 JSONData objects. JSONData objects come in
 different flavors depending on the kind of data
 that is being stored. For example, numbers
 (floating point and integer alike) are stored
 within JSONDataNumber objects; strings, however, are
 stored within JSONDataString objects.
 JSONDataObjects are generally created on the heap,
 and are accessed through shared_ptr objects.
 */
class JSONData {
public:
    enum JSONDataType {
        JSONDataTypeNumber,
        JSONDataTypeString,
        JSONDataTypeBoolean,
        JSONDataTypeNull,
        JSONDataTypeArray,
        JSONDataTypeObject
    };
    
    /*!
     @method type
     @abstract Returns the type of data contained within this JSONData object.
     */
    virtual JSONDataType type() const = 0;
protected:
    JSONData(); // Don't let the user create JSONData objects, except through JSONElement
    
    /*!
     @method makeNewNumber
     @abstract Creates a new JSONData object to represent the specified double value.
     @param number The value to represent with a JSONData object.
     */
    static std::shared_ptr<JSONData> makeNewNumber(double number);
    
    /*!
     @method makeNewString
     @abstract Creates a new JSONData object to represent the specified string value.
     @param string The value to represent with a JSONData object.
     */
    static std::shared_ptr<JSONData> makeNewString(std::string string);
    
    /*!
     @method makeNewString
     @abstract Creates a new JSONData object to represent the specified boolean value.
     @param boolean The value to represent with a JSONData object.
     */
    static std::shared_ptr<JSONData> makeNewBoolean(bool boolean);
    
    /*!
     @method makeNewNull
     @abstract Creates a new JSONData object to represent a 'null' value.
     */
    static std::shared_ptr<JSONData> makeNewNull();
    
    /*!
     @method makeNewObject
     @abstract Creates a new JSONData object to represent a JSON object consisting of the key-value pairs contained within the provided map.
     @param object A collection of key-value pairs that will be inserted into the new JSONData object.
     */
    static std::shared_ptr<JSONData> makeNewObject(std::map<std::string, JSONElement> object);
    
    /*!
     @method makeNewArray
     @abstract Creates a new JSONData object to represent a JSON array containing the provided values.
     @param array
     */
    static std::shared_ptr<JSONData> makeNewArray(std::vector<JSONElement> array);
    
    /*!
     @method newCopy
     @abstract Returns a pointer to a new deep copy of this JSONData object.
     */
    virtual std::shared_ptr<JSONData> newCopy() const = 0;
protected:
    /*!
     @method isAuthorizedToEdit
     @abstract Multiple JSONElement objects can read this data, but only one can edit.
     This method will check whether a particular JSONElement is authorized to edit the
     content of this JSONData; if not, then the JSONElement will need to make a copy of
     the JSONData to edit instead. Whichever JSONElement first attempts to access the 
     edit methods or expose a mutable copy of this JSONData will become authorized to
     edit it.
     @return True if the JSONElement can edit this JSONData object; false otherwise.
     */
    bool isAuthorizedToEdit(const JSONElement* element);

private:
    /*!
     @abstract The only JSONElement object which is authorized to modify the content of
     this JSONData object directly. Others must make copies of this JSONData before editing.
     */
    const JSONElement* m_authorizedElement;
    
    friend class JSONElement; // JSONElement is a friend class so that it can have access to the makeNew mand newCopy methods
};


class JSONElement {
public:
    JSONElement();
    JSONElement(const JSONElement& other);
    
    static JSONElement makeNumber(double number = 0);
    static JSONElement makeString(std::string string = "");
    static JSONElement makeBoolean(bool boolean = false);
    static JSONElement makeNull();
    static JSONElement makeObject(std::map<std::string, JSONElement> object = std::map<std::string, JSONElement>());
    static JSONElement makeArray(std::vector<JSONElement> array = std::vector<JSONElement>());
    
    JSONData::JSONDataType type() const;
    
    JSONElement& operator= (const JSONElement& other);
    JSONElement& operator= (const double& other);
    JSONElement& operator= (const int& other);
    JSONElement& operator= (const unsigned& other);
    JSONElement& operator= (const char* other);
    JSONElement& operator= (const std::string& other);
    JSONElement& operator= (const bool& other);
    JSONElement& operator= (const std::map<std::string, JSONElement>& other);
    JSONElement& operator= (const std::vector<JSONElement>& other);
    
protected:
    JSONElement(std::shared_ptr<JSONData> data);
    
    std::weak_ptr<const JSONData> data() const;
    std::weak_ptr<const JSONDataNumber> dataAsNumber() const;
    std::weak_ptr<const JSONDataString> dataAsString() const;
    std::weak_ptr<const JSONDataBoolean> dataAsBoolean() const;
    std::weak_ptr<const JSONDataNull> dataAsNull() const;
    std::weak_ptr<const JSONDataObject> dataAsObject() const;
    std::weak_ptr<const JSONDataArray> dataAsArray() const;
    std::weak_ptr<JSONData> data();
    std::weak_ptr<JSONDataNumber> dataAsNumber();
    std::weak_ptr<JSONDataString> dataAsString();
    std::weak_ptr<JSONDataBoolean> dataAsBoolean();
    std::weak_ptr<JSONDataNull> dataAsNull();
    std::weak_ptr<JSONDataObject> dataAsObject();
    std::weak_ptr<JSONDataArray> dataAsArray();
private:
    std::shared_ptr<JSONData> m_data;
    
    friend class JSONElementFriend;
};

/*!
 @class      JSONDataNumber
 @abstract   A number value in a JSON hierarchy. A number may
 be a float or an integer.
 */
class JSONDataNumber : public JSONData {
public:
    virtual JSONData::JSONDataType type() const;
    
    double value() const;
    void setValue(const double& val);
    void setValue(const JSONDataNumber& val);
    JSONDataNumber& operator= (const JSONDataNumber& other);
    JSONDataNumber& operator= (const double& other);
protected:
    JSONDataNumber(double val);
    static std::shared_ptr<JSONDataNumber> makeNewNumber(double number);
    virtual std::shared_ptr<JSONData> newCopy() const;
private:
    double m_val;
    
    friend class JSONData;
};

/*!
 @class      JSONDataString
 @abstract   A string value in a JSON hierarchy.
 */
class JSONDataString : public JSONData {
public:
    virtual JSONData::JSONDataType type() const;
    
    std::string value() const;
    void setValue(const std::string& val);
    void setValue(const JSONDataString& val);
    JSONDataString& operator= (const JSONDataString& other);
    JSONDataString& operator= (const std::string& other);
protected:
    JSONDataString(std::string val);
    static std::shared_ptr<JSONDataString> makeNewString(std::string string);
    virtual std::shared_ptr<JSONData> newCopy() const;
private:
    std::string m_val;
    
    friend class JSONData;
};

/*!
 @class      JSONDataBoolean
 @abstract   A boolean value in a JSON hierarchy.
 */
class JSONDataBoolean : public JSONData {
public:
    virtual JSONData::JSONDataType type() const;

    bool value() const;
    void setValue(const bool& val);
    void setValue(const JSONDataBoolean& val);
    JSONDataBoolean& operator= (const JSONDataBoolean& other);
    JSONDataBoolean& operator= (const bool& other);
protected:
    JSONDataBoolean(bool val);
    static std::shared_ptr<JSONDataBoolean> makeNewBoolean(bool boolean);
    virtual std::shared_ptr<JSONData> newCopy() const;
private:
    bool m_val;
    
    friend class JSONData;
};

/*!
 @class      JSONDataNull
 @abstract   A null value in a JSON hierarchy.
 */
class JSONDataNull : public JSONData {
public:
    virtual JSONData::JSONDataType type() const;
protected:
    JSONDataNull();
    static std::shared_ptr<JSONDataNull> makeNewNull();
    virtual std::shared_ptr<JSONData> newCopy() const;
private:
    friend class JSONData;
};

/*!
 @class      JSONDataObject
 @abstract   An object in a JSON hierarchy. Objects maps string
 names to values (that is, they effectively function
 like as std::maps which map strings to JSONElements).
 */
class JSONDataObject : public JSONData {
public:
    virtual JSONData::JSONDataType type() const;
    
    unsigned long size() const;
    
    JSONElement valueForKey(std::string key) const;
    
    std::vector<std::pair<std::string, JSONElement>> items() const;
    std::map<std::string, JSONElement>::const_iterator itemsBegin() const;
    std::map<std::string, JSONElement>::const_iterator itemsEnd() const;
    
    void setValueForKey(const std::string& key, JSONElement& element);
    void erase(const std::string& key);
    void setValue(const JSONDataObject& other);
    void setValue(const std::map<std::string, JSONElement>& other);
    JSONDataObject& operator= (const JSONDataObject& other);
    JSONDataObject& operator= (const std::map<std::string, JSONElement>& other);
    const JSONElement& operator[] (const std::string& key) const;
    JSONElement& operator[] (const std::string& key);
protected:
    JSONDataObject(std::map<std::string, JSONElement> elements);
    static std::shared_ptr<JSONDataObject> makeNewObject(std::map<std::string, JSONElement> object);
    virtual std::shared_ptr<JSONData> newCopy() const;
private:
    std::map<std::string, JSONElement> m_elements;
    
    friend class JSONData;
};

/*!
 @class      JSONDataArray
 @abstract   An array in a JSON hierarchy.
 */
class JSONDataArray : public JSONData {
public:
    virtual JSONData::JSONDataType type() const;
    
    unsigned long size() const;
    
    JSONElement valueAtIndex(unsigned i) const;
    
    std::vector<JSONElement> values() const;
    std::vector<JSONElement>::const_iterator valuesBegin() const;
    std::vector<JSONElement>::const_iterator valuesEnd() const;
    
    void insertValueAtIndex(unsigned i, const JSONElement& element);
    void eraseValueAtIndex(unsigned i);
    void pushBack(const JSONElement& element);
    void setValue(const JSONDataArray& other);
    void setValue(const std::vector<JSONElement>& other);
    JSONDataArray& operator= (const JSONDataArray& other);
    JSONDataArray& operator= (const std::vector<JSONElement>& other);
    const JSONElement& operator[] (int index) const;
    JSONElement& operator[] (int index);
    JSONElement& back();
protected:
    JSONDataArray(std::vector<JSONElement> elements);
    static std::shared_ptr<JSONDataArray> makeNewArray(std::vector<JSONElement> array);
    virtual std::shared_ptr<JSONData> newCopy() const;
private:
    std::vector<JSONElement> m_elements;
    
    friend class JSONData;
};

class JSONElementFriend {
protected:
    std::weak_ptr<const JSONData> fetchDataFromElement(const JSONElement& element) const {
        return element.data();
    }
    
    std::weak_ptr<JSONData> fetchDataFromElement(JSONElement& element) {
        return element.data();
    }
};

template <class DataClass>
class JSONDataAccessorBase : public JSONElementFriend {
public:
    
    const DataClass* operator-> () const {
        return dataPointer();
    }
    
    const DataClass& operator* () const {
        return dereference();
    }
    
protected:
    
    const DataClass* dataPointer() const {
        return std::dynamic_pointer_cast<const DataClass>(this->fetchDataFromElement(element()).lock()).get();
    }
    
    const DataClass& dereference() const {
        return *dataPointer();
    }
    
    virtual const JSONElement& element() const = 0;
};

template <class DataClass>
class JSONDataAccessor : public JSONDataAccessorBase<DataClass> {
private:
    using super = JSONDataAccessorBase<DataClass>;
public:
    JSONDataAccessor(JSONElement& element)
    : m_element(&element)
    {}
    
    JSONDataAccessor()
    {}
    
    using super::operator->;
    
    DataClass* operator-> () {
        return dataPointer();
    }

protected:

    using super::dataPointer;
    
    DataClass* dataPointer() {
        return std::dynamic_pointer_cast<DataClass>(this->fetchDataFromElement(element()).lock()).get();
    }
    
    using super::dereference;
    
    DataClass& dereference() {
        return *dataPointer();
    }
    
    JSONElement& element() {
        return *m_element;
    };
    
    const JSONElement& element() const {
        return *m_element;
    };
    
    
private:
    JSONElement* m_element;
};


template <class DataClass>
class JSONDataConstAccessor : public JSONDataAccessorBase<DataClass> {
public:
    JSONDataConstAccessor(const JSONElement& element)
    : m_element(&element)
    {}
    
    JSONDataConstAccessor()
    {}
    
protected:
    
    const JSONElement& element() const {
        return *m_element;
    };
    
private:
    const JSONElement* m_element;
};

class JSONDataNumberAccessor : public JSONDataAccessor<JSONDataNumber> {
public:
    using JSONDataAccessor<JSONDataNumber>::JSONDataAccessor;
    
    JSONDataNumberAccessor& operator= (const JSONDataNumber& other);
    JSONDataNumberAccessor& operator= (const double& other);
};

class JSONDataNumberConstAccessor : public JSONDataConstAccessor<JSONDataNumber> {
public:
    using JSONDataConstAccessor<JSONDataNumber>::JSONDataConstAccessor;
};

class JSONDataStringAccessor : public JSONDataAccessor<JSONDataString> {
public:
    using JSONDataAccessor<JSONDataString>::JSONDataAccessor;
    
    JSONDataStringAccessor& operator= (const JSONDataString& other);
    JSONDataStringAccessor& operator= (const std::string& other);
};

class JSONDataStringConstAccessor : public JSONDataConstAccessor<JSONDataString> {
public:
    using JSONDataConstAccessor<JSONDataString>::JSONDataConstAccessor;
};

class JSONDataBooleanAccessor : public JSONDataAccessor<JSONDataBoolean> {
public:
    using JSONDataAccessor<JSONDataBoolean>::JSONDataAccessor;
    
    JSONDataBooleanAccessor& operator= (const JSONDataBoolean& other);
    JSONDataBooleanAccessor& operator= (const bool& other);
};

class JSONDataBooleanConstAccessor : public JSONDataConstAccessor<JSONDataBoolean> {
public:
    using JSONDataConstAccessor<JSONDataBoolean>::JSONDataConstAccessor;
};

class JSONDataNullAccessor : public JSONDataAccessor<JSONDataNull> {
public:
    using JSONDataAccessor<JSONDataNull>::JSONDataAccessor;
};

class JSONDataNullConstAccessor : public JSONDataConstAccessor<JSONDataNull> {
public:
    using JSONDataConstAccessor<JSONDataNull>::JSONDataConstAccessor;
};

class JSONDataArrayAccessor : public JSONDataAccessor<JSONDataArray> {
public:
    using JSONDataAccessor<JSONDataArray>::JSONDataAccessor;
    
    JSONDataArrayAccessor& operator= (const JSONDataArray& other);
    JSONDataArrayAccessor& operator= (const std::vector<JSONElement>& other);
    const JSONElement& operator[] (int index) const;
    JSONElement& operator[] (int index);
};

class JSONDataArrayConstAccessor : public JSONDataConstAccessor<JSONDataArray> {
public:
    using JSONDataConstAccessor<JSONDataArray>::JSONDataConstAccessor;
    
    const JSONElement& operator[] (int index) const;
};

class JSONDataObjectAccessor : public JSONDataAccessor<JSONDataObject> {
public:
    using JSONDataAccessor<JSONDataObject>::JSONDataAccessor;
    
    JSONDataObjectAccessor& operator= (const JSONDataObject& other);
    JSONDataObjectAccessor& operator= (const std::map<std::string, JSONElement>& other);
    const JSONElement& operator[] (const std::string& key) const;
    JSONElement& operator[] (const std::string& key);
};

class JSONDataObjectConstAccessor : public JSONDataConstAccessor<JSONDataObject> {
private:
public:
    using JSONDataConstAccessor<JSONDataObject>::JSONDataConstAccessor;
    
    const JSONElement& operator[] (const std::string& key) const;
};


