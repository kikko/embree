// ======================================================================== //
// Copyright 2009-2012 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#ifndef __EMBREE_HANDLE_H__
#define __EMBREE_HANDLE_H__

#include "api/parms.h"

namespace embree
{
  /* generic handle interface */
  struct _RTHandle {
  public:

    Atomic counter;

    _RTHandle() : counter(1) {}

    void incRef() {
      counter++;
    }

    void decRef() {
      if (--counter == 0) {
        delete this;
      }
    }

    /*! Virtual destructor for handles. */
    virtual ~_RTHandle() {}

    /*! Recreated the object the handle refers to. */
    virtual void create() = 0;

    /*! Clear handle. */
    virtual void clear() { }

    /*! Sets a parameter of the handle. */
    virtual void set(const std::string& property, const embree::Variant& data) = 0;
  };

  /*******************************************************************
                 generic handle implementations
  *******************************************************************/

  /*! Base Handle */
  template<typename B> class BaseHandle : public _RTHandle {
  public:
    Parms parms;     //!< Parameter container for handle.
    Ref<B> instance; //!< Referenced object.
  };

  /*! Normal handle. A normal handle type that buffers parameters and
   *  can create the underlying object. */
  template<typename T, typename B> class NormalHandle : public BaseHandle<B> {
  public:

    /*! Creates a new object. */
    void create() { this->instance = new T(this->parms); }

    /*! Clear handle. */
    void clear() { this->parms.clear(); }

    /*! Sets a new parameter. */
    void set(const std::string& property, const Variant& data) { this->parms.add(property,data); }
  };

  /*! Same as a normal handle, but object gets created via create function. */
  template<typename T, typename B> class NormalHandle2 : public BaseHandle<B> {
  public:

    /*! Creates a new object. */
    void create() { this->instance = T::create(this->parms); }

    /*! Clear handle. */
    void clear() { this->parms.clear(); }

    /*! Sets a new parameter. */
    void set(const std::string& property, const Variant& data) { this->parms.add(property,data); }
  };

  /*! Constant handle. A special handle type that does not allow
   *  setting parameters. */
  template<typename T> class ConstHandle : public _RTHandle {
  public:

    /*! Creates a constant handle from the object to reference. */
    ConstHandle(const Ref<T>& ptr) : instance(ptr) {}

    /*! Recreating the underlying object is not allowed. */
    void create() { throw std::runtime_error("cannot modify constant handle"); }

    /*! Setting parameters is not allowed. */
    void set(const std::string& property, const Variant& data) { throw std::runtime_error("cannot modify constant handle"); }

  public:
    Ref<T> instance;  //!< Referenced object.
  };

  /*! Safe handle cast. Casts safely to a specified output type.  */
  template<typename T> static T* castHandle(Device::RTHandle handle_i, const char* name) {
    T* handle = dynamic_cast<T*>((_RTHandle*)handle_i);
    if (!handle          ) throw std::runtime_error("invalid "+std::string(name)+" handle");
    if (!handle->instance) throw std::runtime_error("invalid "+std::string(name)+" value");
    return handle;
  }
}

#endif
