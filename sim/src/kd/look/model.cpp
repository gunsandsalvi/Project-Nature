#include "kd/look/model.hpp"

#include <algorithm>
#include <string>

namespace kd::look {

namespace {

// A placement's name, such as "poles", for the messages.
std::string quoted(const std::string& name) {
    return "\"" + name + "\"";
}

}  // namespace

void check_models(const data::Catalogue& cat, std::vector<data::Problem>& problems) {
    const data::Kind<Model>& models = cat.kind<Model>();
    for (std::uint32_t i = 0; i < models.size(); ++i) {
        const Model& m = models[i];
        const auto refuse = [&](std::string_view key, const std::string& what) {
            problems.push_back(models.at(i, key, what));
        };
        if (m.places.empty()) {
            refuse("place", models.name(i) + " places no part, so nothing of it would be drawn");
        }
        for (std::size_t r = 0; r < m.materials.size(); ++r) {
            const ModelMaterial& material = m.materials[r];
            if (material.textures.empty()) {
                refuse("material", "the role " + quoted(material.role) + " lists no texture to wear");
            }
            for (std::size_t other = 0; other < r; ++other) {
                if (m.materials[other].role == material.role) {
                    refuse("material", "the role " + quoted(material.role) + " is given twice");
                }
            }
        }
        std::vector<const ModelPlace*> before;
        for (const ModelPlace& p : m.places) {
            const std::string name = quoted(p.name);
            if (p.name.empty() || p.name.find('.') != std::string::npos) {
                refuse("place",
                       "a placement's name is not empty and holds no \".\", which joins it to a joint: " + name);
            }
            const bool twice =
                std::any_of(before.begin(), before.end(), [&](const ModelPlace* b) { return b->name == p.name; });
            if (twice) {
                refuse("place", "the placement " + name + " is named twice");
            }
            if (p.parts.empty()) {
                refuse("place", "the placement " + name + " names no part");
            }
            for (const std::string& part : p.parts) {
                if (part.empty()) {
                    refuse("place", "the placement " + name + " names a part with no name");
                }
            }
            if ((p.rule == "root" || p.rule == "plug") && p.count != 1) {
                refuse("place", "the placement " + name + " is a " + p.rule + ", which is one copy, not " +
                                    std::to_string(p.count));
            }
            if ((p.rule == "ring" || p.rule == "span") && p.radius <= 0) {
                refuse("place", "the placement " + name + " is a " + p.rule + " and needs a radius above 0");
            }
            if (p.rule == "span") {
                if (p.joint.empty() || p.to_joint.empty()) {
                    refuse("place", "the span " + name +
                                        " needs both joints: the one on the circle and the one that meets the others");
                } else if (p.joint == p.to_joint) {
                    refuse("place", "the span " + name + " uses the joint " + quoted(p.joint) + " at both ends");
                }
                if (p.height <= 0) {
                    refuse("place", "the span " + name + " needs a height above 0 for the point where its copies meet");
                }
            }
            if (p.rule == "plug") {
                const std::size_t dot = p.onto.find('.');
                if (p.joint.empty()) {
                    refuse("place", "the plug " + name + " needs the joint of the part that goes into place");
                }
                if (dot == std::string::npos || dot == 0 || dot + 1 == p.onto.size()) {
                    refuse("place",
                           "the plug " + name + " names where it goes as \"placement.joint\", not " + quoted(p.onto));
                } else {
                    const std::string target = p.onto.substr(0, dot);
                    const bool earlier = std::any_of(before.begin(), before.end(),
                                                     [&](const ModelPlace* b) { return b->name == target; });
                    if (!earlier) {
                        refuse("place",
                               "the plug " + name + " goes into " + quoted(target) + ", which is not placed before it");
                    }
                }
            }
            before.push_back(&p);
        }
    }
}

}  // namespace kd::look
