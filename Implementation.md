First step:
only use browse service
no multiple returns
no where clauses
we do not try to change direction of reference

GraphTraversal and Filtering

Source

"Generates" nodes, example is the hierachical visitor.

HierachicalVisitor
Sinks can be uses as a source with SinkToSource

PathMatcher

matches a path starting at a start node and preservers the matching paths

Filter

Filters nodes on a certain attribute, for example the typedefinition id.
TypeFilter (specialization of a type filter)

a Source, multiple filters and a PathMatcher can be combined to a FilterChain.

Path

is a alternating sequence of nodes and relationships.

(n1)-[r1]-(n2)-[r2]-(n3)

Metrics
We could say: A typical nodeset has 100 objectTypes with 10 instances each

EmptyPath

is a path with only one node pattern.

Pattern
NodePattern
RelationshipPattern

How to specify the root of the HierachicalVisitor?
Could be done with this:
`match(root:Object{NodeId:"BaseInstance"})-[:HierachicalReferences*0..]->(obj:Objects` \

How to reduce this match clause? Would be a really good optimization.
