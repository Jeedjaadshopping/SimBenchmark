//
// Created by kangd on 10.02.18.
//

#ifndef BULLETSIM_WORLD_RG_HPP
#define BULLETSIM_WORLD_RG_HPP

#define RAI_COLLISION(x) (1<<(x))

#include <raiGraphics/RAI_graphics.hpp>
#include <bullet/btBulletCollisionCommon.h>

#include "common/math.hpp"
#include "common/World_RG.hpp"
#include "common/Configure.hpp"
#include "UserHandle.hpp"
#include "World.hpp"

namespace bullet_sim {

class World_RG: public benchmark::World_RG {

 public:

  /* constructor for visualization */
  World_RG(int windowWidth,
           int windowHeight,
           float cms,
           int flags = 0,
           SolverOption solverOption = SOLVER_SEQUENTIAL_IMPULSE);

  /* constructor for no visualization */
  World_RG(SolverOption solverOption = SOLVER_SEQUENTIAL_IMPULSE);
  virtual ~World_RG();

  /////////////////////////////////////
  /// Visualization related methods ///
  /////////////////////////////////////
  virtual void visEnd() override ;
  virtual void updateFrame() override ;

  //////////////////////////////////
  /// adding or removing objects ///
  //////////World////////////////////////
  benchmark::SingleBodyHandle addSphere(double radius,
                                        double mass,
                                        benchmark::CollisionGroupType collisionGroup = 1,
                                        benchmark::CollisionGroupType collisionMask=-1) override ;
  benchmark::SingleBodyHandle addBox(double xLength,
                                     double yLength,
                                     double zLength,
                                     double mass,
                                     benchmark::CollisionGroupType collisionGroup = 1,
                                     benchmark::CollisionGroupType collisionMask = -1) override ;
  benchmark::SingleBodyHandle addCheckerboard(double gridSize,
                                              double xLength,
                                              double yLength,
                                              double reflectanceI,
                                              benchmark::CollisionGroupType collisionGroup = 1,
                                              benchmark::CollisionGroupType collisionMask = -1,
                                              int flags = 0) override ;
  benchmark::SingleBodyHandle addCapsule(double radius,
                                         double height,
                                         double mass,
                                         benchmark::CollisionGroupType collisionGroup = 1,
                                         benchmark::CollisionGroupType collisionMask=-1) override ;
  ArticulatedSystemHandle addArticulatedSystem(std::string nm,
                                               benchmark::CollisionGroupType collisionGroup = 1,
                                               benchmark::CollisionGroupType collisionMask=-1) ;

  //////////////////////////
  /// simulation methods ///
  //////////////////////////
  virtual void integrate(double dt) override ;
  virtual void setGravity(Eigen::Vector3d gravity) override ;
  void setERP(double erp, double erp2, double frictionErp) override ;

 private:
  bullet_sim::World world_;

  // solver type
  SolverOption solverOption_ = SOLVER_SEQUENTIAL_IMPULSE;

  // list
  std::vector<ArticulatedSystemHandle> asHandles_;


};

} // bullet_sim

#endif //BULLETSIM_WORLD_RG_HPP