## GraphTraversal and Filtering

### Source

Visits nodes which are interesting for a query, example is the HierachicalVisistor

#### ColumnAsSource
columns of a PathResult can be used as inputs for another PathMatcher

## PathMatcher

matches a path starting at a start node and preserves the matching paths.

## MatchClause
a Source (e.g. HierachicalVisitor, ColumnAsSource) and a sink (e.g. PathMatcher, DefaultSink)  can be combined to a MatchClause.

## graph::Path

is a alternating sequence of nodes and relationships.

(n1)-[r1]-(n2)-[r2]-(n3)

Path matching is always done from left to right!

## Metrics
We could say: A typical nodeset has 100 objectTypes with 10 instances each. Each of the objectTypes has 10 variables and 3 methods.

That means the instance tree (nodes below the objects folder) would have 1000 objects, 3000 methods and 10000 variables. So the HierachicalVisitor should always start at the objectTypes. But most of the server implementations doesn't add the inverse reference from an ObjectType to the Object. (It seems like that the open62541 does add this inverse reference).

The path (obj:Object)-[HasTypeDefinition]->[:ObjectType{NodeId:"123}] could then be rewritten to (obj:Object)<-[HasTypeDefinition]-(:ObjectType{"NodeId})

Also a HierachicalVisitor wouldn't be required.

## Cypher

Pattern
NodePattern
RelationshipPattern

EmptyPath

is a path with only one node pattern.

How to specify the root of the HierachicalVisitor?
Could be done with this:
`match(root:Object{NodeId:"BaseInstance"})-[:HierachicalReferences*0..]->(obj:Objects` \

TODO:
How to reduce this match clause? Would be a really good optimization.

## PathResult
a vector of paths (remind, a path can consist only of one node)

Currently the result is a vector of paths which fit the query. A node which references 3 variables, would be represented this way:

Result: Node1 -- VariableNode 1,
        Node1 -- VariableNode 2,
        Node1 -- VariableNode 3

A node can have multiple references with different types.





