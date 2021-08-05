First step:
only use browse service
no multiple returns
no where clauses

GraphTraversal and Filtering

Source

"Generates" nodes, example is the hierachical visitor.

HierachicalVisitor
Sinks can be uses as a source with SinkToSource

Filter

Filters nodes on a certain attribute, for example the typedefinition id.
ReferenceFilter
TypeFilter (specialization of a type filter)

a Source, multiple filters and a sink can be combined to a FilterChain.

Sink

stores nodes at the end of filtering

Path

()-[]-()-()

SimplePath

(Node)-[Relationship]-(Node)

can be broken down to a FilterChain with a source, a referenceFilter and a sink.
sink can be used again as source. With that concept a variable length path can be reduced to n simple path.

Pattern
NodePattern
RelationshipPattern

