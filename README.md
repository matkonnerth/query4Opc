### Query language for OPC UA

## Why?

OPC UA defines multiple services, the interesting ones for getting information about the structure of the address space are the browse services a client can invoke. The drawback when using the browse is the round trip time between client and server. Browsing the hierachical references of a big subtree (~100k nodes) can take a few minutes.
OPC UA has defined a query service, but no server implemented it because it is maybe too generic defined.

This proposal does some experiments with the open62541 opcua stack and tries to map the cypher query language to opc ua services.

Is Cypher a suitable query language for opc ua information models?

## Query Languages

Overview of existing graph query languages
https://www.gqlstandards.org/existing-languages

## Mapping cypher to opc ua services

opc ua meta meta model (model, which must be used by the information model)

nodes with attributes
8 NodeClasses
ObjectNode
ObjectTypeNode
ReferenceTypeNode
...

Each NodeClass defines several attributes, for example every NodeClass has an attribute called NodeId which is an unique identifier of a node inside a address space. Another interesting attribute is the Value attribute of a VariableNode.

References between nodes are typed with the main distinction between hierachical and nonhierachical references

https://s3.amazonaws.com/artifacts.opencypher.org/openCypher9.pdf


## Example cypher queries

Neo4j queries
alle Subtypen \
`match (a:ObjectType {NodeId: "Base"}) -[:HasSubTyp*0..]->(types) return types`

Alle Instanzen \
`match (a:ObjectType {NodeId: "Base"}) -[:HierachicalReferences*0..]->(instances) return instances`

alle Instanzen mit bestimmtem Typ (keine Einschränkung auf Hierachie) \
`match(obj:Object)-[:HasTypeDefinition]->(t:ObjectType{NodeId:"Base"}) return obj`

Hierachische Knoten holen
`match(root:Object{NodeId:"BaseInstance"})-[:HasComponent*0..]->(objs:Object) return objs`

How to gather alle the hierachical references, I think there is a query missing.

HierachicalReferences:
HasComponent
HasSubType

NonHierachicalReferences
HasTypeDefinition

## Questions

Should the cypher queries work direct on the information model or on the meta meta model?

I think that working directly on the information model let's the client write more concise queries.

"Give me all objects with a certain type inclusive subtypes from this subtree".

vs.

"Give me all object nodes with reference HasTypeDefinition to one of this ObjectTypeNodes, the object nodes should have hierachical references below this node"

2nd query sounds more or less like the implementation.

## Performance



## Related papers
https://acris.aalto.fi/ws/portalfiles/portal/55667007/ENG_Hietala_et_al_GraphQL_Interface_for_IEEE_Conference_on_Industrial_Cyberphysical_Systems_ICPS_2020.pdf
https://www.researchgate.net/publication/336623537_Querying_OPC_UA_information_models_with_SPARQL

## neo4j
Implementierung
https://neo4j.com/blog/secret-sauce-neo4j-modeling-graphconnect/ 

import
https://neo4j.com/docs/operations-manual/current/tools/neo4j-admin-import/
