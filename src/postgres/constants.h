#ifndef POSTGRES_CONSTANTS_H_
#define POSTGRES_CONSTANTS_H_

#include <string>

// DO_NOT_SUBMIT(mwhittaker): Document and mention the reset_database script.
namespace fluent {
namespace postgres {

extern const std::string NODES;
extern const std::string NODES_ID;
extern const std::string NODES_NAME;

extern const std::string COLLECTIONS;
extern const std::string COLLECTIONS_NODE_ID;
extern const std::string COLLECTIONS_COLLECTION_NAME;

extern const std::string RULES;
extern const std::string RULES_NODE_ID;
extern const std::string RULES_RULE;

}  // namespace postgres
}  // namespace fluent

#endif  // POSTGRES_CONSTANTS_H_
