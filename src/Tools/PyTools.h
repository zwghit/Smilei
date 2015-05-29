
// -------------------
// Some Python helper functions
// -------------------

#ifndef PYHelper_H
#define PYHelper_H


#include <Python.h>
#include "Tools.h"

//! tools to convert python values to C++ values and vectors
class PyTools {    
private:     
    //! convert Python object to bool
    static bool pyconvert(PyObject* py_val, bool &val) {
        if (py_val && PyBool_Check(py_val)) {
            val=(py_val==Py_True);
            return true;
        }
        return false;        
    }
    
    //! convert Python object to short int
    static bool pyconvert(PyObject* py_val, short int &val) {
        if (py_val && PyInt_Check(py_val)) {
            val=(short int) PyInt_AsLong(py_val);
            return true;
        }
        return false;        
    }
    
    //! convert Python object to unsigned int
    static bool pyconvert(PyObject* py_val, unsigned int &val) {
        if (py_val && PyInt_Check(py_val)) {
            val=(unsigned int) PyInt_AsLong(py_val);
            return true;
        }
        return false;        
    }
    
    //! convert Python object to int
    static bool pyconvert(PyObject* py_val, int &val) {
        if (py_val && PyInt_Check(py_val)) {
            val=(int) PyInt_AsLong(py_val);
            return true;
        }
        return false;        
    }
    
    //! convert Python object to double
    static bool pyconvert(PyObject* py_val, double &val) {
        if(py_val) {
            if (PyFloat_Check(py_val)) {
                val = PyFloat_AsDouble(py_val);
                return true;
            } else if (PyInt_Check(py_val)) {
                val=(double) PyInt_AsLong(py_val);
                return true;
            }
        }
        return false;        
    }

    //! convert Python object to string
    static bool pyconvert(PyObject* py_val, std::string &val) {
        if (py_val && PyString_Check(py_val)) {
            val=std::string(PyString_AsString(py_val));
            return true;
        }
        return false;
    }

public:
    //! check error and display message
    static double get_py_result(PyObject* pyresult) {
        checkPyError();
        double cppresult=0;
        if(!convert(pyresult,cppresult)) {
            PyObject *ptype, *pvalue, *ptraceback;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            ERROR("function does not return float but " << pyresult->ob_type->tp_name);
        }
        Py_XDECREF(pyresult);
        return cppresult;
    }
    
    //! convert vector of Python objects to vector of C++ values
    template <typename T>
    static bool convert(PyObject* py_vec, T &val) {
        bool retval=pyconvert(py_vec, val);
        return retval;
    }

    //! convert vector of Python objects to vector of C++ values
    template <typename T>
    static bool convert(std::vector<PyObject*> py_vec, std::vector<T> &val) {
        bool retval=true;
        val.resize(py_vec.size());
        for (unsigned int i=0;i<py_vec.size();i++) {
            bool thisval=convert(py_vec[i], val[i]);
            if (thisval==false) retval=false;
        }
        return retval;
    }
    static void checkPyError() {
        if (PyErr_Occurred()) {
            PyObject *type, *value, *traceback;
            PyErr_Fetch(&type, &value, &traceback);
            PyErr_Clear();
            
            std::string message;
            if (type) {
                type = PyObject_Str(type);
                message += PyString_AsString(type);
            }
            if (value) {
                value = PyObject_Str(value);
                message += ": ";
                message += PyString_AsString(value);
            }
            Py_XDECREF(type);
            Py_XDECREF(value);
            Py_XDECREF(traceback);
            
            DEBUG(message);
        }                
    }
};

#endif