At the moment it is hard to track down how the query is executed, good debugging output is necessary.
implement explain

valgrind tests

Filter based on node attributes (e.g. value > limit)

includeSubTypes has nothing to do with ObjectTypeNode, maybe add it to Match clause?

Specify opcua method and json result
hierachical result (result with paths from some start node, maybe objects folder)
all paths of the last match statement are returned -> return obj, types not possible, all columns are returned like return *
think this is no problem in the beginning

implement COUNT

implement ORDER_BY

check if the direction of an reference can be inverted (hasTypeDefinition for example)
open62541 adds a IsTypeDefinitionOf from ObjectType to Object

move sdk dependent code to abstraction layer?

match all nodes
match () return * is not working

ideas:

typecache