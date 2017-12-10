#pragma once
/*!
 * @file json.h
 * @brief Defines classes that are used to represent
 * and manipulate a JSON hierarchy.
 */

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <functional>

namespace CalChart {

class JSONElement;
class JSONData;
class JSONDataNumber;
class JSONDataString;
class JSONDataBoolean;
class JSONDataNull;
class JSONDataObject;
class JSONDataArray;

/*!
 * @brief Contains the content of a JSONElement.
 * @details JSONData objects can have one of several types 
 * (e.g. Number, String, Boolean, etc.), and each type is
 * associated with a value in the JSONData::JSONDataType
 * enumeration. The `type()` method can be used to determine
 * the type of the JSONData object. JSONData objects should
 * never be accessed directly; instead, they should be accessed
 * through JSONDataAccessor and JSONDataConstAccessor objects.
 */
class JSONData {
public:
    /*
     * @brief An enumeration which identifies each type of
     * JSONData.
     */
    enum JSONDataType {
        JSONDataTypeNumber,
        JSONDataTypeString,
        JSONDataTypeBoolean,
        JSONDataTypeNull,
        JSONDataTypeArray,
        JSONDataTypeObject
    };

    /*!
     * @brief Returns the type of data contained within this
     * JSONData object.
     */
    virtual JSONDataType type() const = 0;

protected:
    /*!
     * @brief Constructor.
     * @details Note that this constructor is protected.
     * JSONData objects cannot be created freely, since each
     * of them must be coupled uniquely with a JSONElement.
     * Consequently, JSONData objects are created indirectly
     * by creating JSONElement objects.
     */
    JSONData();

    /*!
     * @brief Creates a new JSONData object to represent the
     * specified double value.
     * @param number The value to represent with a JSONData object.
     */
    static std::unique_ptr<JSONData> makeNewNumber(double number);

    /*!
     * @brief Creates a new JSONData object to represent the 
     * specified string value.
     * @param string The value to represent with a JSONData object.
     */
    static std::unique_ptr<JSONData> makeNewString(std::string string);

    /*!
     * @brief Creates a new JSONData object to represent the
     * specified boolean value.
     * @param boolean The value to represent with a JSONData object.
     */
    static std::unique_ptr<JSONData> makeNewBoolean(bool boolean);

    /*!
     * @brief Creates a new JSONData object to represent a
     * 'null' value.
     */
    static std::unique_ptr<JSONData> makeNewNull();

    /*!
     * @brief Creates a new JSONData object to represent a
     * JSON object consisting of the key-value pairs contained
     * within the provided map.
     * @param object A collection of key-value pairs that will
     * be inserted into the new JSONData object.
     */
    static std::unique_ptr<JSONData> makeNewObject(std::map<std::string, JSONElement> object);

    /*!
     * @brief Creates a new JSONData object to represent a JSON
     * array containing the provided values.
     * @param array Array to be created
     */
    static std::unique_ptr<JSONData> makeNewArray(std::vector<JSONElement> array);

    /*!
     * @brief Returns a pointer to a new, deep copy of this
     * JSONData object.
     */
    virtual std::unique_ptr<JSONData> newCopy() const = 0;

private:
    // JSONElement is a friend class so that it can have access
    // to the makeNew mand newCopy methods
    friend class JSONElement;
};

/*!
 * @brief An element in a JSON hierarchy.
 * @details JSON elements can contain several different types
 * of data (e.g. Number, String, Boolean, etc.). A hierarchy
 * can be constructed by composing a JSONElement from other 
 * JSONElements, as is the case when a JSONElement is an 'object'
 * or an 'array'. JSONElement objects can be created in a variety
 * of ways: you can use the class's static 'make' methods
 * (e.g. makeNumber, makeString, ...), or you can initialize
 * using the `=` operator (e.g. element = 4; element = "string", ...).
 * Note that the `=` operator will completely overwrite the content 
 * of the JSONElement with something new; if you would like to
 * manipulate the existing content (e.g. to add values to an array),
 * use a JSONDataAccessor. JSONDataAccessor and JSONDataConstAccessor
 * objects are also used to read the content of a JSONElement.
 */
class JSONElement {
public:
    JSONElement();
    JSONElement(const JSONElement& other);
    JSONElement(const double& other);
    JSONElement(const int& other);
    JSONElement(const unsigned& other);
    JSONElement(const char* other);
    JSONElement(const std::string& other);
    JSONElement(const bool& other);
    JSONElement(const std::map<std::string, JSONElement>& other);
    JSONElement(const std::vector<JSONElement>& other);

    static JSONElement makeNumber(double number = 0);
    static JSONElement makeString(std::string string = "");
    static JSONElement makeBoolean(bool boolean = false);
    static JSONElement makeNull();
    static JSONElement makeObject(std::map<std::string, JSONElement> object = std::map<std::string, JSONElement>());
    static JSONElement makeArray(std::vector<JSONElement> array = std::vector<JSONElement>());

    /*!
     * @brief Returns the type of data contained within this
     * element.
     * @return The type of data within this element.
     */
    JSONData::JSONDataType type() const;

    JSONElement& operator=(const JSONElement& other);
    JSONElement& operator=(const double& other);
    JSONElement& operator=(const int& other);
    JSONElement& operator=(const unsigned& other);
    JSONElement& operator=(const char* other);
    JSONElement& operator=(const std::string& other);
    JSONElement& operator=(const bool& other);
    JSONElement& operator=(const std::map<std::string, JSONElement>& other);
    JSONElement& operator=(const std::vector<JSONElement>& other);

protected:
    JSONElement(std::unique_ptr<JSONData> data);

    const JSONData* data() const;
    JSONData* data();

private:
    std::unique_ptr<JSONData> m_data;

    // Objects that need direct access to the JSONData within a JSONElement
    // should inherit from JSONElementFriend
    friend class JSONElementFriend;
};

/*!
 @class      JSONDataNumber
 @abstract   A number value in a JSON hierarchy. A number may
 be a float or an integer.
 */
class JSONDataNumber : public JSONData {
public:
    JSONData::JSONDataType type() const;

    double value() const;
    void setValue(const double& val);
    void setValue(const JSONDataNumber& val);
    JSONDataNumber& operator=(const JSONDataNumber& other);
    JSONDataNumber& operator=(const double& other);

protected:
    JSONDataNumber(double val);
    static std::unique_ptr<JSONDataNumber> makeNewNumber(double number);
    std::unique_ptr<JSONData> newCopy() const;

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
    JSONData::JSONDataType type() const;

    std::string value() const;
    void setValue(const std::string& val);
    void setValue(const JSONDataString& val);
    JSONDataString& operator=(const JSONDataString& other);
    JSONDataString& operator=(const std::string& other);

protected:
    JSONDataString(std::string val);
    static std::unique_ptr<JSONDataString> makeNewString(std::string string);
    std::unique_ptr<JSONData> newCopy() const;

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
    JSONData::JSONDataType type() const;

    bool value() const;
    void setValue(const bool& val);
    void setValue(const JSONDataBoolean& val);
    JSONDataBoolean& operator=(const JSONDataBoolean& other);
    JSONDataBoolean& operator=(const bool& other);

protected:
    JSONDataBoolean(bool val);
    static std::unique_ptr<JSONDataBoolean> makeNewBoolean(bool boolean);
    std::unique_ptr<JSONData> newCopy() const;

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
    JSONData::JSONDataType type() const;

protected:
    JSONDataNull();
    static std::unique_ptr<JSONDataNull> makeNewNull();
    std::unique_ptr<JSONData> newCopy() const;

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
    JSONData::JSONDataType type() const;

    unsigned long size() const;

    JSONElement valueForKey(std::string key) const;

    std::vector<std::pair<const std::string&, const JSONElement&> > items() const;
    std::map<std::string, JSONElement>::const_iterator itemsBegin() const;
    std::map<std::string, JSONElement>::const_iterator itemsEnd() const;

    void setValueForKey(const std::string& key, JSONElement& element);
    void erase(const std::string& key);
    void setValue(const JSONDataObject& other);
    void setValue(const std::map<std::string, JSONElement>& other);
    JSONDataObject& operator=(const JSONDataObject& other);
    JSONDataObject& operator=(const std::map<std::string, JSONElement>& other);
    const JSONElement& operator[](const std::string& key) const;
    JSONElement& operator[](const std::string& key);

protected:
    JSONDataObject(std::map<std::string, JSONElement> elements);
    static std::unique_ptr<JSONDataObject> makeNewObject(std::map<std::string, JSONElement> object);
    std::unique_ptr<JSONData> newCopy() const;

private:
    std::map<std::string, JSONElement> m_elements;

    friend class JSONData;
};

/*!
 * @brief A JSON array.
 */
class JSONDataArray : public JSONData {
public:
    JSONData::JSONDataType type() const;

    unsigned long size() const;

    JSONElement valueAtIndex(unsigned i) const;

    std::vector<std::reference_wrapper<const JSONElement> > values() const;
    std::vector<JSONElement>::const_iterator valuesBegin() const;
    std::vector<JSONElement>::const_iterator valuesEnd() const;

    void insertValueAtIndex(unsigned i, const JSONElement& element);
    void eraseValueAtIndex(unsigned i);
    void pushBack(const JSONElement& element);
    void setValue(const JSONDataArray& other);
    void setValue(const std::vector<JSONElement>& other);
    JSONDataArray& operator=(const JSONDataArray& other);
    JSONDataArray& operator=(const std::vector<JSONElement>& other);
    const JSONElement& operator[](int index) const;
    JSONElement& operator[](int index);
    JSONElement& back();

protected:
    JSONDataArray(std::vector<JSONElement> elements);
    static std::unique_ptr<JSONDataArray> makeNewArray(std::vector<JSONElement> array);
    std::unique_ptr<JSONData> newCopy() const;

private:
    std::vector<JSONElement> m_elements;

    friend class JSONData;
};

/*!
 * @brief A base class for any type that requires direct access to a
 * JSONElement's data (as a JSONData object).
 */
class JSONElementFriend {
protected:
    const JSONData* fetchDataFromElement(const JSONElement& element) const
    {
        return element.data();
    }

    JSONData* fetchDataFromElement(JSONElement& element)
    {
        return element.data();
    }
};

/*!
 * @brief A base class for all JSONDataAccessor classes, which
 * are used to edit and read the data inside of JSONElement objects
 * without reassigning them.
 * @details 
 * @parblock
 * If you would like to read or edit (without reassigning) the data 
 * inside of a JSONElement, you must use some kind of JSONDataAccessor.
 * For non-const JSONElements, you will need to use a derivative of
 * the JSONDataAccessor class; for const JSONElements, you will need to
 * use a derivative of the JSONDataConstAccessor class.
 *
 * A specialized JSONDataConstAccessor exists for each type of data
 * that can exist in a JSONElement. You must use the correct type
 * of accessor for the data contained within the JSONElement (e.g.
 * you must use a JSONDataNumberAccessor or JSONDataNumberConstAccessor
 * for a non-const or const Number type, respectively; similarly, you
 * must use a JSONDataStringAccessor or JSONDataStringConstAccessor for 
 * a non-const or const String type, respectively). You can use the 
 * JSONElement::type method to determine the type of data in an element.
 * 
 * You can create a JSONDataConstAccessor either by explicitly calling 
 * the constructor, or by using the `=` operator. Both techniques are
 * shown below:
 *
 * @code
 * JSONElement element = JSONElement::makeObject();
 * JSONDataObjectAccessor accessor(element);
 * accessor["key"] = 1;
 * @endcode
 *
 * @code
 * JSONElement element = JSONElement::makeArray();
 * JSONDataArrayAccessor accessor = element;
 * accessor->pushBack(1);
 * @endcode
 *
 * An accessor's `->` operator can be used to access members of the
 * JSONElement's JSONData object. In addition, you can use any of the
 * operators which affect the JSONElement's JSONData object. This can
 * be seen in the two examples above; the `[]` operator was used with
 * the accessor of the first example to assign a value to a key in a
 * JSON object, and the `->` operator was used to call the `pushBack`
 * method of a JSON array.
 * @endparblock
 */
template <class DataClass>
class JSONDataAccessorBase : public JSONElementFriend {
public:
    const DataClass* operator->() const
    {
        return dataPointer();
    }

    const DataClass& operator*() const
    {
        return dereference();
    }

protected:
    const DataClass* dataPointer() const
    {
        return dynamic_cast<const DataClass*>(this->fetchDataFromElement(element()));
    }

    const DataClass& dereference() const
    {
        return *dataPointer();
    }

    virtual const JSONElement& element() const = 0;
};

/*!
 * @brief A derivative of JSONDataAccessorBase which is capable of
 * accessing the data of a non-const JSONElement.
 */
template <class DataClass>
class JSONDataAccessor : public JSONDataAccessorBase<DataClass> {
private:
    using super = JSONDataAccessorBase<DataClass>;

public:
    JSONDataAccessor(JSONElement& element)
        : m_element(&element)
    {
    }

    JSONDataAccessor()
    {
    }

    using super::operator->;

    DataClass* operator->()
    {
        return dataPointer();
    }

protected:
    using super::dataPointer;

    DataClass* dataPointer()
    {
        return dynamic_cast<DataClass*>(this->fetchDataFromElement(element()));
    }

    using super::dereference;

    DataClass& dereference()
    {
        return *dataPointer();
    }

    JSONElement& element()
    {
        return *m_element;
    };

    const JSONElement& element() const
    {
        return *m_element;
    };

private:
    JSONElement* m_element;
};

/*!
 * @brief A derivative of JSONDataAccessorBase which is capable of 
 * accessing a the data of a const JSONElement.
 */
template <class DataClass>
class JSONDataConstAccessor : public JSONDataAccessorBase<DataClass> {
public:
    JSONDataConstAccessor(const JSONElement& element)
        : m_element(&element)
    {
    }

    JSONDataConstAccessor()
    {
    }

protected:
    const JSONElement& element() const
    {
        return *m_element;
    };

private:
    const JSONElement* m_element;
};

/*!
 * @brief A JSONDataAccessor for accessing a non-const JSON Number.
 */
class JSONDataNumberAccessor : public JSONDataAccessor<JSONDataNumber> {
public:
    using JSONDataAccessor<JSONDataNumber>::JSONDataAccessor;

    JSONDataNumberAccessor& operator=(const JSONDataNumber& other);
    JSONDataNumberAccessor& operator=(const double& other);
};

/*!
 * @brief A JSONDataConstAccessor for accessing a const JSON Number.
 */
class JSONDataNumberConstAccessor : public JSONDataConstAccessor<JSONDataNumber> {
public:
    using JSONDataConstAccessor<JSONDataNumber>::JSONDataConstAccessor;
};

/*!
 * @brief A JSONDataAccessor for accessing a non-const JSON String.
 */
class JSONDataStringAccessor : public JSONDataAccessor<JSONDataString> {
public:
    using JSONDataAccessor<JSONDataString>::JSONDataAccessor;

    JSONDataStringAccessor& operator=(const JSONDataString& other);
    JSONDataStringAccessor& operator=(const std::string& other);
};

/*!
 * @brief A JSONDataConstAccessor for accessing a const JSON String.
 */
class JSONDataStringConstAccessor : public JSONDataConstAccessor<JSONDataString> {
public:
    using JSONDataConstAccessor<JSONDataString>::JSONDataConstAccessor;
};

/*!
 * @brief A JSONDataAccessor for accessing a non-const JSON Boolean.
 */
class JSONDataBooleanAccessor : public JSONDataAccessor<JSONDataBoolean> {
public:
    using JSONDataAccessor<JSONDataBoolean>::JSONDataAccessor;

    JSONDataBooleanAccessor& operator=(const JSONDataBoolean& other);
    JSONDataBooleanAccessor& operator=(const bool& other);
};

/*!
 * @brief A JSONDataConstAccessor for accessing a const JSON Boolean.
 */
class JSONDataBooleanConstAccessor : public JSONDataConstAccessor<JSONDataBoolean> {
public:
    using JSONDataConstAccessor<JSONDataBoolean>::JSONDataConstAccessor;
};

/*!
 * @brief A JSONDataAccessor for accessing a non-const JSON Null.
 */
class JSONDataNullAccessor : public JSONDataAccessor<JSONDataNull> {
public:
    using JSONDataAccessor<JSONDataNull>::JSONDataAccessor;
};

/*!
 * @brief A JSONDataConstAccessor for accessing a const JSON Null.
 */
class JSONDataNullConstAccessor : public JSONDataConstAccessor<JSONDataNull> {
public:
    using JSONDataConstAccessor<JSONDataNull>::JSONDataConstAccessor;
};

/*!
 * @brief A JSONDataAccessor for accessing a non-const JSON Array.
 */
class JSONDataArrayAccessor : public JSONDataAccessor<JSONDataArray> {
public:
    using JSONDataAccessor<JSONDataArray>::JSONDataAccessor;

    JSONDataArrayAccessor& operator=(const JSONDataArray& other);
    JSONDataArrayAccessor& operator=(const std::vector<JSONElement>& other);
    const JSONElement& operator[](int index) const;
    JSONElement& operator[](int index);
};

/*!
 * @brief A JSONDataConstAccessor for accessing a const JSON Array.
 */
class JSONDataArrayConstAccessor : public JSONDataConstAccessor<JSONDataArray> {
public:
    using JSONDataConstAccessor<JSONDataArray>::JSONDataConstAccessor;

    const JSONElement& operator[](int index) const;
};

/*!
 * @brief a JSONDataAccessor for accessing a non-const JSON Object.
 */
class JSONDataObjectAccessor : public JSONDataAccessor<JSONDataObject> {
public:
    using JSONDataAccessor<JSONDataObject>::JSONDataAccessor;

    JSONDataObjectAccessor& operator=(const JSONDataObject& other);
    JSONDataObjectAccessor& operator=(const std::map<std::string, JSONElement>& other);
    const JSONElement& operator[](const std::string& key) const;
    JSONElement& operator[](const std::string& key);
};

/*!
 * @brief A JSONDataConstAccessor for accessing a const JSON Object.
 */
class JSONDataObjectConstAccessor : public JSONDataConstAccessor<JSONDataObject> {
private:
public:
    using JSONDataConstAccessor<JSONDataObject>::JSONDataConstAccessor;

    const JSONElement& operator[](const std::string& key) const;
};
}
