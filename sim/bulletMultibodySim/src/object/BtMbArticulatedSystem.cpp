//
// Created by kangd on 25.05.18.
//

#include "BtMbArticulatedSystem.hpp"

namespace bullet_multibody_sim {

object::BtMbArticulatedSystem::BtMbArticulatedSystem(std::string filePath,
                                                     ObjectFileType fileType,
                                                     b3RobotSimulatorClientAPI_NoGUI *api) : api_(api) {

  int objectId = -1;

  // load model file
  switch (fileType) {
    case URDF: {
      b3RobotSimulatorLoadUrdfFileArgs args;
      args.m_flags =
          URDF_USE_INERTIA_FROM_FILE | URDF_USE_SELF_COLLISION | URDF_USE_IMPLICIT_CYLINDER | MJCF_COLORS_FROM_FILE;

      objectId = api->loadURDF(filePath + "/robot.urdf", args);
      break;
    }
    case SDF:
    case MJCF:
    RAIFATAL("currently, only URDF is supported")
      break;
    default:
    RAIFATAL("wrong object file type")
      break;
  }

  // set object id
  RAIFATAL_IF(objectId == -1, "cannot load Object")
  objectId_ = objectId;

  // initialize visual data
  b3VisualShapeInformation visualShapeInfo;
  {
    api_->getVisualShapeData(objectId, visualShapeInfo);
    for (int i = 0; i < visualShapeInfo.m_numVisualShapes; i++) {
      initVisuals(objectId, visualShapeInfo.m_visualShapeData[i]);
    }
  }

  // initialize collision data
  // base (link id = -1)
  {
    b3CollisionShapeInformation collisionShapeInfo;
    api_->getCollisionShapeData(objectId, -1, collisionShapeInfo);
    initCollisions(objectId, -1, collisionShapeInfo);
  }

  // links
  for(int i = 0; i < api->getNumJoints(objectId); i++) {
    b3CollisionShapeInformation collisionShapeInfo;
    api_->getCollisionShapeData(objectId, i, collisionShapeInfo);
    initCollisions(objectId, i, collisionShapeInfo);
  }
}

object::BtMbArticulatedSystem::~BtMbArticulatedSystem() {}

void object::BtMbArticulatedSystem::initVisuals(int objectId, b3VisualShapeData &data) {

  // link id
  int linkId = data.m_linkIndex;

  // orientation
  benchmark::Mat<3, 3> mat;
  benchmark::Vec<4> quat;
  quat[0] = data.m_localVisualFrame[6]; // w
  quat[1] = data.m_localVisualFrame[3]; // x
  quat[2] = data.m_localVisualFrame[4]; // y
  quat[3] = data.m_localVisualFrame[5]; // z
  benchmark::quatToRotMat(quat, mat);

  // position
  benchmark::Vec<3> position;
  position[0] = data.m_localVisualFrame[0];
  position[1] = data.m_localVisualFrame[1];
  position[2] = data.m_localVisualFrame[2];

  // color
  benchmark::Vec<4> color;
  color[0] = data.m_rgbaColor[0];
  color[1] = data.m_rgbaColor[1];
  color[2] = data.m_rgbaColor[2];
  color[3] = data.m_rgbaColor[3];

  switch (data.m_visualGeometryType) {
    case GEOM_SPHERE: {
      benchmark::Vec<4> dim = {
          data.m_dimensions[0],   // radius
          0,
          0,
          0
      };
      visObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Sphere, color));
      visProps_.emplace_back(std::make_pair("", dim));
      break;
    }
    case GEOM_BOX: {
      benchmark::Vec<4> dim = {
          data.m_dimensions[0],
          data.m_dimensions[1],
          data.m_dimensions[2],
          0
      };
      visObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Box, color));
      visProps_.emplace_back(std::make_pair("", dim));
      break;
    }
    case GEOM_CYLINDER: {
      benchmark::Vec<4> dim = {
          data.m_dimensions[1],   // radius
          data.m_dimensions[0],   // length
          0,
          0
      };
      visObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Cylinder, color));
      visProps_.emplace_back(std::make_pair("", dim));
      break;
    }
    case GEOM_MESH: {
      benchmark::Vec<4> dim;
      dim = {data.m_dimensions[0],
             data.m_dimensions[1],
             data.m_dimensions[2],
             0};
      visObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Mesh, color));
      visProps_.emplace_back(std::make_pair(data.m_meshAssetFileName, dim));
      break;
    }
    case GEOM_CAPSULE: {
      benchmark::Vec<4> dim = {
          data.m_dimensions[1],   // radius
          data.m_dimensions[0],   // length
          0
      };
      visObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Capsule, color));
      visProps_.emplace_back(std::make_pair("", dim));
      break;
    }
    case GEOM_PLANE:
    case GEOM_UNKNOWN:
    default:
      RAIFATAL("invalid visual shape")
  }
}

void object::BtMbArticulatedSystem::initCollisions(int objectId, int linkId, b3CollisionShapeInformation &info) {
  for(int i = 0; i < info.m_numCollisionShapes; i++) {
    b3CollisionShapeData &data = info.m_collisionShapeData[i];

    // link id
//    RAIFATAL_IF(data.m_linkIndex != info->m_jointIndex, "joint idx is not same with link idx")
//    int linkId = linkId;

    // orientation
    benchmark::Mat<3, 3> mat;
    benchmark::Vec<4> quat;
    quat[0] = data.m_localCollisionFrame[6];  // w
    quat[1] = data.m_localCollisionFrame[3];  // x
    quat[2] = data.m_localCollisionFrame[4];  // y
    quat[3] = data.m_localCollisionFrame[5];  // z
    benchmark::quatToRotMat(quat, mat);

    // position
    benchmark::Vec<3> position;
    position[0] = data.m_localCollisionFrame[0];
    position[1] = data.m_localCollisionFrame[1];
    position[2] = data.m_localCollisionFrame[2];

    switch (data.m_collisionGeometryType) {
      case GEOM_BOX:
      {
        benchmark::Vec<4> dim = {
            data.m_dimensions[0],
            data.m_dimensions[1],
            data.m_dimensions[2],
            0
        };
        visColObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Box));
        visColProps_.emplace_back(std::make_pair("", dim));
        break;
      }
      case GEOM_CAPSULE:
      {
        benchmark::Vec<4> dim = {
            data.m_dimensions[1],   // radius
            data.m_dimensions[0],   // length
            0
        };
        visColObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Capsule));
        visColProps_.emplace_back(std::make_pair("", dim));
        break;
      }
      case GEOM_CYLINDER:
      {
        benchmark::Vec<4> dim = {
            data.m_dimensions[1],   // radius
            data.m_dimensions[0],   // length
            0,
            0
        };
        visColObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Cylinder));
        visColProps_.emplace_back(std::make_pair("", dim));
        break;
      }
      case GEOM_SPHERE:
      {
        benchmark::Vec<4> dim = {
            data.m_dimensions[0],   // radius
            0,
            0,
            0
        };
        visColObj.emplace_back(std::make_tuple(mat, position, linkId, benchmark::object::Shape::Sphere));
        visColProps_.emplace_back(std::make_pair("", dim));
        break;
      }
      default:
      RAIFATAL("not supported collision shape")
    }
  }
}

int object::BtMbArticulatedSystem::getDOF() {
  return dof_;
}

int object::BtMbArticulatedSystem::getStateDimension() {
  return stateDimension_;
}

const benchmark::object::ArticulatedSystemInterface::EigenVec object::BtMbArticulatedSystem::getGeneralizedCoordinate() {
  return genCoordinate_.e();
}

const benchmark::object::ArticulatedSystemInterface::EigenVec object::BtMbArticulatedSystem::getGeneralizedVelocity() {
  return genVelocity_.e();
}

const benchmark::object::ArticulatedSystemInterface::EigenVec object::BtMbArticulatedSystem::getGeneralizedForce() {
  return genForce_.e();
}

void object::BtMbArticulatedSystem::getState(Eigen::VectorXd &genco, Eigen::VectorXd &genvel) {
  genco = getGeneralizedCoordinate();
  genvel = getGeneralizedVelocity();
}

void object::BtMbArticulatedSystem::setGeneralizedCoordinate(const Eigen::VectorXd &jointState) {
}

void object::BtMbArticulatedSystem::setGeneralizedCoordinate(std::initializer_list<double> jointState) {
}

void object::BtMbArticulatedSystem::setGeneralizedVelocity(const Eigen::VectorXd &jointVel) {
}

void object::BtMbArticulatedSystem::setGeneralizedVelocity(std::initializer_list<double> jointVel) {

}

void object::BtMbArticulatedSystem::setGeneralizedForce(std::initializer_list<double> tau) {
}

void object::BtMbArticulatedSystem::setGeneralizedForce(const Eigen::VectorXd &tau) {
}

void object::BtMbArticulatedSystem::setState(const Eigen::VectorXd &genco, const Eigen::VectorXd &genvel) {
}

const Eigen::Map<Eigen::Matrix<double, 3, 1>> object::BtMbArticulatedSystem::getLinearMomentumInCartesianSpace() {
  return linearMomentum_.e();
}

double object::BtMbArticulatedSystem::getTotalMass() {
  return 0;
}

double object::BtMbArticulatedSystem::getEnergy(const benchmark::Vec<3> &gravity) {
  return 0;
}

void object::BtMbArticulatedSystem::getBodyPose(int linkId,
                                                benchmark::Mat<3, 3> &orientation,
                                                benchmark::Vec<3> &position) {

  if(linkId == -1) {
    // base
    b3Vector3 bPosition;
    b3Quaternion bQuat;
    RAIFATAL_IF(!api_->getBasePositionAndOrientation(objectId_, bPosition, bQuat), "failed to get link state");
    position = {
        bPosition.x,
        bPosition.y,
        bPosition.z
    };

    benchmark::Vec<4> quat = {
        bQuat.w,
        bQuat.x,
        bQuat.y,
        bQuat.z,
    };
    benchmark::quatToRotMat(quat, orientation);
  }
  else {
    // link
    b3LinkState linkState;
    RAIFATAL_IF(!api_->getLinkState(objectId_, linkId, &linkState), "failed to get link state");

    b3Transform comTf_W;
    comTf_W.setOrigin(
        {linkState.m_worldPosition[0],
         linkState.m_worldPosition[1],
         linkState.m_worldPosition[2]});
    comTf_W.setRotation(
        {linkState.m_worldOrientation[0],
         linkState.m_worldOrientation[1],
         linkState.m_worldOrientation[2],
         linkState.m_worldOrientation[3]});

    b3Transform comTf_B;
    comTf_B.setOrigin(
        {linkState.m_localInertialPosition[0],
         linkState.m_localInertialPosition[1],
         linkState.m_localInertialPosition[2]});
    comTf_B.setRotation(
        {linkState.m_localInertialOrientation[0],
         linkState.m_localInertialOrientation[1],
         linkState.m_localInertialOrientation[2],
         linkState.m_localInertialOrientation[3]});

    b3Transform bTf = comTf_W * comTf_B.inverse();

    benchmark::Vec<4> quat = {
        bTf.getRotation().w,
        bTf.getRotation().x,
        bTf.getRotation().y,
        bTf.getRotation().z};
    benchmark::quatToRotMat(quat, orientation);

    position = {
        bTf.getOrigin().x,
        bTf.getOrigin().y,
        bTf.getOrigin().z};
  }
}

} // bullet_multibody_sim