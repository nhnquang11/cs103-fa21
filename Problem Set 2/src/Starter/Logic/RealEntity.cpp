#include "RealEntity.h"
#include <stdexcept>
#include <vector>
#include <sstream>
using namespace std;

bool Person(Entity e) {
  return e->type == EntityType::PERSON;
}
bool Cat(Entity e) {
  return e->type == EntityType::CAT;
}
bool Robot(Entity e) {
  return e->type == EntityType::ROBOT;
}
bool Loves(Entity x, Entity y) {
  return x->loves.count(y.get());
}

string to_string(EntityType type) {
  if (type == EntityType::PERSON) return "Person";
  if (type == EntityType::CAT)    return "Cat";
  if (type == EntityType::ROBOT)  return "Robot";
  throw runtime_error("Unknown entity type.");
}

FOL::BuildContext entityBuildContext() {
  return {
    {}, /* Constants: None */
    {
      { "Person", { 1, [](const vector<Entity>& e) { return Person(e[0]); } } },
      { "Cat",    { 1, [](const vector<Entity>& e) { return Cat(e[0]); } } },
      { "Robot",  { 1, [](const vector<Entity>& e) { return Robot(e[0]); } } },
      { "Loves",  { 2, [](const vector<Entity>& e) { return Loves(e[0], e[1]); } } },
    },
    {} /* Functions: None */
  };
}

