#pragma once

#include "../GraphEditor/GraphViewer.h"
#include "../Logic/RealEntity.h"
#include <istream>
#include <memory>

/* Named predicate. */
struct Predicate {
    std::string name;
    std::function<bool(const World&)> pred;
};

/* World source, along with its predicates. */
struct PredicatedWorld {
    std::vector<Predicate>        predicates;
    std::shared_ptr<std::istream> in;
    std::string                   name;

    /* If something went wrong during parsing, display this message. */
    bool                          isError = false;
    std::string                   errorMessage = {};

    PredicatedWorld() = default;
    PredicatedWorld(const std::vector<Predicate>& predicates,
                    std::shared_ptr<std::istream> in,
                    const std::string& name,
                    bool isError = false,
                    const std::string& errorMessage = "") :
        predicates(predicates),
        in(in),
        name(name),
        isError(isError),
        errorMessage(errorMessage) {
        // Handled above
    }
};

/* Node representing an entity. */
class EntityNode: public GraphEditor::Node {
public:
    using GraphEditor::Node::Node;

    void draw(GraphEditor::ViewerBase* base,
              GCanvas* canvas,
              const GraphEditor::NodeStyle& style) override;

    void type(EntityType type);
    EntityType type();

private:
    EntityType mType;
};

class WorldViewer: public GraphEditor::Viewer<EntityNode, GraphEditor::Edge> {
public:
    WorldViewer(const PredicatedWorld& pw);

    World world();
    std::string name();
    std::vector<Predicate> predicates();

    bool isError();
    std::string errorMessage();

private:
    World mWorld;
    std::string mName;
    std::vector<Predicate> mPredicates;

    bool mIsError = false;
    std::string mErrorMessage;
};
