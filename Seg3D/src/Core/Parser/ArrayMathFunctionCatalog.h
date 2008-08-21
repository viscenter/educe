//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#ifndef CORE_PARSER_ARRAYMATHFUNCTIONCATALOG_H 
#define CORE_PARSER_ARRAYMATHFUNCTIONCATALOG_H 1

#include <Core/Parser/Parser.h>
#include <Core/Parser/ArrayMathInterpreter.h>

#include <Core/Containers/LockingHandle.h>
#include <Core/Thread/Mutex.h>

// Include files needed for Windows
#include <Core/Parser/share.h>

namespace SCIRun {

//-----------------------------------------------------------------------------
// The ArrayMathFunctionCatalog classes are a base class for a
// series of classes that define basic functionality for the parser
// These classes house the main functions.
// The base class provides functionality for registering the functions
// to the main ParserCatalog, which is used by the parser to look up
// functions

class SCISHARE ArrayMathFunctionCatalog : public ParserFunctionCatalog {

  public:
    ArrayMathFunctionCatalog() {}
    
  public:    
    // Add a function to the general database
    void add_function(bool (*function)(ArrayMathProgramCode& pc),
            std::string function_id,
            std::string return_type);


    void add_sym_function(bool (*function)(ArrayMathProgramCode& pc),
            std::string function_id,
            std::string return_type);

    void add_seq_function(bool (*function)(ArrayMathProgramCode& pc),
            std::string function_id,
            std::string return_type);

    void add_sgl_function(bool (*function)(ArrayMathProgramCode& pc),
            std::string function_id,
            std::string return_type);
            
    static ParserFunctionCatalogHandle get_catalog();
};

typedef LockingHandle<ArrayMathFunctionCatalog> ArrayMathFunctionCatalogHandle;

//-----------------------------------------------------------------------------
// Functions for adding Functions to the ArrayMathFunctionCatalog

// Insert the basic functions into the database
void SCISHARE InsertBasicArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog);
void SCISHARE InsertSourceSinkArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog);
void SCISHARE InsertScalarArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog);
void SCISHARE InsertVectorArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog);
void SCISHARE InsertTensorArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog);
void SCISHARE InsertElementArrayMathFunctionCatalog(ArrayMathFunctionCatalogHandle& catalog);

}

#endif
