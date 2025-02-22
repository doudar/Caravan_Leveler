#include "main.h"

//Original from combie
//https://forum.arduino.cc/index.php?topic=648602.msg4374622#msg4374622
template<typename DataType>
class DeepPass {
  private:
    const double factor;
    double       average;

  public:
    DeepPass(const double factor): factor(factor), average(0) {}

    void setInitial(double value) {
      average = value;
    }

    DataType processValue(DataType value) {
      average *= 1.0 - factor;
      average += factor * value;
      return  average;
    }

    DataType operator= (DataType value) {
      return processValue(value);
    }

    DataType operator() (DataType value) {
      return processValue(value);
    }

    DataType operator() () const
    {
      return average;
    }

    operator DataType() const
    {
      return average;
    }
};

