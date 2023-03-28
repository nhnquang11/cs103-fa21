#ifndef Entity_Included
#define Entity_Included

#include <memory>
#include <set>

using Entity = std::shared_ptr<struct RealEntity>;
using World  = std::set<Entity>;

bool Person(Entity e);
bool Cat(Entity e);
bool Robot(Entity e);
bool Loves(Entity x, Entity y);

#endif
