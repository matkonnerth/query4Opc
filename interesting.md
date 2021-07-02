### Query language for OPC UA

## Why?

OPC UA defines multiple services, the interesting ones for getting information about the structure of the address space are the browse services a client can invoke. The drawback when using the browse is the round trip time between client and server. Browsing the hierachical references of a big subtree (~100k nodes) can take a few minutes.
OPC UA has defined a query service, but no server implemented it because it is maybe too generic defined.

Is Cypher a suitable query language for opc ua information models?

## Mapping cypher to opc ua services

opc ua meta meta model (model, which must be used by the information model)

nodes with attributes
8 types of a nodes
ObjectNode
ObjectTypeNode
ReferenceTypeNode
...

References between nodes are type with the main distinction between hierachical and nonhierachical references

Examples

https://s3.amazonaws.com/artifacts.opencypher.org/openCypher9.pdf


opc ua meta model (information model)

Questions:

Should the GraphQL queries work direct on the information model or on the meta meta model?

I think that working directly on the information model let's the client write more concrete queries.

"Give me all objects with a certain type inclusive subtypes from this subtree".

vs.

"Give me all object nodes with reference HasTypeDefinition to one of this ObjectTypeNodes, the object nodes should have hierachical references below this node"

2nd query sounds more or less like the implementation.

Cypher Query Language

Overview of existing graph query languages
https://www.gqlstandards.org/existing-languages


paper
https://acris.aalto.fi/ws/portalfiles/portal/55667007/ENG_Hietala_et_al_GraphQL_Interface_for_IEEE_Conference_on_Industrial_Cyberphysical_Systems_ICPS_2020.pdf

GraphQL
https://neo4j.com/labs/grandstack-graphql/

neo4j import
https://neo4j.com/docs/operations-manual/current/tools/neo4j-admin-import/
