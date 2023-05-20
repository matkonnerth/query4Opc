## GraphTraversal and Filtering

### Source

"Generates" nodes, example is the hierachical visitor.

#### HierachicalVisitor
columns of a PathResult can be used as inputs for another PathMatcher

## PathMatcher

matches a path starting at a start node and preserves the matching paths.

## Filter

Filters nodes on a certain attribute, for example the typedefinition id.
TypeFilter (specialization of a type filter)

## FilterChain
TODO: wording FilterChain?
a Source, multiple filters and a PathMatcher can be combined to a FilterChain.

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

How to reduce this match clause? Would be a really good optimization.

## Result
a vector of paths (remind, a path can consist only of one node)

Currently the result is a vector of paths which fit the query. A node which references 3 variables, would be represented this way:

Result: Node1 -- VariableNode 1,
        Node1 -- VariableNode 2,
        Node1 -- VariableNode 3

A node can have multiple references with different types.

## TODOs:

At the moment it is hard to track down how the query is executed, good debugging output is necessary.

QueryService

Result Query(NodeId startNode, queryString)

Results:
Specify json result
hierachical result (result with paths from some start node, maybe objects folder)
all paths of the last statement match statement are returned

tests, tests, tests

move sdk dependent code to abstraction layer?

check if the direction of an reference can be inverted (hasTypeDefinition for example)

## before Sink (PathMatcher was a member of FilterChain)

queryServerObjectReduced      76486076 ns     76485279 ns            7
queryServerObjectInvertPath  202772078 ns    202749008 ns            3
queryServerObject            189415935 ns    189407763 ns            4
standardBrowse                25198709 ns     25196128 ns           21

## after sink

nearly the same result:
queryServerObjectReduced      74843691 ns     74840762 ns            7






