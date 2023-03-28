#include "WorldViewer.h"
#include "../Logic/RealEntity.h"
#include "../GUI/MiniGUI.h"
#include "../Logic/WorldParser.h"
#include "../GraphEditor/GraphViewer.h"
#include "../GraphEditor/GVector.h"
#include <vector>
using namespace std;

namespace {
    /* Initial theta used for a radial layout. */
    const double kBaseTheta = M_PI / 2;

    /* Our preferred aspect ratio for the graph. */
    const double kAspectRatio = 1.0;

    /* Radius of the positioning circle. */
    const double kCircleRadius = 0.5 / kAspectRatio - GraphEditor::kNodeRadius * 2;

    /* An entity we can draw on the screen. */
    struct GraphicsEntity {
        EntityType type;
        GPoint pos;

        vector<string> out;
        EntityNode* node;
    };

    /* Returns a GImage for the given object type. */
    GImage& imageFor(EntityType type) {
        if (type == EntityType::CAT) {
            static GImage cat("res/images/cat.png");
            return cat;
        } else if (type == EntityType::PERSON) {
            static GImage person("res/images/person.png");
            return person;
        } else if (type == EntityType::ROBOT) {
            static GImage robot("res/images/robot.png");
            return robot;
        } else {
            abort(); // Logic error!
        }
    }
}

/* Node renderer that draws an object of the appropriate type. */
void EntityNode::draw(GraphEditor::ViewerBase* base,
                      GCanvas* canvas,
                      const GraphEditor::NodeStyle& style) {
    auto imageBounds = base->worldToGraphics({
                                 position().x - style.radius,
                                 position().y - style.radius,
                                 2 * style.radius, 2 * style.radius
                       });

    /* Draw the background image. */
    auto& image = imageFor(type());
    image.setBounds(imageBounds);
    canvas->draw(&image);

    /* Draw an overlaying circle. */
    double newRadius = style.radius - style.lineWidth;
    auto circleBounds = base->worldToGraphics({
                            position().x - newRadius,
                            position().y - newRadius,
                            2 * newRadius, 2 * newRadius
                        });

    GOval oval(circleBounds.x, circleBounds.y, circleBounds.width, circleBounds.height);
    oval.setLineWidth(base->worldToGraphics(style.lineWidth));
    oval.setColor(style.borderColor);
    canvas->draw(&oval);
}

void EntityNode::type(EntityType type) {
    mType = type;
}

EntityType EntityNode::type() {
    return mType;
}

WorldViewer::WorldViewer(const PredicatedWorld& pw) {
    mPredicates = pw.predicates;
    mName       = pw.name;

    /* Copy over existing error state. */
    mIsError = pw.isError;
    mErrorMessage = pw.errorMessage;

    /* Another possible error - there was no input stream. */
    if (!mIsError && !pw.in) {
        mIsError = true;
        mErrorMessage = "Section [" + pw.name + "] not found.";
    }

    /* If we aren't already in an error state, try loading the world. */
    if (!mIsError) {
        try {
            mWorld = parseWorld(*pw.in);
        } catch (const exception& e) {
            mErrorMessage = e.what();
            mIsError = true;
        }
    }

    /* At this point we have valid data or we know something is wrong. */
    if (!mIsError) {
        /* Translate logic Entities into graphics entities. */
        map<string, GraphicsEntity> entities;
        for (const auto& entity: mWorld) {
            entities[entity->name].type = entity->type;
            for (auto* dest: entity->loves) {
                entities[entity->name].out.push_back(dest->name);
            }
        }

        /* Position the objects. */

        /* Logical center. The width is always 1. The height is always 1 / kAspectRatio. */
        GPoint center = { 0.5, 0.5 / kAspectRatio };
        if (entities.size() == 0) {
            // Do nothing
        } else if (entities.size() == 1) {
            /* Screen center */
            entities.begin()->second.pos = center;
        } else {
            const double thetaStep = 2 * M_PI / entities.size();
            double theta = kBaseTheta;

            for (auto& entry: entities) {
                entry.second.pos = center + unitToward(theta) * kCircleRadius;
                theta += thetaStep;
            }
        }

        /* Translate into graphics objects. */
        aspectRatio(kAspectRatio);
        for (auto& entry: entities) {
            entry.second.node = newNode(entry.second.pos);
            entry.second.node->type(entry.second.type);
        }

        /* Add links. */
        for (const auto& entry: entities) {
            for (auto link: entry.second.out) {
                auto* src = entry.second.node;
                auto* dst = entities[link].node;
                newEdge(src, dst);
            }
        }
    }
}

World WorldViewer::world() {
    return mWorld;
}
string WorldViewer::name() {
    return mName;
}
vector<Predicate> WorldViewer::predicates() {
    return mPredicates;
}

bool WorldViewer::isError() {
    return mIsError;
}
string WorldViewer::errorMessage() {
    return mErrorMessage;
}
