//
// Created by kangd on 10.02.18.
//

#ifndef BULLETSIM_CHECKERBOARD_HPP
#define BULLETSIM_CHECKERBOARD_HPP

#include "common/interface/CheckerboardInterface.hpp"
#include "BtSingleBodyObject.hpp"

namespace bo = benchmark::object;

namespace bullet_sim {
namespace object {

class BtCheckerBoard: public BtSingleBodyObject,
                    public bo::CheckerboardInterface {

 public:
  BtCheckerBoard(double xLength, double yLength, bo::CheckerboardShape shape = bo::PLANE_SHAPE);

};

} // object
} // bullet_sim

#endif //BULLETSIM_CHECKERBOARD_HPP