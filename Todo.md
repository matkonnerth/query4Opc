How to specify the start node / root node of a match clause?

includeSubTypes has nothing to do with ObjectTypeNode, maybe add it to Match clause?

At the moment it is hard to track down how the query is executed, good debugging output is necessary.

Specify opcua method and json result
hierachical result (result with paths from some start node, maybe objects folder)
all paths of the last statement match statement are returned

check if the direction of an reference can be inverted (hasTypeDefinition for example)
open62541 adds a IsTypeDefinitionOf from ObjectType to Object

move sdk dependent code to abstraction layer?
