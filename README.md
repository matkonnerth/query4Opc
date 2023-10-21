### Query language for OPC UA

A proposal and PROTOTYPIC implementation for using the cypher query language for opc ua information models.

## Why query?

OPC UA information models can get quite big, especially when a server is aggregating multiple machines and therefore implementing multiple specifications.

OPC UA defines the view service set (https://reference.opcfoundation.org/Core/docs/Part4/5.8.1/) of which the browse service is for getting information about the structure of the address space. The drawback when using the browse is the round trip time between client and server. The client has to await the BrowseResults and can then continue browsing. Browsing the hierachical references of a big subtree (~100k nodes) can take a few minutes.

This proposal does some experiments with the open62541 opcua stack and tries to map the cypher query language to opc ua services (mainly the browse). The query implementation is done within the context of an opc ua server because of performance reasons. A other design would be to implement the query service in a standalone server beside the server to be queried. Challenge is here to keep the address space up to date. Schiekhofer has showed this with ModelChangedEvents.

At the moment (May 2023) to me is no opc ua server implementation known which implements the query service, specified here https://reference.opcfoundation.org/Core/Part4/v105/docs/5.9

## Mapping cypher to opc ua

opc ua meta meta model (model, which must be used by the information model)

OPC UA address space is built with different types of nodes:

Object \
Variable \
Method \
ObjectType \
ReferenceType \
DataType \
VariableType \
View

Nodes have several attributes in common, for example the NodeId identifies a node within the address space.
Each type of Node defines several additional attributes, for example the value attribute of a VariableNode.

References between nodes are typed with the main distinction between hierachical and nonhierachical references

label - NodeClass, type of a node \
property - Attribute \
relationship type - ReferenceType: with a property we could state, that also subtypes of the ReferenceType should be considered

## Example cypher queries

Here is a list of example cypher queries:

(1) Get all Objects of an exact ObjectType \
`MATCH (obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId:"Base"}) RETURN obj`

(2) Get all Objects of a certain ObjectType or SubType of this object \
`MATCH (types:ObjectType {NodeId: "BaseType", includeSubTypes: "true"})` \
`MATCH (obj:Object)-[:HasTypeDefinition]->(types)` \
`RETURN obj, types`

(3) Get all Objects of a certain ObjectType, starting at a certain root node \
`MATCH (a:ObjectType {NodeId: "Base"}) -[:HasSubTyp*0..]->(types) RETURN types` \
`MATCH (root:Object{NodeId:"BaseInstance"})-[:HierachicalReferences*0..]->(obj:Object)-[:HasTypeDefinition]->(types)` \
`RETURN obj, types`

(4) Get all Objects of a certain ObjectType and with a hasProperty reference to a certain node (TODO: validate this) \
`(tempdevices:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"TempDeviceType"})` \
`MATCH tempDevices-[:HasProperty]->(:Variable{NodeId:"MySpecialProperty"})`
`RETURN tempdevices`

## Playground

Take a look at the playground folder, there is queryServer and a queryClient. The queryServer implements a method for querying the address space. The queryClient uses this method and also hosts a rest-interface at localhost:12121, where queries can be send from and results are displayed there.

## Performance

### Find server object (benchmark/benchmark.cpp)

address space is populated with 100k object nodes, both implementations have to visit all object nodes and check the typedefinition.

Straightforward find server object: 82ms \
Query: 214ms \
Query (with inverted path): 222ms \
Query (reduced, see below): 132ms

Big difference there is that with naive implementation the nodes are browsed once (to get every node) and then the typedefinition is checked.
With the query all nodes are browsed (in Source.h) to get every node to see, then there is a second browse in the pathMatcher to
get the typdefinition id. Would be cool to get some optimization there.

`MATCH(obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId: \"i=2004\"}) RETURN obj`

can be reduced to

`MATCH (obj:Object{TypeDefinitionId:\"i=2004\"}) RETURN obj`

### browseResultMask

configuring the result mask  with bd.resultMask = UA_BROWSERESULTMASK_TYPEDEFINITION | UA_BROWSERESULTMASK_NODECLASS;

results in big gains: \
Straightforward find server object: 8ms \
Query: 186ms \
Query (with inverted path): 200ms \
Query (reduced, see below): 72ms

### performance with release build open62541 and query

Straightforward find server object: 0,5ms \
Query: 138ms \
Query (with inverted path): 174ms \
Query (reduced, see below): 1,2ms


### std::function

is quite fast, on an i7 it imposes an additional overhead of ~2ns per call, that means for 1000k calls this are approximately 2ms.-> there is not really something to gain.

### Result

The result of a query is a vector of paths. Some clients want to have a qualified path a node of these paths. Idea: specify a vector of nodes and browse the hierachical inverse reference to get the path to root nodes of the specified nodes?

## Not considered use cases
Aggregating server

## Cypher query language

The grammar of the cypher query language in EBNF form is available http://opencypher.org/resources/

Interactive railroad diagrams https://s3.amazonaws.com/artifacts.opencypher.org/M16/railroad/Cypher.html

Overview of existing graph query languages
https://www.gqlstandards.org/existing-languages

There is work going on in the direction to standardize graph query languages: https://www.gqlstandards.org/home

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

# dependencies
libcypher-parser https://github.com/cleishm/libcypher-parser \
nlohmann_json https://github.com/nlohmann/json for serialising the query result \
open62541 (opcua server / client implementation) \
gtest \
benchmark

