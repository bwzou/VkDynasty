#pragma once

namespace DynastyEngine
{
  // 模版
  template<typename T> class UUID 
  {
  public:
      UUID() : uuid_(uuidCounter_++) {}

      inline int get() const {
        return uuid_;
      }

  private:
      int uuid_ = -1;
      static int uuidCounter_;
  };

  template<typename T> int UUID<T>::uuidCounter_ = 0;
}