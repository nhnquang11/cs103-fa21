#ifndef RealEntity_Included
#define RealEntity_Included

#include "Entity.h"
#include "FOLExpressionBuilder.h"
#include <memory>
#include <string>

enum class EntityType {
  PERSON,
  CAT,
  ROBOT
};

std::string to_string(EntityType);

struct RealEntity {
  std::string name;
  EntityType type;
  std::set<RealEntity *> loves;
  
  RealEntity(const std::string& name, EntityType type, std::set<RealEntity *> loves = {}) : name(name), type(type), loves(loves) {}
};

FOL::BuildContext entityBuildContext();

#endif
