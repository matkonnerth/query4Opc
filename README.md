### Query language for OPC UA

A proposal for defining a query language for opc ua information models which also provides a prototypic implementation.

## Why query?

OPC UA information models can get quite big, especially when a server is aggregating multiple machines and therefore implementing multiple specifications.

OPC UA defines the view service set (https://reference.opcfoundation.org/Core/docs/Part4/5.8.1/) of which the browse service is for getting information about the structure of the address space. The drawback when using the browse is the round trip time between client and server. Browsing the hierachical references of a big subtree (~100k nodes) can take a few minutes.
OPC UA has defined a query service, but no server implemented it because it is maybe too generic defined.

This proposal does some experiments with the open62541 opcua stack and tries to map the cypher query language to opc ua services. The query implementation is done within the context of an opc ua server because of performance reasons. A other design would be to implement the query service in a standalone server beside the server to be queried. Challenge is here to keep the address space up to date. Schiekhofer has showed this with ModelChangedEvents.

Is Cypher a suitable query language for opc ua information models?

## Query Languages

Overview of existing graph query languages
https://www.gqlstandards.org/existing-languages

There is work going on in the direction to standardize graph query languages: https://www.gqlstandards.org/home

## Mapping cypher to opc ua

opc ua meta meta model (model, which must be used by the information model)

nodes with attributes
8 NodeClasses
ObjectNode
ObjectTypeNode
ReferenceTypeNode
...

Each NodeClass defines several attributes, for example every NodeClass has an attribute called NodeId which is an unique identifier of a node inside a address space. Another interesting attribute is the Value attribute of a VariableNode.

References between nodes are typed with the main distinction between hierachical and nonhierachical references

label - NodeClass, type of a node
property - Attribute
relationship type - ReferenceType: with a property we could state, that also subtypes of the ReferenceType should be considered

## Example cypher queries

Here is a list of example cypher queries, which we think is interesting for clients:

(1) Get all Objects of an exact ObjectType \
`match(obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId:"Base"}) return obj`

(2) Get all Objects of a certain ObjectType or SubType of this object \
`match (a:ObjectType {NodeId: "BaseType"}) -[:HasSubTyp*0..]->(types)` \
`match (obj:Object)-[:HasTypeDefinition]->(types)` \
`return obj, types`

(3) Get all Objects of a certain ObjectType, starting at a certain root node
`match (a:ObjectType {NodeId: "Base"}) -[:HasSubTyp*0..]->(types) return types`
`match(root:Object{NodeId:"BaseInstance"})-[:HierachicalReferences*0..]->(obj:Object)-[:HasTypeDefinition]->(types)` \
`return obj, types`

(4) Get all Objects of a certain ObjectType and with a hasProperty reference to a certain node (TODO: validate this) \
`tempDevices = (obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"TempDevice"})`
`match tempDevices-[:HasProperty]->(:Variable{NodeId:"MySpecialProperty"})`


Support queries
get all subtypes of an ObjectType \
`match (a:ObjectType {NodeId: "Base"}) -[:HasSubTyp*0..]->(types) return types`

get all hierachical references \
`match (a:Object{NodeId: "Base"}) -[:HierachicalReferences*0..]->(instances) return instances`

### Open questions

How to gather all the hierachical references, I think there is a query missing. Alternative we can make the assumption to follow also subtypes of the given reference types.

HierachicalReferences:
HasComponent
HasSubType

NonHierachicalReferences
HasTypeDefinition

## Cypher query language

The grammar of the cypher query language in EBNF form is available http://opencypher.org/resources/

Interactive railroad diagrams https://s3.amazonaws.com/artifacts.opencypher.org/M16/railroad/Cypher.html


### Examples

(1) MATCH(zone:MyZoneType) return zone vs.

(2) MATCH(zone:Object)-[:HasTypeDefinition]->(zoneType:ObjectType)

When we use (1), how are relationships like "HasSubType" are modelled between labels. Does openCypher specify relationships between labels?
2nd query is the way to go, there is more information in their which can be used for optimization.

## Performance

### Find server object (benchmark/benchmark.cpp)

address space is populated with 100k object nodes, both implementations have to visit all object nodes and check the typedefinition.

Straighforward find server object: 82ms
Query: 214ms
Query (with inverted path): 222ms
Query (reduced, see below): 132ms

Big difference there is that with naive implementation the nodes are browse once (to get every node) and then the typedefinition is checked.
With the query all nodes are browsed (in Source.h) to get every node to see, then there is a second a second browse in the pathMatcher to
get the typdefinition id. Would be cool to get some optimization there.

MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: \"i=2004\"}) RETURN obj

can be reduced to

MATCH (obj:Object{TypeDefinitionId:\"i=2004\"}) RETURN obj

## Not considered use cases
Aggregating server

## Related papers
https://acris.aalto.fi/ws/portalfiles/portal/55667007/ENG_Hietala_et_al_GraphQL_Interface_for_IEEE_Conference_on_Industrial_Cyberphysical_Systems_ICPS_2020.pdf
https://www.researchgate.net/publication/336623537_Querying_OPC_UA_information_models_with_SPARQL

## openCypher
https://s3.amazonaws.com/artifacts.opencypher.org/openCypher9.pdf

## neo4j
Implementierung
https://neo4j.com/blog/secret-sauce-neo4j-modeling-graphconnect/ 

import
https://neo4j.com/docs/operations-manual/current/tools/neo4j-admin-import/

## Implementation

take a look at [Implementation]("Implementation.md").


