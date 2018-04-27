//
// Created by kangd on 26.04.18.
//

#include <raiSim/World_RG.hpp>

#include "AnymalBenchmark.hpp"
#include "raiCommon/utils/StopWatch.hpp"

rai_sim::World_RG *sim;
std::vector<rai_sim::ArticulatedSystemHandle> anymals;

void setupSimulation() {
  if(benchmark::anymal::options.gui)
    sim = new rai_sim::World_RG(800, 600, 0.5, rai_sim::NO_BACKGROUND);
  else
    sim = new rai_sim::World_RG();
}

void setupWorld() {
  auto checkerboard = sim->addCheckerboard(2, 100, 100, 0.1, 1, -1, rai_sim::GRID);

  for(int i = 0; i < benchmark::anymal::options.numRow; i++) {
    for(int j = 0; j < benchmark::anymal::options.numRow; j++) {
      auto anymal = sim->addArticulatedSystem(
          benchmark::anymal::getURDFpath()
      );
//      anymal->setColor({1, 0, 0, 1});
      anymal->setGeneralizedCoordinate(
          {i * 2,
           j * 2,
           benchmark::anymal::params.H,
           benchmark::anymal::params.baseQuat[0],
           benchmark::anymal::params.baseQuat[1],
           benchmark::anymal::params.baseQuat[2],
           benchmark::anymal::params.baseQuat[3],
           benchmark::anymal::params.jointPos[0],
           benchmark::anymal::params.jointPos[1],
           benchmark::anymal::params.jointPos[2],
           benchmark::anymal::params.jointPos[3],
           benchmark::anymal::params.jointPos[4],
           benchmark::anymal::params.jointPos[5],
           benchmark::anymal::params.jointPos[6],
           benchmark::anymal::params.jointPos[7],
           benchmark::anymal::params.jointPos[8],
           benchmark::anymal::params.jointPos[9],
           benchmark::anymal::params.jointPos[10],
           benchmark::anymal::params.jointPos[11]
          });
      anymal->setGeneralizedVelocity(Eigen::VectorXd::Zero(anymal->getDOF()));
      anymal->setGeneralizedForce(Eigen::VectorXd::Zero(anymal->getDOF()));
      anymals.push_back(anymal);
    }
  }

  sim->setGravity({0, 0, -9.8});

  if(benchmark::anymal::options.gui)
    sim->cameraFollowObject(checkerboard, {1.0, 1.0, 1.0});
}

void simulationLoop() {
  Eigen::VectorXd jointNominalConfig(19);
  Eigen::VectorXd jointState(18), jointVel(18), jointForce(18);
  const double kp = benchmark::anymal::params.kp;
  const double kd = benchmark::anymal::params.kd;

  jointNominalConfig
      <<
      0,
      0,
      benchmark::anymal::params.H,
      benchmark::anymal::params.baseQuat[0],
      benchmark::anymal::params.baseQuat[1],
      benchmark::anymal::params.baseQuat[2],
      benchmark::anymal::params.baseQuat[3],
      benchmark::anymal::params.jointPos[0],
      benchmark::anymal::params.jointPos[1],
      benchmark::anymal::params.jointPos[2],
      benchmark::anymal::params.jointPos[3],
      benchmark::anymal::params.jointPos[4],
      benchmark::anymal::params.jointPos[5],
      benchmark::anymal::params.jointPos[6],
      benchmark::anymal::params.jointPos[7],
      benchmark::anymal::params.jointPos[8],
      benchmark::anymal::params.jointPos[9],
      benchmark::anymal::params.jointPos[10],
      benchmark::anymal::params.jointPos[11];

  if(benchmark::anymal::options.gui) {
    // gui
    while(sim->visualizerLoop(benchmark::anymal::params.dt, 1.0)) {
      for(int i = 0; i < anymals.size(); i++) {
        jointState = anymals[i]->getGeneralizedCoordinate();
        jointVel = anymals[i]->getGeneralizedVelocity();
        jointForce = anymals[i]->getGeneralizedForce();

        jointForce = kp * (jointNominalConfig - jointState).tail(18) - kd * jointVel;
        jointForce.head(6).setZero();
        anymals[i]->setGeneralizedForce(jointForce);
      }
      sim->integrate(benchmark::anymal::params.dt);
    }
  } else {
    // no gui
    StopWatch watch;
    watch.start();
    for(int t = 0; t < (int)benchmark::anymal::params.T / (int)benchmark::anymal::params.dt; t++) {
      for(int i = 0; i < anymals.size(); i++) {
        jointState = anymals[i]->getGeneralizedCoordinate();
        jointVel = anymals[i]->getGeneralizedVelocity();
        jointForce = anymals[i]->getGeneralizedForce();

        jointForce = kp * (jointNominalConfig - jointState).tail(18) - kd * jointVel;
        jointForce.head(6).setZero();
        anymals[i]->setGeneralizedForce(jointForce);
      }
      sim->integrate(benchmark::anymal::params.dt);
    }

    std::cout<<"time taken for "
             << (int) benchmark::anymal::params.T / benchmark::anymal::params.dt
             << " steps "<< watch.measure()<<"s \n";
  }
}

int main(int argc, const char* argv[]) {

  benchmark::anymal::getParamsFromArg(argc, argv);

  setupSimulation();
  setupWorld();
  simulationLoop();

  return 0;
}