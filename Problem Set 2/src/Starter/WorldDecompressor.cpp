#include "WorldDecompressor.h"
#include "Logic/RealEntity.h"
using namespace std;

namespace {
    /* Character in compressed format -> Entity type */
    EntityType toType(char ch) {
        if (ch == 'c') return EntityType::CAT;
        if (ch == 'r') return EntityType::ROBOT;
        if (ch == 'p') return EntityType::PERSON;
        abort(); // Bad argument
    }
}

/* Decompresses the given worlds data. The worlds data has been compressed to
 * save space and has the following form:
 *
 * {type}{compressed-data}
 *
 * Here, {type} is either y for a positive example or n for a negative example.
 *
 * The compressed-data consists of a series of bytes. Each byte is either c, p, or
 * r to denote the type, followed by a series of bytes of the form 0x80 | loves, where
 * loves is the index of the loved entity.
 */
namespace Decompressor {
    pair<vector<World>, vector<World>> parse(istream& input) {
        pair<vector<World>, vector<World>> result;

        while (true) {
            /* Read the type of entry (y or n) */
            char type;
            input >> type;
            if (!input) break;

            /* Keep reading until find everything we need. */
            vector<EntityType> types;
            map<size_t, vector<size_t>> loves;
            while (input.peek() != '\n') {
                size_t index = types.size();
                types.push_back(toType(input.get()));

                /* Keep grabbing loves relations. */
                while (input.peek() & 0x80) {
                    loves[index].push_back(input.get() & 0x7F);
                }
            }

            /* Build the entities. */
            vector<shared_ptr<RealEntity>> world;
            for (size_t i = 0; i < types.size(); i++) {
                world.push_back(make_shared<RealEntity>(to_string(i), types[i]));
            }

            /* Link them. */
            for (size_t i = 0; i < types.size(); i++) {
                for (const auto& entry: loves[i]) {
                    world[i]->loves.insert(world[entry].get());
                }
            }

            /* Stash it away. */
            (type == 'y'? result.second : result.first).emplace_back(world.begin(), world.end());
        }
        return result;
    }
}
